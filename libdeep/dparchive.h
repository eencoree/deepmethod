#ifndef DPARCHIVE_H
#define DPARCHIVE_H

#ifdef __cplusplus
extern "C"
{
#endif


#include <stdio.h>
#include <stdlib.h>
#include <dppopulation.h>

typedef struct {
    double* value;
    int generation;
} DifferenceVector;

typedef struct DpArchive{
    int max_size;
    int last_index;
    int iter;
    int generation;
    DifferenceVector** difference_vectors;
} DpArchive;

DpArchive* dp_archive_new(int size, int individSize);

DpArchive* dp_archive_init(DpPopulation* population);

void dp_archive_delete(DpArchive* archive);

void add_difference_vector(DpArchive* archive, DifferenceVector* new_vector);

void shuffle_archive(DpArchive* oldArchive);

#ifdef __cplusplus
}
#endif

#endif /* _DP_RECOMBINATION_H */
