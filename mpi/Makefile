CC=mpicc
# CFLAGS= -Wall -Werror -pedantic -fopenmp
CFLAGS= -fopenmp

all: dir build/mpi_blocking build/mpi_nonblocking

dir:
	mkdir -p build

increase_stack:
	ulimit -S -s 131072

prof: dir
	TAU_TRACE=0
	tau_cc.sh src/mpi_nonblocking.c -o build/mpi_nonblocking
	tau_cc.sh src/mpi_blocking.c -o build/mpi_blocking

trace: dir
	TAU_TRACE=1
	tau_cc.sh src/mpi_nonblocking.c -o build/mpi_nonblocking
	tau_cc.sh src/mpi_blocking.c -o build/mpi_blocking

build/mpi_nonblocking: build/mpi_nonblocking.o
	$(CC) $(CFLAGS) build/mpi_nonblocking.o -o build/mpi_nonblocking

build/mpi_nonblocking.o: src/mpi_nonblocking.c
	$(CC) $(CFLAGS) -c src/mpi_nonblocking.c -o build/mpi_nonblocking.o

build/mpi_blocking: build/mpi_blocking.o
	$(CC) $(CFLAGS) build/mpi_blocking.o -o build/mpi_blocking

build/mpi_blocking.o: src/mpi_blocking.c
	$(CC) $(CFLAGS) -c src/mpi_blocking.c -o build/mpi_blocking.o

clean:
	rm -r build
