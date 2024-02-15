CC=gcc -O3 -fopenmp

life3d: life3d.o world_gen.o
	$(CC) -o $@ $^

world_gen.o: world_gen.c world_gen.h
	$(CC) -c -o $@ $<

life3d.o: life3d.c world_gen.h
	$(CC) -c -o $@ $<

format:
	clang-format -i *.c

clean:
	rm life3d
	rm *.o
