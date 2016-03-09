#include <msp430g2553.h>
#include <inttypes.h>
#include "buffer.h"
#include "motor.h"
#include "clock.h"
#include "encoder.h"
#include "protocols.h"
#include "uart.h"

// timer for sending status report every second
uint32_t report_timer;

volatile uint8_t outmsg; 

#define REPORT_INTERVAL 4000000

void protocol_init(void){
  outmsg=OUT_NOTHING;
  set_timer(report_timer);
}



void output_handler(void){
  uint32_t time;
  // the regular report has priority
  if(interval_elapsed(&report_timer,REPORT_INTERVAL)){
    time=WALLTIME;
    uart_print("S");
    uart_printx(&motor_state,1);
    uart_print("\r\nT");
    uart_printx(&time,4);
    uart_print("\r\nS");
    uart_printx(&encoder_l_t,4);
    uart_printx(&encoder_r_t,4);
    uart_print("\r\nP");
    uart_printx(&encoder_l_pos,4);
    uart_printx(&encoder_r_pos,4);
    uart_print("\r\n");
  }
  switch(outmsg){
  case OUT_NOTHING:
    break;
  case OUT_S1:
    uart_print("READY\r\n");
    break;
  case OUT_S2:
    uart_print("RUN\r\n");
    break;
  case OUT_S3:
    uart_print("SLOW\r\n");
    break;
  case OUT_S4:
    uart_print("FAIL\r\n");
    break;
  }
  outmsg=OUT_NOTHING;
}

void input_handler(void){
  uint8_t c;
  if(uart_async_get_uint8(&c))
    switch(c){
    case IN_P:
      if(motor_state!=MSTATE_SLOWDOWN)
	left_power=right_power=300;
      motor_msg= MMSG_POWER;
      break;
    case 's':
      motor_msg= MMSG_STOP;
      break;
    case 'c':
      motor_msg=MMSG_CLEAR;
      break;
    case 'f':
      motor_msg=MMSG_FAIL;
      break;
    }
}
