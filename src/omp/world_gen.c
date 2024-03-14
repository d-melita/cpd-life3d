#include "world_gen.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

unsigned int seed;

void init_r4uni(int input_seed) { seed = input_seed + 987654321; }

float r4_uni() {
  int seed_in = seed;

  seed ^= (seed << 13);
  seed ^= (seed >> 17);
  seed ^= (seed << 5);

  return 0.5 + 0.2328306e-09 * (seed_in + (int)seed);
}

char *new_grid(uint32_t N) {
  int x, y, z;
  char *grid = (char *)calloc(N * N * N, sizeof(char));
  if (grid == NULL) {
    printf("Failed to allocate matrix\n");
    exit(1);
  }

  return grid;
}

char *gen_initial_grid(uint32_t N, float density, int input_seed) {
  // FIX: very ugly code
  int N2 = N*N;
  int N3 = N2*N;
  int x, y, z;

  char *grid = new_grid(N);
  
  init_r4uni(input_seed);
  for (x = 0; x < N; x++)
    for (y = 0; y < N; y++)
      for (z = 0; z < N; z++)
        if (r4_uni() < density)
          grid[x * N2 + y * N + z] = (int)(r4_uni() * N_SPECIES) + 1;

  return grid;
}
