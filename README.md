# Mylly Game Engine

Mylly Game Engine is a light-weight and minimalistic game engine written entirely in C.

At its current state, Mylly is a 3D rendering framework with a collection of utilities for game development (e.g. AI). Currently Mylly does not handle physics, nor is there a plan to add an extensive physics component into the engine (other than perhaps some ray tracing or similar utilities).

## Features

Mylly engine is developed a few core principles in mind:

* Lightweight and quick to compile
* Consistent and clean C API
* Compatibility with low-end machines (target is OpenGL ES 2.0 for Raspberry Pi)

Several ideas for the future include:

* Support for Windows and Linux alike
* Taking advantage of high-end hardware with a Vulkan renderer

## Structure

The project is structured as follows:

```
mylly
├── engine
├── example
├── external
└── test
```

* **_engine_** contains the code of the engine library. More detailed information about the structure of the engine itself can be found [in the folder itself](engine/).
* **_example_** is a minimal example project which features Mylly to render a simple 3D scene.
* **_external_** contains the 3rd party libraries used in the development of the engine (with the exception of OpenGL). See **_Dependencies_** below.
* **_test_** contains the unit testing suite for the engine.

## Development

The development of the engine is not tied to any IDE, however currently Clang 6.0 is used for compiling. There is no reason though why the project couldn't be compiled with e.g. GCC as well.

### Profiling

Performance is an important factor, because even a good game is not fun if it lags too much. For profiling _valgrind_ is a very useful tool. Use the command below to start the profiler:

```
valgrind --tool=callgrind ./build/bin/mylly.bin
```

Run the game for a brief while and exit it. This creates a log file `callgrind.out.XXX` which can then be opened with _kcachegrind_ and analyzed:

```
kcachegrind callgrind.out.XXX
```

### Testing

A unit test framework (_MinUnit_) is set up for testing the core components of the engine and for ensuring there aren't any unforeseen consequences from future changes. The testing suite is set up in _./test/_.

The testing suite is currently very underutilized, and will hopefully be in more active use in the future.

## Dependencies

The engine uses OpenGL for rendering by default, so OpenGL development libraries and headers are required for development. In addition several 3rd party libraries are required by the engine. These are added to the repository as submodules.

* [cglm](https://github.com/recp/cglm.git) is a C99 compatible 3D graphics math library
* [JSMN](https://github.com/zserge/jsmn.git) is a minimalistic JSON parser
* [MinUnit](https://github.com/siu/minunit.git) is a minimal unit testing framework

Dependencies installed separately:

* OpenGL
* [libpng](http://www.libpng.org/pub/png/libpng.html) for loading PNG encoded textures
