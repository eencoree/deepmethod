#include <glib.h>

int main(int argc, char** argv){
    const gchar *command_line[] = { "./simple2", NULL };
    const gchar *envp[] = { "LD_LIBRARY_PATH=/usr/lib/R/lib", "R_HOME=/usr/lib/R", NULL };
    gchar *output = NULL; // will contain command output
    GError *error = NULL;
    int exit_status = 0;
    GSpawnFlags flags = G_SPAWN_DEFAULT;
    if (!g_spawn_sync(NULL, command_line, envp, flags, NULL, NULL, &output, NULL, &exit_status, &error))
    {
        // handle error here
        g_print("%d", exit_status);
    }

    g_print("%s", output);

    return 0;
}
