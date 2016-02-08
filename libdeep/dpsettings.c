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
#define g_strcmp0(str1, str2) strcmp(str1, str2)
#endif

DpSettings*dp_settings_new()
{
	DpSettings*opts = (DpSettings*)malloc(sizeof(DpSettings));
	opts->stop_type = H_OPT_PROPORTIONAL_STOP;
	opts->criterion = 1e-5;
	opts->stop_count = 10;
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
	opts->pareto_all = -1;
	opts->precision = 9;
	return opts;
}

GKeyFile*dp_settings_read(gchar*filename, GError**err)
{
	GKeyFile*gkf;
	GFile*gfile;
	char *etag;
	gchar*data = NULL;
	gsize size;
	GError *gerror = NULL;
	g_return_val_if_fail (err == NULL || *err == NULL, NULL);
	gfile = g_file_new_for_commandline_arg(filename);
	if ( !g_file_load_contents(gfile, (GCancellable*)NULL, &data, &size, &etag, &gerror) ) {
		g_propagate_error (err, gerror);
		return NULL;
	}
	g_object_unref(gfile);
	gkf = g_key_file_new();
	if ( !g_key_file_load_from_data(gkf, (const gchar*)data, size, G_KEY_FILE_NONE, &gerror ) ) {
		g_propagate_error (err, gerror);
		return NULL;
	}
	g_free(data);
	return gkf;
}

