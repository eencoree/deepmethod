/***************************************************************************
 *            xmmodel.c
 *
 *  Tue Mar 27 12:06:21 2012
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

#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#if defined(__MINGW32__)
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#endif
#ifdef GIO_STANDALONE_SOURCE
#include <gio/gfile.h>
#else
#include <gio/gio.h>
#endif
#include "xmmodel.h"

#ifdef GIO_STANDALONE_SOURCE
#define g_strcmp0(str1, str2) strcmp(str1, str2)
#endif

XmModel*xm_model_new()
{
	XmModel *xmmodel = (XmModel*)g_malloc( sizeof(XmModel) );
	xmmodel->size = 0;
	xmmodel->tweak = NULL;
	xmmodel->command = NULL;
	xmmodel->delimiters = NULL;
	xmmodel->keys = NULL;
	xmmodel->num_values = 0;
	xmmodel->array = NULL;
	xmmodel->mapping = NULL;
	xmmodel->prime_command = NULL;	
	xmmodel->prime_delimiters = NULL;
	xmmodel->prime_keys = NULL;
	xmmodel->num_prime_values = 0;
	xmmodel->prime_array = NULL;
	xmmodel->prime_mapping = NULL;
	xmmodel->current_penalty_index = 0;
	xmmodel->current_functional_value = G_MAXDOUBLE;
	xmmodel->prime_index = 0;
	xmmodel->functional_value = G_MAXDOUBLE;
	xmmodel->parms = NULL;
	xmmodel->index = NULL;
	xmmodel->dparms = NULL;
	xmmodel->bparms = NULL;
	xmmodel->lbound = NULL;
	xmmodel->hbound = NULL;
	xmmodel->convert = NULL;
	return xmmodel;
}

gpointer xm_model_copy_values(gpointer psrc)
{
	int j, k;
	XmModel *src = (XmModel *)psrc;
	XmModel *xmmodel = xm_model_new();
	xmmodel->index_size = src->index_size;
	xmmodel->index = g_new0 ( int, xmmodel->index_size );
	xmmodel->tweak_index = g_new0 ( int, xmmodel->index_size );
	for ( j = 0; j < xmmodel->index_size; j++ ) {
		xmmodel->index[j] = src->index[j];
		xmmodel->tweak_index[j] = src->tweak_index[j];
	}
	xmmodel->num_parts = src->num_parts;
	xmmodel->part = g_new ( XmModelPart, xmmodel->num_parts );
	for ( j = 0; j < xmmodel->num_parts; j++ ) {
		xmmodel->part[j].name = g_strdup(src->part[j].name);
		xmmodel->part[j].num_parms = src->part[j].num_parms;
		xmmodel->part[j].index = g_new( int, xmmodel->part[j].num_parms );
		for ( k = 0; k < xmmodel->part[j].num_parms; k++ ) {
			xmmodel->part[j].index[k] = src->part[j].index[k];
		}
	}
	xmmodel->converter = src->converter;
	xmmodel->size = src->size;
	xmmodel->parms = g_new0 ( int, xmmodel->size );
	xmmodel->iparms = g_new0 ( int, xmmodel->size );
	xmmodel->lookup = g_new0 ( int, xmmodel->size );
	xmmodel->tweak = g_new0 ( int, xmmodel->size );
	xmmodel->mask = g_new0 ( int, xmmodel->size );
	xmmodel->dparms = g_new0 ( double, xmmodel->size );
	xmmodel->bparms = g_new0 ( double, xmmodel->size );
	xmmodel->lbound = g_new0 ( double, xmmodel->size );
	xmmodel->hbound = g_new0 ( double, xmmodel->size );
	for ( j = 0; j < xmmodel->size; j ++ ) {
		xmmodel->parms[j] = src->parms[j];
		xmmodel->iparms[j] = src->iparms[j];
		xmmodel->lookup[j] = src->lookup[j];
		xmmodel->mask[j] = src->mask[j];
		xmmodel->tweak[j] = src->tweak[j];
		xmmodel->dparms[j] = src->dparms[j];
		xmmodel->bparms[j] = src->bparms[j];
		xmmodel->lbound[j] = src->lbound[j];
		xmmodel->hbound[j] = src->hbound[j];
	}
	xmmodel->command = g_strdup(src->command);
	if ( src->prime_command != NULL) {
		xmmodel->prime_command = g_strdup(src->prime_command);
		xmmodel->prime_delimiters = g_strdup(src->prime_delimiters);
		xmmodel->num_prime_values = src->num_prime_values;
		xmmodel->prime_array = g_new0 ( double, xmmodel->num_prime_values );
		xmmodel->prime_keys = g_new0 ( int, xmmodel->num_prime_values );
		xmmodel->prime_mapping = g_new0 ( int, xmmodel->num_prime_values );
		for ( j = 0; j < xmmodel->num_prime_values; j++ ) {
			xmmodel->prime_array[j] = src->prime_array[j];
			xmmodel->prime_keys[j] = src->prime_keys[j];
			xmmodel->prime_mapping[j] = src->prime_mapping[j];
		}
		xmmodel->prime_index = src->prime_index;
	}
	xmmodel->delimiters = g_strdup(src->delimiters);
	xmmodel->num_values = src->num_values;
	xmmodel->array = g_new0 ( double, xmmodel->num_values );
	xmmodel->keys = g_new0 ( int, xmmodel->num_values );
	xmmodel->mapping = g_new0 ( int, xmmodel->num_values );
	for ( j = 0; j < xmmodel->num_values; j++ ) {
		xmmodel->array[j] = src->array[j];
		xmmodel->keys[j] = src->keys[j];
		xmmodel->mapping[j] = src->mapping[j];
	}
	xmmodel->current_penalty_index = src->current_penalty_index;
	xmmodel->current_functional_value = src->current_functional_value;
	xmmodel->convert = g_strdup(src->convert);
	xmmodel->functional_value = src->functional_value;
	return xmmodel;
}

void xm_model_update_values(gpointer psrc, double*x, int prime_index, double functional_value)
{
	XmModel *xmmodel = (XmModel *)psrc;
	int i, index;
	for ( i = 0; i < xmmodel->index_size; i++) {
		index = xmmodel->tweak_index[i];
		xmmodel->dparms[index] = x[i];
	}
	xmmodel->prime_index = prime_index;
	xmmodel->functional_value = functional_value;
}

void xm_model_set_dparms(XmModel *xmmodel, double*x)
{
	int i, index, j;
	for ( i = 0; i < xmmodel->index_size; i++) {
		index = xmmodel->tweak_index[i];
		xmmodel->dparms[index] = x[i];
	}
}

int xm_model_run(GString *params, XmModel *xmmodel)
{
	gchar**result;
	gchar**margv;
	int argcp, i, j;
	GString *command;
	GError*gerror = NULL;
	int flaggs, child_exit_status;
	gchar *standard_output;
	gchar *standard_error;
	gchar*conversion;
	command = g_string_new(xmmodel->command);
	if ( xmmodel->convert != NULL ) {
		if ( ( conversion = xmmodel->converter((gpointer)xmmodel, &gerror) ) == NULL ) {
			g_string_free(command, TRUE);
			for ( i = 0; i < xmmodel->num_values; i++ ) {
				xmmodel->array[i] = G_MAXDOUBLE;
			}
			return child_exit_status;
		}
		if ( gerror ) {
			g_error("converter failed for %s", gerror->message);
		}
		g_string_append_printf(command, " %s", conversion);
	} else {
		g_string_append_printf(command, " '%s'", params->str);
	}
	if ( !g_shell_parse_argv(command->str, &argcp, &margv, &gerror) ) {
		if ( gerror ) {
			g_error("g_shell_parse failed for %s\nwith %s", command->str, gerror->message);
		}
	}
	g_string_free(command, TRUE);
/*	flaggs = G_SPAWN_SEARCH_PATH | G_SPAWN_STDOUT_TO_DEV_NULL; */
/*	flaggs |= G_SPAWN_STDERR_TO_DEV_NULL; */
	flaggs = G_SPAWN_SEARCH_PATH;
	if ( !g_spawn_sync (NULL, margv, NULL, (GSpawnFlags)flaggs, NULL, NULL, &standard_output, &standard_error, &child_exit_status, &gerror) ) {
		if ( gerror ) {
			g_error("g_spawn_sync failed for %s\nwith %s", margv[0], gerror->message);
		}
	}
	g_strfreev(margv);
	result = g_strsplit_set(standard_output, xmmodel->delimiters, -1);
	if ( result != NULL ) {
		for ( i = 0; i < xmmodel->num_values; i++ ) {
			if ( result[xmmodel->keys[i]] != NULL ) {
				xmmodel->array[i] = g_strtod(result[xmmodel->keys[i]], NULL);
			} else {
				g_warning ( "result[%d] doesn't exist", xmmodel->keys[i]);
				for ( j = 0; j < g_strv_length(result); j++ ) {
					g_printf("result[%d] = %s;\n", j, result[j]);
				}
			}
		}
	} else {
		g_warning ( "Couldn't parse output: %s", standard_output);
	}
	g_strfreev(result);
	g_free(standard_output);
	g_free(standard_error);
	if ( xmmodel->convert != NULL ) {
		g_unlink(conversion);
	}
	return child_exit_status;
}

