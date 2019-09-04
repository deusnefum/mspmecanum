MCU=msp430g2553

CFLAGS=-mmcu=${MCU} -std=c99 -pedantic -Wall -O1

.PHONY: all lint prog

all: skateboard.bin

skateboard.bin: main.o trig.o
	msp430-elf-gcc ${CFLAGS} $^ -o skateboard.bin -lm

%.o: %.c
	msp430-elf-gcc ${CFLAGS} -c $<

lint:
	msp430-elf-gcc ${CFLAGS} -fsyntax-only *.c

clean:
	rm -f *.bin *.o

prog: skateboard.bin
	mspdebug rf2500 'prog skateboard.bin'