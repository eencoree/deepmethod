#include<glib.h>
#include<glib/gprintf.h>
#include<stdio.h>

static void cb_child_watch( GPid  pid,
        gint  status)
{
    /* Close pid */
    g_spawn_close_pid( pid );
}

static gboolean cb_out_watch( GIOChannel   *channel,
        GIOCondition  cond, gpointer data)
{
    gchar *string;
    gsize  size;

    if( cond == G_IO_HUP )
    {
        g_io_channel_unref( channel );
        return( FALSE );
    }

    g_io_channel_read_line( channel, &string, &size, NULL, NULL );
    g_print("Out: %s", string);
    g_free( string );

    return( TRUE );
}

static gboolean cb_err_watch( GIOChannel   *channel,
        GIOCondition  cond, gpointer data)
{
    gchar *string;
    gsize  size;

    if( cond == G_IO_HUP )
    {
        g_io_channel_unref( channel );
        return( FALSE );
    }

    g_io_channel_read_line( channel, &string, &size, NULL, NULL );
    g_print("Err: %s", string);
    g_free( string );

    return( TRUE );
}

void init(void){
    GPid        pid;
    //gchar      *argv[] = { "R", "--no-save", NULL };
    gchar      *argv[] = { "R", "--no-save", "--silent", NULL };
    GSpawnFlags flags = G_SPAWN_DO_NOT_REAP_CHILD |
        G_SPAWN_SEARCH_PATH;
    gint        in,
                out,
                err;
    GIOChannel *in_ch,
               *out_ch,
               *err_ch;
    gboolean    ret;

    /* Spawn child process */
    ret = g_spawn_async_with_pipes( NULL, argv, NULL,
            flags, NULL,
            NULL, &pid,
            &in, &out, &err, NULL );
    if( ! ret )
    {
        g_error( "SPAWN FAILED" );
        return;
    }

    g_child_watch_add( pid, (GChildWatchFunc)cb_child_watch, NULL);

    in_ch = g_io_channel_unix_new( in );
    out_ch = g_io_channel_unix_new( out );
    err_ch = g_io_channel_unix_new( err );

    /* Add watches to channels */
    g_io_add_watch( out_ch, G_IO_IN | G_IO_HUP, (GIOFunc)cb_out_watch, NULL);
    g_io_add_watch( err_ch, G_IO_IN | G_IO_HUP, (GIOFunc)cb_err_watch, NULL);

    GString * cmd = g_string_new("q <- 5\r\ncat(q,'\\n')\r\nflush.console()\r\n");
    GError * error = NULL;
    gsize bytes_written;
    //printf("Command sent: %s\n", cmd->str);
    g_io_channel_write_chars( in_ch,
            cmd->str,
            cmd->len,
            &bytes_written,
            &error);
    /*if (error != NULL)
    {
        printf("%s\n", error->message);
    }else{
        printf("bytes: %u\n", bytes_written);
    }*/

    g_io_channel_flush(in_ch, &error);
    /*if (error != NULL)
    {
        printf("%s\n", error->message);
    }else{
        printf("bytes after flush: %u\n", bytes_written);
    }*/

}

int main()
{
    GMainLoop *loop = g_main_loop_new (NULL, FALSE);
    init();
    g_printf("Starting loop\n");
    g_main_loop_run (loop);
    g_printf("Stopping loop");

    return 0;
}
