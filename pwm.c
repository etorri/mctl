#include <msp430g2553.h>
#include <inttypes.h>
#include "pwm.h"

// cap the pwm setting to PWM_MAX

#define PWM_CAP(s) (PWM_MAX<(s)?PWM_MAX:(s))


// Timer A1
// TACCR1 = Pin 9  left P2.1, TA1.1
// TACCR2 = Pin 12 right P2.4, TA1.2
void pwm_init(void) {
  // set port
  P2DIR |= BIT1|BIT4;      // left, right pwm
  P2SEL |= BIT1|BIT4;      // P2.1 set to TA1.1, P2.4 set to TA1.2 
  TA1CCR0 = PWM_PERIOD-1;  // PWM period length
  TA1CCTL1 = OUTMOD_7;     // CCR1 mode = reset/set
  TA1CCTL2 = OUTMOD_7;     //
  TA1CCR1 = PWM_MIN;       // CCR1 PWM left duty cycle length
  TA1CCR2 = PWM_MIN;       // CCR2 PWM right
  TA1CTL = TASSEL_2 + MC_1;// SMCLK, up mode
}

void pwm_lr_set(uint16_t l, uint16_t r) {
  TA1CCR1=PWM_CAP(l);
  TA1CCR2=PWM_CAP(r);
}

void pwm_test_set(uint16_t d, uint16_t l) {
  TA1CCR1=d;
  TA1CCR2=d;
  TA1CCR0=l;
}
