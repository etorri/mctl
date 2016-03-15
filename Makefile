
CC=msp430-gcc
CFLAGS=-O3 -finline-functions -mmcu=msp430g2553 -Wall
#CFLAGS=-mmcu=msp430g2553 -Wall


DEPS=mctl.o pwm.o uart.o clock.o  buffer.o motor.o interrupts.o protocols.o
BIN=mctl

mctl: $(DEPS)
	$(CC) $(CFLAGS) -o $(BIN) $(DEPS)

clean:
	rm $(DEPS) $(BIN) *.s

prog: $(BIN)
	sudo mspdebug rf2500 --force-reset "prog $(BIN)"
