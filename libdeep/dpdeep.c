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

DpDeepInfo *dp_deep_info_new (GKeyFile*gkf, gchar*groupname)
{
	DpDeepInfo*hdeepinfo;
	hdeepinfo = (DpDeepInfo*)malloc(sizeof(DpDeepInfo));
	GError *gerror = NULL;
	gchar*str,**strlist;
	int retval = 0;
	hdeepinfo->population_size = 5;
	hdeepinfo->es_lambda = 2;
	hdeepinfo->es_cutoff = 2;
	hdeepinfo->es_kind = 0;
	hdeepinfo->noglobal_eps = 0;
	hdeepinfo->substeps = 0;
	hdeepinfo->substieps = 0;
	hdeepinfo->mean_cost = 0;
	hdeepinfo->ca_flag = 0;
	if ( ( str = g_key_file_get_string(gkf, groupname, "population_size", &gerror) ) != NULL ) {
		hdeepinfo->population_size = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_debug ("%s", gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "es_lambda", &gerror) ) != NULL ) {
		hdeepinfo->es_lambda = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_debug ("%s", gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "es_cutoff", &gerror) ) != NULL ) {
		hdeepinfo->es_cutoff = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_debug ("%s", gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "es_kind", &gerror) ) != NULL ) {
		hdeepinfo->es_kind = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_debug ("%s", gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "ca_flag", &gerror) ) != NULL ) {
		hdeepinfo->ca_flag = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_debug ("%s", gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "noglobal_eps", &gerror) ) != NULL ) {
		hdeepinfo->noglobal_eps = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_debug ("%s", gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "substeps", &gerror) ) != NULL ) {
		hdeepinfo->substeps = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_debug ("%s", gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "substieps", &gerror) ) != NULL ) {
		hdeepinfo->substieps = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_debug ("%s", gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "mean_cost", &gerror) ) != NULL ) {
		hdeepinfo->mean_cost = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_debug ("%s", gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "max_threads", &gerror) ) != NULL ) {
		hdeepinfo->max_threads = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_debug ("%s", gerror->message );
		g_clear_error (&gerror);
	}
	hdeepinfo->exclusive = FALSE;
	hdeepinfo->selector = DpSelectorNone;
	hdeepinfo->debug = 0;
    hdeepinfo->gthreadpool = NULL;
    hdeepinfo->last_method = -1;
	return hdeepinfo;
}

DpDeepInfo *dp_deep_info_init(DpEvaluation*heval, DpTarget*htarget, int worldid, GKeyFile*gkf, gchar*groupname)
{
	GError *gerror = NULL;
	int i;
	DpDeepInfo*hdeepinfo = dp_deep_info_new(gkf, groupname);
	DpRecombinationStrategy strategy;
	hdeepinfo->hevalctrl = dp_evaluation_init(heval, htarget, worldid, gkf, groupname);
	hdeepinfo->trial = dp_population_new(hdeepinfo->population_size, hdeepinfo->hevalctrl->eval->size, hdeepinfo->hevalctrl->eval_target->array_size, hdeepinfo->hevalctrl->eval_target->precond_size);
	hdeepinfo->population = dp_evaluation_population_init(hdeepinfo->hevalctrl, hdeepinfo->population_size, hdeepinfo->noglobal_eps);
	hdeepinfo->recombination_control = dp_recombination_control_init(hdeepinfo->hevalctrl->hrand, hdeepinfo->population, gkf, groupname);
	hdeepinfo->popunion = dp_population_union(hdeepinfo->population, hdeepinfo->trial);
	if ( hdeepinfo->max_threads > 0 ) {
        hdeepinfo->gthreadpool = g_thread_pool_new ((GFunc) dp_deep_evaluate_func, (gpointer) hdeepinfo, hdeepinfo->max_threads, hdeepinfo->exclusive, &gerror);
		g_thread_pool_set_max_unused_threads (-1);
		if ( gerror != NULL ) {
			g_error("%s", gerror->message);
		}
	}
	for (i = 0; i < hdeepinfo->population->size; i++) {
		hdeepinfo->trial->individ[i]->user_data = hdeepinfo->population->individ[i]->user_data;
	}
	return hdeepinfo;
}

