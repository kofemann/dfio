#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/times.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>

#include "dfio_types.h"

extern void init_stats(stats_t *stats, double rate);
extern void calculate_stats(double *array, int num_elements, stats_t *stats);


/*
 * Get time dela in microseconds between two timevals.
 */
static inline int64_t time_delata(struct timeval *start, struct timeval *end){


    int64_t sec, usec;

	sec = end->tv_sec - start->tv_sec;
	usec = (end->tv_usec - start->tv_usec);
	if (sec > 0 && usec < 0) {
		sec--;
		usec += 1000000;
	}

	if (sec < 0 || (sec == 0 && usec < 0))
		return 0;

	sec *= 1000000;
	return sec + usec;
}

stats_t processFiles(char ** entries, int rank, int count) {

    char io_buffer[4*1024*1024]; // 4MB buffer
    ssize_t bytes_read;
    uint64_t taotal_bytes_read = 0;

    struct timeval start_time, end_time;
    int64_t time_diff;
    double *rates;
    stats_t stats;

    rates = malloc(sizeof(double) * count);
    if (rates == NULL) {
        fprintf(stderr, "Rank %d failed to allocate rates array: %s\n", rank, strerror(errno));
        exit(3);
    }

    for (int i = 0; i < count; i++) {

        taotal_bytes_read = 0;

        const char *fname = entries[rank*count + i];
        int fd = open(fname, O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "Rank %d failed to open file %s: %s\n", rank, fname, strerror(errno));
            continue;
        }

        gettimeofday(&start_time, NULL);

        do {
            bytes_read = read(fd, io_buffer, sizeof(io_buffer));
            if (bytes_read < 0) {
                fprintf(stderr, "Rank %d failed to read file %s: %s\n", rank, fname, strerror(errno));
                break;
            }
            taotal_bytes_read += bytes_read;
        } while (bytes_read > 0);


        gettimeofday(&end_time, NULL);

        close(fd);

        time_diff = time_delata(&start_time, &end_time);

        // bytes per microsecond === MB/s
        rates[i] = (double) taotal_bytes_read /(double)(time_diff);
    }

    calculate_stats(rates, count, &stats);
    free(rates);
    return stats;
}
