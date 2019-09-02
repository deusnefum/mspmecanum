MCU=msp430g2553

CFLAGS=-mmcu=${MCU} -std=c99 -pedantic -Wall -O2
LDFLAGS=-Llibfixmath/libfixmath -lfixmath

.PHONY: all lint prog

all: skateboard.bin

skateboard.bin: main.o
	msp430-elf-gcc ${CFLAGS} $^ -o skateboard.bin ${LDFLAGS}

%.o: %.c
	msp430-elf-gcc ${CFLAGS} -c $<

lint:
	msp430-elf-gcc ${CFLAGS} -fsyntax-only *.c

clean:
	rm -f *.bin *.o

prog: skateboard.bin
	mspdebug rf2500 'prog skateboard.bin'