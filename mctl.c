#include <msp430g2553.h>
#include <inttypes.h>

#include "clock.h"
//#include "pwm.h"
#include "motor.h"
#include "uart.h"
#include "encoder.h"

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
  
  clock_init();
  uart_init();
  motor_init();
  encoder_init();
  // -------------
  //    action
  // -------------
  _BIS_SR(GIE);

  uart_print("Showtime\r\n");
  pwm_lr_set(1600,0);
  uint8_t c;
  while(1){
    c=uart_read();
    switch(c){
    case 'a':
      uart_print("Motor enable\r\n");
      break;
    case 's':
      uart_print("Motor disable\r\n");
      break;
    default:
      uart_print("123456789012345678901234567890123456789012345678901234567890:");
      uart_write(c);
      uart_print("\r\n");
    }
  }
}

