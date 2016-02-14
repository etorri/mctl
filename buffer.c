#include <msp430g2553.h>
#include <inttypes.h>
#include "buffer.h"



void init_buf(struct CircularBuffer *b){
  b->head=0;
  b->tail=0;
  b->len=0;
}

uint8_t put_buf(struct CircularBuffer *b, uint8_t *c) {
  if(b->len <= BUFLEN) {
    b->data[b->head] = *c;
    b->head = MOD16(b->head+1);
    b->len++;
    return 0;
  } else {
    // overflow
    return 1;
  }
}

uint8_t get_buf(struct CircularBuffer *b, uint8_t *c) {
  if(b->len > 0) {
    *c= b->data[b->tail];
    b->tail = MOD16(b->tail+1);
    b->len--;
    return 0;
  } else {
    // underflow
    return 1;
  }
}

