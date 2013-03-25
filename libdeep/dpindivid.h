/***************************************************************************
 *            dpindivid.h
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
 
#ifndef _DP_INDIVID_H
#define _DP_INDIVID_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
//#include <glib/gthread.h>

typedef struct DpIndivid {
	int size;
	double cost;
	int age;
	double *x;
	double *y;
	double *z;
	double *targets;
	int ntargets;
	double *precond;
	int nprecond;
	GRand*hrand;
	int invalid;
	gpointer user_data;
	GMutex *gmutex;
} DpIndivid;

DpIndivid*dp_individ_new(int size, int targets_size, int precond_size, int seed);

void dp_individ_dump(DpIndivid*individ);

void dp_individ_unpack(DpIndivid*individ, double*buffer2recv, int bufferDim);

void dp_individ_pack(DpIndivid*individ, double**buffer2send, int*bufferDim);

void dp_individ_copy_values(DpIndivid*individ, DpIndivid*trial);

void dp_individ_delete(DpIndivid*individ);

void dp_individ_save(FILE*fp, DpIndivid*individ);

void dp_individ_load(FILE*fp, DpIndivid*individ);

#ifdef __cplusplus
}
#endif

#endif /* _DP_INDIVID_H */
