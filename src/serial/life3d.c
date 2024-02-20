#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "world_gen.h"

inline int64_t get_index(int64_t value, uint32_t n) {
  if (value < 0) {
    return value + n;
  } else if (value >= n) {
    return value - n;
  } else {
    return value;
  }
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

  if (args.n < 3) {
    printf("Invalid N provided: %s\n", argv[2]);
    help();
    exit(1);
  }

  args.density = atoi(argv[3]);
  if (args.density < 0 || args.density > 1) {
    printf("Invalid density provided: %s\n", argv[2]);
    help();
    exit(1);
  }

  args.seed = atoi(argv[4]);

  return args;
}

int debug(uint32_t n, char ***grid) {
  for (uint64_t x = 0; x < n; x++) {
    fprintf(stderr, "Layer %ld:\n", x);
    for (uint64_t y = 0; y < n; y++) {
      for (uint64_t z = 0; z < n; z++) {
        char value = grid[x][y][z];
        if (value > 0) {
          fprintf(stderr, "%d ", value);
        } else {
          fprintf(stderr, "  ");
        }
      }
      fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
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

  // fprintf(stderr, "next_inhabitant(%d, %d, %d, %d, grid)\n", x, y, z, n);

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

  // fprintf(stderr, "most_common: %d, most_common_count: %d, live_count: %d\n",
          // most_common, most_common_count, live_count);

  char current = grid[x][y][z];
  if (current != 0) { // if cell is alive
    return (5 <= live_count && live_count <= 13) ? current : 0;
  } else { // if cell is dead
    return (7 <= live_count && live_count <= 10) ? most_common : 0;
  }
}

void simulation(uint32_t n, uint32_t max_gen, char ***grid) {
  uint32_t sim = 1;
  char ***old, ***new, ***tmp;

  uint32_t peak_gen[N_NEIGHBOURS + 1];
  memset(peak_gen, 0, sizeof(uint32_t) * (N_NEIGHBOURS + 1));

  uint64_t max_population[N_NEIGHBOURS + 1];
  memset(max_population, 0, sizeof(uint64_t) * (N_NEIGHBOURS + 1));

  uint64_t population[N_NEIGHBOURS + 1];
  memset(population, 0, sizeof(uint64_t) * (N_NEIGHBOURS + 1));

  old = grid;
  new = new_grid(n);

  // fprintf(stderr, "Initial grid =================================\n");
  // debug(n, grid);

  // Compute initial stats
  for (uint32_t x = 0; x < n; x++) {
    for (uint32_t y = 0; y < n; y++) {
      for (uint32_t z = 0; z < n; z++) {
        max_population[old[x][y][z]]++;
      }
    }
  }

  // fprintf(stderr, "Initial stats ================================\n");
  // for (uint32_t specie = 1; specie <= N_SPECIES; specie++) {
  //   fprintf(stderr, "%d %ld %d\n", specie, max_population[specie],
  //           peak_gen[specie]);
  // }

  for (uint32_t gen = 0; gen < max_gen; gen++) {
    memset(population, 0, sizeof(uint64_t) * (N_NEIGHBOURS + 1));

    for (uint32_t x = 0; x < n; x++) {
      for (uint32_t y = 0; y < n; y++) {
        for (uint32_t z = 0; z < n; z++) {

          new[x][y][z] = next_inhabitant(x, y, z, n, old);
          population[new[x][y][z]]++;
          // fprintf(stderr, "next_inhabitant(%d, %d, %d, %d, grid) = %d\n", x, y,
          //         z, n, new[x][y][z]);
        }
      }
    }

    tmp = old;
    old = new;
    new = tmp;

    // fprintf(stderr, "Grid after generation %d =====================\n",
    //         gen + 1);
    // debug(n, old);
    //
    // fprintf(stderr, "Stats for generation %d ======================\n",
    //         gen + 1);
    // for (uint32_t specie = 1; specie <= N_SPECIES; specie++) {
    //   fprintf(stderr, "%d %ld\n", specie, population[specie]);
    // }

    for (uint32_t specie = 1; specie <= N_SPECIES; specie++) {
      if (max_population[specie] < population[specie]) {
        max_population[specie] = population[specie];
        peak_gen[specie] = gen + 1;
      }
    }
  }

  // fprintf(stderr, "Simulation finished\n");
  for (uint32_t specie = 1; specie <= N_SPECIES; specie++) {
    printf("%d %ld %d\n", specie, max_population[specie], peak_gen[specie]);
  }
}

int main(int argc, char *argv[]) {
  double exec_time;
  Args args = parse_args(argc, argv);
  char ***grid = gen_initial_grid(args.n, args.density, args.seed);

  exec_time = -omp_get_wtime();

  simulation(args.n, args.gen_count, grid);

  exec_time += omp_get_wtime();
  fprintf(stderr, "Took: %.1fs\n", exec_time);

  return EXIT_SUCCESS;
}
