/***************************************************************************
 *            dpevaluation.c
 *
 *  Fri Mar 23 15:42:14 2012
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
#include "dpevaluation.h"
#include "dpindivid.h"
#include "dppopulation.h"

DpEvaluationCtrl*dp_evaluation_ctrl_new()
{
	DpEvaluationCtrl*ec;
	ec = (DpEvaluationCtrl*)malloc(sizeof(DpEvaluationCtrl));
	ec->kount = 0;
	ec->roundoff_error = 1e-8;
	ec->eval = NULL;
	ec->eval_target = NULL;
	return ec;
}

DpEvaluationCtrl*dp_evaluation_ctrl_init(int worldid, int seed, double gamma_init, double roundoff_error, DpEvaluationStrategy eval_strategy)
{
	DpEvaluationCtrl*hevalctrl;
	hevalctrl = dp_evaluation_ctrl_new();
	hevalctrl->seed = seed;
	hevalctrl->yoffset = worldid;
	hevalctrl->gamma_init = gamma_init;
	hevalctrl->roundoff_error = roundoff_error;
	hevalctrl->eval_strategy = eval_strategy;
	return hevalctrl;
}

DpEvaluationCtrl*dp_evaluation_init(DpEvaluation*heval, DpTarget*htarget, int worldid, int seed, double gamma_init, double roundoff_error, DpEvaluationStrategy eval_strategy)
{
	int i;
	DpEvaluationCtrl*hevalctrl;
	hevalctrl = dp_evaluation_ctrl_init(worldid, seed, gamma_init, roundoff_error, eval_strategy);
	hevalctrl->eval = heval;
	hevalctrl->eval_target = htarget;
	for ( i = 0; i < hevalctrl->eval->size; i++) {
		if ( hevalctrl->eval->points[i]->limited ) {
			hevalctrl->eval->points[i]->upper -= hevalctrl->roundoff_error;
			hevalctrl->eval->points[i]->lower += hevalctrl->roundoff_error;
			hevalctrl->eval->points[i]->alpha = 0.5 * (hevalctrl->eval->points[i]->upper + hevalctrl->eval->points[i]->lower);
			hevalctrl->eval->points[i]->beta = 0.5 * (hevalctrl->eval->points[i]->upper - hevalctrl->eval->points[i]->lower);
			hevalctrl->eval->points[i]->gamma = hevalctrl->gamma_init;
		}
	}
	return hevalctrl;
}

void dp_evaluation_individ_evaluate_prime(DpEvaluationCtrl*hevalctrl, DpIndivid*individ, DpIndivid*tabu, int index, double cost)
{
	int max_value_flag = 0;
	individ->invalid = 1;
	dp_evaluation_individ_prepare(hevalctrl, individ);
	dp_target_eval_update_user_data(hevalctrl->eval_target, individ->user_data, tabu->z, index, cost);
	max_value_flag = dp_target_eval_prime (hevalctrl->eval_target, individ->z, &(individ->invalid), individ->y, individ->user_data, index, cost);
}

void dp_evaluation_individ_copy(DpEvaluationCtrl*hevalctrl, DpIndivid*individ, DpIndivid*tabu, int index, double cost)
{
	dp_evaluation_individ_prepare(hevalctrl, individ);
	dp_target_eval_update_user_data(hevalctrl->eval_target, individ->user_data, tabu->z, index, cost);
	dp_evaluation_copy_to_param(hevalctrl, individ->z);
}

void dp_evaluation_individ_evaluate(DpEvaluationCtrl*hevalctrl, DpIndivid*individ, DpIndivid*tabu, int index, double cost)
{
	int max_value_flag = 0;
	individ->invalid = 1;
	dp_evaluation_individ_prepare(hevalctrl, individ);
	dp_target_eval_update_user_data(hevalctrl->eval_target, individ->user_data, tabu->z, index, cost);
	max_value_flag = dp_target_eval (hevalctrl->eval_target, individ->z, &(individ->invalid), &(individ->cost), individ->targets, individ->precond, individ->user_data, index, cost);
}

void dp_evaluation_individ_evaluate_precond(DpEvaluationCtrl*hevalctrl, DpIndivid*individ, DpIndivid*tabu, int index, double cost)
{
	int max_value_flag = 0;
	individ->invalid = 1;
	dp_evaluation_individ_prepare(hevalctrl, individ);
	dp_target_eval_update_user_data(hevalctrl->eval_target, individ->user_data, tabu->z, index, cost);
	max_value_flag = dp_target_eval_precond (hevalctrl->eval_target, individ->z, &(individ->invalid), individ->precond, individ->user_data, index, cost);
}

int dp_evaluation_individ_compare(const void *p1, const void *p2, void *user_data)
{
	int i, need_swap;
	double u;
	DpIndivid**i1 = (DpIndivid**)p1;
	DpIndivid**i2 = (DpIndivid**)p2;
	DpIndivid*individ = *i1;
	DpIndivid*trial = *i2;
	DpEvaluationCtrl*hevalctrl = (DpEvaluationCtrl*)user_data;
	need_swap = 0;
	if ( individ->invalid == 1 && trial->invalid == 0 ) {
		need_swap = 1;
	} else if ( trial->cost < individ->cost ) {
		need_swap = 1;
	} else {
		need_swap = -1;
		for ( i = 0; i < hevalctrl->eval_target->size; i++ ) {
			if ( trial->targets[i] < individ->targets[i] && g_rand_double(individ->hrand) < hevalctrl->eval_target->penalty[i]->rank ) {
				need_swap = 1;
				break;
			}
		}
	}
	return need_swap;
}


/*********************************************************************
 ** gaussian random: normal distribution                            **
 ** N(mean, dev)                                                    **
 **                                                                 **
 ** void ShareNormalRandVec(s, n, mean, dev)                        **
 ** return s=vec(n)                                                 **
 **                                                                 **
 ** source code from                                                **
 **        http://remus.rutgers.edu/~rhoads/Code/code.html          **
 ** taken from Numerical Recipes in C p.289                         **
 *********************************************************************/