void dp_deep_info_save(FILE*fp, DpDeepInfo *hdeepinfo)
{
  dp_population_save(fp, hdeepinfo->trial);
  dp_population_save(fp, hdeepinfo->population);
  dp_recombination_control_save(fp, hdeepinfo->recombination_control);
}

void dp_deep_info_load(FILE*fp, DpDeepInfo *hdeepinfo)
{
  dp_population_load(fp, hdeepinfo->trial);
  dp_population_load(fp, hdeepinfo->population);
  hdeepinfo->popunion = dp_population_union(hdeepinfo->population, hdeepinfo->trial);
  dp_recombination_control_load(fp, hdeepinfo->recombination_control);
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
	int ignore_cost = hdeepinfo->hevalctrl->eval_target->ignore_cost;
	GRand*hrand = hdeepinfo->hevalctrl->hrand;
	r1 = ( ignore_cost == 0 ) ? population->imin : -1;
	do {
		r2 = g_rand_int_range (hrand, 0, hdeepinfo->population_size);
	} while ( r2 == my_id || r2 == r1 );
	do {
		r3 = g_rand_int_range (hrand, 0, hdeepinfo->population_size);
	} while ( r3 == my_id || r3 == r1 || r3 == r2 );
	do {
		r4 = g_rand_int_range (hrand, 0, hdeepinfo->population_size);
	} while ( r4 == my_id || r4 == r1 || r4 == r2 || r4 == r3 );
	r1 = population->imin;
	my_tabu = population->individ[r1];
	start_index = g_rand_int_range (hrand, 0, population->ind_size);
	end_index = population->ind_size;
	dp_individ_copy_values(my_trial, my_individ);
	my_trial->age = 0;
	my_trial->r1 = r1;
	my_trial->r2 = r2;
	my_trial->r3 = r3;
	my_trial->r4 = r4;
	my_trial->moves = 0;
	my_trial->failures = 0;
	my_trial->grads = 0;
	dp_individ_recombination(recombination_control, hrand, my_trial, population->individ[r1], population->individ[r2], population->individ[r3], population->individ[r4], start_index, end_index);
	dp_evaluation_individ_evaluate(hdeepinfo->hevalctrl, my_trial, my_tabu, my_id, my_tabu->cost);
	if ( ignore_cost == 0 && my_id == population->imin ) {
		if ( my_trial->cost >= my_individ->cost ) {
			dp_individ_copy_values(my_trial, my_individ);
			my_trial->age++;
		}
	} else if ( -1 != dp_evaluation_individ_compare((const void *)(&my_individ), (const void *)(&my_trial), (void*)(hdeepinfo->hevalctrl)) ) {
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
        if (hdeepinfo->gthreadpool == NULL) {
            hdeepinfo->gthreadpool = g_thread_pool_new ((GFunc) dp_deep_step_func, (gpointer) hdeepinfo, hdeepinfo->max_threads, hdeepinfo->exclusive, &gerror);
        }
		if ( gerror != NULL ) {
			g_error("%s", gerror->message);
		}
		for ( individ_id = 0; individ_id < population->size; individ_id++ ) {
			g_thread_pool_push (hdeepinfo->gthreadpool, GINT_TO_POINTER(individ_id + 1), &gerror);
			if ( gerror != NULL ) {
				g_error("%s", gerror->message);
			}
		}
		g_thread_pool_free (hdeepinfo->gthreadpool, immediate_stop, wait_finish);
		hdeepinfo->gthreadpool = NULL;
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

void dp_deep_generate_func (gpointer data, gpointer user_data)
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
	int ignore_cost = hdeepinfo->hevalctrl->eval_target->ignore_cost;
	GRand*hrand = hdeepinfo->hevalctrl->hrand;
	r1 = ( ignore_cost == 0 ) ? population->imin : -1;
	do {
		r2 = g_rand_int_range (hrand, 0, hdeepinfo->population_size);
	} while ( r2 == my_id || r2 == r1 );
	do {
		r3 = g_rand_int_range (hrand, 0, hdeepinfo->population_size);
	} while ( r3 == my_id || r3 == r1 || r3 == r2 );
	do {
		r4 = g_rand_int_range (hrand, 0, hdeepinfo->population_size);
	} while ( r4 == my_id || r4 == r1 || r4 == r2 || r4 == r3 );
	r1 = population->imin;
	my_tabu = population->individ[r1];
	start_index = g_rand_int_range (hrand, 0, population->ind_size);
	end_index = population->ind_size;
	dp_individ_copy_values(my_trial, my_individ);
	my_trial->age = 0;
	my_trial->r1 = r1;
	my_trial->r2 = r2;
	my_trial->r3 = r3;
	my_trial->r4 = r4;
	my_trial->moves = 0;
	my_trial->failures = 0;
	my_trial->grads = 0;
	dp_individ_recombination(recombination_control, hrand, my_trial, population->individ[r1], population->individ[r2], population->individ[r3], population->individ[r4], start_index, end_index);
	my_trial->cost = G_MAXDOUBLE;
}

void dp_deep_generate_ca_func (gpointer data, gpointer user_data)
{
	int r1, r2, r3, r4;
	int start_index, end_index;
	DpDeepInfo*hdeepinfo = (DpDeepInfo*)user_data;
	int my_id = GPOINTER_TO_INT(data) - 1;
	DpPopulation*trial = hdeepinfo->trial;
	DpIndivid*my_trial = trial->individ[my_id];
	DpPopulation*population = hdeepinfo->population;
	DpIndivid*my_individ = population->individ[my_id];

	DpRecombinationControl *recombination_control = hdeepinfo->recombination_control;

	int ignore_cost = hdeepinfo->hevalctrl->eval_target->ignore_cost;
	GRand*hrand = hdeepinfo->hevalctrl->hrand;
	r1 = population->ifmax;
	r2 = my_id;
	r3 = my_id;
	r4 = my_id;
	start_index = my_id;
	end_index = hdeepinfo->es_lambda;
	my_trial->age = 0;
	my_trial->r1 = r1;
	my_trial->r2 = r2;
	my_trial->r3 = r3;
	my_trial->r4 = r4;
	my_trial->moves = 0;
	my_trial->failures = 0;
	my_trial->grads = 0;
	dp_individ_recombination_ca(recombination_control, hrand, my_trial, population->individ[r1], population->individ[r2], population->individ[r3], population->individ[r4], start_index, end_index);
	my_trial->cost = G_MAXDOUBLE;
}

void dp_deep_generate_ac_func (gpointer data, gpointer user_data)
{
	int r1, r2, r3, r4;
	int start_index, end_index;
	DpDeepInfo*hdeepinfo = (DpDeepInfo*)user_data;
	int my_id = GPOINTER_TO_INT(data) - 1;
	DpPopulation*trial = hdeepinfo->trial;
	DpIndivid*my_trial = trial->individ[my_id];
	DpPopulation*population = hdeepinfo->population;
	DpIndivid*my_individ = population->individ[my_id];

	DpRecombinationControl *recombination_control = hdeepinfo->recombination_control;

	int ignore_cost = hdeepinfo->hevalctrl->eval_target->ignore_cost;
	GRand*hrand = hdeepinfo->hevalctrl->hrand;
/*	r1 = population->ifmax;
	r2 = my_id;
	r3 = my_id;
	r4 = my_id;*/
	r1 = my_individ->r1;
	r2 = my_individ->r2;
	r3 = my_individ->r3;
	r4 = my_individ->r4;
	start_index = my_id;
	end_index = hdeepinfo->es_lambda;
	my_trial->age = 0;
	my_trial->r1 = r1;
	my_trial->r2 = r2;
	my_trial->r3 = r3;
	my_trial->r4 = r4;
	my_trial->moves = 0;
	my_trial->failures = 0;
	my_trial->grads = 0;
	dp_individ_recombination_ac(recombination_control, hrand, my_trial, population->individ[r1], population->individ[r2], population->individ[r3], population->individ[r4], start_index, end_index);
	my_trial->cost = G_MAXDOUBLE;
}

void dp_deep_evaluate_func (gpointer data, gpointer user_data)
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
	int ignore_cost = hdeepinfo->hevalctrl->eval_target->ignore_cost;
	r1 = population->imin;
	my_tabu = population->individ[r1];
	my_trial->age = 0;
	g_mutex_lock( &(my_trial->m) );
	my_trial->status = 1;
	g_mutex_unlock( &(my_trial->m) );
	dp_evaluation_individ_evaluate(hdeepinfo->hevalctrl, my_trial, my_tabu, my_id, my_tabu->cost);
	g_mutex_lock( &(my_trial->m) );
	my_trial->status = 0;
	g_mutex_unlock( &(my_trial->m) );
}

