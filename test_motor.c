#ifndef TEST
#include <msp430g2553.h>
#endif
#include <inttypes.h>
#include <stdlib.h>
#include "motor.h"
#include "pwm.h"
#include "clock.h"
#include "protocols.h"

// communication to/from the motor
motor_in m_in;
motor_out m_out;

// internal motor control variables
static   int16_t left_power;  
static   int16_t right_power;
// timeout timer to detect broken communication with pi
static  uint32_t power_timer;

// internal encoder timing variables
// set by the interrupts
static volatile uint32_t encoder_l_t, encoder_r_t; 
static volatile uint32_t l_prev,r_prev;
// direction 1=forward, 0=stopped, -1=backward
// set by the interrupts
volatile int8_t encoder_l_dir;
volatile int8_t encoder_r_dir;



// ---------------------- ENCODER ------------------------


void encoder_init(void){
  // initialize these just in case
  m_out.l_pos = m_out.r_pos = 0;
  encoder_l_t= encoder_r_t = 0;
  l_prev= r_prev = 0;
  printf("encoder init\n");
}


static inline void motor_set_power(void);
static inline void motor_ready(void);


void
motor_init(void) {
  // initialize variables
  printf("motor init\n");
  m_out.state=MSTATE_READY;
  m_in.msg=0;
  encoder_init();
}


static inline void
motor_ready(void){
}




 
static inline void
motor_set_left(int power) {
}


static inline void
motor_set_right(int power) {
}

static inline void
motor_set_power(void){
}



void motor_step(void) {
  // ---------  Handle events and inputs ----------
  //
  switch(m_out.state){
  case MSTATE_READY:
    if(m_in.msg & MMSG_FAIL){
      printf("RDY FAIL\n");
      m_out.state=MSTATE_FAIL;
    } else if(m_in.msg & MMSG_POWER) {
      printf("RDY PWR: %d %d\n",m_in.l_pwr,m_in.r_pwr);
      set_timer(&power_timer);
      m_out.state=MSTATE_RUN;
    }
    m_in.msg=0;
    break;	  
  case MSTATE_RUN:
    // if Pi is not talking frequently enough (dead?) stop everything
    // and do not keep driving towards the cliff
    if(interval_elapsed(&power_timer,MAX_POWER_SETTING_INTERVAL)){
      m_in.msg=MMSG_TIMEOUT; //m_in.msg=MMSG_TIMEOUT;
    }
    if(m_in.msg & MMSG_FAIL) {
      printf("RUN FAIL\n");
      m_out.state=MSTATE_FAIL;
      m_in.msg=0;
    } else if(m_in.msg & MMSG_POWER) {
      set_timer(&power_timer);
      printf("RUN PWR: %d %d\n", m_in.l_pwr, m_in.r_pwr);
      m_out.state=MSTATE_RUN;
    } else if(m_in.msg & MMSG_STOP) {
      printf("RUN STP\n");
      m_out.state=MSTATE_READY;
    } else if(m_in.msg & MMSG_TIMEOUT) {
      printf("RUN TO\n");
      m_out.state=MSTATE_SLOWDOWN;
      set_timer(&power_timer);
    }
    m_in.msg=0;
    break;
  case MSTATE_SLOWDOWN:
    if(interval_elapsed(&power_timer,SLOW_DOWN_INTERVAL)){
      printf("SLOW DOWN\n");
    }
    if(m_in.msg & MMSG_FAIL) {
      printf("SLOW FAIL\n");
      m_out.state=MSTATE_FAIL;
    } else if (m_in.msg & MMSG_STOP) {
      // Clear the slowdown state
      printf("SLOW STOP\n");
      m_out.state=MSTATE_READY;
    }
    m_in.msg=0;
    break;
  case MSTATE_FAIL:
    if(m_in.msg & MMSG_CLEAR){
      printf("FAIL CLEAR\n");
      m_out.state=MSTATE_READY;
      m_in.msg=0;
    }
    motor_ready();
    break;
  }

  // ---- produce outputs
  m_out.l_speed=-50;
  m_out.r_speed= 50;
  m_out.l_pos=100;
  m_out.r_pos=-1000;

  
}