int xm_model_run_prime(GString *params, XmModel *xmmodel)
{
	gchar**result;
	gchar**margv;
	int argcp, i;
	GString *command;
	GError*gerror = NULL;
	int flaggs, child_exit_status;
	gchar *standard_output;
	gchar *standard_error;
	gchar*conversion;
	command = g_string_new(xmmodel->prime_command);
	if ( xmmodel->convert != NULL ) {
		if ( ( conversion = xmmodel->converter((gpointer)xmmodel, &gerror) ) == NULL ) {
			g_string_free(command, TRUE);
			for ( i = 0; i < xmmodel->num_values; i++ ) {
				xmmodel->array[i] = G_MAXDOUBLE;
			}
			return child_exit_status;
		}
		if ( gerror ) {
			g_error("converter failed for %s", gerror->message);
		}
		g_string_append_printf(command, " %s", conversion);
	} else {
		g_string_append_printf(command, " '%s'", params->str);
	}
	g_string_append_printf(command, " %d", xmmodel->prime_index);
	if ( !g_shell_parse_argv(command->str, &argcp, &margv, &gerror) ) {
		if ( gerror ) {
			g_error("g_shell_parse failed for %s\nwith %s", command->str, gerror->message);
		}
	}
	g_string_free(command, TRUE);
/*	flaggs = G_SPAWN_SEARCH_PATH | G_SPAWN_STDOUT_TO_DEV_NULL; */
/*	flaggs |= G_SPAWN_STDERR_TO_DEV_NULL; */
	flaggs = G_SPAWN_SEARCH_PATH;
	if ( !g_spawn_sync (NULL, margv, NULL, (GSpawnFlags)flaggs, NULL, NULL, &standard_output, &standard_error, &child_exit_status, &gerror) ) {
		if ( gerror ) {
			g_error("g_spawn_sync failed for %s\nwith %s", margv[0], gerror->message);
		}
	}
	g_strfreev(margv);
	result = g_strsplit_set(standard_output, xmmodel->prime_delimiters, -1);
	for ( i = 0; i < xmmodel->num_prime_values; i++ ) {
		xmmodel->prime_array[i] = g_strtod(result[xmmodel->prime_keys[i]], NULL);
	}
	g_strfreev(result);
	g_free(standard_output);
	g_free(standard_error);
	if ( xmmodel->convert != NULL ) {
		g_unlink(conversion);
	}
	return child_exit_status;
}

int xm_model_parms_sort_index_ascending(gpointer p1, gpointer p2, gpointer user_data)
{
	XmModel *xmmodel = (XmModel *)user_data;
	int*i1 = (int*)p1;
	int*i2 = (int*)p2;
	int ii1 = xmmodel->tweak_index[(*i1)];
	int ii2 = xmmodel->tweak_index[(*i2)];
	int a = xmmodel->dparms[ii1];
	int b = xmmodel->dparms[ii2];
	if ( a < b ) {
		return -1;
	} else if ( a > b ) {
		return 1;
	}
	return 0;
}

