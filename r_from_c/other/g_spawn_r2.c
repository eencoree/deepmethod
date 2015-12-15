#include<glib.h>
#include<stdio.h>
    
static gboolean
cb_out_watch( GIOChannel   *channel, GIOCondition  cond, gpointer user_data )
{
    printf("->reading bash output\n");
    gchar *string;
    gsize  size;

    if( cond == G_IO_HUP )
    {
        g_io_channel_unref( channel );
        return( FALSE );
    }

    g_io_channel_read_line( channel, &string, &size, NULL, NULL );
    printf("output : \n:%s\n", string);
    g_free( string );

    return( TRUE );
}

int main(){
    GPid        pid;
    gchar      *launch[] = {"./helper", NULL };
    gint        in,
                out,
                err;
    GIOChannel *out_ch,
               *err_ch;
    gboolean    ret;

    ret = g_spawn_async_with_pipes( NULL, launch, NULL,
            G_SPAWN_DO_NOT_REAP_CHILD | 
            G_SPAWN_FILE_AND_ARGV_ZERO |
            G_SPAWN_SEARCH_PATH, NULL,
            NULL, &pid, &in, &out, &err, NULL );
    if( ! ret )
    {
        g_error( "SPAWN FAILED" );
        return 0;
    }

    /* Create channels that will be used to read data from pipes. */
    out_ch = g_io_channel_unix_new( out );
    err_ch = g_io_channel_unix_new( err );

    /* Add watches to channels */
    g_io_add_watch( out_ch, G_IO_IN | G_IO_HUP, (GIOFunc)cb_out_watch, NULL );
    g_io_add_watch( err_ch, G_IO_IN | G_IO_HUP, (GIOFunc)cb_out_watch, NULL );

    /*GIOChannel * input_channel;
    input_channel = g_io_channel_unix_new (in);

    GString * cmd = g_string_new("date");
    GString * carriage_return = g_string_new("\r\n");
    g_string_append(cmd,carriage_return->str);
    GError * error;
    error = NULL;
    gsize bytes_written;
    printf("Launching date in bash:\n");
    g_io_channel_write_chars( input_channel,
            cmd->str,
            cmd->len,
            &bytes_written,
            &error);
    if (error != NULL)
    {
        printf("%s\n", error->message);
    }*/

    return 0;
}
