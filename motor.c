#include <msp430g2553.h>
#include <inttypes.h>
#include <stdlib.h>
#include "motor.h"
#include "pwm.h"
#include "encoder.h"
// Motor states
#define MOTOR_BRAKE 0
#define MOTOR_RUN   1
#define MOTOR_FAULT 2

static uint8_t l_prev_sgn;
static uint8_t r_prev_sgn;


volatile uint8_t motor_state;

// left INA port 2.0
#define LINA BIT2
// left INB port 2.2
#define LINB BIT0
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

void motor_init(void) {
  // motor uses pwm
  pwm_init();
  // initial motor state RUN both motors forward pwm 0
  l_prev_sgn=0;      // set fake prev direction backwards
  motor_set_left(0); // so this initializes the INA/B
  // same for right side
  r_prev_sgn=0;
  motor_set_right(0);
  // Initial sate is RUN
  motor_state=MOTOR_RUN;
}

void motor_set_handbrake(void){
  P2OUT= P2OUT & ~(LINA|LINB|RINA|RINB);
  pwm_lr_set(0,0);
  motor_state=MOTOR_BRAKE;
}

void motor_release_handbrake(void) {
  // after handbrake release return to initial state
  motor_init();
}


void motor_set_left(int power) {
  int8_t sgn= (power>=0);
  power=abs(power);
  // change of direction?
  switch((sgn<<1) + l_prev_sgn){
  case 1:
    // previously CW, now CCW
    P2OUT = (P2OUT&~LINB)|LINA;
    break;
  case 2:
    // previously CCW now CW
    P2OUT = (P2OUT&~LINA)|LINB;
    break;
  default:
    // no change, change nothing
    break;
  }
  pwm_l_set(power);
  l_prev_sgn=sgn;
}


void motor_set_right(int power) {
  int8_t sgn= (power>=0);
  power=abs(power);
  // change of direction?
  switch((sgn<<1) + r_prev_sgn){
  case 1:
    // previously CW, now CCW
    P2OUT = (P2OUT&~RINA)|RINB;
    break;
  case 2:
    // previously CCW now CW
    P2OUT = (P2OUT&~RINB)|RINA;
    break;
  default:
    // no change, change nothing
    break;
  }
  pwm_r_set(power);
  r_prev_sgn=sgn;
}
    


