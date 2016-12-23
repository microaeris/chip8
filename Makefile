CFLAGS=-O3 -Wall -std=c11
LDFLAGS=`sdl2-config --cflags --libs`
DEPS=chip8.h

all: chip8

chip8: chip8.c $(DEPS)
	gcc -o $@ $(CFLAGS) -g $< $(LDFLAGS)

clean:
	rm -f chip8 *.o
