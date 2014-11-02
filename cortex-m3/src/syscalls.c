#include "rtos_types.h"

#define STRINGIFY(x) #x

#define rtos_syscall_0(CALL_ID, RET_TYPE, NAME)			 \
   RET_TYPE NAME()						 \
   {								 \
      asm ("mov r12, "STRINGIFY(CALL_ID)"\n\t"		 \
      "svc\t0\n\t");						 \
   }

#define rtos_syscall_1(CALL_ID, RET_TYPE, NAME, TYPE0, NAME0)		\
   RET_TYPE NAME(TYPE0 NAME0)						\
   {									\
      asm volatile ("mov r12, "STRINGIFY(CALL_ID)"\n\t"			\
      "svc\t0\n\t"::);				\
   }

#define rtos_syscall_2(CALL_ID, RET_TYPE, NAME, TYPE0, NAME0, TYPE1, NAME1) \
   RET_TYPE NAME(TYPE0 NAME0, TYPE1 NAME1) \
   { \
      asm ("mov r12, "STRINGIFY(CALL_ID)"\n\t"	\
      "svc\t0\n\t"::);		\
   }

#define rtos_syscall_3(CALL_ID, RET_TYPE, NAME, TYPE0, NAME0, TYPE1, NAME1, TYPE2, NAME2) \
   RET_TYPE NAME(TYPE0 NAME0, TYPE1 NAME1, TYPE2 NAME2)				\
   { \
      asm ("mov r12, "STRINGIFY(CALL_ID)"\n\t"	\
      "svc\t0\n\t"::);		\
   }

rtos_syscall_0(0, void,         rtos_yield);
rtos_syscall_1(1, rtos_address, rtos_alloc, rtos_u32, nbr_bytes);
rtos_syscall_3(2, void,         rtos_send,  rtos_address, buffer_address, rtos_u32, dest_pid, rtos_u32, dest_inbox);
rtos_syscall_1(3, rtos_address, rtos_receive, rtos_u32, inbox);
rtos_syscall_1(4, void,         rtos_dispose, rtos_address, buffer_address);
rtos_syscall_0(5, void,         rtos_tick);
rtos_syscall_1(6, void,         rtos_delay, rtos_u32, nbr_ticks);
rtos_syscall_0(7, void,         rtos_wait_psem);
rtos_syscall_1(8, void,         rtos_signal_psem, rtos_u32, pid);
rtos_syscall_0(9, rtos_u32,     rtos_current_pid);
