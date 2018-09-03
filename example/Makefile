.DEFAULT_GOAL := all
.PHONY: all clean
CC = clang-6.0
CFLAGS = -Wall -O2 -I../engine
LDFLAGS = -static -L../engine/build/bin -lmylly

OBJS = main/main.o
DEPS =

builddirs:
	mkdir -p build/objs/main
	mkdir -p build/bin

clean:
	rm -f build/objs/*.o build/bin/*

mylly.bin: $(OBJS)
	$(CC) $(addprefix build/objs/, $^) $(LDFLAGS) -o build/bin/$@

%.o: %.c $(DEPS)
	$(CC) -c -o build/objs/$@ $< $(CFLAGS)

all:
	make builddirs
	make mylly.bin
