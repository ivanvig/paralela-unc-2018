CC=mpicc
CFLAGS= -Wall -Werror -pedantic

all: dir build/main

dir:
	mkdir -p build

build/main: build/main.o
	$(CC) $(CFLAGS) build/main.o -o build/main

build/main.o: src/main.c
	$(CC) $(CFLAGS) -c src/main.c -o build/main.o

clean:
	rm -r build
