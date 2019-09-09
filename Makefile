MCU=msp430g2553

CFLAGS=-mmcu=${MCU} -std=c99 -pedantic -Wall -O2 -Ilibfixmath -DFIXMATH_NO_CACHE
LDFLAGS=-Llibfixmath -lfixmath

.PHONY: all lint prog

all: mspmecanum.bin

skateboard.bin: libfixmath/libfixmath.a main.o 
	msp430-elf-gcc ${CFLAGS} $^ -o skateboard.bin ${LDFLAGS}

%.o: %.c
	msp430-elf-gcc ${CFLAGS} -c $<

libfixmath/libfixmath.a:
	$(MAKE) -C libfixmath libfixmath.a CC_FLAGS="${CFLAGS}"

lint:
	msp430-elf-gcc ${CFLAGS} -fsyntax-only *.c

clean:
	rm -f *.bin *.o
	$(MAKE) -C libfixmath clean

prog: mspmecanum.bin
	mspdebug rf2500 'prog mspmecanum.bin'