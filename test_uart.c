#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include "clock.h"
#include "uart.h"
#include "buffer.h"
#include "motor.h"


int pty;


// loop trying to put the given byte to tx buffer until it succeeds.
// enable tx interrupt (even if it is already on) to get the tx machine going
void uart_put_uint8(uint8_t c) {
  while(write(pty,&c,1)!=1);
}

void uart_get_uint8(uint8_t *cp){
  while(read(pty,cp,1)!=1);
  printf("%2d ",*cp);
}

uint8_t uart_async_put_uint8(uint8_t c) {
  return(write(pty,&c,1)==1);
}


uint8_t uart_async_get_uint8(uint8_t *cp){
  if(read(pty,cp,1)==1){
    printf("%2d ",*cp);
    return 1;
  } else
    return 0;
}


extern char *ptsname(int);
extern int posix_openpt(int);
extern int unlockpt(int);



int ttySetRaw(int fd, struct termios *prevTermios)
{
    struct termios t;

    if (tcgetattr(fd, &t) == -1)
        return -1;

    if (prevTermios != NULL)
        *prevTermios = t;

    t.c_lflag &= ~(ICANON | ISIG | IEXTEN | ECHO);
                        /* Noncanonical mode, disable signals, extended
                           input processing, and echoing */

    t.c_iflag &= ~(BRKINT | ICRNL | IGNBRK | IGNCR | INLCR |
                      INPCK | ISTRIP | IXON | PARMRK);
                        /* Disable special handling of CR, NL, and BREAK.
                           No 8th-bit stripping or parity error handling.
                           Disable START/STOP output flow control. */

    t.c_oflag &= ~OPOST;                /* Disable all output processing */

    t.c_cc[VMIN] = 1;                   /* Character-at-a-time input */
    t.c_cc[VTIME] = 0;                  /* with blocking */

    if (tcsetattr(fd, TCSAFLUSH, &t) == -1)
        return -1;

    return 0;
}

void uart_init(void) {
  pty=posix_openpt(O_RDWR);
  fcntl(pty,F_SETFL,O_NONBLOCK);
  unlockpt(pty);
  ttySetRaw(pty,NULL);
  printf("Slave device: %s\n", ptsname(pty));
}





void uart_write(const uint8_t c) {
  uart_put_uint8(c);
}

uint8_t uart_read(void) {
  uint8_t c;
  uart_get_uint8(&c);
  return c;
}


void uart_eol(void){
  uart_print("\r\n");
}

void uart_print(const char *s) {
  while(*s) {
    uart_write(*s++);
  }
}

void uart_printx(uint8_t *b, uint8_t n) {
  static char hex_table[] = "0123456789ABCDEF";
  uint8_t c;
  b+=n-1;
  while(n--) {
    c=*(b--);
    uart_write(hex_table[(c & 0xF0) >> 4]);
    uart_write(hex_table[c & 0x0F]);
  }
}

