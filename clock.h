
// read/compose 32-bit clock time
// perhaps should be protected by ~GIE
//
#define WALLTIME ((((uint32_t)wallclock_h)<<16)+TA0R)

extern void clock_init(void);
extern volatile uint16_t wallclock_h;
extern uint32_t timediff(uint32_t time_now, uint32_t time_prev);



