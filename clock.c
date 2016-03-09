#include <msp430g2553.h>
#include <inttypes.h>
#include "clock.h"
// Initialize main clock

volatile uint16_t wallclock_h;

void clock_init() {
  // configure Timer A0 for wallclock op
  // 2MHz per step from 0 to 0xFFFF
  TA0CTL = TASSEL_2|MC_2|ID_3|TACLR|TAIE;
  wallclock_h = 0x0000;
}

void __attribute__((interrupt (TIMER0_A1_VECTOR)))
ta0cc0_isr() {
  TA0CTL &= ~TAIFG;
  wallclock_h++;
}

inline uint32_t timediff(const uint32_t time_now, const uint32_t time_prev){
  uint32_t t_d;
  if(time_now<time_prev){
    t_d = 0xFFFFFFFF - (time_prev-time_now);
  } else {
    t_d = time_now-time_prev;
  }
  return t_d;
}


inline void set_timer(uint32_t *tmr){
  *tmr=WALLTIME;
}

inline uint8_t interval_elapsed(uint32_t *tmr, const uint32_t interval){
  uint32_t now;
  now=WALLTIME;
  if(timediff(now,*tmr) >= interval){
    *tmr += interval;
    return 1;
  }
  return 0;
}
