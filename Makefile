CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -Iinclude
LDFLAGS = -lX11

SRC = \
	src/main.c \
	src/wm.c \
	src/layout.c

OBJ = $(SRC:.c=.o)

all: build

build: $(OBJ)
	$(CC) $(OBJ) -o main $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@
