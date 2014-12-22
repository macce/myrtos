/*****************************************************************************
 * kernel.c - C source for the portable part of RTOS.
 *
 * This file implements portable system calls, process lists, IPC, memory
 * management etc. The function rtos_init is called from the BSP after the
 * board has been properly set up and memory regions such as BSS and DATA have
 * been initialized.
 *
 *
 * TODO:
 * - Introduce DISABLE_SAVE/ENABLE_SAVED macros to handle nested critical
 *     regions.
 *
 *****************************************************************************/

#include "rtos_types.h"
#include "kernel.h"
#include "kernel_int.h" /* TODO, more prototypes in that file. */
#include "kernel_arch.h"
#include "pcb.h"

/*****************************************************************************
 * Defines, Constants, Typedefs and Structs
 *****************************************************************************/

#define kernel_assert(expr) do { if (!(expr)) assertion_failed(); } while (0)

/* The buffer header consists of a magic number and a next-pointer. */
#define BUFFER_HEADER_SIZE 8
#define BUFFER_HEADER_MAGIC 0x11223344

/* The buffer trailer consists of a magic number. */
#define BUFFER_TRAILER_SIZE 4
#define BUFFER_TRAILER_MAGIC 0x55667788

typedef struct BufferHeader
{
      rtos_u32            magic;
      struct BufferHeader *next;
} BufferHeader;

typedef struct BufferTrailer
{
      rtos_u32            magic;
} BufferTrailer;


/******************************************************************************
 * Local Function Prototypes
 *****************************************************************************/

/* assertion_failed - Called if an assertion failed. Loops forever. */
static void assertion_failed(void);

/* rtosint_yield - Called from syscall to handle the 'yield' syscall. The
   current process is sorted into the ready-list and one of the highest-
   priority processes is taken out. A context switch is scheduled using the
   'arch_trigger_pendsv' to take place when all currently active exceptions
   are handled. */
static void rtosint_yield();

/* rtosint_alloc - Called from syscall to handle the 'alloc' syscall. Try to
   find a buffer in corresponding free-list. If not available there; allocate
   from RAM space. */
static rtos_address rtosint_alloc(rtos_u32 nbr_bytes);

static void rtosint_send(rtos_address buffer_address, rtos_u32 dest_pid, rtos_u32 dest_inbox);
static rtos_address rtosint_receive();
static void rtosint_dispose(rtos_address buffer_address);
static void rtosint_tick();
static void rtosint_delay(rtos_u32 nbr_ticks);
static void rtosint_wait_psem();
static void rtosint_signal_psem(rtos_u32 pid);
static rtos_u32 rtosint_current_pid();

static void readylist_insert_pcb(PCB *pcb);
static void receivelist_insert_pcb(PCB *pcb);
static void delaylist_insert_pcb(PCB *pcb, rtos_u32 nbr_ticks);
static rtos_address kernel_alloc_permanent(rtos_u32 size, rtos_u8 alignment);

/*****************************************************************************
 * Variable Declarations
 *****************************************************************************/

extern rtos_address _kernel_pool_start; /* From linker script. */

PCB *new_pcb = 0;
PCB *current_pcb = 0;

/* An array of pointers to portable system call handlers. This array is used
   to perform jumps from architecture specific code directly to the
   handlers. */
void *syscall_pointers[] =
{
   rtosint_yield,
   rtosint_alloc,
   rtosint_send,
   rtosint_receive,
   rtosint_dispose,
   rtosint_tick,
   rtosint_delay,
   rtosint_wait_psem,
   rtosint_signal_psem,
   rtosint_current_pid
};

static rtos_address permanent_data_ptr;
static PCB *ready_pcbs = 0;
static PCB *receive_pcbs = 0;
static PCB *delay_pcbs = 0;
static PCB **pid_pcb_map = 0;
static rtos_u32 next_pid = 0;
static BufferHeader *available_list = 0;

static rtos_u32 current_tick = 0;

/*****************************************************************************
 * Function Implementations
 *****************************************************************************/

static void assertion_failed(void)
{
  while (1);
}

static void list_failed(void)
{
  while (1);
}

static void test_list(PCB *list)
{
  int depth = 0;
  while (list) {
    list = list->next;
    depth++;
    if (depth > 4) list_failed();
  }
}

static void test_lists(void)
{
  test_list(ready_pcbs);
  test_list(delay_pcbs);
}

