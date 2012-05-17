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

typedef double (*DpFunc)(void *user_data);

typedef double (*DpFuncPrime)(void *user_data);

typedef struct DpTargetFunc {
	char*name;
	DpFunc f;
	DpFuncPrime f_prime;
	void *user_data;
	int kount;
	double value;
	double retval;
	double weight;
	double rank;
	int index;
	int invalid;
} DpTargetFunc;

DpTargetFunc*dp_target_func_new(int index, double weight, double rank, char*sname);

typedef struct DpTarget {
	int precond_size;
	DpTargetFunc**precond;
	DpTargetFunc*target;
	int size;
	DpTargetFunc**penalty;
	int invalid;
	double value;
} DpTarget;

DpTarget*dp_target_new();

void dp_target_add_func (DpTarget*htarget, int index, double weight, double rank, char *sname);

void dp_target_insert_func (DpTarget*htarget, DpTargetFunc*func, void *user_data);

int dp_target_eval (DpTarget*htarget);

int dp_target_eval_precond (DpTarget*htarget);

#ifdef __cplusplus
}
#endif

#endif /* _DP_TARGET_H */
