#include <msp430g2553.h>
#include <inttypes.h>
#include "interrupts.h"
#include "uart.h"
#include "buffer.h"
#include "motor.h"
#include "clock.h"

void interrupts_init(void){
  // enable uart interrupts
  IE2 |= UCA0RXIE;
  // enable encoder interrupts
  P1IE |= BIT4|BIT7;
  // enable DIAG interrupts
  P1IE |= BIT3; // LDIAG
  P1IES|= BIT3; // trigger at high to low transition
  P2IE |= BIT7; // RDIAG
  P2IES|= BIT7;
  
}

