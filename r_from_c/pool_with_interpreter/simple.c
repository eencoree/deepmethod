#include<glib.h>
#include<unistd.h>
#include<stdio.h>

struct interpreter{
    GIOChannel *in,
               *out,
               *err;
  GAsyncQueue * queue;
};

void write_to_interpreter(GIOChannel* in, GString* msg){
    g_io_channel_write_chars( in,
            msg->str,
            msg->len,
            NULL,
            NULL);
    g_io_channel_flush(in, NULL);
}

GString * create_command_r(int n){
    GString * cmd = g_string_new("flush.console();q <- ");
    char number[30];
    sprintf(number, "%d", n);
    cmd = g_string_append(cmd, number);
    cmd = g_string_append(cmd, ";a = q + 1;cat(a,'\\n');flush.console()\r\n");
    g_print("VALUE: %s\n", cmd->str);
    return cmd;
}

GString * create_command_b(int n){
    GString * cmd = g_string_new("echo \"$((");
    char number[30];
    sprintf(number, "%d", n);
    cmd = g_string_append(cmd, number);
    cmd = g_string_append(cmd, "+1))\"");
    g_print("VALUE: %s\n", cmd->str);
    return cmd;
}

gpointer pool_func(gpointer data, gpointer user_data){
    GAsyncQueue * queue = (GAsyncQueue*)user_data;

    struct interpreter* intprt = (struct interpreter*)g_async_queue_pop(queue); 
    int val = *((int*)data);

    g_print("interpreter = %d, data = %d\n", intprt, val);
    GString * cmd = create_command_r(val);
    write_to_interpreter(intprt->in, cmd);
//g_usleep(100000);
//    g_async_queue_push(queue, (gpointer)intprt);
    return 0;
}

static gboolean out_watch( GIOChannel   *channel,
        GIOCondition  cond, gpointer data)
{
    gchar *string;
    gsize  size;
	GIOStatus status;
  	GError *gerror = NULL;
	struct interpreter *intprt = (struct interpreter*)data;
    if( cond == G_IO_HUP )
    {
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
/*
GIOStatus
g_io_channel_read_to_end (GIOChannel *channel,
                          gchar **str_return,
                          gsize *length,
                          GError **error);
 */
/*  status = g_io_channel_read_to_end (channel,
                          &string,
                          &size,
                          &gerror);*/
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
/*    g_io_channel_flush(channel, NULL);*/
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
    gchar      *argvb[] = { "bash", "+x", NULL };
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
/*GIOStatus
g_io_channel_set_flags (intprt->out,
                        G_IO_FLAG_NONBLOCK,
                        &gerror);
 */
      /* Add watches to channels */
    g_io_add_watch( intprt->out, G_IO_IN | G_IO_OUT | G_IO_HUP, (GIOFunc)out_watch, intprt);
    g_io_add_watch( intprt->err, G_IO_IN | G_IO_OUT | G_IO_HUP, (GIOFunc)err_watch, intprt);

    return intprt;
}


int main(){
    int i;
    int num_processors = 4;//g_get_num_processors();
    g_print("Number processors: %d\n", num_processors);

    g_print("Create queue\n");
    GAsyncQueue *q = g_async_queue_new();

    g_print("Push interpreters to queue\n");
    struct interpreter* intprt;
    for(i = 0; i < num_processors; i++){
        intprt = init_interpreter(q);
        g_async_queue_push(q, (gpointer)intprt);
    }
    g_print("Number tasks in queue: %d\n", g_async_queue_length(q));

    g_print("Create pool\n");
    GThreadPool *pool = g_thread_pool_new(
            (GFunc)pool_func,
            (gpointer)q,
            g_get_num_processors(),
            TRUE, NULL);

    g_print("Push tasks to pool\n");
    int num_tasks = 20;//2 * num_processors;
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
