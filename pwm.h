#ifndef TEST
#include <msp430g2553.h>
#endif
#include <inttypes.h>


// Notes to self
#define PWMCLOCK   16000000 // 16MHz
#define PWM_PERIOD 1600     // 10KHz
#define PWM_MIN    0
#define PWM_MAX    1600


extern void pwm_init(void);
extern void pwm_lr_set(uint16_t l, uint16_t r);
extern void pwm_l_set(uint16_t pwr);
extern void pwm_r_set(uint16_t pwr);

extern void pwm_test_set(uint16_t d, uint16_t l);

