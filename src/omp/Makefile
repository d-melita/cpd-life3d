CC=gcc
CFLAGS=-O2 -fopenmp
LDFLAGS=-O2 -fopenmp

life3d-omp: life3d-omp.o world_gen.o

world_gen.o: world_gen.h
life3d-omp.o: world_gen.h

format:
	clang-format -i *.c

clean:
	rm life3d-omp || true
	rm *.o || true
