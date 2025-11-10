#include <math.h>
#include "dfio_types.h"


void calculate_stats(double *array, int num_elements, stats_t *stats) {

    stats->sum = 0.;
    stats->avg = 0.;
    stats->min = 0.;
    stats->max = 0.;
    stats->err = 0.;
    stats->count = num_elements;

    double dsum = 0.;
    int i;
    for (i = 0; i < num_elements; i++) {

        if (array[i] > stats->max) {
            stats->max = array[i];
        }

        if (array[i] < stats->min || stats->min == 0.) {
            stats->min = array[i];
        }

        stats->sum += array[i];
    }

    stats->avg = stats->sum / num_elements;
    for (i = 0; i < num_elements; i++) {
        double diff = array[i] - stats->avg;
        dsum += diff*diff;
    }
    stats->err = sqrt( dsum/num_elements);
}


/**
 * Init stats with given rate. It initialized as with a single measurement,
 * as MPI call will update it with values from other nodes.
 */
void init_stats(stats_t *stats, double rate) {
    stats->min = stats->max = stats->avg = stats->sum = rate;
    stats->err = 0.;
    stats->count = 1;
}