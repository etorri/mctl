

extern void motor_init(void);

// state of the motor state machine
extern uint8_t motor_state;

// bitfield of messages sent to the motor state machine
extern uint8_t motor_msg;

// power setting communicated through these
extern int left_power;
extern int right_power;

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

// Motor states
#define MSTATE_READY    1
#define MSTATE_RUN      2
#define MSTATE_WDOG     3
#define MSTATE_FAIL     4

// Motor messages
#define MMSG_POWER      1
#define MMSG_STOP       2
#define MMSG_FAIL       4
#define MMSG_CLEAR      8
#define MMSG_TIMEOUT   16
