#include <msp430g2553.h>
#include <inttypes.h>

#include "clock.h"
//#include "pwm.h"
#include "motor.h"
#include "uart.h"
#include "encoder.h"

//
// special test program for checking the motor routines
//

#define PAUS __delay_cycles(1600000)

int main(void) {
  WDTCTL = WDTPW + WDTHOLD;   // Stop WDT
  // configure the CPU clock (MCLK)
  // to run from SMCLK: DCO @ 16MHz and SMCLK = DCO
  // If no DCO calibration => stop here by loop
  if(CALBC1_16MHZ==0xff)
    while(1)
      _NOP();
  _BIC_SR(GIE);
  DCOCTL = 0;
  BCSCTL1= CALBC1_16MHZ;
  DCOCTL = CALDCO_16MHZ;
  
  P1DIR |= BIT0|BIT6;
  P1OUT |= BIT0|BIT6;
  clock_init();
  uart_init();
  motor_init();
  //encoder_init();
  // -------------
  //    action
  // -------------
  _BIS_SR(GIE);
  __delay_cycles(1600000);
  P1OUT &= ~(BIT0|BIT6);
  __delay_cycles(1600000);

  P2DIR |= BIT0|BIT2;
  pwm_lr_set(1600,0);
  uint8_t c;
  while(1){
    P2OUT = (P2OUT|BIT2)&~BIT0;
    P1OUT|=BIT0;
    c=uart_read();
    P1OUT|=BIT6;
    P2OUT = (P2OUT|BIT0)&~BIT2;
    PAUS;
    uart_write(c);
    P1OUT &= ~(BIT0|BIT6);
    PAUS;
  }
}

