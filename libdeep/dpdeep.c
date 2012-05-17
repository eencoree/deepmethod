/***************************************************************************
 *            dpdeep.c
 *
 *  Fri Mar 23 15:45:54 2012
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
#include "dpdeep.h"

DpDeepInfo *dp_deep_info_new (int population_size, double recombination_weight, double recombination_prob, double recombination_gamma, double es_lambda, double noglobal_eps)
{
	DpDeepInfo*hdeepinfo;
	hdeepinfo = (DpDeepInfo*)malloc(sizeof(DpDeepInfo));
	hdeepinfo->population_size = population_size;
	hdeepinfo->recombination_weight = recombination_weight;
	hdeepinfo->recombination_prob = recombination_prob;
	hdeepinfo->recombination_gamma = recombination_gamma;
	hdeepinfo->es_lambda = es_lambda;
	hdeepinfo->noglobal_eps = noglobal_eps;
	return hdeepinfo;
}

DpDeepInfo *dp_deep_info_init(DpEvaluation*heval, DpTarget*htarget, int worldid, int seed, double gamma_init, double roundoff_error, DpEvaluationStrategy eval_strategy, int population_size, double recombination_weight, double recombination_prob, double recombination_gamma, double es_lambda, double noglobal_eps, DpRecombinationStrategy recomb_strategy)
{
	DpDeepInfo*hdeepinfo = dp_deep_info_new(population_size, recombination_weight, recombination_prob, recombination_gamma, es_lambda, noglobal_eps);
	DpRecombinationStrategy strategy;
	hdeepinfo->hevalctrl = dp_evaluation_init(heval, htarget, worldid, seed, gamma_init, roundoff_error, eval_strategy);
	hdeepinfo->trial = dp_individ_new(hdeepinfo->hevalctrl->eval->size, hdeepinfo->hevalctrl->eval_target->size);
	hdeepinfo->population = dp_evaluation_population_init(hdeepinfo->hevalctrl, hdeepinfo->population_size, hdeepinfo->noglobal_eps);
	hdeepinfo->recombination_control = dp_recombination_control_init(recomb_strategy, hdeepinfo->population, hdeepinfo->hevalctrl->hrand, hdeepinfo->recombination_weight, hdeepinfo->recombination_prob, hdeepinfo->recombination_gamma);
	return hdeepinfo;
}

void dp_deep_step(DpDeepInfo*hdeepinfo)
{
	int i, r1, r2, r3, r4;
	int start_index, end_index;
	DpIndivid *trial = hdeepinfo->trial;
	DpPopulation*population = hdeepinfo->population;
	DpRecombinationControl *recombination_control = hdeepinfo->recombination_control;
	GRand*hrand = hdeepinfo->hevalctrl->hrand;
	population->n_accepted = 0;
	for ( i = 0; i < population->size; i++ ) {
		r1 = population->imin;
		do {
			r2 = g_rand_int_range (hrand, 0, hdeepinfo->population_size);
		} while ( r2 == i || r2 == r1 );
		do {
			r3 = g_rand_int_range (hrand, 0, hdeepinfo->population_size);
		} while ( r3 == i || r3 == r1 || r3 == r2 );
		do {
			r4 = g_rand_int_range (hrand, 0, hdeepinfo->population_size);
		} while ( r4 == i || r4 == r1 || r4 == r2 || r4 == r3 );
		start_index = g_rand_int_range (hrand, 0, population->ind_size);
		end_index = trial->size;
		dp_individ_copy_values(trial, population->individ[i]);
		dp_individ_recombination(recombination_control, hrand, trial, population->individ[r1], population->individ[r2], population->individ[r3], population->individ[r4], start_index, end_index);
		dp_evaluation_individ_evaluate(hdeepinfo->hevalctrl, trial);
		if ( i == population->imin ) {
			if ( trial->cost < population->individ[i]->cost ) {
				dp_individ_copy_values(population->individ[i], trial);
				population->individ[i]->age = 0;
				population->n_accepted++;
			}
		} else if ( 1 == dp_evaluation_individ_compare((const void *)(&(population->individ[i])), (const void *)(&(trial)), (void*)(hdeepinfo->hevalctrl)) ) {
			dp_individ_copy_values(population->individ[i], trial);
			population->individ[i]->age = 0;
			population->n_accepted++;
		} else {
			population->individ[i]->age++;
		}
	}
	dp_population_update(population, 0, population->size);
	population->iter++;
}

void dp_deep_accept_step(DpDeepInfo*hdeepinfo, double*value)
{
	*value = hdeepinfo->population->dmin;
}

void dp_deep_post(DpDeepInfo*hdeepinfo)
{
	DpPopulation*pop = hdeepinfo->population;
	int imin = pop->imin;
	double *x = pop->individ[imin]->x;
	dp_evaluation_prepare(hdeepinfo->hevalctrl, x);
}

void dp_deep_post_evaluate(DpDeepInfo*hdeepinfo)
{
	DpPopulation*pop = hdeepinfo->population;
	dp_evaluation_individ_evaluate_precond(hdeepinfo->hevalctrl, pop->individ[pop->imin]);
}

void dp_deep_update_step(DpDeepInfo*hdeepinfo)
{
	dp_recombination_control_update(hdeepinfo->recombination_control, hdeepinfo->hevalctrl->hrand, hdeepinfo->population, 0, hdeepinfo->population->size);
}
