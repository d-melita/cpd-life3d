CC = gcc -O3 -fopenmp

p1: format life3d

p2: format life3d-omp

p3: format life3d-mpi

p4: format life3d-mpi-omp

format:
	clang-format -i *.c

world_gen.o: world_gen.c
	$(CC) -c $@ $^

life3d.o: life3d.c
	$(CC) -c $@ $^

life3d: life3d.o world_gen.o
	$(CC) -o $@ $^

clean:
	rm life3d
	rm *.o
