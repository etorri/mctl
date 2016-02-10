#include <msp430g2553.h>
#include <inttypes.h>
#include "uart.h"


void uart_init(void) {
  // Select P1.1 and P1.2 for UART
  P1SEL   |= RXD|TXD;
  P1SEL2  |= RXD|TXD;
  // Hold USCI in reset to allow configuration
  UCA0CTL1 = UCSWRST;
  // Configure USCI UART 9600,8,1 @ 16MHz SMCLK 
  UCA0CTL0  = 0x00;
  UCA0CTL1 |= UCSSEL_2; // SMCLK
  /* 9600
  UCA0BR0   = 104;
  UCA0BR1   = 0;
  UCA0MCTL  = UCBRF(3)| UCBRS(0) | UCOS16;
  */
  /* 115200
   */
  UCA0BR0   = 8;
  UCA0BR1   = 0;
  UCA0MCTL  = UCBRF(11)| UCBRS(0) | UCOS16;
  /*
   */
  // Release reset
  UCA0CTL1 &= ~UCSWRST;
}



void uart_write(const uint8_t c) {
  // wait while tx buffer is not empty
  while(!(IFG2&UCA0TXIFG)) _NOP();
  UCA0TXBUF = c;
}

void uart_eol(void){
  uart_print("\r\n");
}

uint8_t uart_read(void) {
  uint8_t c;
  // wait while there is no complete received character in rx buffer
  while(!(IFG2&UCA0RXIFG)) _NOP();
  c= UCA0RXBUF;
  IFG2 &= ~UCA0RXIFG;
  //printx(&c,1);
  //uart_status=0;
  return c;
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
