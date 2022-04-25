#include "dparchive.h"

#include <time.h>


DpArchive* dp_archive_new(int size){
    DpArchive* archive = malloc(sizeof(DpArchive));
    archive->difference_vectors = (DifferenceVector*)malloc(sizeof(DifferenceVector) * 2 * size);
    archive->last_index = size;
    archive->max_size = size * 2;
    archive->iter = 0;
    archive->generation = 0;
    return archive;
}

DpArchive* dp_archive_init(DpArchive* archive){
    DpArchive* newArchive = dp_archive_new(archive->last_index);
    newArchive->difference_vectors = archive->difference_vectors;
    newArchive->last_index = archive->last_index;
    newArchive->max_size = archive->max_size;
    newArchive->iter = archive->iter;
    return archive;
}

void dp_archive_delete(DpArchive* archive){
    free(archive->difference_vectors);
}

void add_difference_vector(DpArchive* archive, DifferenceVector new_vector){
    archive->difference_vectors[archive->last_index] = new_vector;
    archive->last_index++;
}

void shuffle_archive(DpArchive *oldArchive) {

    srand(time(NULL));
    for (int i = oldArchive->max_size - 1; i >= 1; i--) {
        int j = rand() % (i + 1);
        DifferenceVector tmp = oldArchive->difference_vectors[j];
        oldArchive->difference_vectors[j] = oldArchive->difference_vectors[i];
        oldArchive->difference_vectors[i] = tmp;
    }
}
