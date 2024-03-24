#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <mpi.h>

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

int debug_partitions(uint32_t start, uint32_t end, char ***grid, int rank, int N) {
  fprintf(stderr, "Rank %d: Start %d, End %d\n", rank, start, end);
  for (uint64_t x = start; x <= end; x++) {
    fprintf(stderr, "Layer %ld:\n", x);
    for (uint64_t y = 0; y < N; y++) {
      for (uint64_t z = 0; z < N; z++) {
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

void finish(int rank) {
  // fprintf(stderr, "Simulation finished\n");
  if (rank == 0) {
    for (uint32_t specie = 1; specie <= N_SPECIES; specie++) {
      printf("%d %ld %d\n", specie, max_population[specie], peak_gen[specie]);
    }
  }
}

void simulation(int32_t n, int32_t max_gen, char ***grid, int rank, int num_procs) {
  char new_val;
  old = grid;

  // Calculate the portion of the grid for each MPI process
  int32_t start = (rank * n) / num_procs;
  int32_t end = (rank == num_procs - 1) ? n : (((rank + 1) * n) / num_procs);

  int32_t next_rank = (rank + 1) % num_procs;
  int32_t prev_rank = (rank - 1 + num_procs) % num_procs;

  //printf("rank: %d, start: %d, end: %d, next_rank: %d, prev_rank: %d\n", rank, start, end, next_rank, prev_rank);

  // print partitioning
  // debug_partitions(start, end, grid, rank, n);

  // fprintf(stderr, "Initial grid =================================\n");
  // debug(n, grid);

  // Compute initial stats
  uint64_t local_max_population[N_SPECIES + 1];
  memset(local_max_population, 0, sizeof(uint64_t) * (N_SPECIES + 1));
  for (int32_t x = start; x < end; x++) {
    for (int32_t y = 0; y < n; y++) {
      for (int32_t z = 0; z < n; z++) {
        local_max_population[old[x][y][z]]++;
      }
    }
  }

  // Gather initial statistics from all MPI processes to task 0
  MPI_Reduce(local_max_population, max_population, N_SPECIES + 1, MPI_UINT64_T, MPI_SUM, 0, MPI_COMM_WORLD);

  uint64_t local_population[N_SPECIES + 1];

  for (int32_t gen = 0; gen < max_gen; gen++) {
    // send and receive data from the previous and next tasks
    // Initiate non-blocking sends
    MPI_Request send_request_start, send_request_end;
    MPI_Isend(&old[start][0][0], n * n, MPI_CHAR, prev_rank, rank, MPI_COMM_WORLD, &send_request_start);
    MPI_Isend(&old[end - 1][0][0], n * n, MPI_CHAR, next_rank, rank, MPI_COMM_WORLD, &send_request_end);

    int real_start = start == 0 ? n : start;  // this is done to avoid negative indexes and "have" a continuous grid
    int real_end = end == n ? 0 : end;

    // Initiate non-blocking receives directly into the old array
    MPI_Request recv_request_start, recv_request_end;
    MPI_Irecv(&old[real_start - 1][0][0], n * n, MPI_CHAR, prev_rank, prev_rank, MPI_COMM_WORLD, &recv_request_start);
    MPI_Irecv(&old[real_end][0][0], n * n, MPI_CHAR, next_rank, next_rank, MPI_COMM_WORLD, &recv_request_end);

    MPI_Request requests[4] = {send_request_start, send_request_end, recv_request_start, recv_request_end};
    MPI_Status statuses[4];

    MPI_Waitall(4, requests, statuses); 

    memset(local_population, 0, sizeof(uint64_t) * (N_SPECIES + 1));
    for (int32_t x = start; x < end; x++) {
      for (int32_t y = 0; y < n; y++) {
        for (int32_t z = 0; z < n; z++) {
          new_val = next_inhabitant(x, y, z, n, old);
          new[x][y][z] = new_val;
          local_population[new_val]++;
          // fprintf(stderr, "next_inhabitant(%d, %d, %d, %d, grid) = %d\n", x, y,
          //         z, n, new[x][y][z]);
        }
      }
    }

    // Gather population statistics from all MPI processes to task 0
    MPI_Reduce(local_population, population, N_SPECIES + 1, MPI_UINT64_T, MPI_SUM, 0, MPI_COMM_WORLD);

    tmp = old;
    old = new;
    new = tmp;

    if (!rank) {
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
    }
    memset(population, 0, sizeof(uint64_t) * (N_SPECIES + 1));
  }
}

int main(int argc, char *argv[]) {
  int id, p;
  double exec_time;
  MPI_Init (&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &id);
  MPI_Comm_size (MPI_COMM_WORLD, &p);
  Args args = parse_args(argc, argv);
  char ***grid = gen_initial_grid(args.n, args.density, args.seed);

  prepare(args.n);

  MPI_Barrier (MPI_COMM_WORLD);
  exec_time = - MPI_Wtime();

  simulation(args.n, args.gen_count, grid, id, p);

  exec_time += MPI_Wtime();
  
  MPI_Barrier (MPI_COMM_WORLD);
  finish(id);

  if (!id) {
    fprintf(stderr, "Took: %.1fs\n", exec_time);
  }

  MPI_Finalize();
  return EXIT_SUCCESS;
}