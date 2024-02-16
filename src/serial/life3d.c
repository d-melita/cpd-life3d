#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "world_gen.h"

inline uint32_t count_live_neighbours(uint32_t *neighbours) {
  uint32_t count = 0;
  for (uint32_t i = 0; i < N_NEIGHBOURS; i++) {
    count += (neighbours[i] != 1);
  }
  return count;
}

inline uint32_t get_next_specie(uint32_t *neighbours) {
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

inline int64_t get_index(int64_t value, uint32_t n) {
  if (value < 0) {
    return value + n;
  } else if (value >= n) {
    return value - n;
  } else {
    return value;
  }
}

/**
 * TODO: Add description
 * TODO: Check if compiler unrools this properly (if not, just hard code the 26
 * values)
 */
void get_neighbours(int64_t x, int64_t y, int64_t z, uint32_t n, char ***grid,
                    uint32_t *neighbours) {
  uint64_t index = 0;
  for (int64_t i = -NEIGHBOURS_RANGE; i <= NEIGHBOURS_RANGE; i++) {
    for (int64_t j = -NEIGHBOURS_RANGE; j <= NEIGHBOURS_RANGE; j++) {
      for (int64_t k = -NEIGHBOURS_RANGE; k <= NEIGHBOURS_RANGE; k++) {
        if (i == 0 && j == 0 && k == 0) {
          continue;
        }
        neighbours[index] =
            grid[get_index(x + i, n)][get_index(y + j, n)][get_index(z + k, n)];
        index++;
      }
    }
  }
}

void print_neighbours(uint32_t *neighbours) {
  for (int i = 0; i < N_NEIGHBOURS; i++) {
    printf("%d ", neighbours[i]);
  }
  printf("\n");
}

typedef struct args {
  uint32_t gen_count;
  uint32_t n;
  float density;
  int seed;
} Args;

void help() { fprintf(stderr, "\nUsage: life3d gen_count N density seed\n"); }

Args parse_args(int argc, char *argv[]) {
  Args args = {};

  if (argc != 5) {
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

int debug(uint32_t n, char ***grid) {
  for (uint64_t x = 0; x < n; x++) {
    printf("Layer %ld:\n", x);
    for (uint64_t y = 0; y < n; y++) {
      for (uint64_t z = 0; z < n; z++) {
        char value = grid[x][y][z];
        if (value > 0) {
          printf("%d ", value);
        } else {
          printf("  ");
        }
      }
      printf("\n");
    }
    printf("\n");
  }

  return EXIT_SUCCESS;
}

/**
 * Computes new inhabitant for cell at position (x, y, z) of grid
 */
inline char next_inhabitant(uint32_t x, uint32_t y, uint32_t z, uint32_t n,
                            char ***grid) {
  // Compute stats for neighbours
  uint32_t counts[N_NEIGHBOURS + 1];
  memset(counts, 0, (N_NEIGHBOURS + 1) * sizeof(uint32_t));

  for (int64_t i = -NEIGHBOURS_RANGE; i <= NEIGHBOURS_RANGE; i++) {
    for (int64_t j = -NEIGHBOURS_RANGE; j <= NEIGHBOURS_RANGE; j++) {
      for (int64_t k = -NEIGHBOURS_RANGE; k <= NEIGHBOURS_RANGE; k++) {
        if (i == 0 && j == 0 && k == 0) {
          continue;
        }
        char neighbour =
            grid[get_index(x + i, n)][get_index(y + j, n)][get_index(z + k, n)];
        counts[neighbour]++;
      }
    }
  }

  uint32_t most_common = 0;
  uint32_t most_common_count = 0;
  uint32_t live_count = 0;

  for (uint32_t specie = 1; specie <= N_NEIGHBOURS; specie++) {
    live_count += counts[specie];

    if (counts[specie] > most_common_count) {
      most_common_count = counts[specie];
      most_common = specie;
    }
  }

  if (grid[x][y][z] != 0) { // if cell is alive
    return (5 <= live_count && live_count <= 13) ? most_common : 0;
  } else { // if cell is dead
    return (7 <= live_count && live_count <= 10) ? most_common : 0;
  }
}

void simulation(uint32_t n, uint32_t max_gen, char ***grid) {
  uint32_t sim = 1;
  char ***old, ***new, ***tmp;

  old = grid;
  new = new_grid(n);

  // uint32_t *neighbours = (uint32_t *)malloc(N_NEIGHBOURS * sizeof(int));
  printf("Initial grid =================================\n");
  debug(n, grid);

  for (uint32_t gen = 0; gen < max_gen; gen++) {
    for (uint32_t x = 0; x < n; x++) {
      for (uint32_t y = 0; y < n; y++) {
        for (uint32_t z = 0; z < n; z++) {

          // get_neighbours(x, y, z, n, grid, neighbours);
          // uint32_t count = count_live_neighbours(neighbours);
          new[x][y][z] = next_inhabitant(x, y, z, n, old);
        }

        tmp = old;
        old = new;
        new = tmp;
      }
    }

    printf("Grid after generation %d =====================\n", gen + 1);
    debug(n, grid);
  }

  // free(neighbours);
}

int main(int argc, char *argv[]) {
  double exec_time;
  Args args = parse_args(argc, argv);
  char ***grid = gen_initial_grid(args.n, args.density, args.seed);

  // uint32_t *neighbours = get_neighbours(3, 2, 2, args.n, grid);
  // print_neighbours(neighbours);
  // free(neighbours);

  exec_time = -omp_get_wtime();
  simulation(args.n, args.gen_count, grid);
  exec_time += omp_get_wtime();
  fprintf(stderr, "Took: %.1fs\n", exec_time);

  return EXIT_SUCCESS;
}
