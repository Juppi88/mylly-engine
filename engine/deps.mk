# Dependencies required for building a project which uses Mylly.
CFLAGS += -DCGLM_ALL_UNALIGNED
LDFLAGS += -L./mylly/external/jsmn -ljsmn -lpthread -lX11 -lXrandr -lGL -lGLU -lm -lpng -lfreetype
