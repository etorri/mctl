#include <msp430g2553.h>
#include <inttypes.h>

// Initialize main clock

volatile uint16_t wallclock_h;

void clock_init() {
  // configure Timer A0 for wallclock op
  // 2MHz per step from 0 to 0xFFFF
  TA0CTL = TASSEL_2|MC_2|ID_3|TACLR|TAIE;
  wallclock_h = 0xFF00;
}

void __attribute__((interrupt (TIMER0_A1_VECTOR)))
ta0cc0_isr() {
  TA0CTL &= ~TAIFG;
  wallclock_h++;
}

uint32_t timediff(uint32_t time_now, uint32_t time_prev) {
  uint32_t t_d;
  if(time_now<time_prev){
    t_d = 0xFFFFFFFF - (time_prev-time_now);
  } else {
    t_d = time_now-time_prev;
  }
  return t_d;
}
