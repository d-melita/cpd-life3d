#include <stdint.h>

#define N_SPECIES 9
#define N_NEIGHBOURS 26
#define NEIGHBOURS_RANGE 1

char ***new_grid(uint32_t N);
char ***gen_initial_grid(uint32_t N, float density, int input_seed);