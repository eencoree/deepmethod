/***************************************************************************
 *            dpopt.c
 *
 *  Fri Mar 23 15:49:34 2012
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
#include <glib.h>
#include "dploop.h"
#include "dpopt.h"
#include "dpdeep.h"

DpOpt *dp_opt_init(DpEvaluation*heval, DpTarget*htarget, int world_id, int world_count,char*filename, DpOptStopType stop_type, double criterion, int tau, int stop_count)
{
	DpOpt *hopt;
	GList *funcs = NULL;
	GList *run_once_before = NULL;
	GList *run_once_after = NULL;
	hopt = (DpOpt*)malloc(sizeof(DpOpt));
	hopt->world_id = world_id;
	hopt->world_count = world_count;
	hopt->filename = g_strdup (filename);
	hopt->stop_type = stop_type;
	hopt->criterion = criterion;
	hopt->tau = tau;
	hopt->stop_count = stop_count;
	hopt->logname = g_strdup_printf( "%s.hopt_log_%d", hopt->filename, hopt->world_id);
	hopt->tstname = g_strdup_printf( "%s.hopt_time", hopt->filename);
	hopt->htarget = htarget;
	hopt->heval = heval;
	hopt->hloop = dp_loop_new(NULL, NULL, NULL);
	hopt->opt_type = H_OPT_NONE;
	hopt->method_info = NULL;
	return hopt;
}

void dp_opt_add_func(DpOpt *hopt, DpLoopFunc func, int tau_flag, DpOptType opt_type, int order, gpointer method_info)
{
	DpLoopRunFunc *run_func;
	DpLoop *hloop = hopt->hloop;
	run_func = dp_loop_run_func_new(tau_flag, func, (gpointer) hopt);
	if ( hopt->opt_type == H_OPT_NONE ) {
		hopt->opt_type = opt_type;
	}
	if ( hopt->method_info == NULL ) {
		hopt->method_info = method_info;
	}
	if ( order < 0 ) {
		dp_loop_add_func_before(hloop, run_func);
	} else if ( order == 0 ) {
		dp_loop_add_func(hloop, run_func);
	} else {
		dp_loop_add_func_after(hloop, run_func);
	}
}

void dp_opt_add_func_from_list(gchar**list, DpOpt *hopt, int tau_flag, DpOptType opt_type, int order, gpointer method_info)
{
	int i;
	for ( i = 0; list[i]; i++ ) {
		if ( !g_strcmp0(list[i], "writelog") ) {
			dp_opt_add_func(hopt, dp_write_log, tau_flag, opt_type, order, method_info);
		} else if ( !g_strcmp0(list[i], "writestate") ) {
			dp_opt_add_func(hopt, dp_write_state, tau_flag, opt_type, order, method_info);
		} else if ( !g_strcmp0(list[i], "writetst") ) {
			dp_opt_add_func(hopt, dp_write_tst, tau_flag, opt_type, order, method_info);
		} else if ( !g_strcmp0(list[i], "mpicomm") ) {
			dp_opt_add_func(hopt, dp_opt_mpi_comm, tau_flag, opt_type, order, method_info);
		} else if ( !g_strcmp0(list[i], "optpost") ) {
			dp_opt_add_func(hopt, dp_opt_post, tau_flag, opt_type, order, method_info);
		} else if ( !g_strcmp0(list[i], "optposteval") ) {
			dp_opt_add_func(hopt, dp_opt_post_evaluate, tau_flag, opt_type, order, method_info);
		} else if ( !g_strcmp0(list[i], "initstop") ) {
			dp_opt_add_func(hopt, dp_opt_init_stop, tau_flag, opt_type, order, method_info);
		}
	}
}

void dp_opt_run(DpOpt *hopt)
{
	DpLoop *hloop = hopt->hloop;
	dp_loop_zero_counters(hopt->hloop);
	dp_loop_run (hloop);
}

DpOpt *dp_opt_monitor(char*filename, DpEvaluation*heval, DpTarget*htarget, int world_id, int world_count, int monitor, char**dp_error)
{
	DpLoop *hloop;
	DpOpt *hopt;
	GList *funcs = NULL;
	GList *run_once_before = NULL;
	GList *run_once_after = NULL;
	DpLoopRunFunc *func;

	hopt = (DpOpt*)malloc(sizeof(DpOpt));
	hopt->world_id = world_id;
	hopt->world_count = world_count;
	hopt->monitor = monitor;
	hopt->filename = g_strdup (filename);

	func = (DpLoopRunFunc *)malloc(sizeof(DpLoopRunFunc));
	func->tau_flag = 1;
	func->user_data = (void*)hopt;
	func->func = (DpLoopFunc)dp_read_log;
	funcs = g_list_prepend (funcs, (void*)func);

	hopt->logname = (char*)calloc(G_MAXSIZE, sizeof(char) );
	sprintf( hopt->logname, "%s.hopt_log_%d", hopt->filename, hopt->world_id);
	hopt->tstname = (char*)calloc(G_MAXSIZE, sizeof(char) );
	sprintf( hopt->tstname, "%s.hopt_time", hopt->filename);
	hopt->htarget = htarget;
	hopt->heval = heval;

	hloop = dp_loop_new(run_once_before, funcs, run_once_after);
	dp_loop_zero_counters(hloop);
	hopt->hloop = hloop;
	dp_loop_run (hloop);
	return hopt;
}

DpLoopExitCode dp_opt_init_stop(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo;
/*	HOsdaInfo*hosdainfo;*/
	switch (hopt->opt_type) {
		case H_OPT_DEEP:
			hdeepinfo = (DpDeepInfo*)(hopt->method_info);
			dp_deep_accept_step(hdeepinfo, &(hopt->cost));
			hopt->old_cost = hopt->cost;
			hopt->stop_counter = 0;
		break;
		case H_OPT_OSDA:
/*			hosdainfo = (HOsdaInfo*)(hopt->method_info);
			dp_osda_accept_step(hosdainfo, &(hopt->cost));
			hopt->old_cost = hopt->cost;
			hopt->stop_counter = 0;*/
		break;
	}
	return ret_val;
}

