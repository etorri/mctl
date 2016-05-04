#include "clock.h"
#include "pwm.h"
#include "motor.h"
#include "uart.h"
#include "interrupts.h"
#include "protocols.h"
#include <msp430g2553.h>

#define PAUS __delay_cycles(1600000)
#define FOREVER while(1)

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
  
  clock_init();
  uart_init();
  motor_init();
  protocols_init();
  pwm_init();

  // ----------
  // **action**
  // ----------
  _BIS_SR(GIE);
  
  FOREVER {
    // read input
    input_scanner();
    // handle any pending state changes in motor
    motor_step();
    // send status reports to pi
    reporter();
  }
}

