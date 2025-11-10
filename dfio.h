#ifndef DFIO_H
#define DFIO_H

#include "dfio_types.h"
#include <mpi.h>

extern void init_stats(stats_t *stats, double rate);
extern void calculate_stats(double *array, int num_elements, stats_t *stats);
extern stats_t processFiles(char **entries, int rank, int count);
extern int collect_stats(stats_t *local_stats, stats_t *global_stats,
                         MPI_Comm comm, int rank, int size);
extern void freeDirectoryList(char **entries, int count);
extern char **listDirectory(const char *path, int *out_count);

#endif
