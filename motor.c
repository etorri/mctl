#include <msp430g2553.h>
#include <inttypes.h>
#include <stdlib.h>
#include "motor.h"
#include "pwm.h"
#include "clock.h"
#include "protocols.h"

// Motor control ports
// left INA port 2.0
#define LINA BIT0
// left INB port 2.2
#define LINB BIT2
// left DIAG port 2.6
#define LDIAG BIT6
// right INA port 2.3
#define RINA BIT3
// right INB port 2.5
#define RINB BIT5
// right DIAG port 2.7
#define RDIAG BIT7

// Encoder pins at port 1
#define LENCA BIT5
#define LENCB BIT4
#define RENCA BIT6
#define RENCB BIT7

// communication to/from the motor
motor_in m_in;
motor_out m_out;

volatile uint8_t mmsg=0;

// internal motor control variables
static   int16_t left_power;  
static   int16_t right_power;
// timeout timer to detect broken communication with pi
static  uint32_t power_timer;

// forward definitions
static inline void motor_set_power(void);
static inline void motor_ready(void);



// ----------------------------------------------------------------------
// --- initialize encoder inputs
//
void encoder_init(void){
  // clear position counters
  m_out.l_pos = m_out.r_pos = 0;
  // configure encoder pins
  P1SEL &= ~(LENCA|LENCB|RENCA|RENCB); // as gpio
  P1DIR &= ~(LENCA|LENCB|RENCA|RENCB); // as inputs
  P1IE  |= (RENCA|LENCA);              // A pins as interrupts
  P1IES &=  ~(RENCA|LENCA);            // rising edge
  // configure DIAG pins
  P2SEL &= ~(LDIAG|RDIAG);             // as gpio
  P2DIR &= ~(LDIAG|RDIAG);             // as input
  P2IE  |= (LDIAG|RDIAG);              // enable interrupt
  P2IES |= (LDIAG|RDIAG);              // falling edge
  
}

// ----------------------------------------------------------------------
// --- prepare motor driver for the READY state (stopped, breaking)
//
static inline void
motor_ready(void){
  // set pwm to 0
  left_power=right_power=0;
  motor_set_power();
  // motor in breaking state
  P2OUT &= ~(LINA|LINB|RINA|RINB);
}


// ----------------------------------------------------------------------
// --- Initialize the motor subsystem - called by mctl.c
// 
void
motor_init(void) {
  // configure INA/INB pins
  P2SEL  &= ~(LINA|LINB|RINA|RINB); // as gpio
  P2DIR  |= (LINA|LINB|RINA|RINB);  // as outputs
  // P2SEL2 &= ~(LINA|LINB|RINA|RINB); 
  P2IE   &= ~(LINA|LINB|RINA|RINB); 
  // initialize variables
  m_out.state=MSTATE_READY;
  m_in.msg=0;
  // do what is needed to enter the READY state
  motor_ready();
  encoder_init();
  // start with READY state so reporting interval is longer
  report_interval=READY_REPORT_INTERVAL;
}



// ----------------------------------------------------------------------
// --- Set the power for left motor
//
static inline void
motor_set_left(int power) {
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


// ----------------------------------------------------------------------
// --- Set the power for right motor
//
static inline void
motor_set_right(int power) {
  int8_t sgn;
  sgn= (power>=0);
  power=abs(power);
  // set direction based on the sign of the power setting
  if(sgn) {
    P2OUT = (P2OUT|RINA)&(~RINB);
  }else{
    P2OUT = (P2OUT|RINB)&(~RINA);
  }
  // call helper to set the PWM
  pwm_r_set(power);
}


// ----------------------------------------------------------------------
// --- Set the power for both motors
//


static inline void
motor_set_power(void){
  motor_set_right(right_power);
  motor_set_left(left_power);
}



// ----------------------------------------------------------------------
// --- state machine to handle messages (in mmsg variable)
//
// the mmsg is set by protocols.c:input_scanner and any kind of failure
// detection mechanism. There is a risk for timing conflicts


void motor_step(void) {
  switch(m_out.state){
  case MSTATE_READY:
    // inputs
    if(mmsg&MMSG_FAIL){
      motor_ready();
      m_out.state=MSTATE_FAIL;
      report_interval=READY_REPORT_INTERVAL;
    } else if(mmsg&MMSG_POWER) {
      left_power=m_in.l_pwr;
      right_power=m_in.r_pwr;
      motor_set_power();
      set_timer(&power_timer);
      m_out.state=MSTATE_RUN;
      report_interval=RUN_REPORT_INTERVAL;
    }
    // clear inputs
    mmsg=0;
    break;
	  
  case MSTATE_RUN:
    // if Pi is not talking frequently enough (dead?) stop everything
    // and do not keep driving towards the cliff
    if(interval_elapsed(&power_timer,MAX_POWER_SETTING_INTERVAL)){
      mmsg=MMSG_FAIL;
    }
    // inputs
    if(mmsg&MMSG_FAIL) {
      motor_ready();
      report_interval=READY_REPORT_INTERVAL;
      m_out.state=MSTATE_FAIL;
      mmsg=0;
    } else if(mmsg&MMSG_POWER) {
      set_timer(&power_timer);
      left_power=m_in.l_pwr;
      right_power=m_in.r_pwr;
      motor_set_left(left_power);
      motor_set_right(right_power);
      m_out.state=MSTATE_RUN;
    } else if(mmsg&MMSG_STOP) {
      motor_ready();
      m_out.state=MSTATE_READY;
      report_interval=READY_REPORT_INTERVAL;
    }
    // clear inputs
    mmsg=0;
    break;

  case MSTATE_FAIL:
    // inputs
    if(mmsg&MMSG_CLEAR){
      m_out.state=MSTATE_READY;
      report_interval=READY_REPORT_INTERVAL;
    }
    // clear inputs and keep steady 
    mmsg=0;
    motor_ready();
    break;
  }
}


// ----------------------------------------------------------------------
// --- Interrupt Service Routines for rotary encoders and driver failures
//


// Encoders at port 1
void __attribute__((interrupt (PORT1_VECTOR))) p1isr() {
  // Left encoder interrupt
  if(P1IFG & LENCA) {
    // If L-ENC-B also up then going forward
    if(P1IN & LENCB){
      m_out.l_pos++;
    }else{
      m_out.l_pos--;
    }
    // Clear LENCA interrupt
    P1IFG &= ~LENCA;
  }
  // Right encoder interrupt
  if(P1IFG & RENCA) {
    // In right motor R-ENC-A and R-ENC-B means backwards
    if(P1IN & RENCB){
      m_out.r_pos--;
    }else{
      m_out.r_pos++;
    }
    // Clear RENCA interrupt
    P1IFG &= ~RENCA;
  }
}

// Motor driver diagnostic inputs at port 2
void __attribute__((interrupt (PORT2_VECTOR))) p2isr() {
  // RDIAG went down = Right motor fail
  if(P2IFG & RDIAG){
    P2IFG &= ~RDIAG;
    mmsg|= MMSG_FAIL;
  }
  // L-DIAG (2.6) went down = Left motor fail
  if(P2IFG & LDIAG){
    P2IFG &= ~LDIAG;
    mmsg|= MMSG_FAIL;
  }
}
