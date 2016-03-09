
// all other communication with the motor
// happens through sending messages and reading
// the state
extern void motor_init(void);

// state of the motor state machine
extern volatile uint8_t motor_state;

// bitfield of messages sent to the motor state machine
extern volatile uint8_t motor_msg;

// power setting communicated through these
extern volatile int left_power;
extern volatile int right_power;

// Motor control ports
// left INA port 2.0
#define LINA BIT0
// left INB port 2.2
#define LINB BIT2
// left ENA port 1.0
#define LENA BIT0
// left DIAG port 1.3
#define LDIAG BIT3
// right INA port 2.3
#define RINA BIT3
// right INB port 2.5
#define RINB BIT5
// right ENA port 2.6
#define RENA BIT6
// right DIAG port 2.7
#define RDIAG BIT7

//  --------------- Motor states --------------
// Ready to go
#define MSTATE_READY    1
// Motor directions and pwm set
#define MSTATE_RUN      2
// timeout -> slowing down
#define MSTATE_SLOWDOWN 3
// motor driver signalled failure
#define MSTATE_FAIL     4

// ---------------- Letters to motor ----------
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
