NAME := floodit
TARGET := $(NAME)
TARGETDIST := $(NAME).opk
DISTFILES := $(TARGET) ClearSans-Medium.ttf LICENCE.md README.md resources
OBJS := *.c
OPTS := -g -O2 -Wall -Wextra -Wmissing-declarations \
         -Wcast-qual -Wconversion -Wsign-conversion \
         -Wdisabled-optimization \
         -Werror -Wfloat-equal -Wformat=2 \
         -Wformat-nonliteral -Wformat-security \
         -Wformat-y2k \
         -Wimport -Winit-self -Winline \
         -Winvalid-pch \
         -Wlong-long \
         -Wmissing-field-initializers -Wmissing-format-attribute \
         -Wmissing-noreturn \
         -Wpacked -Wpointer-arith \
         -Wredundant-decls \
         -Wshadow -Wstack-protector \
         -Wstrict-aliasing=2 -Wswitch-default \
         -Wswitch-enum \
         -Wunreachable-code -Wunused \
         -Wunused-parameter \
         -Wvariadic-macros \
         -Wwrite-strings \

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
