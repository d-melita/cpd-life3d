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

char ***new_grid(uint32_t N, uint32_t height) {
  int x, y, z;
  char ***grid = (char ***) malloc((height+2) * sizeof(char **));

  //fprintf(stderr, "Generating grid of height %d:\n", height);
  if (grid == NULL) {
    printf("Failed to allocate matrix\n");
    exit(1);
  }

  for (x = 0; x < height+2; x++) {
    grid[x] = (char **)malloc(N * sizeof(char *));
    if (grid[x] == NULL) {
      printf("Failed to allocate matrix\n");
      exit(1);
    }
    grid[x][0] = (char *)calloc(N * N, sizeof(char));
    if (grid[x][0] == NULL) {
      printf("Failed to allocate matrix\n");
      exit(1);
    }
    for (y = 1; y < N; y++)
      grid[x][y] = grid[x][0] + y * N;
  }

  return grid;
}

char ***gen_initial_grid(uint32_t N, float density, int input_seed, uint32_t start, uint32_t end) {
  int x, y, z;

  uint32_t height = end + 1 - start;
  char ***grid = new_grid(N, height);

  init_r4uni(input_seed);
  for (x = 0; x < N; x++) {
    for (y = 0; y < N; y++) {
      for (z = 0; z < N; z++) {
        if (r4_uni() < density) {
          int value = (int)(r4_uni() * N_SPECIES) + 1;
          if (x >= start && x <= end) {
            grid[x-start+1][y][z] = value;
          }

          if ((start > 0 && x == start-1) || (start == 0 && x == N-1)) {
            grid[0][y][z] = value;
          }

          if ((end < N-1 && x == end+1) || (end == N-1 && x == 0)) {
            grid[height+1][y][z] = value; 
          }
        }
      }
    }
  }

  return grid;
}
