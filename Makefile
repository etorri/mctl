
CC=msp430-gcc
CFLAGS=-O3  -finline-functions -mmcu=msp430g2553 -Wall
DEPS=mctl.o pwm.o uart.o clock.o  buffer.o motor.o protocols.o
BIN=mctl


$(BIN): $(DEPS)
	$(CC) $(CFLAGS) -o $(BIN) $(DEPS)


clean:
	rm *.o $(BIN) 

prog: $(BIN)
	sudo mspdebug rf2500 --force-reset "prog $(BIN)"
