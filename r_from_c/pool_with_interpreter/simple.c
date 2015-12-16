#include<glib.h>
#include<unistd.h>

#include<stdlib.h>

struct interpreter{
    GIOChannel *in,
               *out,
               *err;
};

void write_to_interpreter(GIOChannel* in, GString* msg){
    g_io_channel_write_chars( in,
            msg->str,
            msg->len,
            NULL,
            NULL);
    g_io_channel_flush(in, NULL);
}

gpointer pool_func(gpointer data, gpointer user_data){
    GAsyncQueue * queue = (GAsyncQueue*)user_data;

    struct interpreter* intprt = (struct interpreter*)g_async_queue_pop(queue); 
    int val = *((int*)data);

    g_print("interpreter = %d, data = %d\n", intprt, val);
    GString * cmd = g_string_new("q <- ");
    cmd = g_string_append_c(cmd, '0' + val);
    cmd = g_string_append(cmd, "\r\na <- q + 1\r\ncat(a,'\\n')\r\nflush.console()\r\n");

    write_to_interpreter(intprt->in, cmd);

    g_async_queue_push(queue, (gpointer)intprt);
    return 0;
}

static gboolean out_watch( GIOChannel   *channel,
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

    return TRUE;
}

static gboolean err_watch( GIOChannel   *channel,
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

    return TRUE;
}

struct interpreter* init_interpreter(void){
    gchar      *argv[] = { "R", "--no-save", "--silent", NULL };
    gint        in,
                out,
                err;
    gboolean    ret;

    /* Spawn child process */
    ret = g_spawn_async_with_pipes( NULL, argv, NULL,
            G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD, NULL,
            NULL, NULL,
            &in, &out, &err, NULL );
    if( ! ret )
    {
        g_error( "SPAWN FAILED" );
        return NULL;
    }

    struct interpreter* intprt = g_new(struct interpreter, 1);
    intprt->in = g_io_channel_unix_new( in );
    intprt->out = g_io_channel_unix_new( out );
    intprt->err = g_io_channel_unix_new( err );

    /* Add watches to channels */
    g_io_add_watch( intprt->out, G_IO_IN | G_IO_HUP, (GIOFunc)out_watch, NULL);
    g_io_add_watch( intprt->err, G_IO_IN | G_IO_HUP, (GIOFunc)err_watch, NULL);

    return intprt;
}


int main(){
    srand(0);

    int i;

    g_print("Create queue\n");
    GAsyncQueue *q = g_async_queue_new();

    g_print("Push interpreters to queue\n");
    struct interpreter* intprt = init_interpreter();
    g_async_queue_push(q, (gpointer)intprt);
    g_print("Number tasks in queue: %d\n", g_async_queue_length(q));

    g_print("Create pool\n");
    GThreadPool *pool = g_thread_pool_new(
            (GFunc)pool_func,
            (gpointer)q,
            g_get_num_processors(),
            TRUE, NULL);

    int vals[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    for(i = 0; i < 8; i++){
        g_thread_pool_push (pool, (gpointer)(vals+i), NULL);
    }

    GMainLoop *loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (loop);

    /*g_thread_pool_free(pool, FALSE, TRUE);
    g_print("Free pool\n");

    g_async_queue_unref(q);
    g_print("Unref queue\n");*/

    g_print("End main\n");
    return 0;
}
