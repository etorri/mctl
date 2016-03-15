#ifndef PROTOCOLS_H
#define PROTOCOLS_H

#include <msp430g2553.h>
#include <inttypes.h>
#include "buffer.h"
#include "motor.h"
#include "clock.h"
#include "uart.h"

#define REPORT_INTERVAL 4000000

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


extern void protocol_init(void);
extern uint8_t input_scanner(void);
extern void reporter(void);

#endif
