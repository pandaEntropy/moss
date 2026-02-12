BINARY=main
CODEDIRS=src
INCDIRS=include

CC=gcc

# add -I to each include directory
CFLAGS=-Wall -Wextra $(foreach D,$(INCDIRS),-I$(D))
LDFLAGS=-lX11

CFILES=$(foreach D,$(CODEDIRS),$(wildcard $(D)/*.c))
OBJECTS=$(patsubst %.c,%.o,$(CFILES))

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BINARY) $(OBJECTS)