int dp_deep_evaluate_status (gpointer data, gpointer user_data)
{
	DpIndivid*my_tabu;
	DpDeepInfo*hdeepinfo = (DpDeepInfo*)user_data;
	int my_id = GPOINTER_TO_INT(data) - 1, s;
	DpPopulation*trial = hdeepinfo->trial;
	DpIndivid*my_trial = trial->individ[my_id];
	DpPopulation*population = hdeepinfo->population;
	DpIndivid*my_individ = population->individ[my_id];
	DpRecombinationControl *recombination_control = hdeepinfo->recombination_control;
	g_mutex_lock( &(my_trial->m) );
	s = my_trial->status;
	g_mutex_unlock( &(my_trial->m) );
	return s;
}

void dp_deep_select_func (gpointer data, gpointer user_data)
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
	int ignore_cost = hdeepinfo->hevalctrl->eval_target->ignore_cost;
	r1 = population->imin;
	my_tabu = population->individ[r1];
	if ( ignore_cost == 0 && my_id == population->imin ) {
		if ( my_trial->cost >= my_individ->cost ) {
			population->individ[my_id] = my_trial;
			trial->individ[my_id] = my_individ;
			my_individ->age++;
		}
	} else if ( -1 != dp_evaluation_individ_compare((const void *)(&my_individ), (const void *)(&my_trial), (void*)(hdeepinfo->hevalctrl)) ) {
/* individ is accepted if -1 is returned */
		population->individ[my_id] = my_trial;
		trial->individ[my_id] = my_individ; /* accepted */
		my_individ->age++;
	}
}

