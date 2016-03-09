
#define OUT_NOTHING 0
#define OUT_S1      1
#define OUT_S2      2
#define OUT_S3      3
#define OUT_S4      4 

#define IN_P       'p'
#define IN_S       's'
#define IN_F       'f'
#define IN_C       'c'
#define IN_T       't'

extern void protocol_init(void);
extern void output_handler(void);
extern void input_handler(void);

extern volatile uint8_t outmsg;