int dp_settings_load(GKeyFile*gkf, gchar*groupname, DpSettings *hopt, GError**err)
{
	GError *gerror = NULL;
	gchar*str,**strlist;
	int retval = 0;
	if ( ( str = g_key_file_get_string(gkf, groupname, "proportional_stop", &gerror) ) != NULL ) {
		hopt->stop_type = H_OPT_PROPORTIONAL_STOP;
		hopt->criterion = g_strtod( str , NULL);
		g_free(str);
		if ( gerror != NULL ) {
			g_debug ("%s", gerror->message );
			g_clear_error (&gerror);
		}
	} else if ( ( str = g_key_file_get_string(gkf, groupname, "absolute_stop", &gerror) ) != NULL ) {
		hopt->stop_type = H_OPT_ABSOLUTE_STOP;
		hopt->criterion = g_strtod( str , NULL);
		g_free(str);
		if ( gerror != NULL ) {
			g_debug ("%s", gerror->message );
			g_clear_error (&gerror);
		}
	} else if ( ( str = g_key_file_get_string(gkf, groupname, "absolute_score", &gerror) ) != NULL ) {
		hopt->stop_type = H_OPT_ABSOLUTE_SCORE;
		hopt->criterion = g_strtod( str , NULL);
		g_free(str);
		if ( gerror != NULL ) {
			g_debug ("%s", gerror->message );
			g_clear_error (&gerror);
		}
	} else if ( ( str = g_key_file_get_string(gkf, groupname, "absolute_iter", &gerror) ) != NULL ) {
		hopt->stop_type = H_OPT_ABSOLUTE_ITER;
		hopt->criterion = g_strtod( str , NULL);
		g_free(str);
		if ( gerror != NULL ) {
			g_debug ("%s", gerror->message );
			g_clear_error (&gerror);
		}
	} else if ( ( str = g_key_file_get_string(gkf, groupname, "absolute_time", &gerror) ) != NULL ) {
		hopt->stop_type = H_OPT_ABSOLUTE_TIME;
		hopt->criterion = g_strtod( str , NULL);
		g_free(str);
		if ( gerror != NULL ) {
			g_debug ("%s", gerror->message );
			g_clear_error (&gerror);
		}
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "stop_count", &gerror) ) != NULL ) {
		hopt->stop_count = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_debug ("%s", gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "precision", &gerror) ) != NULL ) {
		hopt->precision = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_debug ("%s", gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "pareto_all", &gerror) ) != NULL ) {
		hopt->pareto_all = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_debug ("%s", gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "roundoff_error", &gerror) ) != NULL ) {
		hopt->roundoff_error = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_debug ("%s", gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "step_parameter", &gerror) ) != NULL ) {
		hopt->step_parameter = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_debug ("%s", gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "step_decrement", &gerror) ) != NULL ) {
		hopt->step_decrement = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_debug ("%s", gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "derivative_step", &gerror) ) != NULL ) {
		hopt->derivative_step = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_debug ("%s", gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "number_of_trials", &gerror) ) != NULL ) {
		hopt->number_of_trials = g_strtod( str , NULL);
		g_free(str);
	} else {
		g_debug ("%s", gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( strlist = g_key_file_get_string_list(gkf, groupname, "run_before", NULL, &gerror) ) != NULL ) {
		g_strfreev(hopt->run_before);
		hopt->run_before = strlist;
	} else {
		g_debug ("%s", gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( strlist = g_key_file_get_string_list(gkf, groupname, "run", NULL, &gerror) ) != NULL ) {
		g_strfreev(hopt->run);
		hopt->run = strlist;
	} else {
		g_debug ("%s", gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( strlist = g_key_file_get_string_list(gkf, groupname, "run_after", NULL, &gerror) ) != NULL ) {
		g_strfreev(hopt->run_after);
		hopt->run_after = strlist;
	} else {
		g_debug ("%s", gerror->message );
		g_clear_error (&gerror);
	}
	return retval;
}

GKeyFile*dp_settings_init(gchar*filename, gchar*groupname, DpSettings *hopt, GError**err)
{
	int ret_val = 0;;
	GError *gerror = NULL;
	GKeyFile*gkf = NULL;
	if ( ( gkf = dp_settings_read(filename, &gerror) ) == NULL ) {
		g_propagate_error (err, gerror);
		return NULL;
	}
	ret_val = dp_settings_load(gkf, groupname, hopt, &gerror);
	if ( ret_val == 1 ) {
		g_propagate_error (err, gerror);
		ret_val = 1;
	}
	return gkf;
}

int dp_settings_process_run(DpSettings *dpsettings, GKeyFile*gkf, gchar*groupname, DpOpt *hopt, int world_id, DpEvaluation*heval, DpTarget*htarget, GError**err)
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
		if (hopt->world_id == 0) {
			if ( !g_strcmp0(list[i], "writelog") ) {
				opt_type = H_OPT_NONE;
				method_info = NULL;
				dp_opt_add_func(hopt, dp_write_log, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "printlog") ) {
				opt_type = H_OPT_NONE;
				method_info = NULL;
				dp_opt_add_func(hopt, dp_print_log, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "writestate") ) {
				opt_type = H_OPT_NONE;
				method_info = NULL;
				dp_opt_add_func(hopt, dp_write_state, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "writepareto") ) {
				opt_type = H_OPT_NONE;
				method_info = NULL;
				dp_opt_add_func(hopt, dp_write_pareto, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "evalpareto") ) {
				opt_type = H_OPT_NONE;
				method_info = NULL;
				dp_opt_add_func(hopt, dp_opt_evaluate_pareto_front, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "selpareto") ) {
				opt_type = H_OPT_NONE;
				method_info = NULL;
				dp_opt_add_func(hopt, dp_select_pareto_front, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "sortpareto") ) {
				opt_type = H_OPT_NONE;
				method_info = NULL;
				dp_opt_add_func(hopt, dp_sort_pareto_front, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "permutepop") ) {
				opt_type = H_OPT_NONE;
				method_info = NULL;
				dp_opt_add_func(hopt, dp_opt_permute, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "cr2cost") ) {
				opt_type = H_OPT_NONE;
				method_info = NULL;
				dp_opt_add_func(hopt, dp_opt_cr2cost, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "checkstop") ) {
				opt_type = H_OPT_NONE;
				method_info = NULL;
				dp_opt_add_func(hopt, dp_opt_check_stop, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "initstop") ) {
				opt_type = H_OPT_NONE;
				method_info = NULL;
				dp_opt_add_func(hopt, dp_opt_init_stop, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "substitute") ) {
				opt_type = H_OPT_NONE;
				method_info = NULL;
				dp_opt_add_func(hopt, dp_opt_substitute, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "dpupdate") ) {
				opt_type = H_OPT_NONE;
				method_info = NULL;
				dp_opt_add_func(hopt, dp_opt_deep_update, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "optpost") ) {
				opt_type = H_OPT_NONE;
				method_info = NULL;
				dp_opt_add_func(hopt, dp_opt_post, tau_flag, opt_type, order, method_info);
			} else if ( !g_strcmp0(list[i], "optposteval") ) {
				opt_type = H_OPT_NONE;
				method_info = NULL;
				dp_opt_add_func(hopt, dp_opt_post_evaluate, tau_flag, opt_type, order, method_info);
			}
		}
		if ( !g_strcmp0(list[i], "rotatetarget") ) {
			opt_type = H_OPT_NONE;
			method_info = NULL;
			dp_opt_add_func(hopt, dp_rotate_target, tau_flag, opt_type, order, method_info);
		} else if ( !g_strcmp0(list[i], "mpidistribute") ) {
			opt_type = H_OPT_NONE;
			method_info = NULL;
			dp_opt_add_func(hopt, dp_opt_mpi_distribute, tau_flag, opt_type, order, method_info);
		} else if ( !g_strcmp0(list[i], "mpigather") ) {
			opt_type = H_OPT_NONE;
			method_info = NULL;
			dp_opt_add_func(hopt, dp_opt_mpi_gather, tau_flag, opt_type, order, method_info);
		} else if ( !g_strcmp0(list[i], "deep") ) {
			opt_type = H_OPT_DEEP;
			hdeepinfo = dp_deep_info_init(heval, htarget, world_id, gkf, groupname);
			method_info = (gpointer) hdeepinfo;
			dp_opt_add_func(hopt, dp_opt_deep, tau_flag, opt_type, order, method_info);
		} else if ( !g_strcmp0(list[i], "gdeep") ) {
			opt_type = H_OPT_DEEP;
			hdeepinfo = dp_deep_info_init(heval, htarget, world_id, gkf, groupname);
			method_info = (gpointer) hdeepinfo;
			dp_opt_add_func(hopt, dp_opt_deep_generate, tau_flag, opt_type, order, method_info);
		} else if ( !g_strcmp0(list[i], "edeep") ) {
            opt_type = H_OPT_NONE;
			method_info = NULL;
			dp_opt_add_func(hopt, dp_opt_deep_evaluate, tau_flag, opt_type, order, method_info);
		} else if ( !g_strcmp0(list[i], "sdeep") ) {
			opt_type = H_OPT_NONE;
			method_info = NULL;
			dp_opt_add_func(hopt, dp_opt_deep_select, tau_flag, opt_type, order, method_info);
		} else if ( !g_strcmp0(list[i], "osda") ) {
			opt_type = H_OPT_OSDA;
/*			hosdainfo = dp_osda_info_init(heval, htarget, world_id, dpsettings->seed, dpsettings->gamma_init, dpsettings->roundoff_error, dpsettings->eval_strategy, dpsettings->number_of_trials, dpsettings->step_parameter, dpsettings->step_decrement, dpsettings->derivative_step);*/
			method_info = (gpointer) hosdainfo;
			dp_opt_add_func(hopt, dp_opt_osda, tau_flag, opt_type, order, method_info);
		}
	}
	return 0;
}
