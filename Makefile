NAME := floodit
TARGET := $(NAME)
TARGETDIST := $(NAME).opk
DISTFILES := $(TARGET) ClearSans-Medium.ttf resources
OBJS := *.c
OPTS := -g -O2 -Wall
LIB := -lSDL2 -lSDL2_ttf -lSDL2_net -lSDL2_image
CC := $(CROSS_COMPILE)gcc

SDL2CONF = $(shell which sdl2-config)
CFLAGS += $(shell $(SDL2CONF) --cflags)
LDFLAGS += $(shell $(SDL2CONF) --libs)

$(TARGET): $(OBJS)
	    $(CC) $(OPTS) $(CFLAGS) $^ -o $(NAME) $(LIB)

gcw: $(OBJS)
	$(CC) $(OPTS) $(CFLAGS) -DGCW $^ -o $(NAME) $(LIB)

opk:
	cp -r $(DISTFILES) dist/
	mksquashfs dist $(TARGETDIST) -all-root -noappend -no-exports -no-xattrs

clean:
	rm -f $(TARGET) $(TARGETDIST)
	rm -rf dist/$(TARGET) dist/ClearSans-Medium.ttf dist/resources
