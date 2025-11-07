#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

void processFiles(char ** entries, int rank, int count) {
    for (int i = 0; i < count; i++) {
        // process entries[start_index + i]
        fprintf(stdout, "Rank %d processing entry: %s\n", rank, entries[rank*count + i]);


        int fd = open(entries[rank*count + i], O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "Rank %d failed to open file %s: %s\n", rank, entries[rank*count + i], strerror(errno));
            continue;
        }


        close(fd);

    }
}
