#include <dirent.h>
#include <string.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * Free directory entries list.
 */
void freeDirectoryList(char **entries, int count) {

  if (entries) {
    for (int i = 0; i < count; i++) {
      free(entries[i]);
    }
    free(entries);
  }
}

/**
 * List directory entries excluding . and ..
 */
char **listDirectory(const char *path, int *out_count) {

  struct dirent *dir;
  char **entries = NULL;
  int count = 0;
  int max_count = 128;

  entries = malloc(sizeof(char *) * max_count);
  if (entries == NULL) {
    *out_count = -errno;
    return NULL;
  }

  DIR *d = opendir(path);
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
        if (count >= max_count) {
          max_count *= 2;
          entries = realloc(entries, sizeof(char *) * max_count);
          if (entries == NULL) {
            *out_count = -errno;
            freeDirectoryList(entries, count);
            return NULL;
          }
        }

        entries[count] = malloc(strlen(path) + strlen(dir->d_name) +
                                2); // +2 for '/' and '\0'
        if (entries[count] == NULL) {
          *out_count = -errno;
          freeDirectoryList(entries, count);
          return NULL;
        }
        sprintf(entries[count], "%s/%s", path, dir->d_name);
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