double xm_model_dparms_to_int(gpointer user_data)
{
	XmModel *xmmodel = (XmModel *)user_data;
	double val = -1;
	int i, j;
	for ( i = 0; i < xmmodel->index_size; i++ ) {
		xmmodel->index[i] = i;
	}
	g_qsort_with_data(xmmodel->index, xmmodel->index_size, sizeof(xmmodel->index[0]), (GCompareDataFunc)xm_model_parms_sort_index_ascending, xmmodel);
	j = 0;
	for ( i= 0; i < xmmodel->size; i++ ) {
		if ( xmmodel->tweak[i] == 1 ) {
			xmmodel->parms[ xmmodel->tweak_index[ j ] ] = xmmodel->tweak_index[ xmmodel->index[ j ] ];
			j++;
		} else {
			xmmodel->parms[i] = xmmodel->iparms[i];
		}
	}
/*for ( i = 0; i < xmmodel->size; i++ ) {
	if ( xmmodel->tweak[i] == 1 ) {
		fprintf(stderr, "%f ", xmmodel->dparms[i]);
	}
}
fprintf(stderr, "==>");
for ( i = 0; i < xmmodel->size; i++ ) {
	if ( xmmodel->tweak[i] == 1 ) {
		fprintf(stderr, "%d[%d] ", i, xmmodel->parms[i]);
	}
}
fprintf(stderr, "\n");*/
	return val;
}

double xm_model_parms_double_to_int(gpointer user_data, double*x)
{
	XmModel *xmmodel = (XmModel *)user_data;
	double val = -1;
	int i, j;
	xm_model_set_dparms(xmmodel, x);
	val = xm_model_dparms_to_int(user_data);
	return val;
}

double xm_model_score_double(gpointer user_data, double*x)
{
	XmModel *xmmodel = (XmModel *)user_data;
	double val = G_MAXDOUBLE;
	GString *params = g_string_new("");
	int i;
	xm_model_set_dparms(xmmodel, x);
	for ( i= 0; i < xmmodel->size; i++ ) {
		g_string_append_printf(params, "%f ", xmmodel->dparms[i]);
	}
	xm_model_run(params, xmmodel);
	g_string_free(params, TRUE);
	val = xmmodel->array[xmmodel->mapping[0]];
	xmmodel->current_penalty_index = 1;
	xmmodel->current_functional_value = val;
	return val;
}

double xm_model_score_int(gpointer user_data, double*x)
{
	XmModel *xmmodel = (XmModel *)user_data;
	double val = G_MAXDOUBLE;
	GString *params = g_string_new("");
	int i;
	for ( i= 0; i < xmmodel->size; i++ ) {
		g_string_append_printf(params, "%d ", xmmodel->parms[i]);
	}
	xm_model_run(params, xmmodel);
	g_string_free(params, TRUE);
	val = xmmodel->array[xmmodel->mapping[0]];
	xmmodel->current_penalty_index = 1;
	xmmodel->current_functional_value = val;
	return val;
}

double xm_model_prime_double(gpointer user_data, double*x)
{
	XmModel *xmmodel = (XmModel *)user_data;
	double val = G_MAXDOUBLE;
	GString *params = g_string_new("");
	int i;
	xm_model_set_dparms(xmmodel, x);
	for ( i= 0; i < xmmodel->size; i++ ) {
		g_string_append_printf(params, "%f ", xmmodel->dparms[i]);
	}
	xm_model_run_prime(params, xmmodel);
	g_string_free(params, TRUE);
	val = xmmodel->prime_array[xmmodel->prime_mapping[0]];
	return val;
}

double xm_model_prime_int(gpointer user_data, double*x)
{
	XmModel *xmmodel = (XmModel *)user_data;
	double val = G_MAXDOUBLE;
	GString *params = g_string_new("");
	int i;
	for ( i= 0; i < xmmodel->size; i++ ) {
		g_string_append_printf(params, "%d ", xmmodel->parms[i]);
	}
	xm_model_run_prime(params, xmmodel);
	g_string_free(params, TRUE);
	val = xmmodel->prime_array[xmmodel->prime_mapping[0]];
	return val;
}

double xm_model_read_penalty(gpointer user_data, double*x)
{
	XmModel *xmmodel = (XmModel *)user_data;
	double val = G_MAXDOUBLE;
	val = xmmodel->array[xmmodel->mapping[ xmmodel->current_penalty_index ]];
	xmmodel->current_penalty_index++;
	return val;
}

double xm_model_barrier_penalty(gpointer user_data, double*x)
{
	double sqr_dist, dist;
	int i, index;
	XmModel *xmmodel = (XmModel *)user_data;
	double val = G_MAXDOUBLE;
	sqr_dist = 0;
	for ( i = 0; i < xmmodel->index_size; i++ ) {
		index = xmmodel->tweak_index[i];
		dist = xmmodel->bparms[index] - x[i];
		sqr_dist += dist * dist;
	}
	val = ( sqr_dist > 0 ) ? ( xmmodel->current_functional_value - xmmodel->functional_value ) / sqr_dist : xmmodel->current_functional_value;
	return val;
}

gchar*xm_model_read(gchar*filename, gsize*size, GError**err)
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

