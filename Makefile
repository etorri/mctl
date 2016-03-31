
ifeq (TEST,$(X))
CC=gcc
CFLAGS=-Wall -DTEST
DEPS=mctl.o  test_uart.o test_clock.o buffer.o test_motor.o protocols.o
BIN=test_mctl
else
CC=msp430-gcc
CFLAGS=-O3 -finline-functions -mmcu=msp430g2553 -Wall
DEPS=mctl.o pwm.o uart.o clock.o  buffer.o motor.o interrupts.o protocols.o
BIN=mctl
endif


$(BIN): $(DEPS)
	$(CC) $(CFLAGS) -o $(BIN) $(DEPS)


clean:
	rm *.o $(BIN) *.s

prog: $(BIN)
	sudo mspdebug rf2500 --force-reset "prog $(BIN)"