static void rtosint_yield()
{
   /* Possible races:
      1. An interrupt triggers an rtosint_tick, which may do the following:
         - Insert processes in the readylist.
	 - Change state of current_pcb to PROCESS_STATE_READY.
	 - Update new_pcb.
	 - Update ready_pcbs.

      Notes:
        current_pcb is constant throughout the invocation of this function.
   */
   readylist_insert_pcb(current_pcb);

   /* Schedule a context switch to take place after all active exceptions.
      TODO: 'arch_trigger_pendsv' should have a better name, as it is a
      Cortex-M3 exception name. */
   arch_trigger_pendsv();
}

static rtos_address rtosint_alloc(rtos_u32 wanted_size)
{
   rtos_u32 actual_size = 0;
   rtos_address buffer = 0;

   /* Find out the size of buffer to use, using the configured list
      of buffer sizes. TODO: Implement configured sizes. */
   (void) wanted_size;
   actual_size = 64;

   /* Look in free-list for available buffer of suitable size. */
   INTERRUPT_DISABLE;
   if (available_list != 0)
   {
      BufferHeader *buffer_header = available_list;
      available_list = buffer_header->next;
      INTERRUPT_ENABLE;
      buffer = ((rtos_address)buffer_header) + BUFFER_HEADER_SIZE;
   }
   else
   {
   /* If buffer is not available in free-list, then allocate from RAM
      space: */
      buffer = kernel_alloc_permanent(BUFFER_HEADER_SIZE + actual_size
                                      + BUFFER_TRAILER_SIZE, 4);
      INTERRUPT_ENABLE;
      
      ((BufferHeader *)buffer)->magic = BUFFER_HEADER_MAGIC;
      ((BufferHeader *)buffer)->next = 0;
      ((BufferTrailer *)(buffer + BUFFER_HEADER_SIZE + actual_size))->magic =
	 BUFFER_TRAILER_MAGIC;
      
      buffer += BUFFER_HEADER_SIZE;
   }
   return buffer;
}

static void rtosint_send(rtos_address buffer_address, rtos_u32 dest_pid,
                  rtos_u32 dest_inbox)
{
   PCB *dest_pcb = pid_pcb_map[dest_pid];
   BufferHeader *buffer_header = (BufferHeader *)(buffer_address - BUFFER_HEADER_SIZE);

   buffer_header->next = 0;

   /* Deliver the message. */
   //INTERRUPT_DISABLE;
   if (dest_pcb->inbox[dest_inbox] == 0)
   {
      /* No buffer in inbox. */
      if (dest_pcb->process_state == PROCESS_STATE_RECEIVE &&
          dest_pcb->receive_from == dest_inbox)
      {
	 /* Destination process is in RECEIVE and shall return from the
	    receive call with this message, as it is the only message in it's
	    inbox. */
	 dest_pcb->process_state = PROCESS_STATE_READY;
	 readylist_insert_pcb(dest_pcb);
	 arch_store_retval(buffer_address, dest_pcb);

	 if (dest_pcb->priority < current_pcb->priority)
	 {
	    current_pcb->process_state = PROCESS_STATE_READY;
	    readylist_insert_pcb(current_pcb);

	    /* Schedule a context switch. */
	    arch_trigger_pendsv();
	 }
      }
      else
      {
	 /* Recipient is NOT waiting for a message on this inbox. */
	 dest_pcb->inbox[dest_inbox] = buffer_header;
      }
   }
   else
   {
      /* At least one buffer in inbox. Follow list and put new message at the
	 end of it. TODO: Optimize by keeping a pointer to the last message.
	 No need to schedule here, because even if the destination process is
	 in RECEIVE, it must be lower than or equal than current process
	 (otherwise it would be running now), so we should not switch to it. */
      BufferHeader *dest_inbox_iter = dest_pcb->inbox[dest_inbox];
      
      while (dest_inbox_iter->next != 0)
      {
	 dest_inbox_iter = dest_inbox_iter->next;
      }
      dest_inbox_iter->next = buffer_header;
   }
   //INTERRUPT_ENABLE;
}

static rtos_address rtosint_receive(rtos_u32 inbox)
{
   BufferHeader *received = 0;

   if (current_pcb->inbox[inbox] != 0)
   {
      /* Message waiting in inbox. */
      received = current_pcb->inbox[inbox];
      current_pcb->inbox[inbox] = current_pcb->inbox[inbox]->next;
      return ((rtos_address) received) + BUFFER_HEADER_SIZE;
   }
   else
   {
      /* No message in inbox! */
      /* I see no use of calling receivelist_insert_pcb... */
      //receivelist_insert_pcb(current_pcb);
      current_pcb->process_state = PROCESS_STATE_RECEIVE;
      current_pcb->receive_from = inbox;

      /* Reschedule after all active exceptions. */
      arch_trigger_pendsv();
      return 0;
   }
}

