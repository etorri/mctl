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
#define LDIAG BIT3
// right INA port 2.3
#define RINA BIT3
// right INB port 2.5
#define RINB BIT5
// right DIAG port 2.7
#define RDIAG BIT7

// -- not used
// right ENA port 2.6
// #define RENA BIT6
// left ENA port 1.0
// #define LENA BIT0



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
  // DIAG pins interrupt when going high to low
  P2IE |= (BIT6|BIT7); // LDIAG,RDIAG
  P2IES|= (BIT6|BIT7);
  
}

// max 1 second without power setting commands from Pi
// in RUN state
//#define MAX_POWER_SETTING_INTERVAL 8000000

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
  // start with READY state so reporting interval is longer
  report_interval=READY_REPORT_INTERVAL;
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


// ------- Motor controller state machine -------
//
void motor_step(void) {
  switch(m_out.state){
  case MSTATE_READY:
    // inputs
    if(mmsg&MMSG_FAIL){
      motor_ready();
      m_out.state=MSTATE_FAIL;
      report_interval=RUN_REPORT_INTERVAL;
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

/* Note:
   TODO: The mmsg should be handled as a bit field

   For MMSG_FAIL it should be cleared (mmsg=0) after changing the state to FAIL
   to ignore any other messages

   For others just clear that single flag i.e.  mmsg&= ~MMSG_STOP

   Currently the MMSG_FAIL can be masked by power setting for example
*/


// ------------------ Interrupt Service Routines --------------
//
// Port 1 = Left Encoder A&B, Right Encoder A&B
//
void __attribute__((interrupt (PORT1_VECTOR))) p1isr() {
  // Left motor L-ENC-A (1.4) active
  if(P1IFG&BIT4) {
    // If L-ENC-B also up then going forward
    if(P1IN&BIT5){
      m_out.l_pos++;
    }else{
      m_out.l_pos--;
    }
    // Clear LENCA interrupt
    P1IFG &= ~BIT4;
  }
  // R-ENC-A (1.7) active
  if(P1IFG&BIT7) {
    // In right motor R-ENC-A and R-ENC-B means backwards
    if(P1IN&BIT6){
      m_out.r_pos--;
    }else{
      m_out.r_pos++;
    }
    // Clear RENCA interrupt
    P1IFG &= ~BIT7;
  }
}

// Port 2:  R-DIAG @2.7, L-DIAG @2.6
void __attribute__((interrupt (PORT2_VECTOR))) p2isr() {
  // RDIAG went down = Right motor fail
  if(P2IFG & RDIAG){
    mmsg|= MMSG_FAIL;
  }
  // L-DIAG (2.6) went down = Left motor fail
  if(P2IFG & LDIAG){
    mmsg|= MMSG_FAIL;
  }
}
