	.file	"pwm.c"
	.arch msp430g2553
	.cpu 430
	.mpy none

	.text
	.p2align 1,0
.global	pwm_l_init
	.type	pwm_l_init,@function
/***********************
 * Function `pwm_l_init' 
 ***********************/
pwm_l_init:
	bis.b	#64, &__P1DIR
	bis.b	#64, &__P1SEL
	mov	#7999, &__TA0CCR0
	mov	#224, &__TA0CCTL1
	mov	#0, &__TA0CCR1
	mov	#528, &__TA0CTL
	ret
.Lfe1:
	.size	pwm_l_init,.Lfe1-pwm_l_init
;; End of function 

	.p2align 1,0
.global	pwm_r_init
	.type	pwm_r_init,@function
/***********************
 * Function `pwm_r_init' 
 ***********************/
pwm_r_init:
	bis.b	#2, &__P2DIR
	bis.b	#2, &__P2SEL
	mov	#7999, &__TA1CCR0
	mov	#224, &__TA1CCTL1
	mov	#0, &__TA1CCR1
	mov	#528, &__TA1CTL
	ret
.Lfe2:
	.size	pwm_r_init,.Lfe2-pwm_r_init
;; End of function 

	.p2align 1,0
.global	pwm_l_set_duty
	.type	pwm_l_set_duty,@function
/***********************
 * Function `pwm_l_set_duty' 
 ***********************/
pwm_l_set_duty:
	cmp	#8001, r15
	jlo	.L4
	mov	#8000, r15
.L4:
	mov	r15, &__TA0CCR1
	ret
.Lfe3:
	.size	pwm_l_set_duty,.Lfe3-pwm_l_set_duty
;; End of function 

	.p2align 1,0
.global	pwm_r_set_duty
	.type	pwm_r_set_duty,@function
/***********************
 * Function `pwm_r_set_duty' 
 ***********************/
pwm_r_set_duty:
	cmp	#8001, r15
	jlo	.L7
	mov	#8000, r15
.L7:
	mov	r15, &__TA1CCR1
	ret
.Lfe4:
	.size	pwm_r_set_duty,.Lfe4-pwm_r_set_duty
;; End of function 

	.p2align 1,0
.global	clock_init
	.type	clock_init,@function
/***********************
 * Function `clock_init' 
 ***********************/
clock_init:
	mov.b	&__CALBC1_16MHZ, &__BCSCTL1
	mov.b	&__CALDCO_16MHZ, &__DCOCTL
	mov.b	#0, &__BCSCTL2
	ret
.Lfe5:
	.size	clock_init,.Lfe5-clock_init
;; End of function 

	.p2align 1,0
.global	uart_init
	.type	uart_init,@function
/***********************
 * Function `uart_init' 
 ***********************/
uart_init:
	bis.b	#6, &__P1DIR
	bis.b	#6, &__P1SEL
	bis.b	#6, &__P1SEL2
	bis.b	#1, &__UCA0CTL1
	bis.b	#llo(-128), &__UCA0CTL1
	mov.b	#llo(-126), &__UCA0BR0
	mov.b	#6, &__UCA0BR1
	mov.b	#91, &__UCA0CTL0
	and.b	#llo(-2), &__UCA0CTL1
	ret
.Lfe6:
	.size	uart_init,.Lfe6-uart_init
;; End of function 

	.p2align 1,0
.global	uart_read
	.type	uart_read,@function
/***********************
 * Function `uart_read' 
 ***********************/
uart_read:
.L11:
	mov.b	&__IFG2, r15
	and	#1, r15
	jeq	.L11
	mov.b	&__UCA0RXBUF, r15
	ret
.Lfe7:
	.size	uart_read,.Lfe7-uart_read
;; End of function 

	.p2align 1,0
.global	uart_write
	.type	uart_write,@function
/***********************
 * Function `uart_write' 
 ***********************/
uart_write:
	jmp	.L18
.L16:
	nop
.L18:
	mov.b	&__IFG2, r14
	and	#2, r14
	jeq	.L16
	and.b	#llo(-3), &__IFG2
	mov.b	r15, &__UCA0TXBUF
	ret
