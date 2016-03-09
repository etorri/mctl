#include <msp430g2553.h>
#include <inttypes.h>
#include "uart.h"
#include "buffer.h"
#include "motor.h"
#include "encoder.h"
#include "clock.h"

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

  UCA0BR0   = 104;
  UCA0BR1   = 0;
  UCA0MCTL  = UCBRF(3)| UCBRS(0) | UCOS16;
  // 115200 timings with oversampling
  /*
  UCA0BR0   = 8; // divider low byte
  UCA0BR1   = 0; // divider high byte
  // baud rate modulation bits (UCB) and oversampling
  UCA0MCTL  = UCBRF(11)| UCBRS(0) | UCOS16;
  */
  UCA0CTL1 &= ~UCSWRST; // release reset
  // Enable rx interrupt
  //IE2 |= UCA0RXIE|UCA0TXIE;
}



void uart_write(const uint8_t c) {
  uart_put_uint8(c);
}

uint8_t uart_read(void) {
  uint8_t c;
  uart_get_uint8(&c);
  return c;
}


void uart_eol(void){
  uart_print("\r\n");
}

void uart_print(const char *s) {
  while(*s) {
    uart_write(*s++);
  }
}

void uart_printx(uint8_t *b, uint8_t n) {
  static char hex_table[] = "0123456789ABCDEF";
  uint8_t c;
  b+=n-1;
  while(n--) {
    c=*(b--);
    uart_write(hex_table[(c & 0xF0) >> 4]);
    uart_write(hex_table[c & 0x0F]);
  }
}

