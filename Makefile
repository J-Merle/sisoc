CC=gcc
CFLAGS=-Wall -Wextra -I./lib -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/sysprof-6 -pthread
LDFLAGS=-L./lib
LDLIBS=-lraylib -lm -lpulse-mainloop-glib -lpulse -pthread -lglib-2.0


main: main.c

clean:
	- rm -f main
