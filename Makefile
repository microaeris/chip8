#-O3
CFLAGS=-Wall -std=c11 -g
LDFLAGS=`sdl2-config --cflags --libs`
DEPS=chip8.h

all: chip8

chip8: chip8.c $(DEPS)
	gcc -o $@ $(CFLAGS) $< $(LDFLAGS)

clean:
	rm -f chip8 *.o
