/***************************************************************************
 *            dpsettings.h
 *
 *  Mon Mar 26 16:49:16 2012
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

#ifndef _DP_SETTIMGS_H
#define _DP_SETTIMGS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include "dpevaluation.h"
#include "dprecombination.h"
#include "dploop.h"
#include "dptarget.h"
#include "dpopt.h"

typedef struct DpSettings {
	DpOptStopType stop_type;
	double criterion;
	int stop_count;
	int population_size;
	DpRecombinationStrategy recombination_strategy;
	double recombination_weight;
	double recombination_prob;
	double recombination_gamma;
	int es_lambda;
	double noglobal_eps;
	DpEvaluationStrategy eval_strategy;
	double gamma_init;
	double roundoff_error;
	int seed;
	double step_parameter;
	double step_decrement;
	double derivative_step;
	int number_of_trials;
	gchar**run_before;
	gchar**run;
	gchar**run_after;
	gint max_threads;
	int pareto_all;
	int precision;
} DpSettings;

DpSettings*dp_settings_new();

gchar*dp_settings_read(gchar*filename, gsize*size, GError**err);

int dp_settings_load(gchar*data, gsize size, gchar*groupname, DpSettings *hopt, GError**err);

int dp_settings_init(gchar*filename, gchar*groupname, DpSettings *hopt, GError**err);

int dp_settings_process_run(DpSettings *dpsettings, DpOpt *hopt, int world_id, DpEvaluation*heval, DpTarget*htarget, GError**err);

int dp_settings_target_init(gchar*filename, gchar*groupname, DpTarget *htarget, GError**err);

#ifdef __cplusplus
}
#endif

#endif /* _DP_SETTINGS_H */
