#ifndef CLOCK_H
#define CLOCK_H
#include <inttypes.h>
#include <msp430g2553.h>

// the high 16 bits of the wall clock
extern volatile uint16_t wallclock_h;

extern void clock_init(void);

// utility function to calculate the difference of two uint32_t
// which can warp over
static inline uint32_t
timediff(const uint32_t time_now, const uint32_t time_prev){
  uint32_t t_d;
  if(time_now<time_prev){ // clock wrapped over
    t_d = (0xFFFF-time_prev)+time_now;
  } else {
    t_d = time_now-time_prev;
  }
  return t_d;
}

// get current time in ticks from start
static inline uint32_t
wall_time(void){
  return ((uint32_t)wallclock_h<<16) + TA0R;
}

static inline
void set_timer(uint32_t *tmr){
  *tmr=wall_time();
}

static inline
uint8_t interval_elapsed(uint32_t *tmr, const uint32_t interval){
  uint32_t now;
  now=wall_time();
  if(timediff(now,*tmr) >= interval){
    *tmr += interval;
    return 1;
  }
  return 0;
}


#endif