void dp_de_select_func (gpointer data, gpointer user_data)
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
	int ignore_cost = hdeepinfo->hevalctrl->eval_target->ignore_cost;
	r1 = population->imin;
	my_tabu = population->individ[r1];
	if ( my_trial->cost >= my_individ->cost ) {
		population->individ[my_id] = my_trial;
		trial->individ[my_id] = my_individ;
		my_individ->age++;
	}
}

void dp_deep_generate_step(DpDeepInfo*hdeepinfo)
{
	int individ_id;
	gboolean immediate_stop = FALSE;
	gboolean wait_finish = TRUE;
	DpPopulation*population = hdeepinfo->population;
	DpPopulation*trial = hdeepinfo->trial;
	GError *gerror = NULL;
	for ( individ_id = 0; individ_id < population->size; individ_id++ ) {
		dp_deep_generate_func (GINT_TO_POINTER(individ_id + 1), (gpointer) hdeepinfo);
    }
}

void dp_deep_generate_ca_step(DpDeepInfo*hdeepinfo)
{
	int individ_id;
	gboolean immediate_stop = FALSE;
	gboolean wait_finish = TRUE;
	DpPopulation*population = hdeepinfo->population;
	DpPopulation*trial = hdeepinfo->trial;
	GError *gerror = NULL;
	for ( individ_id = 0; individ_id < population->size; individ_id++ ) {
		dp_deep_generate_ca_func (GINT_TO_POINTER(individ_id + 1), (gpointer) hdeepinfo);
    }
}

void dp_deep_generate_ac_step(DpDeepInfo*hdeepinfo)
{
	int individ_id;
	gboolean immediate_stop = FALSE;
	gboolean wait_finish = TRUE;
	DpPopulation*population = hdeepinfo->population;
	DpPopulation*trial = hdeepinfo->trial;
	GError *gerror = NULL;
	for ( individ_id = 0; individ_id < population->size; individ_id++ ) {
		dp_deep_generate_ac_func (GINT_TO_POINTER(individ_id + 1), (gpointer) hdeepinfo);
    }
}

