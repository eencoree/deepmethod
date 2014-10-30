/***************************************************************************
 *            dppopulation.h
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


#ifndef _DP_POPULATION_H
#define _DP_POPULATION_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include "dpindivid.h"

typedef struct DpPopulation {
	int size;
	int iter;
	int imin;
	int target_index;
	double dmin;
	int iage;
	int aage;
	int ind_size;
	double *mean;
	double *variance;
	DpIndivid**individ;
	int *ages_descending;
	int *cost_ascending;
	int nfronts;
	GArray*fronts;
} DpPopulation;

DpPopulation*dp_population_new(int size, int ind_size, int targets_size, int precond_size, int seed);

void dp_population_delete(DpPopulation*pop);

void dp_population_update(DpPopulation*pop, int start_index, int end_index);

int dp_individ_ages_descending(void *p1, void *p2, void *user_data);

int dp_individ_cost_ascending(void *p1, void *p2, void *user_data);

void dp_population_mpi_distribute(DpPopulation*pop, int mpi_id, int mpi_nnodes);

void dp_population_mpi_gather(DpPopulation*pop, int mpi_id, int mpi_nnodes);

void dp_population_save(FILE*fp, DpPopulation*pop);

void dp_population_load(FILE*fp, DpPopulation*pop);

void dp_population_cr2cost(DpPopulation*pop);

DpPopulation*dp_population_union(DpPopulation*population, DpPopulation*trial);

#ifdef __cplusplus
}
#endif

#endif /* _DP_POPULATION_H */

