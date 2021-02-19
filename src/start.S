	.section .text

	.global reset_vec

reset_vec:
	j start

start:
	// init stack pointer
	lui sp, %hi(_stack)
	addi sp, sp, %lo(_stack)

	// jump to C main function
	jal ra, main

	// if it returns, loop forever
loop:
	j loop
		
	// stack for the interrupt handler - 512 bytes (128*4), 16 byte aligned
        // grows from label stack downwards!