void dp_deep_evaluate_step(DpDeepInfo*hdeepinfo)
{
	GMainContext *gcontext = g_main_context_default();
	int individ_id;
	gboolean immediate_stop = FALSE;
	gboolean wait_finish = TRUE;
	DpPopulation*population = hdeepinfo->population;
	DpPopulation*trial = hdeepinfo->trial;
	GError *gerror = NULL;
	gulong microseconds = G_USEC_PER_SEC / 1000;
	int notdone, currstatus;
	if ( hdeepinfo->max_threads > 0 ) {
		for ( individ_id = population->slice_a; individ_id < population->slice_b; individ_id++ ) {
			g_thread_pool_push (hdeepinfo->gthreadpool, GINT_TO_POINTER(individ_id + 1), &gerror);
			if ( gerror != NULL ) {
				g_error("%s", gerror->message);
			}
		}
		notdone = 1;
//        while(g_thread_pool_unprocessed (hdeepinfo->gthreadpool) > 0) {
		while(notdone == 1 && g_thread_pool_unprocessed (hdeepinfo->gthreadpool) > 0) {
			g_main_context_iteration(gcontext, FALSE);
            g_usleep (microseconds);
			notdone = 0;
			for ( individ_id = population->slice_a; individ_id < population->slice_b; individ_id++ ) {
				currstatus = dp_deep_evaluate_status (GINT_TO_POINTER(individ_id + 1), (gpointer) hdeepinfo);
				if ( currstatus == 1 ) {
					notdone = 1;
				}
			}
        }
	} else {
		for ( individ_id = population->slice_a; individ_id < population->slice_b; individ_id++ ) {
			dp_deep_evaluate_func (GINT_TO_POINTER(individ_id + 1), (gpointer) hdeepinfo);
		}
	}
}

/* can be moved to ppopulaiton.c */
void dp_deep_select_step(DpDeepInfo*hdeepinfo)
{
	int individ_id;
	gboolean immediate_stop = FALSE;
	gboolean wait_finish = TRUE;
	DpPopulation*population = hdeepinfo->population;
	DpPopulation*trial = hdeepinfo->trial;
	GError *gerror = NULL;
	for ( individ_id = 0; individ_id < population->size; individ_id++ ) {
		dp_deep_select_func (GINT_TO_POINTER(individ_id + 1), (gpointer) hdeepinfo);
	}
	dp_population_update(trial, 0, trial->size);
	trial->iter = population->iter + 1;
	for ( individ_id = 0; individ_id < population->size; individ_id++ ) {
		int r4 = trial->individ[individ_id]->r4;
		if (r4 > -1 && trial->individ[individ_id]->age > 0 && trial->individ[r4]->age > 0) {
			trial->individ[r4]->failures++;
		}
	}
	hdeepinfo->population = trial; /* accepted */
	hdeepinfo->trial = population;
}

void dp_de_select_step(DpDeepInfo*hdeepinfo)
{
	int individ_id;
	gboolean immediate_stop = FALSE;
	gboolean wait_finish = TRUE;
	DpPopulation*population = hdeepinfo->population;
	DpPopulation*trial = hdeepinfo->trial;
	GError *gerror = NULL;
	for ( individ_id = 0; individ_id < population->size; individ_id++ ) {
		dp_de_select_func (GINT_TO_POINTER(individ_id + 1), (gpointer) hdeepinfo);
	}
	dp_population_update(trial, 0, trial->size);
	trial->iter = population->iter + 1;
	for ( individ_id = 0; individ_id < population->size; individ_id++ ) {
		int r4 = trial->individ[individ_id]->r4;
		if (r4 > -1 && trial->individ[individ_id]->age > 0 && trial->individ[r4]->age > 0) {
			trial->individ[r4]->failures++;
		}
	}
	hdeepinfo->population = trial; /* accepted */
	hdeepinfo->trial = population;
}

void dp_deep_accept_step(DpDeepInfo*hdeepinfo, double*value)
{
	if (hdeepinfo->mean_cost == 0) {
		*value = hdeepinfo->population->dmin;
	} else 	if (hdeepinfo->mean_cost == 1) {
		*value = hdeepinfo->population->fmean;
	}
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
	dp_recombination_control_update(hdeepinfo->recombination_control, hdeepinfo->hevalctrl->hrand, hdeepinfo->population, 0, hdeepinfo->population->size);
    dp_population_update(hdeepinfo->population, 0, hdeepinfo->population->size);
}

