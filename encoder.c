#include <msp430g2553.h>
#include <inttypes.h>
#include "clock.h"
#include "encoder.h"

volatile uint32_t encoder_l_t, encoder_r_t; 
volatile uint32_t l_prev,r_prev;

volatile int32_t encoder_l_pos,encoder_r_pos;


// Encoder interrupts
// left P1.3
// right P1.7
//
void encoder_init(void){
  // initialize these just in case
  encoder_l_pos = encoder_r_pos = 0;
  encoder_l_t= encoder_r_t = 0;
  // no interrupts yet
  // initialize the previous time at 0 so first interrupt will get very long measure
  l_prev= r_prev = WALLTIME;
  P1DIR &= ~(BIT4|BIT5|BIT6|BIT7);
  // enable encoder input pin interrupts
  P1IE |= BIT4|BIT7;
}


void encoder_machine(void){
  static uint32_t now;
  now=WALLTIME;
  if(timediff(now,l_prev)>0xFFFF){
    encoder_l_t=NOT_MOVING;
  }
  if(timediff(now,r_prev)>0xFFFF){
    encoder_r_t=NOT_MOVING;
  }
}
