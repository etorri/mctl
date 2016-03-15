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


// motor control variables
volatile uint8_t motor_state;
volatile uint8_t motor_msg;
volatile int16_t left_power_in;
static   int16_t left_power;  
volatile int16_t right_power_in;
static   int16_t right_power;
uint32_t power_timer;

// encoder position visible outside
volatile int32_t encoder_l_pos,encoder_r_pos;

// encoder control variables
static volatile uint32_t encoder_l_t, encoder_r_t; 
static volatile uint32_t l_prev,r_prev;
// direction 1=forward, 0=stopped, -1=backward
volatile int8_t encoder_l_dir;
volatile int8_t encoder_r_dir;



// ---------------------- ENCODER ------------------------
// Encoder interrupts
// left P1.3
// right P1.7
//
void encoder_init(void){
  // initialize these just in case
  encoder_l_pos = encoder_r_pos = 0;
  encoder_l_t= encoder_r_t = 0;
  // no interrupts yet
  // initialize the previous time at 0
  l_prev= r_prev = 0;
  P1DIR &= ~(BIT4|BIT5|BIT6|BIT7);
  // enable encoder input pin interrupts
  P1IE |= BIT4|BIT7;
}




void get_lr_speed(int16_t *l, int16_t *r){
  uint32_t now;
  now=wall_time();
  if((encoder_l_t>ENCODER_INF) || (timediff(now,l_prev)>ENCODER_INF))
    *l=0;
  else
    *l=encoder_l_t;

  if((encoder_r_t>ENCODER_INF) || (timediff(now,r_prev)>ENCODER_INF))
    *r=0;
  else
    *r=encoder_r_t;
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
  motor_state=MSTATE_READY;
  motor_msg=0;
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



void motor_machine(void) {
  switch(motor_state){
  case MSTATE_READY:
    if(motor_msg&MMSG_FAIL){
      motor_ready();
      motor_state=MSTATE_FAIL;
    } else if(motor_msg&MMSG_POWER) {
      left_power=left_power_in;
      right_power=right_power_in;
      motor_set_power();
      set_timer(&power_timer);
      motor_state=MSTATE_RUN;
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
    } else if(motor_msg&MMSG_POWER) {
      set_timer(&power_timer);
      left_power=left_power_in;
      right_power=right_power_in;
      motor_set_left(left_power);
      motor_set_right(right_power);
      motor_state=MSTATE_RUN;
    } else if(motor_msg&MMSG_STOP) {
      motor_ready();
      motor_state=MSTATE_READY;
    } else if(motor_msg&MMSG_TIMEOUT) {
      motor_state=MSTATE_SLOWDOWN;
      set_timer(&power_timer);
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
    } else if (motor_msg&MMSG_STOP) {
      // Clear the slowdown state
      motor_ready();
      motor_state=MSTATE_READY;
    }
    motor_msg=0;
    break;

  case MSTATE_FAIL:
    if(motor_msg&MMSG_CLEAR){
      motor_state=MSTATE_READY;
      motor_msg=0;
    }
    motor_ready();
    break;
  }
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
      encoder_l_pos++;
    }else{
      encoder_l_dir=-1;
      encoder_l_pos--;
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
      encoder_r_pos--;
    }else{
      encoder_r_dir=1;
      encoder_r_pos++;
    }
    // Clear RENCA interrupt
    P1IFG &= ~BIT7;
  }  
  if(P1IFG&BIT3){
    // Motor driver failure = LDIAG went down
    // the interrupt will be cleared only after CLEAR message
    motor_msg|= MMSG_FAIL;
  }
}


void __attribute__((interrupt (PORT2_VECTOR))) p2isr() {
  // Motor driver failure = RDIAG went down
  if(P2IFG&BIT7){
    motor_msg|=MMSG_FAIL;
  }
}
