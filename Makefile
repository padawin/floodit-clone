NAME := floodit
TARGET := $(NAME)
TARGETDIST := $(NAME).opk
OBJS := *.c
OPTS := -g -O2 -Wall
LIB := -lSDL2 -lSDL2_ttf
CC := $(CROSS_COMPILE)gcc

SYSROOT := $(shell $(CC) --print-sysroot)
CFLAGS += $(shell $(SYSROOT)/usr/bin/sdl2-config --cflags)
LDFLAGS += $(shell $(SYSROOT)/usr/bin/sdl2-config --libs)

$(TARGET): $(OBJS)
	    $(CC) $(OPTS) $(CFLAGS) $^ -o $(NAME) $(LIB)

gcw: $(OBJS)
	$(CC) $(OPTS) $(CFLAGS) -DGCW $^ -o $(NAME) $(LIB)

opk:
	cp floodit ClearSans-Medium.ttf dist/
	mksquashfs dist $(TARGETDIST) -all-root -noappend -no-exports -no-xattrs

clean:
	rm -f dist/$(TARGET) $(TARGET) $(TARGETDIST)
