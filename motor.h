#ifndef MOTOR_H
#define MOTOR_H

//  --------------- Motor states --------------
#define MSTATE_READY    1 // idle
#define MSTATE_RUN      2 // running
#define MSTATE_FAIL     4 // motor driver (or test) failure state

// ---------------- messages to motor ----------
#define MMSG_POWER      1 // pwm settings command
#define MMSG_STOP       2 // stop
#define MMSG_FAIL       4 // signal motor failure
#define MMSG_CLEAR      8 // clear motor failure
// ---------------- messages to Pi ----------
// Status report to the Pi (motor state, direction, speed, position)
#define MMSG_REPORT     49

// go to FAIL state from RUN state if not receiving power settings
// with this interval = 1/10s ( < 16cm at full speed )
#define MAX_POWER_SETTING_INTERVAL 1000000


// Input message to the motor
struct mi {
  volatile int16_t l_pwr;
  volatile int16_t r_pwr;
  volatile uint8_t msg; // message to the state machine 
}  __attribute__((__packed__));

typedef struct mi motor_in;


// output message from the motor
// note: MSP architecture requires that 16-bit values
//       can only be read from even addresses.
//       If there are odd number of uint8 before uint16
//       this would cause there be a hole and sizeof()
//       would return unexpect results.
struct mo {
  volatile int32_t l_pos;  
  volatile int32_t r_pos;  
  volatile uint32_t clock;
  volatile uint8_t msg;    
  volatile uint8_t state;  
  volatile uint8_t error;
} __attribute__((__packed__));

typedef struct mo motor_out;

extern motor_in m_in;
extern motor_out m_out;

extern volatile uint8_t mmsg;

extern void motor_init(void);
extern void motor_step(void);


#endif
