.DEFAULT_GOAL := all
.PHONY: all builddirs clean
CC = clang
CFLAGS = -Wall -g -I../mylly
LDFLAGS = -L../mylly/build/bin -lmylly -lpthread -lX11 -lGL -lGLU -lm -lpng

# Include library dependencies.
LDFLAGS += -L../external/jsmn
include ../mylly/deps.mk

OBJS = main/main.o
DEPS =

builddirs:
	mkdir -p build/objs/main
	mkdir -p build/bin

clean:
	$(MAKE) -C ../mylly clean
	rm -rf build/objs/*.o build/bin/shaders/ build/bin/mylly.bin

game.bin: $(OBJS)
	$(CC) $(addprefix build/objs/, $^) $(LDFLAGS) -o build/bin/$@
	cp -r ../mylly/resources/shaders/ build/bin/

%.o: %.c $(DEPS)
	$(CC) -c -o build/objs/$@ $< $(CFLAGS)

all:
	$(MAKE) -C ../mylly builddirs
	$(MAKE) -C ../mylly
	make builddirs
	make game.bin
