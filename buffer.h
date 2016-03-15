#define BUFLEN 16

#define MOD16(n) ((n)&0x0f)
		       
struct CircularBuffer {
  volatile uint8_t data[BUFLEN];
  volatile uint8_t head; // write index
  volatile uint8_t tail; // read index
  volatile uint8_t len;  // for quick over/underflow evaluation
};

extern void init_buf(volatile struct CircularBuffer *b);

// get/put return 0 if ok, 1 if under/overflow
extern inline uint8_t put_buf(volatile struct CircularBuffer *b, uint8_t *c) {
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

extern inline uint8_t get_buf(volatile struct CircularBuffer *b, uint8_t *c) {
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

