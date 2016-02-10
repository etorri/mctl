#include <msp430g2553.h>
#include <inttypes.h>
#include "clock.h"
#include "encoder.h"

volatile uint8_t encoder_event;
volatile uint32_t encoder_l_t, encoder_r_t; 
volatile int32_t encoder_l_pos,encoder_r_pos;

static uint32_t l_prev,r_prev,l_now,r_now;

// Encoder interrupts
// left P1.3
// right P1.7
//
void encoder_init(void){
  encoder_l_t= encoder_r_t = 0xffffffff;
  l_prev= r_prev = WALLTIME;
  P1IE |= BIT3|BIT7;
  P1REN |= BIT3|BIT7;
  encoder_event=0;
  
  //P1OUT |= BIT3|BIT7;
}


void __attribute__((interrupt (PORT1_VECTOR))) p1isr() {
  if(P1IFG&BIT3) {
    l_now=WALLTIME;
    encoder_l_t=timediff(l_now,l_prev);
    l_prev=l_now;
    P1IFG &= ~BIT3;
  }
  if(P1IFG&BIT7) {
    r_now=WALLTIME;
    encoder_r_t=timediff(r_now,r_prev);
    r_prev=r_now;
    P1IFG &= ~BIT7;
  }
}
