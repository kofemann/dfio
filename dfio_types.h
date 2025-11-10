#ifndef DFIO_TYPES_H
#define DFIO_TYPES_H

/**
 * requests statistics record
 */
typedef struct stats {
  double avg;
  double min;
  double max;
  double err;
  int count;
} stats_t;

#endif