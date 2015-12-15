#include <glib.h>

int main(int argc, char** argv){
    const gchar *command_line[] = { "R", "--no-save", NULL };
    gchar output = NULL; // will contain command output
    gchar err = NULL;
    GError *error = NULL;
    int exit_status = 0;
    GSpawnFlags flags = G_SPAWN_SEARCH_PATH;
    if (!g_spawn_async_with_pipes(NULL, command_line, NULL, flags, NULL, NULL, NULL, &output, &err, &exit_status, &error))
    {
        // handle error here
        g_print("%d", exit_status);
    }

    g_print("%s", output);
    g_print("%s", err);

    return 0;
}
