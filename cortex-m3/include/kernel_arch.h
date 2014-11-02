/*****************************************************************************
 * kernel_arch.h - Macros to be called from the kernel
 * (portable or arch specific).
 *
 *****************************************************************************/

#ifndef KERNEL_ARCH_H
#define KERNEL_ARCH_H

/* Macros to disable/enable interrupts with a priority less than or equal to
   8. Interrupt with a priority higher than 8 may not call API functions in
   the kernel, as that might corrupt internal data structures.
   TODO: These macros assume that 4 bits are used for representing a priority.
   This number varies with different implementations of Cortex-M3. 4 bits are
   used for the STM32. */
#define INTERRUPT_DISABLE						\
   do { asm volatile("mov r12, 0x00000080\n\tmsr BASEPRI, r12":::"r12"); } while(0)

#define INTERRUPT_ENABLE \
   do { asm volatile("mov r12, 0x00000000\n\tmsr BASEPRI, r12":::"r12"); } while(0)

#endif
