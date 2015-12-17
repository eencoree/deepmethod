#include<glib.h>
#include<unistd.h>
#include<stdio.h>

struct interpreter{
    GIOChannel *in,
               *out,
               *err;
    GAsyncQueue *queue;
};

void write_to_interpreter(GIOChannel* in, GString* msg){
    g_io_channel_write_chars( in,
            msg->str,
            msg->len,
            NULL,
            NULL);
    g_io_channel_flush(in, NULL);
}

GString * create_command(int n){
    GString * cmd = g_string_new("flush.console();q <- ");
    char number[30];
    sprintf(number, "%d", n);
    cmd = g_string_append(cmd, number);
    cmd = g_string_append(cmd, ";cat(q,'\\n');flush.console()\r\n");
    g_print("Command: %s\n", cmd->str);
    return cmd;
}

gpointer pool_func(gpointer data, gpointer user_data){
    GAsyncQueue * queue = (GAsyncQueue*)user_data;

    struct interpreter* intprt = (struct interpreter*)g_async_queue_pop(queue); 
    int val = *((int*)data);

    g_print("interpreter = %d, data = %d\n", intprt, val);
    GString * cmd = create_command(val);
    write_to_interpreter(intprt->in, cmd);

    return 0;
}

static gboolean out_watch( GIOChannel * channel,
        GIOCondition  cond, gpointer data)
{
    gchar *string;
    gsize  size;
    GIOStatus status;
    GError *gerror = NULL;

    struct interpreter *intprt = (struct interpreter*)data;
    if( cond == G_IO_HUP ){
        g_print("Cond is G_IO_HUP %d\n", (int)cond);
        g_io_channel_unref( channel );
        return( FALSE );
    } else if( cond == G_IO_IN ) {
        g_print("Cond is G_IO_IN %d\n", (int)cond);
        fflush(stdout);
    } else if( cond == G_IO_OUT ) {
        g_print("Cond is not G_IO_OUT %d\n", (int)cond);
        fflush(stdout);
    }

    status = g_io_channel_read_line( channel, &string, &size, NULL, NULL );
    if (status != G_IO_STATUS_NORMAL) {
        g_print("Status is not NORMAL %d\n", status);
    } else {
        g_print("Status is NORMAL %d\n", status);
    }
    fflush(stdout);
    if (gerror != NULL) {
        g_print(gerror->message);
        g_error_free(gerror);
    }
    g_print("Out: %s", string);
    fflush(stdout);
    g_free( string );
    
    g_async_queue_push(intprt->queue, (gpointer)intprt);

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
    g_io_channel_flush(channel, NULL);
    g_free( string );

    return TRUE;
}

struct interpreter* init_interpreter(GAsyncQueue * queue){
    gchar      *argvr[] = { "R", "--no-save", "--silent", "--vanilla", NULL };
    gint        in,
                out,
                err;
    gboolean    ret;
    GError *gerror = NULL;

    /* Spawn child process */
    ret = g_spawn_async_with_pipes( NULL, argvr, NULL,
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
    intprt->queue = queue;

    /* Add watches to channels */
    g_io_add_watch( intprt->out, G_IO_IN | G_IO_OUT | G_IO_HUP, (GIOFunc)out_watch, intprt);
    g_io_add_watch( intprt->err, G_IO_IN | G_IO_OUT | G_IO_HUP, (GIOFunc)err_watch, intprt);

    return intprt;
}


int main(){
    int i;
    int num_processors = 2;//g_get_num_processors();
    g_print("Number processors: %d\n", num_processors);

    g_print("Create queue\n");
    GAsyncQueue *q = g_async_queue_new();

    g_print("Push interpreters to queue\n");
    struct interpreter* intprt;
    for(i = 0; i < num_processors; i++){
        intprt = init_interpreter(q);
        g_async_queue_push(q, (gpointer)intprt);
    }
    g_print("Number interpreters in queue: %d\n", g_async_queue_length(q));

    g_print("Create pool\n");
    GThreadPool *pool = g_thread_pool_new(
            (GFunc)pool_func,
            (gpointer)q,
            num_processors,
            TRUE, NULL);

    int num_tasks = 2 * num_processors;
    g_print("Push %d tasks to pool\n", num_tasks);
    int* tasks = g_new(int, num_tasks);
    for(i = 0; i < num_tasks; i++){
        tasks[i] = i;
        g_thread_pool_push (pool, (gpointer)(tasks+i), NULL);
    }

    GMainLoop *loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (loop);

    g_print("End main\n");
    return 0;
}
