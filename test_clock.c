#include "clock.h"
#include <time.h>
#include <sys/time.h>
#include <unistd.h>


uint32_t
wall_time(void){
  struct timeval tv;
  uint32_t t;
  gettimeofday(&tv,NULL);
  t=tv.tv_sec*1000000.0+tv.tv_usec;
  return t;
}


void clock_init() {

}

  