int xm_model_load(gchar*data, gsize size, gchar*groupname, XmModel *xmmodel, GError**err)
{
	GKeyFile*gkf;
	GError *gerror = NULL;
	gchar*str, **strlist;
	gsize length;
	int *ilist, ii, j, jj, k, kk;
	double *dlist, dval;
	int retval = 0;
	g_return_val_if_fail (err == NULL || *err == NULL, 1);
	gkf = g_key_file_new();
	if ( !g_key_file_load_from_data(gkf, (const gchar*)data, size, G_KEY_FILE_NONE, &gerror ) ) {
		g_propagate_error (err, gerror);
		return 1;
	}
	if ( ( ilist = g_key_file_get_integer_list(gkf, groupname, "parms", &length, &gerror) ) != NULL ) {
		xmmodel->size = length;
		xmmodel->parms = ilist;
		xmmodel->iparms = g_new0 ( int, length );
		xmmodel->lookup = g_new0 ( int, length );
		for ( j = 0; j < xmmodel->size; j ++ ) {
			xmmodel->iparms[j] = xmmodel->parms[j];
		}
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( length = g_key_file_get_integer(gkf, groupname, "numparms", &gerror) ) != 0  || gerror == NULL ) {
		xmmodel->size = length;
		xmmodel->parms = g_new0 ( int, length );
		xmmodel->iparms = g_new0 ( int, length );
		xmmodel->lookup = g_new0 ( int, length );
		for ( j = 0; j < xmmodel->size; j ++ ) {
			xmmodel->parms[j] = j;
			xmmodel->iparms[j] = xmmodel->parms[j];
		}
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( ilist = g_key_file_get_integer_list(gkf, groupname, "mask", &length, &gerror) ) != NULL ) {
		xmmodel->mask = ilist;
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( length = g_key_file_get_integer(gkf, groupname, "nummask", &gerror) ) != 0  || gerror == NULL ) {
		xmmodel->mask = g_new0 ( int, xmmodel->size );
		for ( j = 0; j < length; j ++ ) {
			xmmodel->mask[j] = 1;
		}
		for ( j = length; j < xmmodel->size; j ++ ) {
			xmmodel->mask[j] = 0;
		}
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( ilist = g_key_file_get_integer_list(gkf, groupname, "tweak", &length, &gerror) ) != NULL ) {
		xmmodel->tweak = ilist;
		xmmodel->index_size = 0;
		for ( j = 0; j < xmmodel->size; j++) {
			if ( xmmodel->tweak[j] == 1 ) {
				xmmodel->index_size++;
			}
		}
		xmmodel->index = g_new0 ( int, xmmodel->index_size );
		xmmodel->tweak_index = g_new0 ( int, xmmodel->index_size );
		k = 0;
		for ( j = 0; j < xmmodel->size; j++) {
			if ( xmmodel->tweak[j] == 1 ) {
				xmmodel->tweak_index[k] = j;
				k++;
			}
		}

	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( length = g_key_file_get_integer(gkf, groupname, "numtweak", &gerror) ) != 0  || gerror == NULL ) {
		xmmodel->index_size = length;
		xmmodel->tweak = g_new0 ( int, xmmodel->size );
		xmmodel->index = g_new0 ( int, xmmodel->index_size );
		xmmodel->tweak_index = g_new0 ( int, xmmodel->index_size );
		for ( j = 0; j < length; j ++ ) {
			xmmodel->tweak[j] = 1;
			xmmodel->tweak_index[j] = j;
		}
		for ( j = length; j < xmmodel->size; j ++ ) {
			xmmodel->tweak[j] = 0;
		}
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( ilist = g_key_file_get_integer_list(gkf, groupname, "valnotweak", &length, &gerror) ) != NULL ) {
		ii = xmmodel->size - length;
		xmmodel->index_size = ii;
		xmmodel->tweak = g_new0 ( int, xmmodel->size );
		xmmodel->index = g_new0 ( int, ii );
		xmmodel->tweak_index = g_new0 ( int, ii );
		for ( j = 0; j < length; j ++ ) {
			xmmodel->tweak[ ilist[j] ] = -1;
		}
		k = 0;
		for ( j = 0; j < xmmodel->size; j ++ ) {
			if ( xmmodel->tweak[j] == 0 ) {
				xmmodel->tweak[j] = 1;
				xmmodel->tweak_index[k] = j;
				k++;
			}
		}
		for ( j = 0; j < length; j ++ ) {
			xmmodel->tweak[ ilist[j] ] = 0;
		}
		g_free(ilist);
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( dlist = g_key_file_get_double_list(gkf, groupname, "dparms", &length, &gerror) ) != NULL ) {
		xmmodel->dparms = dlist;
		xmmodel->bparms = g_new0 ( double, xmmodel->size );
		for ( j = 0; j < xmmodel->size; j ++ ) {
			xmmodel->bparms[j] = xmmodel->dparms[j];
		}
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( length = g_key_file_get_integer(gkf, groupname, "numdparms", &gerror) ) != 0  || gerror == NULL ) {
		xmmodel->dparms = g_new0 ( double, xmmodel->size );
		xmmodel->bparms = g_new0 ( double, xmmodel->size );
		for ( j = 0; j < xmmodel->size; j ++ ) {
			xmmodel->dparms[j] = j;
			xmmodel->bparms[j] = xmmodel->dparms[j];
		}
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( dlist = g_key_file_get_double_list(gkf, groupname, "lbound", &length, &gerror) ) != NULL ) {
		xmmodel->lbound = dlist;
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( dval = g_key_file_get_double(gkf, groupname, "vallbound", &gerror) ) != 0 || gerror == NULL ) {
		xmmodel->lbound = g_new0 ( double, xmmodel->size );
		for ( j = 0; j < xmmodel->size; j ++ ) {
			xmmodel->lbound[j] = dval;
		}
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( dlist = g_key_file_get_double_list(gkf, groupname, "hbound", &length, &gerror) ) != NULL ) {
		xmmodel->hbound = dlist;
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( dval = g_key_file_get_double(gkf, groupname, "valhbound", &gerror) ) != 0  || gerror == NULL ) {
		xmmodel->hbound = g_new0 ( double, xmmodel->size );
		for ( j = 0; j < xmmodel->size; j ++ ) {
			xmmodel->hbound[j] = dval;
		}
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( ilist = g_key_file_get_integer_list(gkf, groupname, "keys", &length, &gerror) ) != NULL ) {
		xmmodel->keys = ilist;
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( ilist = g_key_file_get_integer_list(gkf, groupname, "mapping", &length, &gerror) ) != NULL ) {
		xmmodel->mapping = ilist;
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( ii = g_key_file_get_integer(gkf, groupname, "num_values", &gerror) ) != 0  || gerror == NULL ) {
		xmmodel->num_values = ii;
		xmmodel->array = g_new0 ( double, ii );
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "delimiters", &gerror) ) != NULL ) {
		xmmodel->delimiters = str;
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "command", &gerror) ) != NULL ) {
		xmmodel->command = str;
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( ilist = g_key_file_get_integer_list(gkf, groupname, "prime_keys", &length, &gerror) ) != NULL ) {
		xmmodel->prime_keys = ilist;
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( ilist = g_key_file_get_integer_list(gkf, groupname, "prime_mapping", &length, &gerror) ) != NULL ) {
		xmmodel->prime_mapping = ilist;
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( ii = g_key_file_get_integer(gkf, groupname, "num_prime_values", &gerror) ) != 0  || gerror == NULL ) {
		xmmodel->num_prime_values = ii;
		xmmodel->prime_array = g_new0 ( double, ii );
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "prime_delimiters", &gerror) ) != NULL ) {
		xmmodel->prime_delimiters = str;
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "prime_command", &gerror) ) != NULL ) {
		xmmodel->prime_command = str;
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( str = g_key_file_get_string(gkf, groupname, "convert", &gerror) ) != NULL ) {
		xmmodel->convert = str;
		if ( !g_strcmp0 ( xmmodel->convert, "sdf" ) ) {
			xmmodel->converter = xm_model_convert_parms_to_sdf;
		} else if ( !g_strcmp0 ( xmmodel->convert, "gemstat" ) ) {
			xmmodel->converter = xm_model_convert_parms_to_gemstat;
		} else if ( !g_strcmp0 ( xmmodel->convert, "subset" ) ) {
			xmmodel->converter = xm_model_convert_parms_to_subset;
		} else if ( !g_strcmp0 ( xmmodel->convert, "subsubset" ) ) {
			xmmodel->converter = xm_model_convert_parms_to_subsubset;
		} else if ( !g_strcmp0 ( xmmodel->convert, "octave" ) ) {
			xmmodel->converter = xm_model_convert_parms_to_octave;
		}
	} else {
		g_warning ( gerror->message );
		g_clear_error (&gerror);
	}
	if ( ( strlist = g_key_file_get_string_list(gkf, groupname, "parts", &length, &gerror) ) != NULL ) {
		xmmodel->num_parts = length / 2;
		xmmodel->part = g_new ( XmModelPart, xmmodel->num_parts );
		kk = 0;
		for ( j = 0, jj = 0; j < length; j += 2, jj++ ) {
			xmmodel->part[jj].name = g_strdup(strlist[j]);
			xmmodel->part[jj].num_parms = g_strtod (strlist[j + 1], NULL);
			xmmodel->part[jj].index = g_new( int, xmmodel->part[jj].num_parms );
			for ( k = 0; k < xmmodel->part[jj].num_parms; k++ ) {
				xmmodel->part[jj].index[k] = kk;
				xmmodel->lookup[kk] = jj;
				kk++;
			}
		}
	} else {
		g_warning ( gerror->message );	
		g_clear_error (&gerror);
	}
	g_key_file_free(gkf);
	return retval;
}

