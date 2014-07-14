/***************************************************************************
 *            dpindivid.c
 *
 *  Fri Mar 23 15:25:40 2012
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
#include "dpindivid.h"

DpIndivid*dp_individ_new(int size, int targets_size, int precond_size, int seed)
{
	DpIndivid*individ;
	individ = (DpIndivid*)malloc(sizeof(DpIndivid));
	individ->size = size;
	individ->cost = 0;
	individ->x = (double*)calloc(size, sizeof(double));
	individ->y = (double*)calloc(size, sizeof(double));
	individ->z = (double*)calloc(size, sizeof(double));
	individ->targets = (double*)calloc(targets_size, sizeof(double));
	individ->ntargets = targets_size;
	individ->precond = (double*)calloc(precond_size, sizeof(double));
	individ->nprecond = precond_size;
	individ->hrand = g_rand_new_with_seed ((guint32)seed);
	individ->age = 0;
	individ->pareto_front = -1;
	individ->dom_count = 0;
	individ->user_data = NULL;
	individ->gmutex = g_mutex_new();
	individ->dominated = NULL;
	individ->crdist = 0;
	return individ;
}

void dp_individ_delete(DpIndivid*individ)
{
	free(individ->x);
	free(individ->y);
	free(individ->z);
	free(individ->targets);
	free(individ->precond);
	g_rand_free(individ->hrand);
	g_mutex_free(individ->gmutex);
	free(individ);
}

void dp_individ_copy_values(DpIndivid*individ, DpIndivid*trial)
{
	int i;
	individ->cost = trial->cost;
	individ->age = trial->age;
	individ->pareto_front = trial->pareto_front;
	individ->dom_count = trial->dom_count;
	individ->crdist = trial->crdist;
	for ( i = 0; i < individ->size; i++ ) {
		individ->x[i] = trial->x[i];
		individ->y[i] = trial->y[i];
		individ->z[i] = trial->z[i];
	}
	for ( i = 0; i < individ->ntargets && i < trial->ntargets; i++ ) {
		individ->targets[i] = trial->targets[i];
	}
	for ( i = 0; i < individ->nprecond && i < trial->nprecond; i++ ) {
		individ->precond[i] = trial->precond[i];
	}
	individ->user_data = trial->user_data;
	individ->hrand = trial->hrand;
}

void dp_individ_save(FILE*fp, DpIndivid*individ)
{
	int i;
	fprintf(fp, "%13.9f\n", individ->cost);
	fprintf(fp, "%d\n", individ->age);
	for ( i = 0; i < individ->size; i++ ) {
	  fprintf(fp, "%13.9f ", individ->x[i]);
	}
	fprintf(fp, "\n");
	for ( i = 0; i < individ->size; i++ ) {
	  fprintf(fp, "%13.9f ", individ->y[i]);
	}
	fprintf(fp, "\n");
	for ( i = 0; i < individ->size; i++ ) {
	  fprintf(fp, "%13.9f ", individ->z[i]);
	}
	fprintf(fp, "\n");
	for ( i = 0; i < individ->ntargets; i++ ) {
	  fprintf(fp, "%13.9f ", individ->targets[i]);
	}
	fprintf(fp, "\n");
	for ( i = 0; i < individ->nprecond; i++ ) {
	  fprintf(fp, "%13.9f ", individ->precond[i]);
	}
	fprintf(fp, "\n");
	/*	individ->hrand = trial->hrand;*/
}

void dp_individ_load(FILE*fp, DpIndivid*individ)
{
	int i;
	fscanf(fp, "%lf", &(individ->cost));
	fscanf(fp, "%d", &(individ->age));
	for ( i = 0; i < individ->size; i++ ) {
	  fscanf(fp, "%lf", &(individ->x[i]));
	}
	for ( i = 0; i < individ->size; i++ ) {
	  fscanf(fp, "%lf", &(individ->y[i]));
	}
	for ( i = 0; i < individ->size; i++ ) {
	  fscanf(fp, "%lf", &(individ->z[i]));
	}
	for ( i = 0; i < individ->ntargets; i++ ) {
	  fscanf(fp, "%lf", &(individ->targets[i]));
	}
	for ( i = 0; i < individ->nprecond; i++ ) {
	  fscanf(fp, "%lf", &(individ->precond[i]));
	}
	/*	individ->hrand = trial->hrand;*/
}

void dp_individ_pack(DpIndivid*individ, double**buffer2send, int*bufferDim)
{
	int i;
	int ind_size = individ->size;
	*bufferDim = ind_size + 1 + ind_size + 1;
	*buffer2send = (double*)calloc(*bufferDim, sizeof(double));
	for( i = 0; i < ind_size; i++ ) {
		(*buffer2send)[i] = individ->x[i];
		(*buffer2send)[i + ind_size + 1] = individ->y[i];
	}
	(*buffer2send)[ind_size] = individ->cost;
}

void dp_individ_unpack(DpIndivid*individ, double*buffer2recv, int bufferDim)
{
	int i;
	int ind_size = individ->size;
	for( i = 0; i < ind_size; i++ ) {
		individ->x[i] = buffer2recv[i];
		individ->y[i] = buffer2recv[i + ind_size + 1];
	}
	individ->cost = buffer2recv[ ind_size ];
	individ->age = 0;
}

void dp_individ_dump(DpIndivid*individ)
{
	int i;
	int ind_size = individ->size;
	fprintf(stdout, "\n%% Individ dump (%d):\n", ind_size);
	for( i = 0; i < ind_size; i++ ) {
		fprintf(stdout, "%%%d %e %e %e\n", i, individ->x[i], individ->y[i], individ->z[i]);
	}
}

