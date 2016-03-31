
#include "clock.h"
#include "pwm.h"
#include "motor.h"
#include "uart.h"
#include "interrupts.h"
#include "protocols.h"

#ifndef TEST
#include <msp430g2553.h>
#endif
#include <inttypes.h>

#ifndef TEST
#define PAUS __delay_cycles(1600000)
#endif

int main(void) {

#ifndef TEST
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
#endif
  
  clock_init();
  uart_init();
  motor_init();
  protocols_init();

#ifndef TEST
  pwm_init();
  interrupts_init();

  //protocol_init();
  // -------------
  //    action
  // -------------
  _BIS_SR(GIE);
#endif
  
  while(1){
    // read input
    input_scanner();
    // handle any pending state changes in motor
    motor_step();
    // send status reports to pi
    reporter();
  }
}