int xm_model_init(gchar*filename, gchar*groupname, XmModel*xmmodel, GError **err)
{
	int ret_val = 0;
	gchar*data = NULL;
	gsize size;
	GError *gerror = NULL;
	g_return_val_if_fail (err == NULL || *err == NULL, 1);
	if ( ( data = xm_model_read(filename, &size, &gerror) ) == NULL ) {
		g_propagate_error (err, gerror);
		return 1;
	}
	ret_val = xm_model_load(data, size, groupname, xmmodel, &gerror);
	if ( ret_val == 1 ) {
		g_propagate_error (err, gerror);
		ret_val = 1;
	}
	g_free(data);
	return ret_val;
}

int xm_model_get_next_free_conn(int connected_to_conn, XmModel*xmmodel)
{
	int curr, i;
	int ret_val = connected_to_conn;
	int connected_to;
	curr = connected_to_conn;
	for ( i = 0; i < xmmodel->size; i++ ) {
		curr++;
		if ( curr > xmmodel->size - 1 ) {
			curr = 0;
		}
		connected_to = xmmodel->lookup[curr];
		if ( xmmodel->parms[curr] != -1 && xmmodel->tweak[curr] == 1 && xmmodel->part[connected_to].checked_num_conn < xmmodel->part[connected_to].num_parms ) {
			ret_val = curr;
			break;
		}
	}
	return ret_val;
}

gint xm_model_sort_atoms (gconstpointer a, gconstpointer b)
{
	int i = GPOINTER_TO_INT(a);
	int j = GPOINTER_TO_INT(b);
	if ( i > j ) {
		return 1;
	} else if ( i < j ) {
		return -1;
	}
	return 0;
}

