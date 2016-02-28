#include <msp430g2553.h>
#include <inttypes.h>
#include "uart.h"
#include "buffer.h"


volatile struct CircularBuffer txbuf;
volatile struct CircularBuffer rxbuf;


// Put byte to uart tx buffer and enable TX interrupts
void uart_put_uint8(uint8_t c) {
  if(IE2&UCA0TXIE) {
    while(put_buf(&txbuf,&c))
      _NOP();
  } else {
    IE2 |= UCA0TXIE;
    UCA0TXBUF=c;
  }
}

void uart_get_uint8(uint8_t *cp){
  while(get_buf(&rxbuf,cp))
	_NOP();
}


// uart TX ready for next byte ISR
void __attribute__((interrupt (USCIAB0TX_VECTOR)))
uart_tx_isr(void) {
  uint8_t c;
  if(get_buf(&txbuf,&c)==0) {
    // buffer had one character
    UCA0TXBUF=c;
  } else {
    // buffer underflow, TX isr done 
    IE2 &= ~UCA0TXIE;
  }
}


void __attribute__((interrupt (USCIAB0RX_VECTOR)))
uart_rx_isr(void) {
  uint8_t c;
  c=UCA0RXBUF; // clears interrupt flag too
  put_buf(&rxbuf,&c);
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
  IE2 |= UCA0RXIE;
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
