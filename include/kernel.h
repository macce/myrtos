#ifndef KERNEL_H
#define KERNEL_H

#include "rtos_types.h"

/* Syscalls for applications. */

void rtos_yield();
rtos_address rtos_alloc(rtos_u32 nbr_bytes);
void rtos_send(rtos_address buffer_address, rtos_u32 dest_pid, rtos_u32 dest_inbox);
rtos_address rtos_receive(rtos_u32 inbox);
void rtos_dispose(rtos_address buffer_address);
void rtos_tick();
void rtos_delay(rtos_u32 nbr_ticks);
void rtos_wait_psem();
void rtos_signal_psem(rtos_u32 pid);
rtos_u32 rtos_current_pid();

#endif