GString*xm_model_sdf_contents(XmModel*xmmodel)
{
	GQueue *gqueue;
	GList*bonds = NULL, *curr_bond;
	GList*atoms = NULL, *curr_atom;
	GString*file_contents, *file_contents_atoms, *file_contents_bonds;
	int i, j, k;
	int ABORT = 0;
	XmModelConn*conn, *curr_conn;
	for ( i = 0; i < xmmodel->num_parts; i++ ) {
		xmmodel->part[i].checked_flag = 0;
		xmmodel->part[i].checked_num_conn = 0;
	}
/*
for ( i = 0; i < xmmodel->size; i++ ) {
	if ( xmmodel->tweak[i] == 1 ) {
		fprintf(stderr, "%d[%d] ", i, xmmodel->parms[i]);
	}
}
fprintf(stderr, "\n");
*/
	int num_atoms = 0;
	int num_bonds = 0;
	file_contents_atoms = g_string_new("");
	file_contents_bonds = g_string_new("");
	gqueue = g_queue_new ();
/*	g_queue_init(gqueue);*/
	for ( i = 0; i < xmmodel->size; i++ ) {
		if ( xmmodel->mask[i] == 1 ) {
			int index_of_part, index_of_part_conn;
			int connected_to, connected_to_conn;
			g_queue_push_head(gqueue, GINT_TO_POINTER(i));
			while ( !g_queue_is_empty(gqueue) ) {
				index_of_part_conn = GPOINTER_TO_INT(g_queue_pop_head(gqueue));
				if ( ( connected_to_conn = xmmodel->parms[index_of_part_conn] ) == -1 ) {
					continue;
				}
				index_of_part = xmmodel->lookup[index_of_part_conn];
				if ( xmmodel->tweak[connected_to_conn] == 0 && index_of_part_conn != xmmodel->parms[connected_to_conn] ) {
					connected_to_conn = xm_model_get_next_free_conn(connected_to_conn, xmmodel);
					xmmodel->parms[index_of_part_conn] = connected_to_conn;
				}
				connected_to = xmmodel->lookup[connected_to_conn];
				if ( xmmodel->part[connected_to].checked_num_conn > xmmodel->part[connected_to].num_parms - 1 || connected_to_conn == index_of_part_conn ) {
					ABORT = 1;
					break;
				}
				xmmodel->parms[connected_to_conn] = -1;
				if ( xmmodel->part[index_of_part].checked_flag == 0 ) {
					xmmodel->part[index_of_part].checked_flag = 1;
					atoms = g_list_prepend(atoms, GINT_TO_POINTER(index_of_part));
					num_atoms++;
					for ( k = 0; k < xmmodel->part[index_of_part].num_parms; k++ ) {
						if ( xmmodel->part[index_of_part].index[k] != index_of_part_conn ) {
							g_queue_push_head(gqueue, GINT_TO_POINTER(xmmodel->part[index_of_part].index[k]));
						}
					}
				}
				if ( xmmodel->part[connected_to].checked_flag == 0 ) {
					xmmodel->part[connected_to].checked_flag = 1;
					atoms = g_list_prepend(atoms, GINT_TO_POINTER(connected_to));
					num_atoms++;
					for ( k = 0; k < xmmodel->part[connected_to].num_parms; k++ ) {
						if ( xmmodel->part[connected_to].index[k] != connected_to_conn ) {
							g_queue_push_head(gqueue, GINT_TO_POINTER(xmmodel->part[connected_to].index[k]));
						}
					}
				}
				conn = xm_model_conn_new(index_of_part, index_of_part_conn, connected_to, connected_to_conn, 0);
				if ( ( curr_bond = g_list_find_custom ( bonds, (gconstpointer) conn, (GCompareFunc)xm_model_conn_comp ) ) != NULL ) {
					curr_conn = (XmModelConn*)(curr_bond->data);
					if ( conn->dest_conn != curr_conn->dest_conn && conn->source_conn != curr_conn->source_conn ) {
						xm_model_conn_free(conn);
						curr_conn->rank++;
						xmmodel->part[curr_conn->dest].checked_num_conn++;
						xmmodel->part[curr_conn->source].checked_num_conn++;
					}
				} else {
					xmmodel->part[conn->dest].checked_num_conn++;
					xmmodel->part[conn->source].checked_num_conn++;
					bonds = g_list_prepend(bonds, (gpointer)conn);
					num_bonds++;
				}
			}
		}
	}
	g_queue_free (gqueue);
	if ( ABORT == 1 ) {
		g_list_free(bonds);
		g_list_free(atoms);
		g_string_free(file_contents_atoms, TRUE);
		g_string_free(file_contents_bonds, TRUE);
		return NULL;
	}
	atoms = g_list_sort (atoms, (GCompareFunc) xm_model_sort_atoms);
	for (i = 0, curr_atom = g_list_first(atoms); i < num_atoms; i++, curr_atom = g_list_next(curr_atom)) {
		j = GPOINTER_TO_INT(curr_atom->data);
		g_string_append_printf(file_contents_atoms, "        0          0    0      %s   0  0  0  0  0  0  0  0  0  0  0  0\n", xmmodel->part[j].name);
	}
	for (i = 0, curr_bond = g_list_first(bonds); i < num_bonds; i++, curr_bond = g_list_next(curr_bond)) {
		conn = (XmModelConn*)(curr_bond->data);
		int source = g_list_index ( atoms, GINT_TO_POINTER(conn->source) );
		int dest = g_list_index ( atoms, GINT_TO_POINTER(conn->dest) );
		g_string_append_printf(file_contents_bonds, "%3d%3d%3d  0  0  0  0\n", source + 1, dest + 1, conn->rank + 1);
		xm_model_conn_free(conn);
	}
	g_list_free(bonds);
	g_list_free(atoms);
	file_contents = g_string_new("Name\n  Soft\n\n");
	g_string_append_printf(file_contents, "%3d%3d  0  0  0  0  0  0  0  0  1 V2000\n", num_atoms, num_bonds);
	file_contents = g_string_append(file_contents, (const gchar*)(file_contents_atoms->str));
	g_string_free(file_contents_atoms, TRUE);
	file_contents = g_string_append(file_contents, (const gchar*)(file_contents_bonds->str));
	g_string_free(file_contents_bonds, TRUE);
	file_contents = g_string_append(file_contents, "M END\n$$$$");
/*
for ( i = 0; i < xmmodel->size; i++ ) {
	if ( xmmodel->tweak[i] == 1 ) {
		fprintf(stderr, "%d[%d] ", i, xmmodel->parms[i]);
	}
}
fprintf(stderr, "\n");
fprintf (stderr, "%s", file_contents->str);*/
	return file_contents;
}

GString*xm_model_gemstat_contents(XmModel*xmmodel)
{
	GString*file_contents;
	int i, j, k;
	file_contents = g_string_new("");
	for ( i = 0; i < xmmodel->num_parts; i++ ) {
		g_string_append_printf(file_contents, "%s ", xmmodel->part[i].name);
		for ( j = 0; j < xmmodel->part[i].num_parms; j++ ) {
			k = xmmodel->part[i].index[j];
			g_string_append_printf(file_contents, "%16.9f ", xmmodel->dparms[k]);
		}
		file_contents = g_string_append_c(file_contents, '\n');
	}
	return file_contents;
}

