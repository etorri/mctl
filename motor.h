#ifndef MOTOR_H
#define MOTOR_H

// ---------------- Encoder definitions -------

// If the time between encoder interrupts is longer than what fits in 16-bit signed int
// it is considered "infinite". This is signalled to Pi as 0 because the interval can never
// practically be zero.
#define ENCODER_INF (0xFFFF>>1)

//  --------------- Motor states --------------
// Ready to go
#define MSTATE_READY    1
// Motor directions and pwm set
#define MSTATE_RUN      2
// timeout -> slowing down
#define MSTATE_SLOWDOWN 3
// motor driver signalled failure
#define MSTATE_FAIL     4

// ---------------- messages to motor ----------
// apply the left and right motor power settings
#define MMSG_POWER      1
// Stop right here now 
#define MMSG_STOP       2
// Signal failure (from the interrupts)
#define MMSG_FAIL       4
// Clear the failure state (from Pi)
#define MMSG_CLEAR      8
// No new power settings from Pi for long time
// so it is best to stop slowly and wait
#define MMSG_TIMEOUT   16

// read state of the motor state machine
extern volatile uint8_t motor_state;
// bitfield of messages sent to the motor state machine
extern volatile uint8_t motor_msg;
// power setting communicated through these
extern volatile int16_t left_power_in;
extern volatile int16_t right_power_in;


extern void motor_init(void);
extern void motor_machine(void);

extern void get_lr_speed(int16_t *l, int16_t *r);






#endif