.Lfe8:
	.size	uart_write,.Lfe8-uart_write
;; End of function 

	.p2align 1,0
.global	uart_print
	.type	uart_print,@function
/***********************
 * Function `uart_print' 
 ***********************/
uart_print:
	mov.b	@r15, r13
	cmp.b	#0, r13
	jeq	.L29
.L24:
	add	#1, r15
	mov.b	&__IFG2, r14
	and	#2, r14
	jne	.L21
.L25:
	nop
	mov.b	&__IFG2, r14
	and	#2, r14
	jeq	.L25
.L21:
	and.b	#llo(-3), &__IFG2
	mov.b	r13, &__UCA0TXBUF
	mov.b	@r15, r13
	cmp.b	#0, r13
	jne	.L24
.L29:
	ret
.Lfe9:
	.size	uart_print,.Lfe9-uart_print
;; End of function 

	.p2align 1,0
.global	printx
	.type	printx,@function
/***********************
 * Function `printx' 
 ***********************/
printx:
	mov.b	r15, r14
	clrc
	rrc.b	r14
	clrc
	rrc.b	r14
	clrc
	rrc.b	r14
	clrc
	rrc.b	r14
	mov.b	r14, r14
	mov.b	hex_table.1445(r14), r13
	mov.b	&__IFG2, r14
	and	#2, r14
	jne	.L31
.L36:
	nop
	mov.b	&__IFG2, r14
	and	#2, r14
	jeq	.L36
.L31:
	and.b	#llo(-3), &__IFG2
	mov.b	r13, &__UCA0TXBUF
	and	#15, r15
	mov.b	hex_table.1445(r15), r14
	mov.b	&__IFG2, r15
	and	#2, r15
	jne	.L33
.L35:
	nop
	mov.b	&__IFG2, r15
	and	#2, r15
	jeq	.L35
.L33:
	and.b	#llo(-3), &__IFG2
	mov.b	r14, &__UCA0TXBUF
	ret
.Lfe10:
	.size	printx,.Lfe10-printx
;; End of function 

	.section	.rodata
.LC0:
	.string	"HELLO\n"
.LC1:
	.string	"HELLO WORLD!\n"
	.section	.init9,"ax",@progbits
	.p2align 1,0
.global	main
	.type	main,@function
/***********************
 * Function `main' 
 ***********************/
main:
	mov	#23168, &__WDTCTL
	mov.b	&__CALBC1_16MHZ, &__BCSCTL1
	mov.b	&__CALDCO_16MHZ, &__DCOCTL
	mov.b	#0, &__BCSCTL2
	bis.b	#64, &__P1DIR
	bis.b	#64, &__P1SEL
	mov	#7999, &__TA0CCR0
	mov	#224, &__TA0CCTL1
	mov	#0, &__TA0CCR1
	mov	#528, &__TA0CTL
	bis.b	#2, &__P2DIR
	bis.b	#2, &__P2SEL
	mov	#7999, &__TA1CCR0
	mov	#224, &__TA1CCTL1
	mov	#0, &__TA1CCR1
	mov	#528, &__TA1CTL
	mov	#6000, &__TA0CCR1
	mov	#6, &__TA1CCR1
	mov	#.LC1, r15
	call	#uart_print
	mov	#.LC0, r14
	jmp	.L47
.L43:
	add	#1, r14
	mov.b	&__IFG2, r15
	and	#2, r15
	jne	.L41
.L45:
	nop
	mov.b	&__IFG2, r15
	and	#2, r15
	jeq	.L45
.L41:
	and.b	#llo(-3), &__IFG2
	mov.b	r13, &__UCA0TXBUF
.L47:
	mov.b	@r14, r13
	cmp.b	#0, r13
	jne	.L43
	mov	#.LC0, r14
	jmp	.L47
.LIRD0:
.Lfe11:
	.size	main,.Lfe11-main
;; End of function 

	.section	.rodata
	.type	hex_table.1445,@object
	.size	hex_table.1445,17
hex_table.1445:
	.string	"0123456789abcdef"