DpLoopExitCode dp_opt_check_stop(DpLoop*hloop, gpointer user_data)
{
	DpOpt*hopt = (DpOpt*)user_data;
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	double delta;
	if ( hopt->stop_type != H_OPT_ABSOLUTE_TIME && hopt->stop_type != H_OPT_ABSOLUTE_ITER ) {
		switch( hopt->stop_type ) {
			case H_OPT_PROPORTIONAL_STOP:
				delta = ( hopt->old_cost - hopt->cost );
				if ( hopt->old_cost != 0 ) {
					delta /= hopt->old_cost;
				}
			break;
			case H_OPT_ABSOLUTE_STOP:
				delta = hopt->old_cost - hopt->cost;
			break;
			case H_OPT_ABSOLUTE_SCORE:
				delta = hopt->cost;
			break;
			default:
				hloop->exit_str = g_strdup_printf ( "dp_opt_checkstop: unknown %d", (int)(hopt->stop_type));
				return DP_LOOP_EXIT_ERROR;
		}
		if ( fabs(delta) < hopt->criterion ) {
			hopt->stop_counter++;
		} else {
			hopt->stop_counter = 0;
			hopt->old_cost = hopt->cost;
		}
		if ( hopt->stop_counter == hopt->stop_count) {
			hopt->stop_counter = 0;
			hopt->old_cost = hopt->cost;
			ret_val = DP_LOOP_EXIT_SUCCESS;
		}
	}
	return ret_val;
}

DpLoopExitCode dp_opt_deep(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo = (DpDeepInfo*)(hopt->method_info);
	double energy_start;
	int iteration;
	energy_start = hopt->cost;
	for( iteration = 0; iteration < hopt->tau; iteration++ ) {
		dp_deep_step(hdeepinfo);
		dp_deep_accept_step(hdeepinfo, &(hopt->cost));
	}
	dp_deep_update_step(hdeepinfo);
	return ret_val;
}

DpLoopExitCode dp_write_log(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpEvaluation*heval = (DpEvaluation*)(hopt->heval);
	DpTarget*htarget = (DpTarget*)(hopt->htarget);
	DpDeepInfo*hdeepinfo;
/*	HOsdaInfo*hosdainfo;*/
	FILE*fp;
	int i;
	fp = fopen(hopt->logname, "a");
	if ( !fp ) {
		hloop->exit_str = g_strdup_printf ( "dp_write_log: can't open %s", hopt->logname);
		return DP_LOOP_EXIT_ERROR;
	}
/*	dp_opt_post(hloop, user_data);*/
	fprintf(fp, "wtime:%e tau:%d cost:%f", hloop->w_time, hloop->tau_counter, hopt->cost);
	for ( i = 0; i < heval->size; i++) {
		fprintf(fp, " p[%d]:%f", heval->points[i]->index, *(heval->points[i]->param));
	}
	fprintf(fp, "\n");
	fclose(fp);
	return ret_val;
}

DpLoopExitCode dp_read_log(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpEvaluation*heval = (DpEvaluation*)(hopt->heval);
	DpTarget*htarget = (DpTarget*)(hopt->htarget);
	int monitor = hopt->monitor;
	FILE*fp;
	int i;
	fp = fopen(hopt->logname, "r");
	if ( !fp ) {
		hloop->exit_str = g_strdup_printf ( "dp_read_log: can't open %s", hopt->logname);
		return DP_LOOP_EXIT_ERROR;
	}
	do {
		fscanf(fp, "wtime:%lf tau:%d cost:%lf", &(hloop->w_time), &(hloop->tau_counter), &(hopt->cost));
		for ( i = 0; i < heval->size; i++) {
			fscanf(fp, " p[%d]:%lf", &(heval->points[i]->index), heval->points[i]->param);
		}
	} while ( !feof(fp) && monitor != hloop->tau_counter );
	fclose(fp);
	return ret_val;
}