static void rtosint_dispose(rtos_address buffer_address)
{
   BufferHeader *buffer_header = ((BufferHeader *)(buffer_address - BUFFER_HEADER_SIZE));
   
   buffer_header->next = available_list;
   available_list = buffer_header;
}

static void rtosint_tick()
{
   PCB *ready_pcb = 0;
   int do_schedule = 0;

   test_lists();

   /* TODO: Handle timeout wrap-around problem below.
      Move appropriate processes from the delay list to the ready list. */

   /* The checking and manipulation of the list constitues a critical
      section. */
   INTERRUPT_DISABLE;
   current_tick++;

   kernel_assert(delay_pcbs->next != delay_pcbs);

   int nbr_delay_pcbs = 0;
   PCB *iter = delay_pcbs;
   for (;iter != 0 && nbr_delay_pcbs < 10;iter = iter->next,nbr_delay_pcbs++);

   while (delay_pcbs != 0 && delay_pcbs->delay_until <= current_tick)
   {
      /* Take out the first element of the list (earliest timeout). */
      ready_pcb = delay_pcbs;
      delay_pcbs = delay_pcbs->next;
      //INTERRUPT_ENABLE;

      /* The PCB pointed out by ready_pcb is not available to anyone else now
	 so we can manipulate it at will. */
      ready_pcb->process_state = PROCESS_STATE_READY;
      readylist_insert_pcb(ready_pcb);
      if (ready_pcb->priority < current_pcb->priority)
      {
        kernel_assert(current_pcb->next != current_pcb);
        do_schedule = 1;
      }

      /* Enter critical section for next list manipulation. */
      //INTERRUPT_DISABLE;
   }

   INTERRUPT_ENABLE;

   if (do_schedule)
   {
     readylist_insert_pcb(current_pcb);

     /* Schedule a context switch to take place after all active exceptions.
        TODO: 'arch_trigger_pendsv' should have a better name, as it is a
        Cortex-M3 exception name. */
     arch_trigger_pendsv();
   }

   test_lists();
}


static void rtosint_delay(rtos_u32 nbr_ticks)
{
   test_lists();
#if 1
   delaylist_insert_pcb(current_pcb, nbr_ticks);

   /* Schedule a context switch to take place after all active exceptions.
      TODO: 'arch_trigger_pendsv' should have a better name, as it is a
      Cortex-M3 exception name. */
   arch_trigger_pendsv();
#endif
   test_lists();
}


static void rtosint_wait_psem()
{
   if (current_pcb->psem_value == 0)
   {
      current_pcb->process_state = PROCESS_STATE_PSEM;
      arch_trigger_pendsv();
   }
   else
   {
      current_pcb->psem_value--;
   }
}


static void rtosint_signal_psem(rtos_u32 pid)
{
   PCB *signal_pcb = pid_pcb_map[pid];
   
   if (signal_pcb->process_state == PROCESS_STATE_PSEM)
   {
      signal_pcb->process_state = PROCESS_STATE_READY;
      readylist_insert_pcb(signal_pcb);

      if (signal_pcb->priority < current_pcb->priority)
      {
	 current_pcb->process_state = PROCESS_STATE_READY;
	 readylist_insert_pcb(current_pcb);
	 arch_trigger_pendsv();
      }
   }
   else
   {
      signal_pcb->psem_value++;
   }
}


static rtos_u32 rtosint_current_pid()
{
   return current_pcb->pid;
}

/****************************************************************************** 
 * Function: readylist_insert_pcb
 *
 * Called to put the supplied PCB in the readylist. The PCB is sorted into the
 * list such that it is placed AFTER all PCBs with the same or higher
 * priority. That way, when picking processes from the head of the list, a
 * round-robin scheduling scheme within priorities is implemented.
 */
