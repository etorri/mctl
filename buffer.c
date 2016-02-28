#include <msp430g2553.h>
#include <inttypes.h>
#include "buffer.h"



void init_buf(volatile struct CircularBuffer *b){
  b->head=0;
  b->tail=0;
  b->len=0;
}

uint8_t put_buf(volatile struct CircularBuffer *b, uint8_t *c) {
  uint8_t res=1;
  if(b->len <= BUFLEN) {
    b->data[b->head] = *c;
    b->head = MOD16(b->head+1);
    _BIC_SR(GIE);
    b->len++;
    _BIS_SR(GIE);
    res=0;
  }
  return res;
}

uint8_t get_buf(volatile struct CircularBuffer *b, uint8_t *c) {
  uint8_t res=1;
  if(b->len > 0) {
    *c= b->data[b->tail];
    b->tail = MOD16(b->tail+1);
    _BIC_SR(GIE);
    b->len--;
    _BIS_SR(GIE);
    res=0;
  }
  return res;
}

