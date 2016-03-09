
// read 32-bit wall clock time

#define WALLTIME ((((uint32_t)wallclock_h)<<16)+TA0R)


// the high 16 bits of the wall clock
extern volatile uint16_t wallclock_h;

extern void clock_init(void);

// utility function to calculate the difference of clock values
// in ticks (1/2_000_000 s)
extern inline uint32_t timediff(const uint32_t time_now, const uint32_t time_prev);
extern inline void set_timer(uint32_t *tmr);
extern inline uint8_t interval_elapsed(uint32_t *tmr, const uint32_t interval);
