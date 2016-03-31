#ifndef PROTOCOLS_H
#define PROTOCOLS_H

#ifndef TEST
#include <msp430g2553.h>
#else
#include <stdio.h>
#endif
#include <inttypes.h>
#include "buffer.h"
#include "motor.h"
#include "clock.h"
#include "uart.h"


#ifdef TEST
#define REPORT_INTERVAL 10000
#else
#define REPORT_INTERVAL 20000
#endif

/*
// ---------------- messages to controller ----------
// PWM settings for left and right motors (2 * int16_t)
#define MMSG_POWER      50
// Ask motor to go to read state (from run or timeout)
#define MMSG_STOP       51
// (test - simulate motor transition to FAIL state)
#define MMSG_FAIL       52
// Ask motor to move from FAIL state to READY state
#define MMSG_CLEAR      53
// (test - simulate timeout in RUN state)
#define MMSG_TIMEOUT    54
*/
// ---------------- messages to Pi ----------
// Status report to the Pi (motor state, direction, speed, position)
#define MMSG_REPORT     49


// packet start stop values
#define C_START  0xFF
#define C_END    0x00

// Maximum length for input message
#define MAXLEN   5


extern void protocols_init(void);
extern void input_scanner(void);
extern void reporter(void);

#endif
