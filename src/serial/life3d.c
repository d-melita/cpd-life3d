#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "world_gen.h"

char ***grid;

void simulation() {
  // TODO
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

int main(int argc, char *argv[]) {
  double exec_time;
  Args args = parse_args(argc, argv);
  grid = gen_initial_grid(args.n, args.seed, args.density);
  exec_time = -omp_get_wtime();
  simulation();
  exec_time += omp_get_wtime();
  fprintf(stderr, "Took: %.1fs\n", exec_time);

  return EXIT_SUCCESS;
}
