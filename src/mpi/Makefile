CC=gcc
CFLAGS=-O2 -fopenmp -g
LDFLAGS=-O2 -fopenmp -g

life3d-mpi: life3d-mpi.o world_gen.o
	mpicc -g -o life3d-mpi life3d-mpi.o world_gen.o

world_gen.o: world_gen.h
life3d-mpi.o: world_gen.h

format:
	clang-format -i *.c

clean:
	rm life3d-omp || true
	rm *.o || true
