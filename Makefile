MCU=msp430g2553

CFLAGS=-mmcu=${MCU} -std=c99 -pedantic -Wall -Os

.PHONY: all lint prog

all: skateboard.bin

skateboard.bin: *.o
	msp430-elf-gcc ${CFLAGS} $< -o skateboard.bin -lm

%.o: %.c
	msp430-elf-gcc ${CFLAGS} -c $<

lint:
	msp430-elf-gcc -fsyntax-only -std=c99 -mmcu=${MCU} -std=c99 -pedantic -Wall -g -O2 osk2.c -o osk.bin

clean:
	rm -f *.bin *.o

prog:
	mspdebug rf2500 'prog osk.bin'