#ifndef KERNEL_INT_H
#define KERNEL_INT_H

#include "rtos_types.h"
#include "pcb.h"

/******************************************************************************
 * Function: arch_init_stack
 * 
 * Initialize the stack for the process described by 'pcb'.
 */
extern void arch_init_stack(PCB *pcb);

#endif