static void readylist_insert_pcb(PCB *pcb)
{
   INTERRUPT_DISABLE;
   test_lists();

   pcb->process_state = PROCESS_STATE_READY;
   if (ready_pcbs == 0)
   {
      pcb->next = 0;
      ready_pcbs = pcb;
      test_lists();
   }
   else if (ready_pcbs->priority > pcb->priority)
   {
      /* First PCB has lower priority than pcb. */
      pcb->next = ready_pcbs;
      ready_pcbs = pcb;
      kernel_assert(pcb->next != pcb);
      test_lists();
   }
   else if (ready_pcbs->priority <= pcb->priority &&
	    ready_pcbs->next != 0)
   {
      /* First PCB has same or higher prio and is not alone. */
      PCB *iter = ready_pcbs;

      /* Check next PCBs in a loop. */
      while (iter->next != 0 && iter->next->priority <= pcb->priority)
      {
        kernel_assert(iter->next != iter);
        iter = iter->next;
      }
      if (iter->next == 0)
      {
	 pcb->next = 0;
	 iter->next = pcb;
         test_lists();
      }
      else
      {
	 pcb->next = iter->next;
	 iter->next = pcb;
         kernel_assert(pcb->next != pcb);
         test_lists();
      }
   }
   else if (ready_pcbs->next == 0)
   {
      /* First PCB has same or higher priority than pcb and is alone. */
      pcb->next = 0;
      ready_pcbs->next = pcb;
   }
   test_lists();

   INTERRUPT_ENABLE;
}


/****************************************************************************** 
 * Function: receivelist_insert_pcb
 *
 * Called to put the supplied PCB in the receivelist. The PCB is sorted into the
 * list such that it is placed AFTER all PCBs with the same or higher
 * priority. That way, when looking for processes to receive messages, high-
 * priority processes are found faster, improving overall performance.
 *
 * FIXME: This is a duplicate of readylist_insert_pcb.
 */
static void receivelist_insert_pcb(PCB *pcb)
{
   test_lists();
   if (receive_pcbs == 0)
   {
      pcb->next = 0;
      receive_pcbs = pcb;
   }
   else if (receive_pcbs->priority > pcb->priority)
   {
      /* First PCB has lower priority than pcb. */
      pcb->next = receive_pcbs;
      receive_pcbs = pcb;
   }
   else if (receive_pcbs->priority <= pcb->priority &&
	    receive_pcbs->next != 0)
   {
      /* First PCB has same or higher prio and is not alone. */
      PCB *iter = receive_pcbs;

      /* Check next PCBs in a loop. */
      while (iter->next != 0 && iter->next->priority <= pcb->priority)
      {
	 iter = iter->next;
      }

      /* TODO: Optimize like in delaylist_insert_pcb. */
      if (iter->next == 0)
      {
	 pcb->next = 0;
	 iter->next = pcb;
      }
      else
      {
	 pcb->next = iter->next;
	 iter->next = pcb;
      }
   }
   else if (receive_pcbs->next == 0)
   {
      /* First PCB has same or higher priority than pcb and is alone. */
      pcb->next = 0;
      receive_pcbs->next = pcb;
   }
   test_lists();
}

/****************************************************************************** 
 * Function: delaylist_insert_pcb
 *
 * Called to put the supplied PCB in the delaylist. The PCB is sorted into the
 * list such that it is placed AFTER all PCBs with the same or higher
 * priority. That way, when looking for processes to receive messages, high-
 * priority processes are found faster, improving overall performance.
 *
 * FIXME: This is a duplicate of readylist_insert_pcb.
 */
static void delaylist_insert_pcb(PCB *pcb, rtos_u32 nbr_ticks)
{
   INTERRUPT_DISABLE;
   test_lists();
   pcb->delay_until = current_tick + nbr_ticks;
   pcb->process_state = PROCESS_STATE_DELAY;
   if (delay_pcbs == 0)
   {
      /* No PCBs in the list. */
      pcb->next = 0;
      delay_pcbs = pcb;
   }
   else if (delay_pcbs->delay_until > pcb->delay_until)
   {
     /* First PCB has longer timeout. */
     pcb->next = delay_pcbs;
     delay_pcbs = pcb;
     kernel_assert(delay_pcbs->next != delay_pcbs);
   }
   else if (delay_pcbs->next != 0)
   {
      /* First PCB has same or shorter timeout and is not alone. */
      PCB *iter = delay_pcbs;

      /* Check next PCBs until next has longer timeout. */
      while (iter->next != 0 && iter->next->delay_until <= pcb->delay_until)
      {
	 iter = iter->next;
      }

      pcb->next = iter->next;
      iter->next = pcb;
      kernel_assert(pcb->next != pcb);
   }
   else
   {
      /* First PCB has same or shorter timeout and is alone. */
      pcb->next = 0;
      delay_pcbs->next = pcb;
   }
   test_lists();
   INTERRUPT_ENABLE;
}


