/***************************************************************************
 *            dppopulation.c
 *
 *  Fri Mar 23 15:30:20 2012
 *  Copyright  2012  Konstantin Kozlov
 *  <kozlov@spbcas.ru>
 ****************************************************************************/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef MPI
#include <mpi.h>
#endif

#include "dpindivid.h"
#include "dppopulation.h"

DpPopulation*dp_population_new(int size, int ind_size, int targets_size, int precond_size, int seed)
{
	int i;
	DpPopulation*pop;
	pop = (DpPopulation*)malloc(sizeof(DpPopulation));
	pop->size = size;
	pop->individ = (DpIndivid**)calloc(size, sizeof(DpIndivid*));
	pop->mean = (double*)calloc(ind_size, sizeof(double));
	pop->variance = (double*)calloc(ind_size, sizeof(double));
	pop->cost_ascending = (int*)calloc(size, sizeof(int));
	pop->ages_descending = (int*)calloc(size, sizeof(int));
	pop->ind_size = ind_size;
	for ( i = 0; i < size; i++ ) {
		pop->individ[i] = dp_individ_new(ind_size, targets_size, precond_size, seed + i);
		pop->cost_ascending[i] = i;
		pop->ages_descending[i] = i;
	}
	pop->imin = 0;
	pop->dmin = -1;
	pop->iage = 0;
	pop->aage = 0;
	pop->iter = 0;
	return pop;
}

void dp_population_save(FILE*fp, DpPopulation*pop)
{
	int i;
	for ( i = 0; i < pop->size; i++ ) {
	  dp_individ_save(fp, pop->individ[i]);
	  fprintf(fp, "%d\n", pop->cost_ascending[i]);
	  fprintf(fp, "%d\n", pop->ages_descending[i]);
	}
	for ( i = 0; i < pop->ind_size; i++ ) {
		fprintf(fp, "%13.9f ", pop->mean[i]);
	}
	fprintf(fp, "\n");
	for ( i = 0; i < pop->ind_size; i++ ) {
		fprintf(fp, "%13.9f ", pop->variance[i]);
	}
	fprintf(fp, "\n");
	fprintf(fp, "%d\n", pop->imin);
	fprintf(fp, "%13.9f\n", pop->dmin);
	fprintf(fp, "%d\n", pop->iage);
	fprintf(fp, "%d\n", pop->aage);
	fprintf(fp, "%d\n", pop->iter);
}

void dp_population_load(FILE*fp, DpPopulation*pop)
{
	int i;
	for ( i = 0; i < pop->size; i++ ) {
	  dp_individ_load(fp, pop->individ[i]);
	  fscanf(fp, "%d", &(pop->cost_ascending[i]));
	  fscanf(fp, "%d", &(pop->ages_descending[i]));
	}
	for ( i = 0; i < pop->ind_size; i++ ) {
		fscanf(fp, "%lf", &(pop->mean[i]));
	}
	for ( i = 0; i < pop->ind_size; i++ ) {
		fscanf(fp, "%lf", &(pop->variance[i]));
	}
	fscanf(fp, "%d", &(pop->imin));
	fscanf(fp, "%lf", &(pop->dmin));
	fscanf(fp, "%d", &(pop->iage));
	fscanf(fp, "%d", &(pop->aage));
	fscanf(fp, "%d", &(pop->iter));
}

void dp_population_delete(DpPopulation*pop)
{
	int i;
	for ( i = 0; i < pop->size; i++ ) {
		dp_individ_delete(pop->individ[i]);
	}
	free(pop->individ);
	free(pop->mean);
	free(pop->variance);
	free(pop);
}

void dp_population_update(DpPopulation*pop, int start_index, int end_index)
{
	int i, j, n;
	double smean, smean2, s;
	double fmin;
	int amax, imax, imin;
	fmin = pop->individ[start_index]->cost;
	imin = start_index;
	amax = pop->individ[start_index]->age;
	imax = start_index;
	n = end_index - start_index + 1;
	for ( j = start_index + 1; j < end_index; j++) {
		if ( pop->individ[j]->cost < fmin ) {
			fmin = pop->individ[j]->cost;
			imin = j;
		}
	}
	if ( imax == imin )
		imax = end_index - 1;
	for ( j = start_index + 1; j < end_index; j++) {
		if ( j != imin && pop->individ[j]->age > amax ) {
			amax = pop->individ[j]->age;
			imax = j;
		}
	}
	for ( i = 0; i < pop->ind_size; i++) {
		smean = 0;
		smean2 = 0;
		for ( j = start_index; j < end_index; j++) {
			s = pop->individ[j]->x[i];
			smean += s;
			smean2 += s * s;
		}
		smean /= n;
		smean2 /= n;
		pop->mean[i] = smean;
		pop->variance[i] = ( smean2 - smean * smean ) / ( n - 1 ) ;
	}
	pop->imin = imin;
	pop->dmin = fmin;
	pop->iage = imax;
	pop->aage = amax;
}

