#include <msp430g2553.h>
#include <inttypes.h>

#include "clock.h"
#include "pwm.h"
#include "uart.h"
#include "encoder.h"


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
  BCSCTL1= CALBC1_16MHZ; // Set DCO
  DCOCTL = CALDCO_16MHZ;
  
  clock_init();
  pwm_init();
  uart_init();
  pwm_test_set(5,200);
  encoder_init();
  _BIS_SR(GIE);
  uart_print("Start\r\n");
  while(1){
    uart_printx((uint8_t *)&encoder_r_t, 4);
    uart_eol();
  }
}
