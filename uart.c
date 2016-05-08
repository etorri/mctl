#include <msp430g2553.h>
#include <inttypes.h>
#include "clock.h"
#include "uart.h"
#include "buffer.h"
#include "motor.h"

volatile struct CircularBuffer txbuf;
volatile struct CircularBuffer rxbuf;


// loop trying to put the given byte to tx buffer until it succeeds.
// enable tx interrupt (even if it is already on) to get the tx machine going
void uart_put_uint8(uint8_t c) {
  while(put_buf(&txbuf,&c))
    _NOP();
  IE2 |= UCA0TXIE;
}

void uart_get_uint8(uint8_t *cp){
  while(get_buf(&rxbuf,cp))
	_NOP();
}

uint8_t uart_async_put_uint8(uint8_t c) {
  if(put_buf(&txbuf,&c)){
    return 0;
  }
  IE2 |= UCA0TXIE;
  return 1;
}


uint8_t uart_async_get_uint8(uint8_t *cp){
  return !get_buf(&rxbuf,cp);
}



void uart_init(void) {
  init_buf(&txbuf);
  init_buf(&rxbuf);
  // Select P1.1 and P1.2 for UART
  P1SEL   |= RXD|TXD;
  P1SEL2  |= RXD|TXD;
  // Hold USCI in reset to allow configuration
  UCA0CTL1 = UCSWRST;
  // Configure USCI UART 8 data 1 stop
  UCA0CTL0  = 0x00;
  UCA0CTL1 |= UCSSEL_2; // SMCLK

  // 9600 timings with oversampling
  /*
  UCA0BR0   = 104;
  UCA0BR1   = 0;
  UCA0MCTL  = UCBRF(3)| UCBRS(0) | UCOS16;
  */

  // 115200 timings with oversampling
  UCA0BR0   = 8; // divider low byte
  UCA0BR1   = 0; // divider high byte

  // baud rate modulation bits (UCB) and oversampling
  UCA0MCTL  = UCBRF(11)| UCBRS(0) | UCOS16;

  UCA0CTL1 &= ~UCSWRST; // release reset
  // Enable rx interrupt
  IE2 |= UCA0RXIE;
}


// uart TX ready for next byte ISR
void __attribute__((interrupt (USCIAB0TX_VECTOR)))
uart_tx_isr(void) {
  static uint8_t c;
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
  static uint8_t c;
  //c=UCA0RXBUF;
  put_buf(&rxbuf,&UCA0RXBUF);
}

