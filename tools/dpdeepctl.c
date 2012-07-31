/***************************************************************************
 *            dpdeepctl.c
 *
 *  Fri Mar 23 16:45:04 2012
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
#include <glib/gthread.h>
#include <gio/gio.h>
#include "dpdeep.h"
#include "dprecombination.h"
#include "dpopt.h"
#include "dpsettings.h"
#include "xmmodel.h"
#include "xmtranslate.h"

static char*model_file;
static char*model_group;
static char*settings_file;
static char*settings_group;
static char*target_file;
static char*target_group;
static char*output_file;
static char*operation;

static GOptionEntry entries[] = 
{
	{ "model-file", 0, 0, G_OPTION_ARG_STRING, &model_file, "File name where model is described", "Model file name" },
	{ "model-group", 0, 0, G_OPTION_ARG_STRING, &model_group, "Group name where model is described", "Model group name" },
	{ "settings-file", 0, 0, G_OPTION_ARG_STRING, &settings_file, "File name where settings are described", "Settings file name" },
	{ "settings-group", 0, 0, G_OPTION_ARG_STRING, &settings_group, "Group name where settings are described", "Settings group name" },
	{ "target-file", 0, 0, G_OPTION_ARG_STRING, &target_file, "File name where target is described", "Target file name" },
	{ "target-group", 0, 0, G_OPTION_ARG_STRING, &target_group, "Group name where target is described", "Target group name" },
	{ "output-file", 0, 0, G_OPTION_ARG_STRING, &output_file, "File name to write", "Output file name" },
	{ "operation", 0, 0, G_OPTION_ARG_STRING, &operation, "What to do", "Operation" },
	{ NULL }
};

int main(int argc, char **argv)
{
	XmModel*xmmodel;
	DpOpt *hopt;
	DpSettings*dpsettings = dp_settings_new();
	DpEvaluation*heval;
	DpTarget*htarget;
	int world_id = 0, world_count = 1;
	GOptionContext *context;
	GError *gerror = NULL;
	g_thread_init (NULL);
	g_type_init();
	context = g_option_context_new ("- DEEP optimizer");
	g_option_context_add_main_entries(context, (const GOptionEntry *)entries, "deep");
	g_option_context_set_ignore_unknown_options(context, TRUE);
	if (!g_option_context_parse (context, &argc, &argv, &gerror)) {
		g_error ("option parsing failed: %s\n", gerror->message);
	}
	g_option_context_free (context);
	if ( model_file == NULL || model_group == NULL ) {
		g_error("%s called with wrong options for model", argv[0]);
	}
	if ( settings_file == NULL || settings_group == NULL ) {
		g_error("%s called with wrong options for settings", argv[0]);
	}
	if ( target_file == NULL || target_group == NULL ) {
		g_error("%s called with wrong options for target", argv[0]);
	}
	if ( output_file == NULL ) {
		g_error("%s called with wrong options for output", argv[0]);
	}
	if ( operation == NULL ) {
		g_error("%s called with wrong options for operation", argv[0]);
	}
	dp_settings_init(settings_file, settings_group, dpsettings, &gerror);
	xmmodel = xm_model_new();
	xm_model_init(model_file, model_group, xmmodel);
	if ( !g_strcmp0 ( operation, "optimize" ) ) {
		heval = xm_translate_parms(xmmodel);
		htarget = dp_target_new();
		dp_settings_target_init(target_file, target_group, htarget, &gerror);
		xm_translate_score(htarget, xmmodel);
		hopt = dp_opt_init(heval, htarget, world_id, world_count, settings_file, dpsettings->stop_type, dpsettings->criterion, dpsettings->tau, dpsettings->stop_count);
		dp_settings_process_run(dpsettings, hopt, world_id, heval, htarget, &gerror);
		dp_opt_run(hopt);
	}
	if ( hopt->hloop->stop_flag != DP_LOOP_EXIT_ERROR ) {
		xm_model_save(xmmodel, output_file);
	} else {
		g_error("Loop finished with an unknown error.\nOutput not produced.");
	}
	return 0;
}