GString*xm_model_subset_contents(XmModel*xmmodel)
{
	GString*file_contents;
	int i, j, k;
	file_contents = g_string_new("");
	k = 0;
	for ( i = 0; i < xmmodel->size; i++ ) {
		if ( xmmodel->tweak[i] == 1 ) {
			if ( xmmodel->mask[k] == 1 ) {
				g_string_append_printf(file_contents, "%d\n", xmmodel->parms[i]);
				k++;
			}
		} else {
			g_string_append_printf(file_contents, "%d\n", xmmodel->parms[i]);
		}
	}
	return file_contents;
}

GString*xm_model_subsubset_contents(XmModel*xmmodel)
{
	GString*file_contents;
	int i, j, k;
	int main_index = 0;
	int main_index_parm = xmmodel->parms[main_index];
	if ( main_index_parm == main_index ) {
		main_index_parm = xmmodel->parms[xmmodel->size - 1];
		xmmodel->parms[xmmodel->size - 1] = main_index;
		xmmodel->parms[main_index] = main_index_parm;
	}
	k = main_index_parm + 1;
	xmmodel->mask[main_index] = 0;
	for ( i = 1; i < k; i++ ) {
		xmmodel->mask[i] = 1;
		if ( xmmodel->parms[i] == main_index ) {
			xmmodel->parms[i] = main_index_parm;
		}
	}
	for ( i = k; i < xmmodel->size; i++ ) {
		xmmodel->mask[i] = 0;
	}
	file_contents = g_string_new("");
	k = 1;
	for ( i = 1; i < xmmodel->size; i++ ) {
		if ( xmmodel->tweak[i] == 1 ) {
			if ( xmmodel->mask[k] == 1 ) {
				g_string_append_printf(file_contents, "%d\n", xmmodel->parms[i]);
				k++;
			}
		} else {
			g_string_append_printf(file_contents, "%d\n", xmmodel->parms[i]);
		}
	}
	return file_contents;
}

GString*xm_model_octave_contents(XmModel*xmmodel)
{
	GString*file_contents;
	int i, j, k;
	file_contents = g_string_new("");
	for ( i = 0; i < xmmodel->num_parts; i++ ) {
		g_string_append_printf(file_contents, "%s =", xmmodel->part[i].name);
		if ( xmmodel->part[i].num_parms == 1 ) {
			k = xmmodel->part[i].index[0];
			g_string_append_printf(file_contents, "%16.9f;", xmmodel->dparms[k]);		
		} else if ( xmmodel->part[i].num_parms > 1 ) {
			g_string_append_printf(file_contents, "[");
			for ( j = 0; j < xmmodel->part[i].num_parms; j++ ) {
				k = xmmodel->part[i].index[j];
				g_string_append_printf(file_contents, "%16.9f ", xmmodel->dparms[k]);
			}			
			g_string_append_printf(file_contents, "];");
		}
		file_contents = g_string_append_c(file_contents, '\n');
	}
	return file_contents;
}

gchar *xm_model_convert_parms_to_sdf(gpointer *user_data, GError **err)
{
	XmModel*xmmodel = (XmModel*)user_data;
	gint fhandle;
	const gchar *tmpl = NULL;
	gchar *name_used = NULL;
	GError *gerror = NULL;
	GString*file_contents;
	g_return_val_if_fail (err == NULL || *err == NULL, NULL);
	if ( ( fhandle = g_file_open_tmp (tmpl, &name_used, &gerror) ) == -1 ) {
		g_propagate_error (err, gerror);
		return NULL;
	}
	close(fhandle);
	if ( ( file_contents = xm_model_sdf_contents(xmmodel) ) == NULL ) {
		g_unlink(name_used);
		return NULL;
	}
	if ( !g_file_set_contents ((const gchar *)name_used, (const gchar *)(file_contents->str), (gssize)(file_contents->len), &gerror) ) {
		g_string_free(file_contents, TRUE);
		g_propagate_error (err, gerror);
		g_unlink(name_used);
		return NULL;
	}
	g_string_free(file_contents, TRUE);
	return name_used;
}

gchar *xm_model_convert_parms_to_gemstat(gpointer *user_data, GError **err)
{
	XmModel*xmmodel = (XmModel*)user_data;
	gint fhandle;
	const gchar *tmpl = NULL;
	gchar *name_used = NULL;
	GError *gerror = NULL;
	GString*file_contents;
	g_return_val_if_fail (err == NULL || *err == NULL, NULL);
	if ( ( fhandle = g_file_open_tmp (tmpl, &name_used, &gerror) ) == -1 ) {
		g_propagate_error (err, gerror);
		return NULL;
	}
	close(fhandle);
	if ( ( file_contents = xm_model_gemstat_contents(xmmodel) ) == NULL ) {
		g_unlink(name_used);
		return NULL;
	}
	if ( !g_file_set_contents ((const gchar *)name_used, (const gchar *)(file_contents->str), (gssize)(file_contents->len), &gerror) ) {
		g_string_free(file_contents, TRUE);
		g_propagate_error (err, gerror);
		g_unlink(name_used);
		return NULL;
	}
	g_string_free(file_contents, TRUE);
	return name_used;
}

gchar *xm_model_convert_parms_to_subset(gpointer *user_data, GError **err)
{
	XmModel*xmmodel = (XmModel*)user_data;
	gint fhandle;
	const gchar *tmpl = NULL;
	gchar *name_used = NULL;
	GError *gerror = NULL;
	GString*file_contents;
	g_return_val_if_fail (err == NULL || *err == NULL, NULL);
	if ( ( fhandle = g_file_open_tmp (tmpl, &name_used, &gerror) ) == -1 ) {
		g_propagate_error (err, gerror);
		return NULL;
	}
	close(fhandle);
	if ( ( file_contents = xm_model_subset_contents(xmmodel) ) == NULL ) {
		g_unlink(name_used);
		return NULL;
	}
	if ( !g_file_set_contents ((const gchar *)name_used, (const gchar *)(file_contents->str), (gssize)(file_contents->len), &gerror) ) {
		g_string_free(file_contents, TRUE);
		g_propagate_error (err, gerror);
		g_unlink(name_used);
		return NULL;
	}
	g_string_free(file_contents, TRUE);
	return name_used;
}

