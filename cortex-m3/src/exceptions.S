@@ Code to be generated for the thumb-2 instruction set.
	.syntax	unified
	.thumb

@@@ Code to .text segment:
	.section	.text

	
@@@ 
@@@ Function:	CM3_handler_svc
@@@   Exception handler for the SVC (syscall) exception. TODO.
@@@
@@@ Parameters:
@@@   TODO.
@@@ 
	.global	CM3_handler_svc
	.thumb_func
	.extern	syscall_pointers
CM3_handler_svc:
	@@ First, retrieve syscall arguments from the stack. If we are called
	@@ from thread mode, arguments are on the process stack. If we are
	@@ called from handler mode, arguments are on the main stack.

	@@ Bit 2 in EXC_RETURN tells if we are to return to thread mode or
	@@ handler mode. If bit 2 is zero, it is handler mode.
	tst	lr,	#4
	ite	eq
	mrseq	r0,	MSP
	mrsne	r0,	PSP

	@@ Push r4, r5 and lr on main stack. Must do this pushing after the
	@@ reading of the calling context stack pointer above. If we are
	@@ called from handler mode, the pushing would be to the same stack
	@@ as the one we are reading the arguments from, which would require
	@@ different offsets to be used when reading the arguments.
	push	{r4, r5, lr}

	@@ Remember stack value in r4.
	mov	r4,	r0
	
	@@ Now, get the arguments.
	ldr	r0,	[r4, #0]
	ldr	r1,	[r4, #4]
	ldr	r2,	[r4, #8]
	ldr	r3,	[r4, #12]
	ldr	r12,	[r4, #16]
	
	@@ Now, find out and call the right syscall.
	ldr	r5,	=syscall_pointers	@ r5 points at syscall table.
	ldr	r12,	[r5, r12, lsl #2]	@ r12 holds syscall address.
	blx	r12				@ Perform syscall.
	
	@@ Store syscall return value on process stack.
 	str	r0,	[r4]	@ Store syscall return value on right stack.

	@@ Pop registers and return from exception.
	pop	{r4, r5, pc}
	

@@@ 
@@@ Function:	CM3_handler_pendsv
@@@   Exception handler for the PendSV exception. If any other exception
@@@   handler concludes that a reschedule is necessary, it pends the PendSV
@@@   exception which is then called after all other exceptions are handled
@@@   (as it has the lowest priority, together with SVC). This handler will
@@@   then call the portable kernel to reschedule and then perform a context
@@@   switch. The context switch is performed from current_pcb to new_pcb and
@@@   exception return will be to new_pcb.
@@@
@@@ Parameters:
@@@   None.
@@@
	.global	CM3_handler_pendsv
	.thumb_func
	.extern rtos_reschedule_hook
	.extern current_pcb
	.extern new_pcb
CM3_handler_pendsv:
	bl	rtos_reschedule_hook 	@ Update new_pcb.
	mrs	r12,	PSP		@ Get PSP for current process.
	stmfd	r12!,	{r4-r11}	@ Save remaining registers.
	ldr	r0,	=current_pcb	@ r0 = &current_pcb.
	ldr	r1,	[r0]		@ r1 = current_pcb 
	str	r12,	[r1, 8]		@ Update SP in PCB.
	ldr	r1,	=new_pcb	@ r1 = &new_pcb
	ldr	r1,	[r1]		@ r1 = new_pcb
	ldr	r12,	[r1, 8]		@ r12 = SP for new process.
	ldmfd	r12!,	{r4-r11}	@ Restore r4-r11 for new process.
	msr	PSP,	r12		@ Update SP for new process.
	str	r1,	[r0]		@ current_pcb = new_pcb
	ldr	lr,	=0xfffffffd	@ Use process stack when returning.
	bx	lr			@ Return to new process.