double dp_rand_gauss(GRand*hrand, double mean, double dev)
{
	static double V2, fac;
	static int phase = 0;
	double S, Z, U1, U2, V1;
	if ( phase == 1 ) {
		Z = V2 * fac;
	} else {
		do {
			U1 = g_rand_double(hrand);
			U2 = g_rand_double(hrand);
			V1 = 2 * U1 - 1;
			V2 = 2 * U2 - 1;
			S = V1 * V1 + V2 * V2;
		} while ( S >= 1 || S == 0.0 );
		fac = sqrt (-2 * log(S) / S);
		Z = V1 * fac;
	}
	phase = 1 - phase;
	Z = mean + dev*Z;
	return Z;
}

void dp_evaluation_individ_prepare(DpEvaluationCtrl*hevalctrl, DpIndivid*individ)
{
	int i;
	double y;
	for ( i = 0; i < hevalctrl->eval->size; i++) {
		if ( hevalctrl->eval->points[i]->limited ) {
			if ( hevalctrl->eval_strategy == sin_trans_flag ) {
				y = hevalctrl->eval->points[i]->alpha + hevalctrl->eval->points[i]->beta * sin(hevalctrl->eval->points[i]->gamma * individ->x[i]);
				y /= hevalctrl->eval->points[i]->scale;
				individ->z[i] = y;
			} else if ( hevalctrl->eval_strategy == tanh_trans_flag ) {
				y = hevalctrl->eval->points[i]->alpha + hevalctrl->eval->points[i]->beta * tanh(hevalctrl->eval->points[i]->gamma * individ->x[i]);
				y /= hevalctrl->eval->points[i]->scale;
				individ->z[i] = y;
			} else if ( hevalctrl->eval_strategy == alg_trans_flag ) {
				if ( individ->x[i] > hevalctrl->eval->points[i]->lower && individ->x[i] < hevalctrl->eval->points[i]->upper ) {
					individ->z[i] = individ->x[i] / hevalctrl->eval->points[i]->scale;
				} else {
					individ->x[i] = hevalctrl->eval->points[i]->lower + 2.0 * g_rand_double(individ->hrand) * hevalctrl->eval->points[i]->beta;
					individ->z[i] = individ->x[i] / hevalctrl->eval->points[i]->scale;
				}
			} else if ( hevalctrl->eval_strategy == rand_trans_flag ) {
				while ( individ->x[i] < hevalctrl->eval->points[i]->lower || individ->x[i] > hevalctrl->eval->points[i]->upper ) {
					individ->x[i] = hevalctrl->eval->points[i]->alpha + dp_rand_gauss(individ->hrand, 0, 1) * hevalctrl->eval->points[i]->beta;
				}
				individ->z[i] = individ->x[i] / hevalctrl->eval->points[i]->scale;
			}
		} else {
			individ->z[i] = individ->x[i] / hevalctrl->eval->points[i]->scale;
		}
	}
}

void dp_evaluation_copy_to_grad(DpEvaluationCtrl*hevalctrl, double*x)
{
	int i;
	for ( i = 0; i < hevalctrl->eval->size; i++) {
		(*(hevalctrl->eval->points[i]->grad)) = x[i];
	}
}

void dp_evaluation_copy_to_param(DpEvaluationCtrl*hevalctrl, double*x)
{
	int i;
	for ( i = 0; i < hevalctrl->eval->size; i++) {
		(*(hevalctrl->eval->points[i]->param)) = x[i];
	}
}

