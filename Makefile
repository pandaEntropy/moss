all: build

build:
	gcc mywm.c -o mywm -Wall -Wextra -std=c17 -lX11
