#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <time.h>

#include "world_gen.h"

typedef struct args {
  int32_t gen_count;
  int32_t n;
  float density;
  int seed;
} Args;

uint32_t peak_gen[N_SPECIES + 1];

uint64_t max_population[N_SPECIES + 1];

uint64_t population[N_SPECIES + 1];

char ***old, ***new, ***tmp;

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

  args.density = atof(argv[3]);
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
char next_inhabitant(int32_t x, int32_t y, int32_t z, int32_t n,
                            char ***grid) {
  // Compute stats for neighbours
  char counts[N_SPECIES + 1];
  char neighbour, current;
  memset(counts, 0, (N_SPECIES + 1) * sizeof(char));

  // fprintf(stderr, "next_inhabitant(%d, %d, %d, %d, grid)\n", x, y, z, n);

  int32_t left = (x - 1 + n) % n;
  int32_t right = (x + 1 + n) % n;

  int32_t front = (y - 1 + n) % n;
  int32_t back = (y + 1 + n) % n;

  int32_t up = (z - 1 + n) % n;
  int32_t down = (z + 1 + n) % n;

  counts[grid[left][front][up]]++;
  counts[grid[left][front][z]]++;
  counts[grid[left][front][down]]++;
  counts[grid[left][y][up]]++;
  counts[grid[left][y][z]]++;
  counts[grid[left][y][down]]++;
  counts[grid[left][back][up]]++;
  counts[grid[left][back][z]]++;
  counts[grid[left][back][down]]++;

  counts[grid[x][front][up]]++;
  counts[grid[x][front][z]]++;
  counts[grid[x][front][down]]++;
  counts[grid[x][y][up]]++;
  current = grid[x][y][z];
  counts[grid[x][y][down]]++;
  counts[grid[x][back][up]]++;
  counts[grid[x][back][z]]++;
  counts[grid[x][back][down]]++;

  counts[grid[right][front][up]]++;
  counts[grid[right][front][z]]++;
  counts[grid[right][front][down]]++;
  counts[grid[right][y][up]]++;
  counts[grid[right][y][z]]++;
  counts[grid[right][y][down]]++;
  counts[grid[right][back][up]]++;
  counts[grid[right][back][z]]++;
  counts[grid[right][back][down]]++;

  char most_common = 0;
  char most_common_count = 0;
  char live_count = 0;

  for (char specie = 1; specie <= N_SPECIES; specie++) {
    live_count += counts[specie];

    if (counts[specie] > most_common_count) {
      most_common_count = counts[specie];
      most_common = specie;
    }
  }

  // fprintf(stderr, "most_common: %d, most_common_count: %d, live_count: %d\n",
          // most_common, most_common_count, live_count);

  if (current != 0) { // if cell is alive
    return (5 <= live_count && live_count <= 13) ? current : 0;
  } else { // if cell is dead
    return (7 <= live_count && live_count <= 10) ? most_common : 0;
  }
}

void prepare(int n) {
  new = new_grid(n);
  memset(peak_gen, 0, sizeof(uint32_t) * (N_SPECIES + 1));
  memset(max_population, 0, sizeof(uint64_t) * (N_SPECIES + 1));
  memset(population, 0, sizeof(uint64_t) * (N_SPECIES + 1));
}

void finish() {
  // fprintf(stderr, "Simulation finished\n");
  for (uint32_t specie = 1; specie <= N_SPECIES; specie++) {
    printf("%d %ld %d\n", specie, max_population[specie], peak_gen[specie]);
 }
}

void simulation(int32_t n, int32_t max_gen, char ***grid) {
  char new_val;
  old = grid;

  // fprintf(stderr, "Initial grid =================================\n");
  // debug(n, grid);

  // Compute initial stats
  #pragma omp parallel
  {
    #pragma omp for collapse(3) reduction(+:max_population[:N_SPECIES+1])
    for (int32_t x = 0; x < n; x++) {
      for (int32_t y = 0; y < n; y++) {
        for (int32_t z = 0; z < n; z++) {
          max_population[old[x][y][z]]++;
        }
      }
    }

    for (int32_t gen = 0; gen < max_gen; gen++) {

      #pragma omp for collapse(3) reduction(+:population[:N_SPECIES+1]) private(new_val)
      for (int32_t x = 0; x < n; x++) {
        for (int32_t y = 0; y < n; y++) {
          for (int32_t z = 0; z < n; z++) {
            new_val = next_inhabitant(x, y, z, n, old);
            new[x][y][z] = new_val;
            population[new_val]++;
            // fprintf(stderr, "next_inhabitant(%d, %d, %d, %d, grid) = %d\n", x, y,
            //         z, n, new[x][y][z]);
          }
        }
      }

      #pragma omp single
      {
        tmp = old;
        old = new;
        new = tmp;

        if (max_population[1] < population[1]) {
          max_population[1] = population[1];
          peak_gen[1] = gen + 1;
        }

        if (max_population[2] < population[2]) {
          max_population[2] = population[2];
          peak_gen[2] = gen + 1;
        }

        if (max_population[3] < population[3]) {
          max_population[3] = population[3];
          peak_gen[3] = gen + 1;
        }

        if (max_population[4] < population[4]) {
          max_population[4] = population[4];
          peak_gen[4] = gen + 1;
        }
        
        if (max_population[5] < population[5]) {
          max_population[5] = population[5];
          peak_gen[5] = gen + 1;
        }

        if (max_population[6] < population[6]) {
          max_population[6] = population[6];
          peak_gen[6] = gen + 1;
        }

        if (max_population[7] < population[7]) {
          max_population[7] = population[7];
          peak_gen[7] = gen + 1;
        }

        if (max_population[8] < population[8]) {
          max_population[8] = population[8];
          peak_gen[8] = gen + 1;
        }

        if (max_population[9] < population[9]) {
          max_population[9] = population[9];
          peak_gen[9] = gen + 1;
        }

        memset(population, 0, sizeof(uint64_t) * (N_SPECIES + 1));
      }
    }
  }
}

int main(int argc, char *argv[]) {
  double exec_time;
  Args args = parse_args(argc, argv);
  char ***grid = gen_initial_grid(args.n, args.density, args.seed);

  prepare(args.n);

  exec_time = -omp_get_wtime();

  simulation(args.n, args.gen_count, grid);

  exec_time += omp_get_wtime();
  
  finish();

  fprintf(stderr, "Took: %.1fs\n", exec_time);

  return EXIT_SUCCESS;
}
