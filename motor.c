#include <msp430g2553.h>
#include <inttypes.h>
#include <stdlib.h>
#include "motor.h"
#include "pwm.h"
#include "encoder.h"
#include "clock.h"
#include "protocols.h"

volatile uint8_t motor_state;
volatile uint8_t motor_msg;
volatile int left_power;
volatile int right_power;
uint32_t power_timer;

// max 1 second without power setting commands from Pi
// in RUN state
#define MAX_POWER_SETTING_INTERVAL 8000000
// in Slow
#define SLOW_DOWN_INTERVAL         20000

void motor_set_power(void);

inline void motor_ready(void){
  // set pwm to 0
  left_power=right_power=0;
  motor_set_power();
  // motor in breaking state
  P2OUT &= ~(LINA|LINB|RINA|RINB);
}


void motor_init(void) {
  // Set INA and INB as normal IO pin outputs
  P2DIR  |= LINA|LINB|RINA|RINB;
  P2SEL  &= ~(LINA|LINB|RINA|RINB);
  P2SEL2 &= ~(LINA|LINB|RINA|RINB);
  // initialize variables
  motor_state=MSTATE_READY;
  motor_msg=0;
  // do what is needed to enter the READY state
  motor_ready();
}



 
inline void motor_set_left(int power) {
  int8_t sgn;
  sgn= (power>=0);
  power=abs(power);
  if(sgn) {
    // power is positive => CW
    P2OUT = (P2OUT|LINA)&(~LINB);
  }else{
    // power is negative => CCW
    P2OUT = (P2OUT|LINB)&(~LINA);
  }
  pwm_l_set(power);
}


inline void motor_set_right(int power) {
  int8_t sgn;
  sgn= (power>=0);
  power=abs(power);
  if(sgn) {
    P2OUT = (P2OUT|RINA)&(~RINB);
  }else{
    P2OUT = (P2OUT|RINB)&(~RINA);
  }
  pwm_r_set(power);
}

inline void motor_set_power(void){
  motor_set_right(right_power);
  motor_set_left(left_power);
}



void motor_machine(void) {
  switch(motor_state){
  case MSTATE_READY:
    if(motor_msg&MMSG_FAIL){
      motor_ready();
      motor_state=MSTATE_FAIL;
      outmsg=OUT_S4;
    } else if(motor_msg&MMSG_POWER) {
      motor_set_power();
      set_timer(&power_timer);
      motor_state=MSTATE_RUN;
      outmsg=OUT_S2;
    }
    motor_msg=0;
    break;
	  
  case MSTATE_RUN:
    // if Pi is not talking frequently enough (dead?) stop everything
    // and do not keep driving towards the cliff
    if(interval_elapsed(&power_timer,MAX_POWER_SETTING_INTERVAL)){
      motor_msg=MMSG_TIMEOUT; //motor_msg=MMSG_TIMEOUT;
    }
    if(motor_msg&MMSG_FAIL) {
      motor_ready();
      motor_state=MSTATE_FAIL;
      motor_msg=0;
      outmsg=OUT_S4;
    } else if(motor_msg&MMSG_POWER) {
      set_timer(&power_timer);
      motor_set_left(left_power);
      motor_set_right(right_power);
      motor_state=MSTATE_RUN;
      outmsg=OUT_S2;
    } else if(motor_msg&MMSG_STOP) {
      motor_ready();
      motor_state=MSTATE_READY;
      outmsg=OUT_S1;
    } else if(motor_msg&MMSG_TIMEOUT) {
      motor_state=MSTATE_SLOWDOWN;
      set_timer(&power_timer);
      outmsg=OUT_S3;
    }
    motor_msg=0;
    break;

  case MSTATE_SLOWDOWN:
    if(interval_elapsed(&power_timer,SLOW_DOWN_INTERVAL)){
      if(left_power!=0){
	left_power += (left_power<0)?1:-1;
      }
      if(right_power!=0){
	right_power += (right_power<0)?1:-1;
      }
      motor_set_power();
    }
    if(motor_msg&MMSG_FAIL) {
      motor_ready();
      motor_state=MSTATE_FAIL;
      outmsg=OUT_S4;
    } else if (motor_msg&MMSG_STOP) {
      // Clear the slowdown state
      motor_ready();
      motor_state=MSTATE_READY;
      outmsg=OUT_S1;
    }
    motor_msg=0;
    break;

  case MSTATE_FAIL:
    if(motor_msg&MMSG_CLEAR){
      motor_state=MSTATE_READY;
      motor_msg=0;
      outmsg=OUT_S1;
    }
    motor_ready();
    break;
  }
}