gchar *xm_model_convert_parms_to_subsubset(gpointer *user_data, GError **err)
{
	XmModel*xmmodel = (XmModel*)user_data;
	gint fhandle;
	const gchar *tmpl = NULL;
	gchar *name_used = NULL;
	GError *gerror = NULL;
	GString*file_contents;
	g_return_val_if_fail (err == NULL || *err == NULL, NULL);
	if ( ( fhandle = g_file_open_tmp (tmpl, &name_used, &gerror) ) == -1 ) {
		g_propagate_error (err, gerror);
		return NULL;
	}
	close(fhandle);
	if ( ( file_contents = xm_model_subsubset_contents(xmmodel) ) == NULL ) {
		g_unlink(name_used);
		return NULL;
	}
	if ( !g_file_set_contents ((const gchar *)name_used, (const gchar *)(file_contents->str), (gssize)(file_contents->len), &gerror) ) {
		g_string_free(file_contents, TRUE);
		g_propagate_error (err, gerror);
		g_unlink(name_used);
		return NULL;
	}
	g_string_free(file_contents, TRUE);
	return name_used;
}

gchar *xm_model_convert_parms_to_octave(gpointer *user_data, GError **err)
{
	XmModel*xmmodel = (XmModel*)user_data;
	gint fhandle;
	const gchar *tmpl = NULL;
	gchar *name_used = NULL;
	GError *gerror = NULL;
	GString*file_contents;
	g_return_val_if_fail (err == NULL || *err == NULL, NULL);
	if ( ( fhandle = g_file_open_tmp (tmpl, &name_used, &gerror) ) == -1 ) {
		g_propagate_error (err, gerror);
		return NULL;
	}
	close(fhandle);
	if ( ( file_contents = xm_model_octave_contents(xmmodel) ) == NULL ) {
		g_unlink(name_used);
		return NULL;
	}
	if ( !g_file_set_contents ((const gchar *)name_used, (const gchar *)(file_contents->str), (gssize)(file_contents->len), &gerror) ) {
		g_string_free(file_contents, TRUE);
		g_propagate_error (err, gerror);
		g_unlink(name_used);
		return NULL;
	}
	g_string_free(file_contents, TRUE);
	return name_used;
}

XmModelConn*xm_model_conn_new(int source, int source_conn, int dest, int dest_conn, int rank)
{
	XmModelConn*conn;
	conn = (XmModelConn*)malloc(sizeof(XmModelConn));
	conn->source = source;
	conn->source_conn = source_conn;
	conn->dest = dest;
	conn->dest_conn = dest_conn;
	conn->rank = rank;
	return conn;
}

int xm_model_conn_comp (gconstpointer a, gconstpointer b)
{
	XmModelConn*conn1 = (XmModelConn*)a;
	XmModelConn*conn2 = (XmModelConn*)b;
	if ( conn1->source < conn2->source && conn1->dest < conn2->dest ) {
		return -1;
	} else if ( conn1->source == conn2->source && conn1->dest == conn2->dest ) {
		return 0;
	} else if ( conn1->source == conn2->dest && conn1->dest == conn2->source ) {
		return 0;
	}
	return 1;
}

int xm_model_conn_comp_ports (gconstpointer a, gconstpointer b)
{
	XmModelConn*conn1 = (XmModelConn*)a;
	XmModelConn*conn2 = (XmModelConn*)b;
	if ( conn1->source < conn2->source && conn1->dest < conn2->dest ) {
		return -1;
	} else if ( conn1->source == conn2->source && conn1->dest == conn2->dest && conn1->source_conn == conn2->source_conn && conn1->dest_conn == conn2->dest_conn ) {
		return 0;
	} else if ( conn1->source == conn2->dest && conn1->dest == conn2->source && conn1->source_conn == conn2->dest_conn && conn1->dest_conn == conn2->source_conn) {
		return 0;
	}
	return 1;
}

void xm_model_conn_free(XmModelConn*conn)
{
	free(conn);
}

void xm_model_save(XmModel*xmmodel, gchar*filename)
{
	GString*file_contents;
	GError*gerror = NULL;
	if ( !g_strcmp0 ( xmmodel->convert, "sdf" ) ) {
		if ( ( file_contents = xm_model_sdf_contents(xmmodel) ) == NULL ) {
			g_error("Can't get sdf contents");
		}
	} else if ( !g_strcmp0 ( xmmodel->convert, "gemstat" ) ) {
		if ( ( file_contents = xm_model_gemstat_contents(xmmodel) ) == NULL ) {
			g_error("Can't get gemstat contents");
		}
	} else if ( !g_strcmp0 ( xmmodel->convert, "subset" ) ) {
		if ( ( file_contents = xm_model_subset_contents(xmmodel) ) == NULL ) {
			g_error("Can't get subset contents");
		}
	} else if ( !g_strcmp0 ( xmmodel->convert, "subsubset" ) ) {
		if ( ( file_contents = xm_model_subsubset_contents(xmmodel) ) == NULL ) {
			g_error("Can't get subsubset contents");
		}
	} else if ( !g_strcmp0 ( xmmodel->convert, "octave" ) ) {
		if ( ( file_contents = xm_model_octave_contents(xmmodel) ) == NULL ) {
			g_error("Can't get subsubset contents");
		}
	}
	if ( !g_file_set_contents ((const gchar *)filename, (const gchar *)(file_contents->str), (gssize)(file_contents->len), &gerror) ) {
		g_string_free(file_contents, TRUE);
		g_error (gerror->message);
	}
	g_string_free(file_contents, TRUE);
}
