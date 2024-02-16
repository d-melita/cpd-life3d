#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "world_gen.h"

char ***grid;

void simulation() {
  // TODO
}

int64_t get_index(int64_t value, long long n) {
  if (value < 0) {
    return n + value;
  } else if (value >= n) {
    return value%n;
  } else {
    return value;
  }
}

uint32_t* get_neighbours(uint64_t x, uint64_t y, uint64_t z, long long n) {
  uint32_t *neighbours = (uint32_t *)malloc(N_NEIGHBOURS * sizeof(int));
  uint64_t index = 0;
  for (int64_t i = -NEIGHBOURS_RANGE; i <= NEIGHBOURS_RANGE; i++) {
    for (int64_t j = -NEIGHBOURS_RANGE; j <= NEIGHBOURS_RANGE; j++) {
      for (int64_t k = -NEIGHBOURS_RANGE; k <= NEIGHBOURS_RANGE; k++) {
        if (i == 0 && j == 0 && k == 0) {
          continue;
        }
        neighbours[index] = grid[get_index(x + i, n)][get_index(y + j, n)][get_index(z + k, n)];
        index++;
      }
    }
  }
  return neighbours;
}

void print_neighbours(uint32_t *neighbours) {
  for (int i = 0; i < N_NEIGHBOURS; i++) {
    printf("%d ", neighbours[i]);
  }
  printf("\n");
  free(neighbours);
}

typedef struct args {
  uint32_t gen_count;
  long long n;
  float density;
  int seed;
} Args;

void help() {
  fprintf(stderr, "\nUsage: life3d gen_count N density seed\n");
}

Args parse_args(int argc, char* argv[]) {
  Args args = {};

  if (argc < 5) {
    printf("Insufficient arguments (5 required, %d provided)\n", argc);
    help();
    exit(1);
  }

  if (argc > 5) {
    printf("Too many arguments (5 required, %d provided)\n", argc);
    help();
    exit(1);
  }

  args.gen_count = atoi(argv[1]);

  if (args.gen_count == 0) {
    printf("Invalid generation count provided: %s\n", argv[1]);
    help();
    exit(1);
  }

  args.n = atoi(argv[2]);

  if (args.n == 0) {
    printf("Invalid N provided: %s\n", argv[2]);
    help();
    exit(1);
  }

  args.density = atof(argv[3]);
  if (args.density < 0 || args.density > 1) {
    printf("Invalid density provided: %s\n", argv[2]);
    help();
    exit(1);
  }


  args.seed = atoi(argv[4]);

  if (args.seed == 0) {
    printf("Invalid seed provided: %s\n", argv[4]);
    help();
    exit(1);
  }

  return args;
}

int debug (long long n) {
    for (uint64_t x = 0; x < n; x++) {
        printf("Layer %ld:\n", x);
        for (uint64_t y = 0; y < n; y++) {
            for (uint64_t z = 0; z < n; z++) {
              printf("%d ", grid[x][y][z]);
            }
            printf("\n");
        }
        printf("\n");
    }

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
  double exec_time;
  Args args = parse_args(argc, argv);
  grid = gen_initial_grid(args.n, args.density, args.seed);
  debug(args.n);
  print_neighbours(get_neighbours(3, 2, 2, args.n));
  exec_time = -omp_get_wtime();
  simulation();
  exec_time += omp_get_wtime();
  fprintf(stderr, "Took: %.1fs\n", exec_time);

  return EXIT_SUCCESS;
}