void dp_population_nbest_pack(DpPopulation*pop, int index, double**buffer2send, int*bufferDim)
{
	int i;
	int j;
	int Dim;
	int *cost_ascending = pop->cost_ascending;
	int ind_size = pop->ind_size;
	int target_size = pop->individ[0]->ntargets;
	Dim = ind_size + 1 + ind_size + 1 + target_size;
	*bufferDim = index * Dim;
	*buffer2send = (double*)calloc(*bufferDim, sizeof(double));
	for ( j = 0; j < index; j++) {
		for( i = 0; i < ind_size; i++ ) {
			(*buffer2send)[j * Dim + i] = pop->individ[cost_ascending[j]]->x[i];
			(*buffer2send)[j * Dim + i + ind_size + 1] = pop->individ[cost_ascending[j]]->y[i];
		}
		(*buffer2send)[j * Dim + ind_size] = pop->individ[cost_ascending[j]]->cost;
		for( i = 0; i < target_size; i++ ) {
			(*buffer2send)[j * Dim + ind_size + 1 + ind_size + 1 + i] = pop->individ[cost_ascending[j]]->targets[i];
		}
	}
}

void dp_population_nbest_unpack(DpPopulation*pop, int index, double*buffer2recv, int bufferDim)
{
	int i;
	int j;
	int Dim;
	int *ages_descending = pop->ages_descending;
	int ind_size = pop->ind_size;
	int target_size = pop->individ[0]->ntargets;
	Dim = ind_size + 1 + ind_size + 1 + target_size;
	for ( j = 0; j < index; j++) {
		for( i = 0; i < ind_size; i++ ) {
			pop->individ[ages_descending[j]]->x[i] = buffer2recv[j * Dim + i];
			pop->individ[ages_descending[j]]->y[i] = buffer2recv[j * Dim + i + ind_size + 1];
		}
		pop->individ[ages_descending[j]]->cost = buffer2recv[j * Dim + ind_size ];
		for( i = 0; i < target_size; i++ ) {
			pop->individ[ages_descending[j]]->targets[i] = buffer2recv[j * Dim + ind_size + 1 + ind_size + 1 + i];
		}
		pop->individ[ages_descending[j]]->age = 0;
	}
}


int dp_individ_ages_descending(void *p1, void *p2, void *user_data)
{
	DpPopulation*population = (DpPopulation*)user_data;
	int*i1 = (int*)p1;
	int*i2 = (int*)p2;
	DpIndivid*individ_1 = population->individ[(*i1)];
	DpIndivid*individ_2 = population->individ[(*i2)];
	if ( individ_1->age < individ_2->age ) {
		return 1;
	} else if ( individ_1->age > individ_2->age ) {
		return -1;
	}
	return 0;
}

int dp_individ_cost_ascending(void *p1, void *p2, void *user_data)
{
	DpPopulation*population = (DpPopulation*)user_data;
	int*i1 = (int*)p1;
	int*i2 = (int*)p2;
	DpIndivid*individ_1 = population->individ[(*i1)];
	DpIndivid*individ_2 = population->individ[(*i2)];
	if ( individ_1->cost > individ_2->cost ) {
		return 1;
	} else if ( individ_1->cost < individ_2->cost ) {
		return -1;
	}
	return 0;
}

void dp_population_mpi_comm_nbest(DpPopulation*pop, int mpi_id, int mpi_nnodes, int *frozen, int es_lambda)
{
	int dest;
	int source;
	int sumFrozen;
	int bufferDim;
	double *buffer2send;
	double *buffer2recv;
#ifdef MPI
	MPI_Status *status;       /* status array of the MPI_Waitall function */
	MPI_Request *request;           /* handle array for receiving messages */
	MPI_Allreduce(frozen, &sumFrozen, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	*frozen = ( sumFrozen > 0 );
#endif
	if ( (*frozen) == 0 ) {
#ifdef MPI
		request = (MPI_Request *)calloc(1, sizeof(MPI_Request));
		status =  (MPI_Status *)calloc(1, sizeof(MPI_Status));
#endif
		dp_population_nbest_pack(pop, es_lambda, &buffer2send, &bufferDim);
#ifdef MPI
		dest = mpi_id + 1;
		if ( dest >= mpi_nnodes ) {
			dest = 0;
		}
		source = mpi_id - 1;
		if ( source < 0 ) {
			source = mpi_nnodes - 1;
		}
#endif

#ifdef MPI
		buffer2recv = (double*)calloc(bufferDim, sizeof(double));
#else
		buffer2recv = buffer2send;
#endif
#ifdef MPI
		MPI_Irecv(buffer2recv, bufferDim, MPI_DOUBLE, source, source, MPI_COMM_WORLD, &request[0]);
		MPI_Send(buffer2send, bufferDim, MPI_DOUBLE, dest, mpi_id, MPI_COMM_WORLD);
		MPI_Waitall(1, request, status);
#endif
		dp_population_nbest_unpack(pop, es_lambda, buffer2recv, bufferDim);
#ifdef MPI
		free(buffer2recv);
#endif
		free(buffer2send);
#ifdef MPI
		free(request);
		free(status);
#endif
	}
}
