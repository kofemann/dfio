#include <dirent.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dfio.h"

#define OUT_FORMAT "%16s: agg: %.2f MB/s, per thread: %.2f MB/s \u00B1%.2f MB/s, min: %.2f MB/s, max: %.2f MB/s, total number of files: %d.\n"

void usage(const char *progname, int exit_code) {
  printf("Distributed File I/O Benchmark\n");
  printf("\n");
  printf("Usage: %s <dir>\n", progname);
  printf("Example:\n");
  printf("   %s /path/to/data\n", progname);
  printf("\n");
  exit(exit_code);
}

int main(int argc, char *argv[]) {

  int files;

  int rc = 1;
  int res;
  int c;
  double rate;
  stats_t stats;

  double *rates = NULL;

  stats_t global = {0};

  int community_size;
  int rank;

  while ((c = getopt(argc, argv, "f:uw:")) != EOF) {
    switch (c) {
    case '?':
      usage(basename(argv[0]), 0);
    }
  }

  if (((argc - optind) != 1)) {
    usage(basename(argv[0]), 1);
  }

  res = MPI_Init(&argc, &argv);
  if (res != MPI_SUCCESS) {
    fprintf(stderr, "MPI_Init failed\n");
    exit(1);
  }

  res = MPI_Comm_size(MPI_COMM_WORLD, &community_size);
  if (res != MPI_SUCCESS) {
    fprintf(stderr, "MPI_Comm_size failed\n");
    exit(1);
  }

  res = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (res != MPI_SUCCESS) {
    fprintf(stderr, "MPI_Comm_rank failed\n");
    exit(1);
  }

  if (rank == 0) {
    fprintf(stdout, "DFIO Benchmark Initialized. MPI Community size: %d\n",
            community_size);
  }

  char **dir_entries = listDirectory(argv[optind], &files);
  if (dir_entries == NULL) {
    fprintf(stderr, "Failed to list directory %s: %s\n", argv[optind],
            strerror(-files));
    exit(-files);
  }

  if (files < community_size) {
    fprintf(stderr,
            "Not enough entries (%d) in the directory %s for the MPI community "
            "size (%d)\n",
            files, argv[optind], community_size);
    freeDirectoryList(dir_entries, files);
    exit(1);
  }

  int per_rank_chunk = files / community_size;
  if (rank == 0) {
    fprintf(stdout, "Each rank will process %d entries\n", per_rank_chunk);
  }

  MPI_Barrier(MPI_COMM_WORLD);

  stats = processFiles(dir_entries, rank, per_rank_chunk);

  MPI_Barrier(MPI_COMM_WORLD);

  rates = malloc(sizeof(double) * community_size);
  if (rates == NULL) {
    fprintf(stderr, "Rank %d failed to allocate rates array: %s\n", rank,
            strerror(errno));
    freeDirectoryList(dir_entries, files);
    exit(3);
  }

  collect_stats(&stats, &global, MPI_COMM_WORLD, rank, community_size);

  if (rank == 0) {
    fprintf(stdout, OUT_FORMAT, "Read", global.avg * community_size, global.avg,
            global.err, global.min, global.max, global.count);
  }

  res = MPI_Finalize();
  if (res != MPI_SUCCESS) {
    fprintf(stderr, "MPI_Finalize failed\n");
  }

  free(rates);
  freeDirectoryList(dir_entries, files);
  return 0;
}
