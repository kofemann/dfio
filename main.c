#include <sys/stat.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <dirent.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/times.h>
#include <getopt.h>
#include <libgen.h>
#include <errno.h>
#include <mpi.h>


#include "dfio_types.h"

#define OUT_FORMAT "%16s rate: total: %8.2f\t%8.2f rps \u00B1%8.2f, min: %8.2f, max: %8.2f, count: %8d\n"


extern void processFiles(char ** entries, int rank, int count);


void usage(const char* progname, int exit_code) {
    printf("Distributed File I/O Benchmark\n");
    printf("\n");
    printf("Usage: %s <dir>\n", progname);
    printf("Example:\n");
    printf("   %s /path/to/data\n", progname);
    printf("\n");
    exit(exit_code);
}


void freeDirectoryList(char ** entries, int count) {
    for (int i = 0; i < count; i++) {
        free(entries[i]);
    }
    free(entries);
}


/**
 * List directory entries excluding . and ..
 */
char ** listDirectory(const char* path, int* out_count) {

    DIR *d;
    struct dirent *dir;
    char ** entries = NULL;
    int count = 0;
    int max_count = 128;

    entries = malloc(sizeof(char*) * max_count);
    if (entries == NULL) {
        *out_count = -errno;
        return NULL;
    }

    d = opendir(path);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
                if (count >= max_count) {
                    max_count *= 2;
                    entries = realloc(entries, sizeof(char*) * max_count);
                    if (entries == NULL) {
                        *out_count = -errno;
                        freeDirectoryList(entries, count);
                        return NULL;
                    }
                }   
                entries[count] = strdup(dir->d_name);
                count++;
            }
        }

        closedir(d);
    } else {
        *out_count = -errno;
        free(entries);
        return NULL;
    }

    *out_count = count;
    return entries;
}


int main(int argc, char *argv[]) {


    int files;

    int rc = 1;
    int res;
    int c;
    int pid;
    double rate;
    stats_t stats;

    double *rates = NULL;

    // in case of MPI it will be reassigned
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
        fprintf (stderr, "MPI_Init failed\n");
        exit(1);
    }

    res = MPI_Comm_size(MPI_COMM_WORLD, &community_size);
    if (res != MPI_SUCCESS) {
        fprintf (stderr, "MPI_Comm_size failed\n");
        exit(1);
    } 

    res = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (res != MPI_SUCCESS) {
        fprintf (stderr, "MPI_Comm_rank failed\n");
        exit(1);
    }

    if (rank == 0) {
        fprintf(stdout, "DFIO Benchmark Initialized. MPI Community size: %d\n", community_size);
    }

    char ** dir_entries = listDirectory(argv[optind], &files);
    if (dir_entries == NULL) {
        fprintf(stderr, "Failed to list directory %s: %s\n", argv[optind], strerror(-files));
        exit(-files);
    }


    if (files < community_size) {
        fprintf(stderr, "Not enough entries (%d) in the directory %s for the MPI community size (%d)\n",
                files, argv[optind], community_size);
        freeDirectoryList(dir_entries, files);
        exit(1);
    }


    int per_runk_chunk = files / community_size;
    if (rank == 0) {
        fprintf(stdout, "Each rank will process %d entries\n", per_runk_chunk);
    }


    MPI_Barrier(MPI_COMM_WORLD);


    processFiles(dir_entries, rank, per_runk_chunk);

    MPI_Barrier(MPI_COMM_WORLD);
    res = MPI_Finalize();
    if (res != MPI_SUCCESS) {
        fprintf (stderr, "MPI_Finalize failed\n");
    }

    freeDirectoryList(dir_entries, files);
    return 0;
}
