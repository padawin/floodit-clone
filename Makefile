TARGET := floodit
OBJS := main.c
OPTS := -g -O2 -Wall
LIB := -lSDL2
CC := $(CROSS_COMPILE)gcc

SYSROOT := $(shell $(CC) --print-sysroot)
CFLAGS += $(shell $(SYSROOT)/usr/bin/sdl2-config --cflags)
LDFLAGS += $(shell $(SYSROOT)/usr/bin/sdl2-config --libs)

$(TARGET): $(OBJS)
	    $(CC) $(OPTS) $(CFLAGS) $^ -o $@ $(LIB)

clean:
	rm -f $(TARGET)