/****************************************************************************** 
 * Function: kernel_alloc_permanent
 *
 * Permanently allocates memory with the specified size and alignment.
 *
 * TODO:
 * - Use DISABLE_SAVE/ENABLE_SAVED here.
 */
static rtos_address kernel_alloc_permanent(rtos_u32 size, rtos_u8 alignment)
{
   rtos_address data_block = 0;
   
   /* Move permanent_data_ptr up to the next correctly aligned address. */
   if ((permanent_data_ptr & (alignment - 1)) != 0)
   {
      permanent_data_ptr += alignment;
      permanent_data_ptr &= ~(alignment - 1);
   }
   
   data_block = permanent_data_ptr;
   permanent_data_ptr += size;

   return data_block;
}

/****************************************************************************** 
 * Function:  rtos_context_switch_hook
 *
 * Called to do administration before context switch.
 * Updates new_pcb before context switch is performed by arch specific
 * assembly code.
 */
void rtos_reschedule_hook()
{
   new_pcb = ready_pcbs;
   ready_pcbs = ready_pcbs->next;
   new_pcb->process_state = PROCESS_STATE_RUNNING;
}


/****************************************************************************** 
 * Function:   rtos_init
 *
 * Called from boot code, after BSS and DATA have been initialized.
 */
void rtos_init()
{
  PCB *pcb_iterator = 0;

  /* Set up static variables. */
  permanent_data_ptr = (rtos_address) &_kernel_pool_start;

  /* Allow application to create processes. */
  rtos_hook_create_processes();

  /* Create an array that maps pids to PCB:s. Fill it with PCB pointers. */
  pid_pcb_map = (PCB **)
    kernel_alloc_permanent(next_pid * sizeof(PCB *), sizeof(PCB *));

  for (pcb_iterator = ready_pcbs;
       pcb_iterator != 0;
       pcb_iterator = pcb_iterator->next) {
    pid_pcb_map[pcb_iterator->pid] = pcb_iterator;
  }

  /* Enable peripherals, peripheral clocks, interrupts etc. from BSP.
     This should be done as late as possible to save power. */
  soc_start_hook();

  /* Activate the highest-priority process (or one of them). */
  if (ready_pcbs != 0) {
    current_pcb = ready_pcbs;
    ready_pcbs = ready_pcbs->next;

    current_pcb->process_state = PROCESS_STATE_RUNNING;
    arch_start((struct PCB *) current_pcb);
  }

  /* Loop forever. We should never come here. */
  for(;;);
}

/******************************************************************************
 * SECTION: Kernel Configuration Calls
 *
 * In this section are functions that are called from the application during
 * the execution of different hooks. These functions perform tasks such as
 * creating processes, configuring kernel signal handling etc.
 * 
 *****************************************************************************/


/******************************************************************************
 * Function: rtos_create_process
 *
 * Only to be called from application during rtos_hook_create_processes.
 */
rtos_u32 rtos_create_process(rtos_address entry, rtos_u16 stack_size,
rtos_u8 priority)
{
  PCB *pcb = 0;
  rtos_address stack_base = 0;
  rtos_u32 pid = next_pid;

   /* Allocate permanent space for the PCB and stack.
      Make stack 8-byte aligned, this is required for Cortex-M3,
      but this could of course be configured per architecture. */
   pcb = (PCB *) kernel_alloc_permanent(sizeof(PCB), sizeof(rtos_u32));
   stack_base = (rtos_address) kernel_alloc_permanent(stack_size, 8);

   pcb->entry = entry;
   pcb->thread_stack_top = stack_base + stack_size;
   pcb->sp = 0;
   pcb->next = 0;
   pcb->priority = priority;
   pcb->pid = pid;

   pcb->inbox[0] = 0;
   pcb->inbox[1] = 0;
   pcb->inbox[2] = 0;
   pcb->inbox[3] = 0;
   pcb->process_state = PROCESS_STATE_READY;
   pcb->receive_from = 0;

   /* Initialize process specific semaphore. */
   pcb->psem_value = 0;

   /* Let the arch-specific code init PCB and stack. */
   arch_init_stack(pcb);

   /* Put in readylist. */
   readylist_insert_pcb(pcb);

   next_pid++;
   return pid;
}
