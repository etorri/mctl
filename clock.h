#ifndef CLOCK_H
#define CLOCK_H

#ifndef TEST
#include <msp430g2553.h>
#endif
#include <inttypes.h>

// the high 16 bits of the wall clock
extern volatile uint16_t wallclock_h;

extern void clock_init(void);

// utility function to calculate the difference of clock values
// in ticks (1/2_000_000 s)
static inline uint32_t
timediff(const uint32_t time_now, const uint32_t time_prev){
  uint32_t t_d;
  if(time_now<time_prev){ // clock wrapped over
#ifdef TEST
    t_d = (1000000-time_prev)+time_now;
#else    
    t_d = (0xFFFF-time_prev)+time_now;
#endif
  } else {
    t_d = time_now-time_prev;
  }
  return t_d;
}


#ifdef TEST
extern uint32_t wall_time(void);
#else
// get current time in ticks from start
static inline uint32_t
wall_time(void){
  return ((uint32_t)wallclock_h<<16) + TA0R;
}
#endif


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
