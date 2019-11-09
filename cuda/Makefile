CC=nvcc #-Xcompiler
CFLAGS= #"-Werror, -Wall, -pedantic"
BUILDPATH=build


all: dir $(BUILDPATH)/main

dir:
	mkdir -p $(BUILDPATH)

debug: CFLAGS += -g -G
debug: BUILDPATH=debug
debug: all

$(BUILDPATH)/main: $(BUILDPATH)/main.o
	$(CC) $(CFLAGS) $(BUILDPATH)/main.o -o $(BUILDPATH)/main

$(BUILDPATH)/main.o: src/main.cu
	$(CC) $(CFLAGS) -c src/main.cu -o $(BUILDPATH)/main.o

clean:
	rm -rf build
	rm -rf debug
