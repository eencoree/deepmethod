#include "dparchive.h"

#include <time.h>


DpArchive* dp_archive_new(int size, int individSize){
    DpArchive* archive = malloc(sizeof(DpArchive));
    archive->difference_vectors = (DifferenceVector**)malloc(sizeof(DifferenceVector) * 2 * size);
    for (int i = 0; i < size * 2; i++){
        archive->difference_vectors[i] = (DifferenceVector*)malloc(sizeof(DifferenceVector));
        archive->difference_vectors[i]->value = (double*)malloc(sizeof(double) * individSize);
    }
    archive->last_index = size;
    archive->max_size = size * 2;
    archive->iter = 0;
    archive->generation = 0;
    return archive;
}

DpArchive* dp_archive_init(DpPopulation* population){
    DpArchive* newArchive = dp_archive_new(population->size, population->individ[0]->size);
    for (int i = 0; i < population->size * 2; i++){
        int i1, i2;
        i1 = i < population->size ? i : i - population->size;
        i2 = i1 == population->size - 1 ? 0 : i1 + 1;
        for (int j = 0; j < population->individ[0]->size; j++){
            newArchive->difference_vectors[i]->value[j] = population->individ[i1]->x[j] - population->individ[i2]->x[j];
        }
        newArchive->difference_vectors[i]->generation = 0;
    }
    return newArchive;
}

void dp_archive_delete(DpArchive* archive){
    free(archive->difference_vectors);
}

void add_difference_vector(DpArchive* archive, DifferenceVector* new_vector){
    archive->difference_vectors[archive->last_index] = new_vector;
    archive->last_index++;
}

void shuffle_archive(DpArchive *oldArchive) {

    srand(time(NULL));
    for (int i = oldArchive->max_size - 1; i >= 1; i--) {
        int j = rand() % (i + 1);
        DifferenceVector* tmp = oldArchive->difference_vectors[j];
        oldArchive->difference_vectors[j] = oldArchive->difference_vectors[i];
        oldArchive->difference_vectors[i] = tmp;
    }
}