DpLoopExitCode dp_write_state(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	return ret_val;
}

DpLoopExitCode dp_write_tst(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpEvaluation*heval = (DpEvaluation*)(hopt->heval);
	DpTarget*htarget = (DpTarget*)(hopt->htarget);
	FILE*fp;
	int i;
	fp = fopen(hopt->tstname, "w");
	if ( !fp ) {
		hloop->exit_str = g_strdup_printf ( "dp_write_tst: can't open %s", hopt->tstname);
		return DP_LOOP_EXIT_ERROR;
	}
	fprintf(fp, "user       time:%e\n", hloop->u_time);
	fprintf(fp, "wall clock time:%e\n", hloop->w_time);
	fprintf(fp, "tau     counter:%d\n", hloop->tau_counter);
	fprintf(fp, "final      cost:%e\n", hopt->cost);
	fprintf(fp, "cost       runs:%d\n", hopt->htarget->target->kount);
	for ( i = 0; i < hopt->htarget->size; i++ ) {
		fprintf(fp, "penalty[%d]cost:%e\n", i, hopt->htarget->penalty[i]->retval);
		fprintf(fp, "penalty[%d]runs:%d\n", i, hopt->htarget->penalty[i]->kount);
	}
	fprintf(fp, "\n");
	fclose(fp);
	return ret_val;
}
/*
DpLoopExitCode dp_opt_osda(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	HOsdaInfo*hosdainfo = (HOsdaInfo*)(hopt->method_info);
	double energy_start;
	int iteration;
	energy_start = hopt->cost;
	for( iteration = 0; iteration < hopt->tau; iteration++ ) {
		dp_osda_generate_step(hosdainfo);
		dp_osda_step(hosdainfo);
		dp_osda_accept_step(hosdainfo, &(hopt->cost));
	}
	dp_osda_update_step(hosdainfo);
	return ret_val;
}

DpLoopExitCode dp_opt_es(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	HEsInfo*hesinfo = (HEsInfo*)(hopt->method_info);
	double energy_start;
	int iteration;
	energy_start = hopt->cost;
	for( iteration = 0; iteration < hopt->tau; iteration++ ) {
		dp_es_generate_step(hesinfo);
		dp_es_step(hesinfo);
		dp_es_accept_step(hesinfo, &(hopt->cost));
	}
	dp_es_update_step(hesinfo);
	return ret_val;
}
*/
DpLoopExitCode dp_opt_mpi_comm(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo;
	int stop_flag = ( hloop->stop_flag == DP_LOOP_EXIT_SUCCESS ) ? 1 : 0;
	switch (hopt->opt_type) {
		case H_OPT_DEEP:
			hdeepinfo = (DpDeepInfo*)(hopt->method_info);
			g_qsort_with_data(hdeepinfo->population->ages_descending, hdeepinfo->population->size, sizeof(hdeepinfo->population->ages_descending[0]), (GCompareDataFunc)dp_individ_ages_descending, hdeepinfo->population);
			g_qsort_with_data(hdeepinfo->population->cost_ascending, hdeepinfo->population->size, sizeof(hdeepinfo->population->cost_ascending[0]), (GCompareDataFunc)dp_individ_cost_ascending, hdeepinfo->population);
			dp_population_mpi_comm_nbest(hdeepinfo->population, hopt->world_id, hopt->world_count, &stop_flag, hdeepinfo->es_lambda);
			dp_population_update(hdeepinfo->population, 0, hdeepinfo->population->size);
		break;
	}
	hloop->stop_flag = ( stop_flag == 1 ) ? DP_LOOP_EXIT_SUCCESS : DP_LOOP_EXIT_NOEXIT;
	return ret_val;
}

DpLoopExitCode dp_opt_post(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo;
/*	HOsdaInfo*hosdainfo;*/
	switch (hopt->opt_type) {
		case H_OPT_DEEP:
			hdeepinfo = (DpDeepInfo*)(hopt->method_info);
			dp_deep_post(hdeepinfo);
		break;
		case H_OPT_OSDA:
/*			hosdainfo = (HOsdaInfo*)(hopt->method_info);
			h_osda_post(hosdainfo);*/
		break;
	}
	return ret_val;
}

DpLoopExitCode dp_opt_post_evaluate(DpLoop*hloop, gpointer user_data)
{
	DpLoopExitCode ret_val = DP_LOOP_EXIT_NOEXIT;
	DpOpt*hopt = (DpOpt*)user_data;
	DpDeepInfo*hdeepinfo;
/*	HOsdaInfo*hosdainfo;*/
	switch (hopt->opt_type) {
		case H_OPT_DEEP:
			hdeepinfo = (DpDeepInfo*)(hopt->method_info);
			dp_deep_post_evaluate(hdeepinfo);
		break;
		case H_OPT_OSDA:
/*			hosdainfo = (HOsdaInfo*)(hopt->method_info);
			h_osda_post(hosdainfo);*/
		break;
	}
	return ret_val;
}
