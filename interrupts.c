#include <msp430g2553.h>
#include <inttypes.h>
#include "interrupts.h"
#include "uart.h"
#include "encoder.h"
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

// uart TX ready for next byte ISR
void __attribute__((interrupt (USCIAB0TX_VECTOR)))
uart_tx_isr(void) {
  uint8_t c;
  if(get_buf(&txbuf,&c)) {
    // got nothing to send, stop interrupting 
    IE2 &= ~UCA0TXIE;
  } else {
    // got byte from the buffer, send it out
    UCA0TXBUF=c;
  }
}


// received a byte, get it (and clear the interrupt), put it to rx buffer
void __attribute__((interrupt (USCIAB0RX_VECTOR)))
uart_rx_isr(void) {
  uint8_t c;
  c=UCA0RXBUF;
  put_buf(&rxbuf,&c);
}


uint32_t time_now;

void __attribute__((interrupt (PORT1_VECTOR))) p1isr() {
  time_now=WALLTIME;
  if(P1IFG&BIT4) {
    // LENCA Left encoder A input
    encoder_l_t=timediff(time_now,l_prev);
    l_prev=time_now;
    if(P1IN&BIT5){
      encoder_l_pos++;
    }else{
      encoder_l_pos--;
    }
    P1IFG &= ~BIT4;
  }
  
  if(P1IFG&BIT7) {
    // RENCA Right encoder A input
    encoder_r_t=timediff(time_now,r_prev);
    r_prev=time_now;
    if(P1IN&BIT6){
      encoder_r_pos--;
    }else{
      encoder_r_pos++;
    }
    P1IFG &= ~BIT7;
  }  
  if(P1IFG&BIT3){
    // LDIAG Left diagnostic input
    // the interrupt will be cleared only after CLEAR message
    //P1IFG &= ~BIT3;
    // signal failure in motor driver
    motor_msg|= MMSG_FAIL;
  }
}


void __attribute__((interrupt (PORT2_VECTOR))) p2isr() {
  if(P2IFG&BIT7){
    // RDIAG Right diagnostic input
    // P2IFG &= ~BIT7;
    motor_msg|=MMSG_FAIL;
  }
}
