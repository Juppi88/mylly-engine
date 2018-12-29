.DEFAULT_GOAL := all
.PHONY: all clean
CC = clang
CFLAGS = -Wall -g -I../engine -DCGLM_ALL_UNALIGNED
LDFLAGS = -L../engine/build/bin -lmylly -lpthread -lX11 -lGL -lGLU -lm -lpng

OBJS = main/main.o
DEPS =

builddirs:
	mkdir -p build/objs/main
	mkdir -p build/bin

clean:
	$(MAKE) -C ../engine clean
	rm -rf build/objs/*.o build/bin/shaders/ build/bin/mylly.bin

mylly.bin: $(OBJS)
	$(CC) $(addprefix build/objs/, $^) $(LDFLAGS) -o build/bin/$@
	cp -r ../engine/resources/shaders/ build/bin/

%.o: %.c $(DEPS)
	$(CC) -c -o build/objs/$@ $< $(CFLAGS)

all:
	$(MAKE) -C ../engine builddirs
	$(MAKE) -C ../engine
	make builddirs
	make mylly.bin
