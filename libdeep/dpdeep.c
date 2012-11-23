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

DpDeepInfo *dp_deep_info_new (int population_size, double recombination_weight, double recombination_prob, double recombination_gamma, double es_lambda, double noglobal_eps, gint max_threads)
{
	DpDeepInfo*hdeepinfo;
	hdeepinfo = (DpDeepInfo*)malloc(sizeof(DpDeepInfo));
	hdeepinfo->population_size = population_size;
	hdeepinfo->recombination_weight = recombination_weight;
	hdeepinfo->recombination_prob = recombination_prob;
	hdeepinfo->recombination_gamma = recombination_gamma;
	hdeepinfo->es_lambda = es_lambda;
	hdeepinfo->noglobal_eps = noglobal_eps;
	hdeepinfo->exclusive = FALSE;
	hdeepinfo->max_threads = max_threads;
	return hdeepinfo;
}

DpDeepInfo *dp_deep_info_init(DpEvaluation*heval, DpTarget*htarget, int worldid, int seed, double gamma_init, double roundoff_error, DpEvaluationStrategy eval_strategy, int population_size, double recombination_weight, double recombination_prob, double recombination_gamma, double es_lambda, double noglobal_eps, DpRecombinationStrategy recomb_strategy, gint max_threads)
{
	DpDeepInfo*hdeepinfo = dp_deep_info_new(population_size, recombination_weight, recombination_prob, recombination_gamma, es_lambda, noglobal_eps, max_threads);
	DpRecombinationStrategy strategy;
	hdeepinfo->hevalctrl = dp_evaluation_init(heval, htarget, worldid, seed, gamma_init, roundoff_error, max_threads, eval_strategy);
	hdeepinfo->trial = dp_population_new(hdeepinfo->population_size, hdeepinfo->hevalctrl->eval->size, hdeepinfo->hevalctrl->eval_target->size, hdeepinfo->hevalctrl->eval_target->precond_size, hdeepinfo->hevalctrl->seed + hdeepinfo->hevalctrl->yoffset);
	hdeepinfo->population = dp_evaluation_population_init(hdeepinfo->hevalctrl, hdeepinfo->population_size, hdeepinfo->noglobal_eps);
	hdeepinfo->recombination_control = dp_recombination_control_init(recomb_strategy, hdeepinfo->population, hdeepinfo->population->individ[0]->hrand, hdeepinfo->recombination_weight, hdeepinfo->recombination_prob, hdeepinfo->recombination_gamma);
	return hdeepinfo;
}

void dp_deep_step_func (gpointer data, gpointer user_data)
{
	int r1, r2, r3, r4;
	int start_index, end_index;
	DpIndivid*my_tabu;
	DpDeepInfo*hdeepinfo = (DpDeepInfo*)user_data;
	int my_id = GPOINTER_TO_INT(data) - 1;
	DpPopulation*trial = hdeepinfo->trial;
	DpIndivid*my_trial = trial->individ[my_id];
	DpPopulation*population = hdeepinfo->population;
	DpIndivid*my_individ = population->individ[my_id];
	DpRecombinationControl *recombination_control = hdeepinfo->recombination_control;
	GRand*hrand = my_individ->hrand;
	r1 = population->imin;
	do {
		r2 = g_rand_int_range (hrand, 0, hdeepinfo->population_size);
	} while ( r2 == my_id || r2 == r1 );
	do {
		r3 = g_rand_int_range (hrand, 0, hdeepinfo->population_size);
	} while ( r3 == my_id || r3 == r1 || r3 == r2 );
	do {
		r4 = g_rand_int_range (hrand, 0, hdeepinfo->population_size);
	} while ( r4 == my_id || r4 == r1 || r4 == r2 || r4 == r3 );
	my_tabu = population->individ[r1];
	start_index = g_rand_int_range (hrand, 0, population->ind_size);
	end_index = population->ind_size;
	dp_individ_copy_values(my_trial, my_individ);
	my_trial->age = 0;
	dp_individ_recombination(recombination_control, hrand, my_trial, population->individ[r1], population->individ[r2], population->individ[r3], population->individ[r4], start_index, end_index);
	dp_evaluation_individ_evaluate(hdeepinfo->hevalctrl, my_trial, my_tabu, my_id, my_tabu->cost);
	if ( my_id == population->imin ) {
		if ( my_trial->cost >= my_individ->cost ) {
			dp_individ_copy_values(my_trial, my_individ);
			my_trial->age++;
		}
	} else if ( 1 != dp_evaluation_individ_compare((const void *)(&my_individ), (const void *)(&my_trial), (void*)(hdeepinfo->hevalctrl)) ) {
		dp_individ_copy_values(my_trial, my_individ);
		my_trial->age++;
	}
}

void dp_deep_step(DpDeepInfo*hdeepinfo)
{
	int individ_id;
	gboolean immediate_stop = FALSE;
	gboolean wait_finish = TRUE;
	DpPopulation*population = hdeepinfo->population;
	DpPopulation*trial = hdeepinfo->trial;
	GError *gerror = NULL;
	if ( hdeepinfo->max_threads > 0 ) {
		hdeepinfo->gthreadpool = g_thread_pool_new ((GFunc) dp_deep_step_func, (gpointer) hdeepinfo, hdeepinfo->max_threads, hdeepinfo->exclusive, &gerror);
		if ( gerror != NULL ) {
			g_error(gerror->message);
		}
		for ( individ_id = 0; individ_id < population->size; individ_id++ ) {
			g_thread_pool_push (hdeepinfo->gthreadpool, GINT_TO_POINTER(individ_id + 1), &gerror);
			if ( gerror != NULL ) {
				g_error(gerror->message);
			}
		}
		g_thread_pool_free (hdeepinfo->gthreadpool, immediate_stop, wait_finish);
	} else {
		for ( individ_id = 0; individ_id < population->size; individ_id++ ) {
			dp_deep_step_func (GINT_TO_POINTER(individ_id + 1), (gpointer) hdeepinfo);
		}
	}
	dp_population_update(trial, 0, trial->size);
	trial->iter = population->iter + 1;
	hdeepinfo->population = trial;
	hdeepinfo->trial = population;
}

void dp_deep_accept_step(DpDeepInfo*hdeepinfo, double*value)
{
	*value = hdeepinfo->population->dmin;
}

void dp_deep_post(DpDeepInfo*hdeepinfo)
{
	DpPopulation*pop = hdeepinfo->population;
	double *x = pop->individ[pop->imin]->x;
	dp_evaluation_individ_copy(hdeepinfo->hevalctrl, pop->individ[pop->imin], pop->individ[pop->imin], pop->imin, pop->dmin);
}

void dp_deep_post_evaluate(DpDeepInfo*hdeepinfo)
{
	DpPopulation*pop = hdeepinfo->population;
	dp_evaluation_individ_evaluate_precond(hdeepinfo->hevalctrl, pop->individ[pop->imin], pop->individ[pop->imin], pop->imin, pop->dmin);
}

void dp_deep_update_step(DpDeepInfo*hdeepinfo)
{
	dp_recombination_control_update(hdeepinfo->recombination_control, hdeepinfo->population->individ[0]->hrand, hdeepinfo->population, 0, hdeepinfo->population->size);
}

