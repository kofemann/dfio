#include "dfio_types.h"
#include <math.h>
#include <mpi.h>

void calculate_stats(double *array, int num_elements, stats_t *stats) {

  stats->avg = 0.;
  stats->min = 0.;
  stats->max = 0.;
  stats->err = 0.;
  stats->count = num_elements;

  double dsum = 0.;
  double sum = 0.;
  int i;
  for (i = 0; i < num_elements; i++) {

    if (array[i] > stats->max) {
      stats->max = array[i];
    }

    if (array[i] < stats->min || stats->min == 0.) {
      stats->min = array[i];
    }

    sum += array[i];
  }

  stats->avg = sum / num_elements;
  for (i = 0; i < num_elements; i++) {
    double diff = array[i] - stats->avg;
    dsum += diff * diff;
  }
  stats->err = sqrt(dsum / num_elements);
}

/*
 Custom MPI reduction operation for combining stats
 */
void stats_reduce_op(void *in, void *inout, int *len, MPI_Datatype *dptr) {
  stats_t *in_stats = (stats_t *)in;
  stats_t *inout_stats = (stats_t *)inout;

  for (int i = 0; i < *len; i++) {
    // Sum the sums and counts
    inout_stats[i].avg += in_stats[i].avg;
    inout_stats[i].count += in_stats[i].count;

    // Track global min/max
    if (in_stats[i].min < inout_stats[i].min)
      inout_stats[i].min = in_stats[i].min;
    if (in_stats[i].max > inout_stats[i].max)
      inout_stats[i].max = in_stats[i].max;

    // Accumulate squared error for standard deviation
    inout_stats[i].err += in_stats[i].err;
  }
}

/*
 Create MPI datatype for stats_t
*/
MPI_Datatype create_stats_type() {
  MPI_Datatype stats_type;
  int blocklengths[2] = {4, 1}; // 4 doubles, 1 int, padding
  MPI_Aint displacements[2];
  MPI_Datatype types[2] = {MPI_DOUBLE, MPI_INT};

  // Calculate displacements
  displacements[0] = offsetof(stats_t, avg);
  displacements[1] = offsetof(stats_t, count);

  MPI_Type_create_struct(2, blocklengths, displacements, types, &stats_type);
  MPI_Type_commit(&stats_type);

  return stats_type;
}

/*
 Collect and aggregate statistics across all nodes
 */
int collect_stats(stats_t *local_stats, stats_t *global_stats, MPI_Comm comm,
                  int rank, int size) {

  // Create custom MPI datatype and operation
  MPI_Datatype stats_type = create_stats_type();
  MPI_Op stats_op;
  MPI_Op_create(stats_reduce_op, 1, &stats_op);

  // Perform reduction
  MPI_Reduce(local_stats, global_stats, 1, stats_type, stats_op, 0, comm);

  // Final averages and standard deviation
  if (rank == 0 && size > 1) {
    global_stats->avg = global_stats->avg / size;
    // Calculate standard deviation if err contains sum of squared differences
    global_stats->err = sqrt(global_stats->err / (global_stats->count - 1));
  }

  // Cleanup
  MPI_Op_free(&stats_op);
  MPI_Type_free(&stats_type);

  return 0;
}
