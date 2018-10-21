.DEFAULT_GOAL := all
.PHONY: all clean
CC = clang-6.0
CFLAGS = -Wall -g -I../engine -I../external/minunit
LDFLAGS = -L../engine/build/bin -lmylly -lpthread -lX11 -lGL -lGLU -lm -lpng -lrt

OBJS = main.o
DEPS =

builddirs:
	mkdir -p build/objs
	mkdir -p build/bin

clean:
	$(MAKE) -C ../engine clean
	rm -rf build/objs/*.o build/bin/test.bin

test.bin: $(OBJS)
	$(CC) $(addprefix build/objs/, $^) $(LDFLAGS) -o build/bin/$@

test:
	build/bin/test.bin

%.o: %.c $(DEPS)
	$(CC) -c -o build/objs/$@ $< $(CFLAGS)

all:
	$(MAKE) -C ../engine builddirs
	$(MAKE) -C ../engine
	make builddirs
	make test.bin
	make test
