.DEFAULT_GOAL := all
.PHONY: all clean analysis
CC = clang
CFLAGS = -Wall -Wextra -g -D_DEBUG -DCGLM_ALL_UNALIGNED -I. -I../external -I/usr/include/freetype2

OBJS = ai/ai.o ai/behaviour.o ai/node.o\
       collections/array.o\
       core/mylly.o core/memory.o core/string.o core/time.o\
       io/config.o io/console.o io/file.o io/input.o io/log.o\
       math/matrix.o math/quaternion.o math/random.o math/vector.o\
       mgui/mgui.o mgui/text.o mgui/uiinput.o mgui/vectori.o mgui/widget.o\
       mgui/widgets/button.o mgui/widgets/checkbox.o mgui/widgets/grid.o mgui/widgets/inputbox.o\
       mgui/widgets/label.o mgui/widgets/panel.o\
       platform/inputhook.o platform/thread.o platform/timer.o platform/window.o\
       renderer/buffer.o renderer/buffercache.o renderer/colour.o renderer/debug.o renderer/font.o\
       renderer/fontpacker.o renderer/mesh.o renderer/rendersystem.o renderer/shader.o\
       renderer/texture.o renderer/vbcache.o\
       renderer/opengl/extensions.o renderer/opengl/renderer.o\
       resources/objparser.o resources/resourceparser.o resources/resources.o\
       scene/animator.o scene/camera.o scene/emitter.o scene/model.o scene/object.o scene/scene.o\
       scene/sprite.o scene/spriteanimation.o
DEPS =

builddirs:
	mkdir -p build/objs
	mkdir -p build/objs/ai
	mkdir -p build/objs/collections
	mkdir -p build/objs/core
	mkdir -p build/objs/io
	mkdir -p build/objs/math
	mkdir -p build/objs/mgui
	mkdir -p build/objs/mgui/widgets
	mkdir -p build/objs/platform
	mkdir -p build/objs/renderer
	mkdir -p build/objs/renderer/opengl
	mkdir -p build/objs/resources
	mkdir -p build/objs/scene
	mkdir -p build/bin

clean:
	rm -rf build/objs/ build/bin/*.a

libmylly.a: $(OBJS)
	ar -rv build/bin/$@ $(addprefix build/objs/, $^)

%.o: %.c $(DEPS)
	$(CC) -c -o build/objs/$@ $< $(CFLAGS)

analysis:
	scan-build --view --show-description --keep-going --use-cc=/usb/bin/clang make

all:
	make builddirs
	make libmylly.a
