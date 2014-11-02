#include "pcb.h"

/******************************************************************************
 * Defines, constants and structs.
 *****************************************************************************/
#define VECTOR_SVC    ((volatile unsigned long *) 0x2000002C)
#define VECTOR_PENDSV ((volatile unsigned long *) 0x20000038)


/******************************************************************************
 * External Functions
 *****************************************************************************/
extern void start_process(PCB *pcb);
extern void CM3_handler_svc(void);
extern void CM3_handler_pendsv(void);

/******************************************************************************
 * External Variables
 */

/******************************************************************************
 * Local Variables
 */

/****************************************************************************** 
 * Function: arch_start
 *
 * Do all preparations for starting the created processes, in this case
 * install the necessary exception vectors and call assembly to return to the
 * process pointed out by the 'pcb' argument.
 *
 * Parameters:
 *  pcb - The first process to execute in the OS.
 */
void arch_start(PCB *pcb)
{
   /* Install the necessary vectors, i.e. those that are necessary for the
      operation of the kernel. */
   *VECTOR_SVC = (unsigned long) CM3_handler_svc;
   *VECTOR_PENDSV = (unsigned long) CM3_handler_pendsv;

   /* Now, return from handler mode to thread mode, running the process
      referred to by 'pcb'. */
   start_process(pcb);
}

