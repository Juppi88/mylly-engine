.DEFAULT_GOAL := all
.PHONY: all builddirs clean
CC = clang++
CFLAGS = -Wall -g -std=c++11 -I. -I../ -I../mylly
LDFLAGS = -L../mylly/build/bin -lmylly -L../external/jsmn

# Include library dependencies.
include ../mylly/deps.mk

OBJS = main.o asteroid.o asteroidhandler.o collisionhandler.o entity.o floatingobject.o\
       game.o gamescene.o inputhandler.o menuscene.o powerup.o projectile.o projectilehandler.o\
       scene.o ship.o ufo.o ui.o utils.o
DEPS =

builddirs:
	mkdir -p build/objs
	mkdir -p build/bin

clean:
	$(MAKE) -C ../mylly clean
	rm -rf build/objs/*.o build/bin/shaders/ build/bin/mylly.bin

engine:
	$(MAKE) -C ../mylly builddirs
	$(MAKE) -C ../mylly

game.bin: $(OBJS)
	$(CC) $(addprefix build/objs/, $^) $(LDFLAGS) -o build/bin/$@
	cp -r ../mylly/resources/shaders/ build/bin/

%.o: %.cpp $(DEPS)
	$(CC) -c -o build/objs/$@ $< $(CFLAGS)

all:
	make engine
	make builddirs
	make game.bin
