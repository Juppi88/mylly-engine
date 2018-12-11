.DEFAULT_GOAL := all
.PHONY: all clean
CC = clang-6.0
CFLAGS = -Wall -g -D_DEBUG -DCGLM_ALL_UNALIGNED -I. -I../external

OBJS = collections/array.o\
       core/mylly.o core/memory.o core/string.o core/time.o\
       input/input.o\
       io/config.o io/console.o io/file.o io/log.o\
       math/matrix.o math/quaternion.o math/random.o math/vector.o\
       platform/input.o platform/thread.o platform/timer.o platform/window.o\
       renderer/mesh.o renderer/rendersystem.o renderer/shader.o renderer/vbcache.o\
       renderer/texture.o\
       renderer/opengl/extensions.o renderer/opengl/renderer.o\
       resources/resourceparser.o resources/resources.o\
       scene/animator.o scene/camera.o scene/emitter.o scene/model.o scene/object.o scene/scene.o\
       scene/sprite.o scene/spriteanimation.o
DEPS =

builddirs:
	mkdir -p build/objs
	mkdir -p build/objs/collections
	mkdir -p build/objs/core
	mkdir -p build/objs/input
	mkdir -p build/objs/io
	mkdir -p build/objs/math
	mkdir -p build/objs/platform
	mkdir -p build/objs/renderer
	mkdir -p build/objs/renderer/opengl
	mkdir -p build/objs/resources
	mkdir -p build/objs/scene
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
