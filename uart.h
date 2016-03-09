#include <msp430g2553.h>
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


extern void uart_init(void);
extern void uart_write(uint8_t c);
extern uint8_t uart_read(void);
extern void uart_print(const char *s);
extern void uart_printx(uint8_t *b, uint8_t n);
extern void uart_eol(void);
