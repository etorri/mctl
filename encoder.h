
// the ISR sets these when there is a new
// encoder interrupt. The purpose is to signal
// that there is a new encoder_x_t reading.
#define ENCODER_LSTEP 0b00000001
#define ENCODER_RSTEP 0b00000010

extern volatile uint8_t encoder_event;

// number of clock steps between encoder interrupts 
extern volatile uint32_t encoder_l_t, encoder_r_t;

// position of the encoders in 
extern volatile int32_t encoder_l_pos,encoder_r_pos;


extern void encoder_init(void);
