#include "protocols.h"
#include "motor.h"
#include <msp430g2553.h>



// ********************************************************************************************
// States of the 

#define S_START        1 // Scan for the start character
#define S_DATA         2 // read data




// state machine function to handle next input character in case there is one
void input_scanner(void){
     uint8_t c;
     static uint8_t scanner_state=S_START;
     static uint8_t *dp;
     static uint8_t i;
     static uint8_t data_esc=0;
     static uint8_t f_err=0;
     
     if(uart_async_get_uint8(&c)){
       switch(scanner_state){
       case S_START:
	 // expecting this first byte to be C_FLAG, anything else tells about framing error
	 //
	 if(data_esc) { // just skip escaped char in START state
	   data_esc=0;
	   
	 } else if(c==C_ESC) {
	   // escaped state here is error, flag that (also escape the next byte)
	   data_esc=1;
	   f_err=1;
	   
	 } else if(c==C_FLAG){ // start of frame move to DATA state
	   i=0; // we have received 0 bytes at start
	   dp=(uint8_t *) &m_in; // received data goes to m_in struct
	   if(f_err) {
	     // increase error counter if we got anything but C_FLAG
	     m_out.framing_errors++;
	   }
	   f_err=0; // for next time
	   scanner_state=S_DATA; // only way to get to next state
	   
	 } else {
	   // whatever not C_FLAG byte is wrong here
	   f_err=1;
	 }
	 break;

       case S_DATA:  
	 if(data_esc){
	   // this char is escaped
	   dp[i++]= c ^ C_XOR;
	   if(i == sizeof(m_in)){
	     // enough bytes read, ship it forward
	     mmsg=m_in.msg; // tell the motor we got message
	     i=0;
	     scanner_state=S_START;
	   }
	   data_esc=0;
	   
	 } else if(c==C_ESC){
	   // next char is escaped
	   data_esc=1;

	 } else {
	   // ordinary data byte, add it in
	   dp[i++] = c;
	   if(i == sizeof(m_in)){
	     // enough bytes read, ship it forward
	     mmsg=m_in.msg; // tell the motor we got message
	     i=0;
	     scanner_state=S_START;
	   }
	 };
	 break;

       } // switch scanner_state
     } // if input byte
}
		    

// ---------------- messages to Pi ----------
// Status report to the Pi (motor state, position)
#define MMSG_REPORT     49
       
// timer for sending status reports with the given interval
static uint32_t report_timer=0;

// this is the reporting interval set by motor.c depending on the state there
uint32_t report_interval;



// Send n bytes starting at lw pointer to the uart as one data frame
//
static inline void send_frame(uint8_t *lw, uint8_t n)
{
  uint8_t c;
  
  uart_put_uint8(C_FLAG);
  while(n--){
    c= *(lw++);
    if((c==C_FLAG)|(c==C_ESC)) {
      // escape frame flag and esc bytes in data
      uart_put_uint8(C_ESC);
      uart_put_uint8(c ^ C_XOR);
    } else
      uart_put_uint8(c);
  }
}

// called from main to send report when it is the time to do so
void reporter(void){
  if(interval_elapsed(&report_timer,report_interval)){
    send_frame((uint8_t *)&m_out,sizeof(motor_out));
  }	  
}

void protocols_init(void){
  m_out.msg=MMSG_REPORT;
}
