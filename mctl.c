#include <msp430g2553.h>
#include <inttypes.h>

// Pwm
//  Left motor P1.6
//  Right motor P2.1
#include "pwm.h"

// Serial port
//  RX P1.1
//  TX P1.2
#include "uart.h"

// uint32_t clock32
//   Timer B, 16-bit @2MHz
//   interrupt handler 
#include "clock32.h"

// Rotary encoders
// PX.X - Left A
// PX.X - Left B
// PX.X - Right A
#include "encoders.h"

// Notes to self
#define PWMCLOCK   16000000 // 16MHz
#define PWM_PERIOD 8000     // 10KHz
#define PWM_MIN    0
#define PWM_MAX     8000

void pwm_l_init() {
  // set port
  P1DIR |= BIT6;          // P1.6  output
  P1SEL |= BIT6;          // P1.6  set to TA0.1 
  TA0CCR0 = PWM_PERIOD-1;    // Timer counts to this many
  TA0CCTL1 = OUTMOD_7;     // CCR1 reset/set
  TA0CCR1 = PWM_MIN;       // CCR1 PWM Ton -> duty cycle
  TA0CTL = TASSEL_2 + MC_1;// SMCLK, up mode
}

void pwm_r_init() {
  // set port
  P2DIR |= BIT1;          // P2.1  output
  P2SEL |= BIT1;          // P2.1  set to TA1.1 
  TA1CCR0 = PWM_PERIOD-1;    // Timer counts to this many
  TA1CCTL1 = OUTMOD_7;     // CCR1 reset/set
  TA1CCR1 = PWM_MIN;       // CCR1 PWM Ton -> duty cycle
  TA1CTL = TASSEL_2 + MC_1;// SMCLK, up mode
}


void pwm_l_set_duty(uint16_t duty){
  if (duty>PWM_MAX) duty=PWM_MAX;
  TA0CCR1 = duty;
}

void pwm_r_set_duty(uint16_t duty){
  if (duty>PWM_MAX) duty=PWM_MAX;
  TA1CCR1 = duty;
}


void clock_init() {
  // configure the CPU clock (MCLK)
  // to run from DCO @ 16MHz and SMCLK = DCO
  if(CALBC1_16MHZ==0xff)
    while(1) _NOP();
  DCOCTL = 0;
  BCSCTL1= CALBC1_16MHZ; // Set DCO
  DCOCTL = CALDCO_16MHZ;
  // let it settle for a while (1/100 s)
  __delay_cycles(16000000/100);
}


#define RXD    BIT1
#define TXD    BIT2
#define BPS    9600
#define DBITS  8
#define PARITY 1

#define UCBRF(X) ((X)<<4)
#define UCBRS(X) ((X)<<1)

void uart_init(void) {
  // Select P1.1 and P1.2 for UART
  P1SEL   |= RXD|TXD;
  P1SEL2  |= RXD|TXD;
  // Hold USCI in reset to allow configuration
  UCA0CTL1 = UCSWRST;
  // Configure USCI UART 9600,8,1 @ 16MHz SMCLK 
  UCA0CTL0  = 0x00;
  UCA0CTL1 |= UCSSEL_2; // SMCLK
  UCA0BR0   = 104;
  UCA0BR1   = 0;
  UCA0MCTL  = UCBRF(3)| UCBRS(0) | UCOS16;
  // Release reset
  UCA0CTL1 &= ~UCSWRST;
}


#define UART_NO_INPUT 1
static uint8_t uart_status=0;

void uart_write(uint8_t c) {
  // wait while tx buffer is not empty
  while(!(IFG2&UCA0TXIFG)) _NOP();
  UCA0TXBUF = c;
}

uint8_t uart_read(void) {
  uint8_t c;
  // wait while there is no complete received character in rx buffer
  while(!(IFG2&UCA0RXIFG)) _NOP();
  c= UCA0RXBUF;
  IFG2 &= ~UCA0RXIFG;
  printx(c);
  //uart_status=0;
  return c;
}

void uart_print(const char *s) {
  while(*s) {
    uart_write(*s++);
    __delay_cycles(1600000);
  }
}

void printx(const uint8_t c) {
  static char hex_table[] = "0123456789abcdef";
  uart_write(hex_table[(c & 0xF0) >> 4]);
  uart_write(hex_table[c & 0x0F]);
}

    
void main(void) {
  uint16_t l,r;
  uint8_t b;
  uint8_t state=0;
  WDTCTL = WDTPW + WDTHOLD;   // Stop WDT

  clock_init();
  pwm_l_init();
  pwm_r_init();
  uart_init();
  while(1){
    switch(state) {
    case 0:
      uart_print("(0)\r\n");
      // wait for start character
      if(uart_read()=='a') state=1;      
      break;
    case 1:
      uart_print("(1)\r\n");
      l=uart_read();
      l= (l<<8) | uart_read();
      pwm_l_set_duty(l%8000);
      r=uart_read();
      r= (r<<8) | uart_read();
      pwm_r_set_duty(r%8000);
      state=0;
      break;
    }
  }
}
