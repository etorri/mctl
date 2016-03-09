#include <msp430g2553.h>
#include <inttypes.h>

#include "clock.h"
#include "pwm.h"
#include "motor.h"
#include "uart.h"
#include "encoder.h"
#include "interrupts.h"
#include "protocols.h"

#define PAUS __delay_cycles(1600000)

int main(void) {
  int speed;
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
  pwm_init();
  encoder_init();
  motor_init();
  interrupts_init();
  protocol_init();
  // -------------
  //    action
  // -------------
  _BIS_SR(GIE);

  while(1){
    // run state machines
    encoder_machine();
    motor_machine();
    input_handler();
    output_handler();
  }
}

