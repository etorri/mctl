#include "clock.h"
#include "pwm.h"
#include "motor.h"
#include "uart.h"
#include "interrupts.h"
#include "protocols.h"
#include <msp430g2553.h>

#define PAUS __delay_cycles(1600000)
#define FOREVER while(1)

// Watchdog controls
#define WD_HOLD  WDTCTL = (WDTPW|WDTHOLD)
// Clear, select VLO clock (4KHz..20Khz, nominal 12KHz), divide by 8192
#define WD_CLEAR WDTCTL = (WDTPW|WDTCNTCL)
// WDT_ARST_1000
// (WDTPW|WDTCNTCL|WDTSSEL_2|WDTIS_2)


int main(void) {
  WD_HOLD;
  // configure the CPU clock (MCLK)
  // to run from SMCLK: DCO @ 16MHz and SMCLK = DCO
  // If no DCO calibration => stop here by loop
  if(CALBC1_16MHZ==0xff)
    while(1)
      _NOP();

  // ------- without interrupts
  _BIC_SR(GIE);
  //DCOCTL = 0;
  BCSCTL1= CALBC1_16MHZ;
  DCOCTL = CALDCO_16MHZ;
  //BCSCTL3= LFXT1S_2;     // VLO source
  PAUS;
  // initialize modules
  clock_init();
  pwm_init();
  motor_init();
  uart_init();
  protocols_init();

  // ------- interrupts enabled 
  _BIS_SR(GIE);
  WD_CLEAR;
  
  FOREVER {
    WD_CLEAR;
    // deal with possible input bytes from uart
    input_scanner();
    WD_CLEAR;
    // execute the possible input command
    motor_step();
    WD_CLEAR;
    // send status report
    reporter();
  }
}

