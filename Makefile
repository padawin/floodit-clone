NAME := floodit
TARGET := $(NAME)
TARGETDIST := $(NAME).opk
OBJS := *.c
OPTS := -g -O2 -Wall
LIB := -lSDL2 -lSDL2_ttf -lSDL2_net
CC := $(CROSS_COMPILE)gcc

SDL2CONF = $(shell which sdl2-config)
CFLAGS += $(shell $(SDL2CONF) --cflags)
LDFLAGS += $(shell $(SDL2CONF) --libs)

$(TARGET): $(OBJS)
	    $(CC) $(OPTS) $(CFLAGS) $^ -o $(NAME) $(LIB)

gcw: $(OBJS)
	$(CC) $(OPTS) $(CFLAGS) -DGCW $^ -o $(NAME) $(LIB)

opk:
	cp floodit ClearSans-Medium.ttf dist/
	mksquashfs dist $(TARGETDIST) -all-root -noappend -no-exports -no-xattrs

clean:
	rm -f dist/$(TARGET) $(TARGET) $(TARGETDIST)
