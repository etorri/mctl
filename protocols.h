#ifndef PROTOCOLS_H
#define PROTOCOLS_H

#include <msp430g2553.h>
#include <inttypes.h>
#include "buffer.h"
#include "motor.h"
#include "clock.h"
#include "uart.h"

// Reporting interval when in RUN or FAIL states = 1/100 2
#define RUN_REPORT_INTERVAL   200000
// when idling = 1s
#define READY_REPORT_INTERVAL   2000000

extern uint32_t report_interval;

// Frame flags
#define C_FLAG  0x7E
#define C_ESC   0x7F
#define C_XOR   0x20

// Maximum length for input message
// change if 
//#define MAXLEN   5

extern void protocols_init(void);
extern void input_scanner(void);
extern void reporter(void);

#endif
