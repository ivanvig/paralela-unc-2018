CC=nvcc
CFLAGS= #"-Werror, -Wall, -pedantic"

all: build/main

build/main: build/main.o
	$(CC) -Xcompiler $(CFLAGS) build/main.o -o build/main

build/main.o: src/main.cu
	$(CC) -Xcompiler $(CFLAGS) -c src/main.cu -o build/main.o

clean:
	rm build/main*
