#ifndef TEST
#include <msp430g2553.h>
#endif
#include <inttypes.h>

// Select P1.1 and P1.2 for UART
#define RXD    BIT1
#define TXD    BIT2
#define BPS    9600
#define DBITS  8
#define PARITY 1

#define UCBRF(X) ((X)<<4)
#define UCBRS(X) ((X)<<1)

extern volatile struct CircularBuffer txbuf;
extern volatile struct CircularBuffer rxbuf;

// blocking version
extern void uart_get_uint8(uint8_t *cp);
extern void uart_put_uint8(uint8_t c);

// non-blocking version, returns 0 when data sent/received otherwise 1
extern uint8_t uart_async_get_uint8(uint8_t *cp);
extern uint8_t uart_async_put_uint8(uint8_t c);

// initialize the buffers, pins and interrupts
extern void uart_init(void);



