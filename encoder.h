

extern volatile uint8_t encoder_event;

// number of clock steps between encoder interrupts 
extern volatile uint32_t encoder_l_t, encoder_r_t;

// position of the encoders in 
extern volatile int32_t encoder_l_pos,encoder_r_pos;

extern volatile uint32_t l_prev,r_prev;

extern void encoder_init(void);
extern void encoder_machine(void);

#define NOT_MOVING 0xFFFF
