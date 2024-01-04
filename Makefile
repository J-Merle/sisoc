CC=gcc
CFLAGS=-Wall -Wextra -I./lib
LDFLAGS=-L./lib
LDLIBS=-lraylib -lm -lpulse


main: main.c

clean:
	- rm -f main

run:
	- ./main
