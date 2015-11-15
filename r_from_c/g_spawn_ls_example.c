#include <glib.h>

int main(int argc, char** argv){
    const gchar *command_line = "ls";
    gchar *output = NULL; // will contain command output
    GError *error = NULL;
    int exit_status = 0;
    if (!g_spawn_command_line_sync(command_line, &output, NULL, &exit_status, &error))
    {
        // handle error here
        g_print("%d", exit_status);
    }

    g_print("%s", output);

    return 0;
}
