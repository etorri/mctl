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
// left ENA port 1.0
#define LENA BIT0
// left DIAG port 1.3
#define LDIAG BIT3
// right INA port 2.3
#define RINA BIT3
// right INB port 2.5
#define RINB BIT5
// right ENA port 2.6
#define RENA BIT6
// right DIAG port 2.7
#define RDIAG BIT7



// communication to/from the motor
motor_in m_in;
motor_out m_out;

volatile uint8_t mmsg=0;

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
//
void encoder_init(void){
  // initialize these just in case
  m_out.l_pos = m_out.r_pos = 0;
  encoder_l_t= encoder_r_t = 0;
  // initialize the previous time at 0
  l_prev= r_prev = 0;
  // encoder pins as inputs
  P1DIR &= ~(BIT4|BIT5|BIT6|BIT7);
  // enable ENC-A pins as interrupts (low->high)
  P1IE |= BIT4|BIT7;
}




// max 1 second without power setting commands from Pi
// in RUN state
#define MAX_POWER_SETTING_INTERVAL 8000000
// in Slow
#define SLOW_DOWN_INTERVAL         20000


static inline void motor_set_power(void);
static inline void motor_ready(void);


void
motor_init(void) {
  // Set INA and INB as normal IO pin outputs
  P2DIR  |= LINA|LINB|RINA|RINB;
  P2SEL  &= ~(LINA|LINB|RINA|RINB);
  P2SEL2 &= ~(LINA|LINB|RINA|RINB);
  // initialize variables
  m_out.state=MSTATE_READY;
  m_in.msg=0;
  // do what is needed to enter the READY state
  motor_ready();
  encoder_init();
}


static inline void
motor_ready(void){
  // set pwm to 0
  left_power=right_power=0;
  motor_set_power();
  // motor in breaking state
  P2OUT &= ~(LINA|LINB|RINA|RINB);
}




 
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


static inline void
motor_set_right(int power) {
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

static inline void
motor_set_power(void){
  motor_set_right(right_power);
  motor_set_left(left_power);
}



void motor_step(void) {
  uint32_t now;
  now=wall_time();
  
  // ---------- Handle input events
  switch(m_out.state){
  case MSTATE_READY:
    if(mmsg&MMSG_FAIL){
      motor_ready();
      m_out.state=MSTATE_FAIL;
    } else if(mmsg&MMSG_POWER) {
      left_power=m_in.l_pwr;
      right_power=m_in.r_pwr;
      motor_set_power();
      set_timer(&power_timer);
      m_out.state=MSTATE_RUN;
    }
    mmsg=0;
    break;
	  
  case MSTATE_RUN:
    // if Pi is not talking frequently enough (dead?) stop everything
    // and do not keep driving towards the cliff
    if(interval_elapsed(&power_timer,MAX_POWER_SETTING_INTERVAL)){
      mmsg=MMSG_TIMEOUT; //mmsg=MMSG_TIMEOUT;
    }
    if(mmsg&MMSG_FAIL) {
      motor_ready();
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
    } else if(mmsg&MMSG_TIMEOUT) {
      m_out.state=MSTATE_SLOWDOWN;
      set_timer(&power_timer);
    }
    mmsg=0;
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
    if(mmsg&MMSG_FAIL) {
      motor_ready();
      m_out.state=MSTATE_FAIL;
    } else if (mmsg&MMSG_STOP) {
      // Clear the slowdown state
      motor_ready();
      m_out.state=MSTATE_READY;
    }
    mmsg=0;
    break;

  case MSTATE_FAIL:
    if(mmsg&MMSG_CLEAR){
      m_out.state=MSTATE_READY;
      mmsg=0;
    }
    motor_ready();
    break;
  }

  // ------------- Prepare outputs
  if((encoder_l_t>ENCODER_INF) || (timediff(now,l_prev)>ENCODER_INF))
    m_out.l_speed=0;
  else
    m_out.l_speed= encoder_l_t*encoder_l_dir;

  if((encoder_r_t>ENCODER_INF) || (timediff(now,r_prev)>ENCODER_INF))
    m_out.r_speed=0;
  else
    m_out.r_speed= encoder_r_t*encoder_r_dir;
  
}


// ------------------ Interrupt Service Routines --------------
//


void __attribute__((interrupt (PORT1_VECTOR))) p1isr() {
  static uint32_t time_now;
  time_now=wall_time();

  // Left motor LENCA went up
  if(P1IFG&BIT4) {
    // LENCA Left encoder A input
    encoder_l_t=timediff(time_now,l_prev);
    l_prev=time_now;
    // If LENCB also up then going forward
    if(P1IN&BIT5){
      encoder_l_dir=1;
      m_out.l_pos++;
    }else{
      encoder_l_dir=-1;
      m_out.l_pos--;
    }
    // Clear LENCA interrupt
    P1IFG &= ~BIT4;
  }
  
  // Right motor encoders
  if(P1IFG&BIT7) {
    // Right motor RENCA went up
    encoder_r_t=timediff(time_now,r_prev);
    r_prev=time_now;
    // In right motor RENCA&RENCB means backwards
    if(P1IN&BIT6){
      encoder_r_dir=-1;
      m_out.r_pos--;
    }else{
      encoder_r_dir=1;
      m_out.r_pos++;
    }
    // Clear RENCA interrupt
    P1IFG &= ~BIT7;
  }  
  if(P1IFG&BIT3){
    // Motor driver failure = LDIAG went down
    // the interrupt will be cleared only after CLEAR message
    mmsg|= MMSG_FAIL;
  }
}


void __attribute__((interrupt (PORT2_VECTOR))) p2isr() {
  // Motor driver failure = RDIAG went down
  if(P2IFG&BIT7){
    mmsg|= MMSG_FAIL;
  }
}
