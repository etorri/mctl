#include <msp430g2553.h>
#include <inttypes.h>

#include "clock.h"
#include "pwm.h"
#include "motor.h"
#include "uart.h"
#include "encoder.h"

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
  // -------------
  //    action
  // -------------
  _BIS_SR(GIE);

  uart_print("Initialized\r\n");
  uint8_t c;
  while(1){
    c=uart_read();
    switch(c){
    case 'e':
      uart_print("Motor enable\r\n");
      motor_enable();
      break;
    case 'd':
      uart_print("Motor disable\r\n");
      motor_disable();
      break;
    case 'i':
      uart_print("Motor initial state\r\n");
      motor_disable();
      break;
    case 'f':
      uart_print("Motor Forward\r\n");
      speed=10;
      motor_set_left(speed);
      motor_set_right(speed);
      break;
    case 'b':
      uart_print("Motor Backward\r\n");
      speed=-10;
      motor_set_right(speed);
      motor_set_left(speed);
      break;
    case '[':
      uart_print("Motor -\r\n");
      speed-=10;
      motor_set_left(speed);
      motor_set_right(speed);
      break;
    case ']':
      uart_print("Motor +\r\n");
      speed+=10;
      motor_set_left(speed);
      motor_set_right(speed);
      break;
    case 'p':
      uart_print("Motor position\r\n");
      uart_printx(&encoder_l_pos,4);
      uart_print("\r\n");
      uart_printx(&encoder_r_pos,4);
      uart_print("\r\n");
      break;
    case 's':
      uart_print("Motor speed\r\n");
      uart_printx(&encoder_l_t,4);
      uart_print("\r\n");
      uart_printx(&encoder_r_t,4);
      uart_print("\r\n");
      break;
    case 't':
      uart_print("Time\r\n");
      uint32_t tm;
      tm=WALLTIME;
      uart_printx(&tm,4);
      uart_print("\r\n");
      break;
    default:
      uart_print("What?\r\n");
    }
  }
}

