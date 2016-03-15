#include "clock.h"


// high 16 bits of the wall clock
volatile uint16_t wallclock_h;

void clock_init() {
  // configure Timer A0 for wallclock op
  // 2MHz per step from 0 to 0xFFFF
  TA0CTL = TASSEL_2|MC_2|ID_3|TACLR|TAIE;
  TA0R = 0x0000;
  wallclock_h = 0x0000;
}

// increase the high 16 bits of wall clock every 0xffff ticks of the low 16 bits
void __attribute__((interrupt (TIMER0_A1_VECTOR)))
ta0cc0_isr() {
  TA0CTL &= ~TAIFG;
  wallclock_h++;
}

