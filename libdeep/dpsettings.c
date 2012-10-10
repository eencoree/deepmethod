/***************************************************************************
 *            dpsettings.c
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
 
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#ifdef GIO_STANDALONE_SOURCE
#include <gio/gfile.h>
#else
#include <gio/gio.h>
#endif
#include "dpdeep.h"
#include "dposda.h"
#include "dprecombination.h"
#include "dpopt.h"
#include "dpsettings.h"

#ifdef GIO_STANDALONE_SOURCE
int g_strcmp0(char*str1, char*str2)
{
	return (int)strcmp(str1, str2);
}
#endif

DpSettings*dp_settings_new()
{
	DpSettings*opts = (DpSettings*)malloc(sizeof(DpSettings));
	opts->stop_type = H_OPT_PROPORTIONAL_STOP;
	opts->criterion = 1e-5;
	opts->stop_count = 10;
	opts->tau = 2;
	opts->population_size = 5;
	opts->recombination_strategy = DE_3_bin_T;
	opts->recombination_weight = 0;
	opts->recombination_prob = 0;
	opts->recombination_gamma = 0.9;
	opts->es_lambda = 2;
	opts->noglobal_eps = 0;
	opts->eval_strategy = tanh_trans_flag;
	opts->gamma_init = 50.0;
	opts->roundoff_error = 1e-12;
	opts->seed = 2064286;
	opts->step_parameter = 0.2;
	opts->step_decrement = 0.1;
	opts->derivative_step = 0.1;
	opts->number_of_trials = 1;
	opts->run_before = g_new( gchar* , 3 );
	opts->run_before[0] = g_strdup("initstop");
	opts->run_before[1] = g_strdup("writelog");
	opts->run_before[2] = NULL;
	opts->run = g_new( gchar* , 11 );
	opts->run[0] = g_strdup("deep");
	opts->run[1] = g_strdup("1");
	opts->run[2] = g_strdup("mpicomm");
	opts->run[3] = g_strdup("2");
	opts->run[4] = g_strdup("checkstop");
	opts->run[5] = g_strdup("1");
	opts->run[6] = g_strdup("writestate");
	opts->run[7] = g_strdup("1");
	opts->run[8] = g_strdup("writelog");
	opts->run[9] = g_strdup("1");
	opts->run[10] = NULL;
	opts->run_after = g_new( gchar* , 4 );
	opts->run_after[0] = g_strdup("mpicomm");
	opts->run_after[1] = g_strdup("optpost");
	opts->run_after[2] = g_strdup("writelog");
	opts->run_after[3] = NULL;
	return opts;
}

gchar*dp_settings_read(gchar*filename, gsize*size, GError**err)
{
	GFile*gfile;
	char *etag;
	gchar*data = NULL;
	GError *gerror = NULL;
	g_return_val_if_fail (err == NULL || *err == NULL, NULL);
	gfile = g_file_new_for_commandline_arg(filename);
	if ( !g_file_load_contents(gfile, (GCancellable*)NULL, &data, size, &etag, &gerror) ) {
		g_propagate_error (err, gerror);
		return NULL;
	}
	g_object_unref(gfile);
	return data;
}

int dp_settings_load(gchar*data, gsize size, gchar*groupname, DpSettings *hopt, GError**err)
{
	GKeyFile*gkf;
	GError *gerror = NULL;
	gchar*str,**strlist;
	int retval = 0;
	gkf = g_key_file_new();
	if ( !g_key_file_load_from_data(gkf, (const gchar*)data, size, G_KEY_FILE_NONE, &gerror ) ) {
		g_propagate_error (err, gerror);
		return 1;
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "proportional_stop", &gerror) ) != NULL ) {
		hopt->stop_type = H_OPT_PROPORTIONAL_STOP;
		hopt->criterion = g_strtod( str , NULL);
		g_free(str);
		if ( gerror != NULL ) {
			g_warning ( gerror->message );
			g_clear_error (&gerror);
		}
	} else if ( ( str = g_key_file_get_string(gkf, groupname, "absolute_stop", &gerror) ) != NULL ) {
		hopt->stop_type = H_OPT_ABSOLUTE_STOP;
		hopt->criterion = g_strtod( str , NULL);
		g_free(str);
		if ( gerror != NULL ) {
			g_warning ( gerror->message );
			g_clear_error (&gerror);
		}
	} else if ( ( str = g_key_file_get_string(gkf, groupname, "absolute_score", &gerror) ) != NULL ) {
		hopt->stop_type = H_OPT_ABSOLUTE_SCORE;
		hopt->criterion = g_strtod( str , NULL);
		g_free(str);
		if ( gerror != NULL ) {
			g_warning ( gerror->message );
			g_clear_error (&gerror);
		}
	} else if ( ( str = g_key_file_get_string(gkf, groupname, "absolute_iter", &gerror) ) != NULL ) {
		hopt->stop_type = H_OPT_ABSOLUTE_ITER;
		hopt->criterion = g_strtod( str , NULL);
		g_free(str);
		if ( gerror != NULL ) {
			g_warning ( gerror->message );
			g_clear_error (&gerror);
		}
	} else if ( ( str = g_key_file_get_string(gkf, groupname, "absolute_time", &gerror) ) != NULL ) {
		hopt->stop_type = H_OPT_ABSOLUTE_TIME;
		hopt->criterion = g_strtod( str , NULL);
		g_free(str);
		if ( gerror != NULL ) {
			g_warning ( gerror->message );
			g_clear_error (&gerror);
		}		
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "stop_count", &gerror) ) != NULL ) {
		hopt->stop_count = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "tau", &gerror) ) != NULL ) {
		hopt->tau = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "population_size", &gerror) ) != NULL ) {
		hopt->population_size = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "recombination_weight", &gerror) ) != NULL ) {
		hopt->recombination_weight = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "recombination_prob", &gerror) ) != NULL ) {
		hopt->recombination_prob = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "recombination_gamma", &gerror) ) != NULL ) {
		hopt->recombination_gamma = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "es_lambda", &gerror) ) != NULL ) {
		hopt->es_lambda = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "noglobal_eps", &gerror) ) != NULL ) {
		hopt->noglobal_eps = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "gamma_init", &gerror) ) != NULL ) {
		hopt->gamma_init = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "roundoff_error", &gerror) ) != NULL ) {
		hopt->roundoff_error = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "seed", &gerror) ) != NULL ) {
		hopt->seed = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "step_parameter", &gerror) ) != NULL ) {
		hopt->step_parameter = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "step_decrement", &gerror) ) != NULL ) {
		hopt->step_decrement = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "derivative_step", &gerror) ) != NULL ) {
		hopt->derivative_step = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "number_of_trials", &gerror) ) != NULL ) {
		hopt->number_of_trials = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( strlist = g_key_file_get_string_list(gkf, groupname, "run_before", NULL, &gerror) ) != NULL ) {
		g_strfreev(hopt->run_before);
		hopt->run_before = strlist;
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( strlist = g_key_file_get_string_list(gkf, groupname, "run", NULL, &gerror) ) != NULL ) {
		g_strfreev(hopt->run);
		hopt->run = strlist;
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( strlist = g_key_file_get_string_list(gkf, groupname, "run_after", NULL, &gerror) ) != NULL ) {
		g_strfreev(hopt->run_after);
		hopt->run_after = strlist;
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "recombination_strategy", &gerror) ) != NULL ) {
		if ( !g_strcmp0(str, "simple") ) {
			hopt->recombination_strategy = Simple;
		} else if ( !g_strcmp0(str, "de_3_bin") ) {
			hopt->recombination_strategy = DE_3_bin;
		} else if ( !g_strcmp0(str, "de_3_exp") ) {
			hopt->recombination_strategy = DE_3_exp;
		} else if ( !g_strcmp0(str, "de_3_exp_T") ) {
			hopt->recombination_strategy = DE_3_exp_T;
		} else if ( !g_strcmp0(str, "de_3_bin_T") ) {
			hopt->recombination_strategy = DE_3_bin_T;
		}
		g_free(str);
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "transform", &gerror) ) != NULL ) {
		if ( !g_strcmp0(str, "tanh") ) {
			hopt->eval_strategy = tanh_trans_flag;
		} else if ( !g_strcmp0(str, "sin") ) {
			hopt->eval_strategy = sin_trans_flag;
		} else if ( !g_strcmp0(str, "alg") ) {
			hopt->eval_strategy = alg_trans_flag;
		} else if ( !g_strcmp0(str, "rand") ) {
			hopt->eval_strategy = rand_trans_flag;
		}
		g_free(str);
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "max_threads", &gerror) ) != NULL ) {
		hopt->max_threads = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	g_key_file_free(gkf);
	return retval;
}

int dp_settings_init(gchar*filename, gchar*groupname, DpSettings *hopt, GError**err)
{
	int ret_val = 0;;
	gchar*data = NULL;
	gsize size;
	GError *gerror = NULL;
	if ( ( data = dp_settings_read(filename, &size, &gerror) ) == NULL ) {
		g_propagate_error (err, gerror);
		return 1;
	}
	ret_val = dp_settings_load(data, size, groupname, hopt, &gerror);
	if ( ret_val == 1 ) {
		g_propagate_error (err, gerror);
		ret_val = 1;
	}
	g_free(data);
	return ret_val;
}

int dp_settings_process_run(DpSettings *dpsettings, DpOpt *hopt, int world_id, DpEvaluation*heval, DpTarget*htarget, GError**err)
{
	DpDeepInfo*hdeepinfo;
	DpOsdaInfo*hosdainfo;
	gpointer method_info;
	int i, order, tau_flag;
	gchar**list;
	DpOptType opt_type;
	list = dpsettings->run_before;
	order = -1;
	tau_flag = 1;
	opt_type = H_OPT_NONE;
	method_info = NULL;
	dp_opt_add_func_from_list(list, hopt, tau_flag, opt_type, order, method_info);
	order = 1;
	list = dpsettings->run_after;
	dp_opt_add_func_from_list(list, hopt, tau_flag, opt_type, order, method_info);
	order = 0;
	list = dpsettings->run;
	for ( i = 0; list[i]; i += 2 ) {
		tau_flag = g_strtod(list[i + 1], NULL);
		if ( !g_strcmp0(list[i], "writelog") ) {
			opt_type = H_OPT_NONE;
			method_info = NULL;
			dp_opt_add_func(hopt, dp_write_log, tau_flag, opt_type, order, method_info);
		} else if ( !g_strcmp0(list[i], "writestate") ) {
			opt_type = H_OPT_NONE;
			method_info = NULL;
			dp_opt_add_func(hopt, dp_write_state, tau_flag, opt_type, order, method_info);
		} else if ( !g_strcmp0(list[i], "checkstop") ) {
			opt_type = H_OPT_NONE;
			method_info = NULL;
			dp_opt_add_func(hopt, dp_opt_check_stop, tau_flag, opt_type, order, method_info);
		} else if ( !g_strcmp0(list[i], "initstop") ) {
			opt_type = H_OPT_NONE;
			method_info = NULL;
			dp_opt_add_func(hopt, dp_opt_init_stop, tau_flag, opt_type, order, method_info);
		} else if ( !g_strcmp0(list[i], "mpicomm") ) {
			opt_type = H_OPT_NONE;
			method_info = NULL;
			dp_opt_add_func(hopt, dp_opt_mpi_comm, tau_flag, opt_type, order, method_info);
		} else if ( !g_strcmp0(list[i], "optpost") ) {
			opt_type = H_OPT_NONE;
			method_info = NULL;
			dp_opt_add_func(hopt, dp_opt_post, tau_flag, opt_type, order, method_info);
		} else if ( !g_strcmp0(list[i], "deep") ) {
			opt_type = H_OPT_DEEP;
			hdeepinfo = dp_deep_info_init(heval, htarget, world_id, dpsettings->seed, dpsettings->gamma_init, dpsettings->roundoff_error, dpsettings->eval_strategy, dpsettings->population_size, dpsettings->recombination_weight, dpsettings->recombination_prob, dpsettings->recombination_gamma, dpsettings->es_lambda, dpsettings->noglobal_eps, dpsettings->recombination_strategy, dpsettings->max_threads);
			method_info = (gpointer) hdeepinfo;
			dp_opt_add_func(hopt, dp_opt_deep, tau_flag, opt_type, order, method_info);
		} else if ( !g_strcmp0(list[i], "osda") ) {
			opt_type = H_OPT_OSDA;
			hosdainfo = dp_osda_info_init(heval, htarget, world_id, dpsettings->seed, dpsettings->gamma_init, dpsettings->roundoff_error, dpsettings->eval_strategy, dpsettings->number_of_trials, dpsettings->step_parameter, dpsettings->step_decrement, dpsettings->derivative_step);
			method_info = (gpointer) hosdainfo;
			dp_opt_add_func(hopt, dp_opt_osda, tau_flag, opt_type, order, method_info);
		}
	}
	return 0;
}

int dp_settings_target_load(gchar*data, gsize size, gchar*groupname, DpTarget *htarget, GError**err)
{
	GKeyFile*gkf;
	GError *gerror = NULL;
	gchar**keys, **array;
	int retval = 0, i;
	gsize length, ksize;
	int index;
	double rank;
	double weight;
	gkf = g_key_file_new();
	if ( !g_key_file_load_from_data(gkf, (const gchar*)data, size, G_KEY_FILE_NONE, &gerror ) ) {
		g_propagate_error (err, gerror);
		return 1;
	}
	if ( ( keys = g_key_file_get_keys(gkf, groupname, &ksize, &gerror) ) == NULL ) {
		g_propagate_error (err, gerror);
		return 1;
	}
	for ( i = 0; i < (int)ksize; i++ ) {
		if ( ( array = g_key_file_get_string_list(gkf, groupname, keys[i], &length, &gerror) ) != NULL ) {
			if ( length == 3 ) {
				index = g_strtod(array[0], NULL);
				rank = g_strtod(array[1], NULL);
				weight = g_strtod(array[2], NULL);
				dp_target_add_func (htarget, index, weight, rank, keys[i]);
			} else if ( length == 4 ) {
				index = g_strtod(array[0], NULL);
				rank = g_strtod(array[1], NULL);
				weight = g_strtod(array[2], NULL);
				dp_target_insert_prime_func (htarget, index, weight, rank, keys[i]);
			}
			g_strfreev(array);
		}
	}
	g_strfreev(keys);
	g_key_file_free(gkf);
	return retval;
}

int dp_settings_target_init(gchar*filename, gchar*groupname, DpTarget *htarget, GError**err)
{
	int ret_val = 0;;
	gchar*data = NULL;
	gsize size;
	GError *gerror = NULL;
	if ( ( data = dp_settings_read(filename, &size, &gerror) ) == NULL ) {
		g_propagate_error (err, gerror);
		return 1;
	}
	ret_val = dp_settings_target_load(data, size, groupname, htarget, &gerror);
	if ( ret_val == 1 ) {
		g_propagate_error (err, gerror);
		ret_val = 1;
	}
	g_free(data);
	return ret_val;
}
