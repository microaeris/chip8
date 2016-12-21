CFLAGS=-O3 -Wall -std=c11
LDFLAGS=`sdl2-config --cflags --libs`

all: chip8

chip8: chip8.c
	gcc -o $@ $(CFLAGS) -g $< $(LDFLAGS)

clean:
	rm -f chip8 *.o