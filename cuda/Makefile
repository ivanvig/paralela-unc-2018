CC=nvcc #-Xcompiler
CFLAGS= #"-Werror, -Wall, -pedantic"

all: dir build/main

dir:
	mkdir -p build

build/main: build/main.o
	$(CC) $(CFLAGS) build/main.o -o build/main

build/main.o: src/main.cu
	$(CC) $(CFLAGS) -c src/main.cu -o build/main.o

clean:
	rm -r build
