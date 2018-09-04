.DEFAULT_GOAL := all
.PHONY: all clean
CC = clang-6.0
CFLAGS = -Wall -g -I../engine
LDFLAGS = -L../engine/build/bin -lmylly -lpthread -lX11 -lGL -lGLU

OBJS = main/main.o
DEPS =

builddirs:
	mkdir -p build/objs/main
	mkdir -p build/bin

clean:
	$(MAKE) -C ../engine clean
	rm -f build/objs/*.o build/bin/*

mylly.bin: $(OBJS)
	$(CC) $(addprefix build/objs/, $^) $(LDFLAGS) -o build/bin/$@

%.o: %.c $(DEPS)
	$(CC) -c -o build/objs/$@ $< $(CFLAGS)

all:
	$(MAKE) -C ../engine builddirs
	$(MAKE) -C ../engine
	make builddirs
	make mylly.bin