void dp_evaluation_individ_scramble(DpEvaluationCtrl*hevalctrl, DpIndivid*individ, double eps)
{
	int i;
	double y, z;
	for ( i = 0; i < hevalctrl->eval->size; i++) {
		if ( eps > 0 ) {
			z = (*(hevalctrl->eval->points[i]->param)) * hevalctrl->eval->points[i]->scale;
			y = g_rand_double_range(individ->hrand, (1.0 - eps) * z, (1.0 + eps) * z);
		} else {
			y = g_rand_double_range(individ->hrand, hevalctrl->eval->points[i]->lower, hevalctrl->eval->points[i]->upper);
		}
		if ( hevalctrl->eval->points[i]->limited ) {
			if ( hevalctrl->eval_strategy == sin_trans_flag )
				individ->x[i] = asin ( ( y - hevalctrl->eval->points[i]->alpha ) / hevalctrl->eval->points[i]->beta) / hevalctrl->eval->points[i]->gamma;
			else if ( hevalctrl->eval_strategy == tanh_trans_flag )
				individ->x[i] = atanh ( ( y - hevalctrl->eval->points[i]->alpha ) / hevalctrl->eval->points[i]->beta) / hevalctrl->eval->points[i]->gamma;
			else if ( hevalctrl->eval_strategy == alg_trans_flag )
				individ->x[i] = y;
			else if ( hevalctrl->eval_strategy == rand_trans_flag )
				individ->x[i] = y;
		} else {
			individ->x[i] = y;
		}
	}
}


void dp_evaluation_individ_set(DpEvaluationCtrl*hevalctrl, DpIndivid*individ)
{
	int i;
	double y;
	for ( i = 0; i < hevalctrl->eval->size; i++) {
		y = (*(hevalctrl->eval->points[i]->param)) * hevalctrl->eval->points[i]->scale;
		if ( hevalctrl->eval->points[i]->limited ) {
			if ( hevalctrl->eval_strategy == sin_trans_flag )
				individ->x[i] = asin ( ( y - hevalctrl->eval->points[i]->alpha ) / hevalctrl->eval->points[i]->beta) / hevalctrl->eval->points[i]->gamma;
			else if ( hevalctrl->eval_strategy == tanh_trans_flag )
				individ->x[i] = atanh ( ( y - hevalctrl->eval->points[i]->alpha ) / hevalctrl->eval->points[i]->beta) / hevalctrl->eval->points[i]->gamma;
			else if ( hevalctrl->eval_strategy == alg_trans_flag )
				individ->x[i] = y;
			else if ( hevalctrl->eval_strategy == rand_trans_flag )
				individ->x[i] = y;
		} else {
			individ->x[i] = y;
		}
	}
}

void dp_evaluation_individ_set_grad(DpEvaluationCtrl*hevalctrl, DpIndivid*individ)
{
	int i;
	double y, z, w;
	for ( i = 0; i < hevalctrl->eval->size; i++) {
		y = (*(hevalctrl->eval->points[i]->param)) * hevalctrl->eval->points[i]->scale;
		z = (*(hevalctrl->eval->points[i]->grad)) * hevalctrl->eval->points[i]->scale;
		if ( hevalctrl->eval->points[i]->limited ) {
			if ( hevalctrl->eval_strategy == sin_trans_flag )
				individ->y[i] = z * hevalctrl->eval->points[i]->gamma * hevalctrl->eval->points[i]->beta * cos ( individ->x[i] * hevalctrl->eval->points[i]->gamma );
			else if ( hevalctrl->eval_strategy == tanh_trans_flag ) {
				w = tanh( individ->x[i] * hevalctrl->eval->points[i]->gamma );
				individ->y[i] = z * hevalctrl->eval->points[i]->gamma * hevalctrl->eval->points[i]->beta * ( 1 - w * w );
			} else if ( hevalctrl->eval_strategy == alg_trans_flag )
				individ->y[i] = z;
			else if ( hevalctrl->eval_strategy == rand_trans_flag )
				individ->y[i] = z;
		} else {
			individ->y[i] = z;
		}
	}
}

DpPopulation*dp_evaluation_population_init(DpEvaluationCtrl*hevalctrl, int size, double noglobal_eps)
{
	DpPopulation*pop;
	int i;
	pop = dp_population_new(size, hevalctrl->eval->size, hevalctrl->eval_target->size, hevalctrl->eval_target->precond_size, hevalctrl->seed + hevalctrl->yoffset);
	dp_evaluation_individ_set(hevalctrl, pop->individ[0]);
	pop->individ[0]->user_data = dp_target_eval_get_user_data(hevalctrl->eval_target);
	dp_evaluation_individ_evaluate(hevalctrl, pop->individ[0], pop->individ[0], 0, G_MAXDOUBLE);
	for ( i = 1; i < size; i++) {
		pop->individ[i]->user_data = dp_target_eval_get_user_data(hevalctrl->eval_target);
		dp_evaluation_individ_scramble(hevalctrl, pop->individ[i], noglobal_eps);
		dp_evaluation_individ_evaluate(hevalctrl, pop->individ[i], pop->individ[i], i, G_MAXDOUBLE);
	}
	dp_population_update(pop, 0, pop->size);
	return pop;
}

DpIndivid*dp_evaluation_individ_init(DpEvaluationCtrl*hevalctrl)
{
	DpIndivid*individ;
	individ = dp_individ_new(hevalctrl->eval->size, hevalctrl->eval_target->size, hevalctrl->eval_target->precond_size, hevalctrl->seed + hevalctrl->yoffset);
	dp_evaluation_individ_set(hevalctrl, individ);
	individ->user_data = dp_target_eval_get_user_data(hevalctrl->eval_target);
	dp_evaluation_individ_evaluate(hevalctrl, individ, individ, 0, G_MAXDOUBLE);
	return individ;
}

