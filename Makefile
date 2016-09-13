NAME := floodit
TARGET := $(NAME)
TARGETDIST := $(NAME).opk
OBJS := main.c
OPTS := -g -O2 -Wall
LIB := -lSDL2
CC := $(CROSS_COMPILE)gcc

SYSROOT := $(shell $(CC) --print-sysroot)
CFLAGS += $(shell $(SYSROOT)/usr/bin/sdl2-config --cflags)
LDFLAGS += $(shell $(SYSROOT)/usr/bin/sdl2-config --libs)

$(TARGET): $(OBJS)
	    $(CC) $(OPTS) $(CFLAGS) $^ -o $@ $(LIB)

opk:
	cp floodit dist/
	mksquashfs dist $(TARGETDIST) -all-root -noappend -no-exports -no-xattrs

clean:
	rm -f dist/$(TARGET) $(TARGET) $(TARGETDIST)
