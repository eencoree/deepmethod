#ifndef DPARCHIVE_H
#define DPARCHIVE_H

#endif // DPARCHIVE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct{
    double value;
    int generation;
} DifferenceVector;

typedef struct DpArchive{
    int max_size;
    int last_index;
    int iter;
    int generation;
    DifferenceVector* difference_vectors;
} DpArchive;

DpArchive* dp_archive_new(int size);

DpArchive* dp_archive_init(DpArchive* archive);

void dp_archive_delete(DpArchive* archive);

void add_difference_vector(DpArchive* archive, DifferenceVector new_vector);

void shuffle_archive(DpArchive* oldArchive);
