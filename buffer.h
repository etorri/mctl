#define BUFLEN 16

#define MOD16(n) ((n)&0x0f)
		       
struct CircularBuffer {
  uint8_t data[BUFLEN];
  volatile uint8_t head; // write index
  volatile uint8_t tail; // read index
  volatile uint8_t len;  // for quick over/underflow evaluation
};

extern void init_buf(struct CircularBuffer *b);

// get/put return 0 if ok, 1 if under/overflow
extern uint8_t put_buf(struct CircularBuffer *b, uint8_t *c);
extern uint8_t get_buf(struct CircularBuffer *b, uint8_t *c);
