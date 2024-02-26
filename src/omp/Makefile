CC=gcc
CFLAGS=-O3 -fopenmp
LDFLAGS=-O3 -fopenmp

life3d: life3d.o world_gen.o

world_gen.o: world_gen.h
life3d.o: world_gen.h

format:
	clang-format -i *.c

clean:
	rm life3d || true
	rm *.o || true
