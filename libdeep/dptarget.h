/***************************************************************************
 *            dptarget.h
 *
 *  Fri Mar 23 15:59:35 2012
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

#ifndef _DP_TARGET_H
#define _DP_TARGET_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

typedef double (*DpFunc)(void *user_data, double*x);

typedef gpointer (*DpFuncCopyModel)(gpointer user_data);

typedef void (*DpFuncUpdateModel)(gpointer user_data, double*buffer, int index, double cost0);

typedef GString* (*DpFuncParamsToString)(void *user_data, double*x);

typedef struct DpTargetFunc {
	char*name;
	DpFunc f;
	double weight;
	double rank;
	int index;
} DpTargetFunc;

DpTargetFunc*dp_target_func_new(int index, double weight, double rank, char*sname);


typedef enum DpTargetAggrType {
	DpTargetAggrSum = (1 << 0),
	DpTargetAggrMax  = (1 << 1),
	DpTargetAggrNone = (1 << 2)
} DpTargetAggrType;

typedef struct DpTarget {
	DpTargetAggrType penalty_aggr_type;
	DpTargetAggrType constrain_aggr_type;
	int precond_size;
	DpTargetFunc**precond;
	DpTargetFunc*target;
	int size;
	DpTargetFunc**penalty;
	DpTargetFunc*prime;
	gpointer user_data;
	DpFuncCopyModel copy_model;
	DpFuncUpdateModel update_model;
	DpFuncParamsToString params_to_string;
	int debug;
	int ignore_cost;
	double use_crdist;
} DpTarget;

DpTarget*dp_target_new();

void dp_target_add_func (DpTarget*htarget, int index, double weight, double rank, char *sname);

void dp_target_insert_prime_func (DpTarget*htarget, int index, double weight, double rank, char *sname);

int dp_target_eval (DpTarget*htarget, double*x, int*invalid, double*cost, double*penalty, double*precond, gpointer user_data, int index, double cost0);

int dp_target_eval_precond (DpTarget*htarget, double*x, int*invalid, double*precond, gpointer user_data, int index, double cost0);

gpointer dp_target_eval_get_user_data(DpTarget*htarget);

void dp_target_eval_update_user_data(DpTarget*htarget, gpointer user_data, double*buffer, int index, double cost0);

int dp_target_eval_prime (DpTarget*htarget, double*x, int*invalid, double*prime, gpointer user_data, int index, double cost0);

void dp_target_shift_penalty_weights (DpTarget*htarget);

void dp_target_shift_penalty_ranks (DpTarget*htarget);

int dp_target_init(gchar*filename, gchar*groupname, DpTarget *htarget, GError**err);

#ifdef __cplusplus
}
#endif

#endif /* _DP_TARGET_H */
