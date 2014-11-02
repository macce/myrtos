@@ Code to be generated for the thumb-2 instruction set.
	.syntax	unified
	.thumb

	
@@@ 
@@@ Function:	arch_init_stack
@@@   Called from portable part to initialize the stack for a process.
@@@   The stack is set up in such a way that a context switch to the
@@@   process makes it start running from it's entrypoint.
@@@
@@@ Parameters:
@@@   r0 = pointer to PCB for the process whose stack is to be
@@@     initialized.
@@@ 
	.global arch_init_stack
	.thumb_func
arch_init_stack:
	ldr	r1,	[r0]		@ r1 = entry
	ldr	r12,	[r0, #4]	@ r12 = thread_stack_top
	sub	r12,	r12, #32	@ Subtract space for first 8 regs. 
	str	r1,	[r12, #24]	@ Push entrypoint on stack.
	mov	r1,	0
	str	r1,	[r12, #20]
	mov	r1,	0x01000000
	str	r1,	[r12, #28]	@ Set the T-bit in xPSR.
	sub	r12,	r12, 32		@ Subtract space for r4-r11.
	str	r12,	[r0, #8]	@ Store SP in PCB. 
	bx	lr

@@@ 
@@@ Function:	start_process
@@@   Called from portable part to start the selected process. This is where
@@@   the OS is started, i.e. from now on there is always a process running,
@@@   or the OS is in sleep mode.
@@@
@@@   The function prepares the specified process to be started by pointing
@@@   the process stack pointer at the top of its stack. It then resets the
@@@   main stack pointer (which is used when this function is called), so
@@@   this function will never return to it's caller. Instead it returns to
@@@   the process to be started.
@@@
@@@ Parameters:
@@@   r0 = pointer to PCB for process to start.
@@@ 
	.global start_process
	.thumb_func
	.extern _start_stack_end
start_process:
	add	r1,	r0, #4		@ r1 = &thread_stack_top
	ldr	r1,	[r1]		@ Set r1 = thread_stack_top.
	sub	r1,	r1, #32		@ 8 regs will be popped during 'bx lr'.
	msr	PSP,	r1		@ Initialize process stack pointer.
	ldr	r1,	=_start_stack_end
	mov	sp,	r1	    	@ Reset the main stack pointer.
	ldr	lr,	=0xfffffffd	@ Use process stack when returning.
	bx	lr			@ Return, starting the process.

	
@@@ 
@@@ Function:	arch_trigger_pendsv
@@@   Called from portable part when a context switch should be
@@@   pended.
@@@
@@@ Parameters:
@@@   None
@@@ 
	.global arch_trigger_pendsv
	.thumb_func
arch_trigger_pendsv:
	ldr	r0,	=0xE000ED04	@ ICSR
	mov	r1,	0x10000000	@ PENDSVSET
	str	r1,	[r0]		@ Set the bit.
	bx	lr

	
@@@ 
@@@ Function:	arch_trigger_pendsv_with_retval
@@@   Called from portable part when a context switch should be
@@@   pended. The process to be switched to is pointed out by new_pcb and
@@@   it will return from a syscall with the return value retval (r0).
@@@   This function is used for instance from the portable 'send' syscall
@@@   to prepare for a context switch to the process that received the
@@@   message. That process will return from a receive system call and
@@@   therefore requires the adress to the received message as it's return
@@@   value.
@@@
@@@ Parameters:
@@@   r0 = Return value from syscall.
@@@ 
	.global arch_trigger_pendsv_with_retval
	.thumb_func
	.extern new_pcb
arch_trigger_pendsv_with_retval:
	@@ Store retval (r0) on stack for new_pcb.
	ldr	r1,	=new_pcb 	@ r1 = &new_pcb
	ldr	r1,	[r1]		@ r1 = new_pcb
	ldr	r1, 	[r1, 8]		@ r1 = SP
	str	r0,	[r1, 0x20]	@ Store return value on stack.

	@@ Pend a pendsv request bit.
	ldr	r1,	=0xE000ED04	@ ICSR
	mov	r2,	0x10000000	@ PENDSVSET
	str	r2,	[r1]		@ Set the bit.
	bx	lr

@@@ 
@@@ Function:	arch_store_retval
@@@
@@@ Parameters:
@@@   r0 = Return value from syscall.
@@@   r1 = Pointer to PCB.
@@@ 
	.global arch_store_retval
	.thumb_func
arch_store_retval:
	@@ Store retval (r0) on stack for PCB pointed out by r1.
	ldr	r1, 	[r1, 8]		@ r1 = SP
	str	r0,	[r1, 0x20]	@ Store return value on stack.
	bx	lr
