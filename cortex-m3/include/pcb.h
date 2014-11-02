#ifndef PCB_H
#define PCB_H

#include "rtos_types.h"

typedef enum
{
   PROCESS_STATE_RUNNING,
   PROCESS_STATE_READY,
   PROCESS_STATE_RECEIVE,
   PROCESS_STATE_DELAY,
   PROCESS_STATE_PSEM
} ProcessState;

typedef struct PCB
{     
      /* BEGIN DON'T TOUCH */
      rtos_address        entry;
      rtos_address        thread_stack_top;
      rtos_address        sp;
      /* END DON'T TOUCH */
      struct PCB          *next;
      rtos_u8             priority;
      rtos_u32            pid;

      /* 'inbox[0]' is used for holding a temporary pid value during
	 process creation (in rtos_init). */
      struct BufferHeader *inbox[4];
      ProcessState        process_state;
      rtos_u32            receive_from;
      
      /* 'delay_until' is the tick time when the process should
	 be put in the readylist again. */
      rtos_u32            delay_until;

      /* 'psem_value' is the value of the process specific semaphore. */
      rtos_u32            psem_value;
} PCB;

#endif
