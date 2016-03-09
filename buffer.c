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
// returns 0 on success, 1 if overflow
uint8_t put_buf(volatile struct CircularBuffer *b, uint8_t *c) {
  uint8_t of;
  of=1; // assume it would overflow
  // if adding the byte would cause head==tail then this would be overflow
  if(MOD16(b->head+1)!=(b->tail)) {
    b->data[b->head] = *c;
    b->head = MOD16(b->head+1);
    of=0; // success
  }
  return of;
}

// get byte from buffer if there are any
// returns 0 on success, 1 if would cause underflow
uint8_t get_buf(volatile struct CircularBuffer *b, uint8_t *c) {
  uint8_t uf;
  uf=1;
  // if head==tail then the buffer would be empty
  if((b->tail)!=(b->head)) {
    *c= b->data[b->tail];
    b->tail = MOD16(b->tail+1);
    uf=0;
  }
  return uf;
}

