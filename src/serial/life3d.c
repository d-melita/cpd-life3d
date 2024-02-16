#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "world_gen.h"

char ***grid;

char *** second_grid;

uint32_t count_live_neighbours(uint32_t *neighbours) {
  uint32_t count = 0;
  for (uint32_t i = 0; i < N_NEIGHBOURS; i++) {
    count += (neighbours[i] != 1);
  }
  return count;
}

uint32_t get_next_specie(uint32_t *neighbours) {
  uint32_t count[9] = {0};
  for (uint32_t i = 0; i < N_NEIGHBOURS; i++) {
    count[neighbours[i]]++;
  }
  uint32_t gen = 1;
  for (uint32_t i = 2; i < 9; i++) {
    gen = (count[i] > count[gen]) ? i : gen;
  }
  return gen;
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

uint32_t* get_neighbours(int64_t x, int64_t y, int64_t z, long long n) {
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

  if (argc < 5 || argc > 5) {
    printf("Wrong number of arguments (5 required, %d provided)\n", argc);
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

void simulation(long long n) {
  uint32_t sim = 1;
  uint32_t *neighbours = (uint32_t *)malloc(N_NEIGHBOURS * sizeof(int));
  // for simulations 1, 3, 5, 7, and 9 we use sim = 1 and second_grid
  // for simulations 2, 4, 6, 8, and 10 we use sim = 0 and grid

  for (uint32_t gen = 0; gen < 10; gen++) {
    for (uint64_t x = 0; x < n; x++) {
      for (uint64_t y = 0; y < n; y++) {
        for (uint64_t z = 0; z < n; z++) {
          neighbours = get_neighbours(x, y, z, n);
          uint32_t count = count_live_neighbours(neighbours);

          if (sim) {
            if (grid[x][y][z] != 0) {  // if cell is alive
              second_grid[x][y][z] = (5 <= count <= 13) ? get_next_specie(neighbours) : 0;
            } else {  // if cell is dead
              second_grid[x][y][z] = (7 <= count <= 10) ? get_next_specie(neighbours) : 0;
            }
          } else {
            if (second_grid[x][y][z] != 0) {  // if cell is alive
              grid[x][y][z] = (5 <= count <= 13) ? get_next_specie(neighbours) : 0;
            } else {  // if cell is dead
              grid[x][y][z] = (7 <= count <= 10) ? get_next_specie(neighbours) : 0;
            }
          }
          free(neighbours);
        }
        sim = !sim;
      }
    }
  }
}

int main(int argc, char *argv[]) {
  double exec_time;
  Args args = parse_args(argc, argv);
  grid = gen_initial_grid(args.n, args.density, args.seed);
  debug(args.n);
  print_neighbours(get_neighbours(3, 2, 2, args.n));
  exec_time = -omp_get_wtime();
  simulation(args.n);
  exec_time += omp_get_wtime();
  fprintf(stderr, "Took: %.1fs\n", exec_time);

  return EXIT_SUCCESS;
}
