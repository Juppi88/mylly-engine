.DEFAULT_GOAL := all
.PHONY: all clean
CC = clang-6.0
CFLAGS = -Wall -O2 -I.
LDFLAGS = -static

OBJS = core/memory.o core/string.o\
       io/config.o io/console.o io/file.o io/log.o\
       platform/thread.o platform/timer.o
DEPS =

builddirs:
	mkdir -p build/objs
	mkdir -p build/objs/core
	mkdir -p build/objs/io
	mkdir -p build/objs/platform
	mkdir -p build/bin

clean:
	rm -f objs/* bin/*

libmylly.a: $(OBJS)
	ar -rv build/bin/$@ $(addprefix build/objs/, $^)

%.o: %.c $(DEPS)
	$(CC) -c -o build/objs/$@ $< $(CFLAGS)

all:
	make builddirs
	make libmylly.a
