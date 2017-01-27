#include <msp430g2553.h>
#include <inttypes.h>
#include "buffer.h"


/* Yet another circular buffer implementation
   "This is my buffer implementation. There are many like it, but this one is mine."
*/

void init_buf(volatile struct CircularBuffer *b){
  // initially head and tail point to same position = buffer is empty
  b->head=0;
  b->tail=0;
}

// put byte to buffer unless it would overflow
// 1=ok, 0=overflow
uint8_t put_buf(volatile struct CircularBuffer *b, uint8_t *c) {
  uint8_t ok;
  ok=0; // assume it would overflow
  // (head+1)==tail  is overflow
  if(MOD16(b->head+1)!=(b->tail)) {
    b->data[b->head] = *c;
    b->head = MOD16(b->head+1);
    ok=1; // success
  }
  return ok;
}

// get byte from buffer if there are any
// 1=ok, 0=underflow
uint8_t get_buf(volatile struct CircularBuffer *b, uint8_t *c) {
  uint8_t ok;
  ok=0;
  // if head==tail then the buffer is empty
  if((b->tail)!=(b->head)) {
    *c= b->data[b->tail];
    b->tail = MOD16(b->tail+1);
    ok=1;
  }
  return ok;
}

