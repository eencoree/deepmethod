#include<glib.h>
#include<unistd.h>
#include<stdio.h>

struct interpreter{
    GIOChannel *in,
               *out,
               *err;
    GAsyncQueue * queue;
    GMutex m;
    GCond cond;
    gchar *response;
};

struct task{
    int data;
    gchar *result;
    GMutex m;
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
    GString * cmd = g_string_new("q <- ");
    char number[30];
    sprintf(number, "%d", n);
    cmd = g_string_append(cmd, number);
    cmd = g_string_append(cmd, ";a = q + 1;cat('R response:',a,'\\ndone\\n')\r\n");
#ifdef DEBUG
    g_print("VALUE: %s\n", cmd->str);
#endif
    return cmd;
}

gpointer pool_func(gpointer data, gpointer user_data){
    GAsyncQueue * queue = (GAsyncQueue*)user_data;
    struct interpreter* intprt = (struct interpreter*)g_async_queue_pop(queue);
    struct task *t = (struct task *)data;
    int val = t->data;
    gchar *string;

    g_print("interpreter = %d, data = %d\n", intprt, val);
    fflush(stdout);

    GString * cmd = create_command_r(val);
    write_to_interpreter(intprt->in, cmd);

    g_usleep(200);
    g_mutex_lock( &(intprt->m) );

    while( !(intprt->response) ){
        g_cond_wait( &(intprt->cond), &(intprt->m) );
    }

    string = g_strdup( intprt->response );
    g_free( intprt->response );
    intprt->response = NULL;

    g_mutex_unlock (&(intprt->m));

    g_mutex_lock (&(t->m));
    t->result = string;
    g_mutex_unlock (&(t->m));

    g_async_queue_push(queue, (gpointer)intprt);

    return 0;
}

static gboolean out_watch( GIOChannel   *channel,
        GIOCondition  cond, gpointer data)
{
    GString *response = g_string_new("");
    gsize size;
    GError *gerror = NULL;
    struct interpreter *intprt = (struct interpreter*)data;

    if( cond == G_IO_HUP ){
        g_print("Cond is G_IO_HUP %d\n", (int)cond);
        g_io_channel_unref( channel );
        return( FALSE );
    } else if( cond == G_IO_IN ) {
#ifdef DEBUG
        g_print("Cond is G_IO_IN %d\n", (int)cond);
        fflush(stdout);
#endif
    } else if( cond == G_IO_OUT ) {
#ifdef DEBUG
        g_print("Cond is G_IO_OUT %d\n", (int)cond);
        fflush(stdout);
#endif
    }

    GIOStatus status = G_IO_STATUS_NORMAL;
    gchar *response_line = NULL;
    gboolean is_done = FALSE;
    do{
        status = g_io_channel_read_line( channel, &response_line, &size, NULL, NULL );
        response = g_string_append(response, response_line);
        is_done = g_str_has_suffix(response_line, "done\n");
        g_free(response_line);
    } while( !is_done );

    if (status != G_IO_STATUS_NORMAL) {
        g_print("Status is not NORMAL %d\n", status);
    } else {
#ifdef DEBUG
        g_print("Status is NORMAL %d\n", status);
#endif
    }
    fflush(stdout);

    if (gerror != NULL) {
        g_print("%s", gerror->message);
        g_error_free(gerror);
    }

    g_mutex_lock( &(intprt->m) );
    intprt->response = g_string_free(response, FALSE);
    g_cond_signal( &(intprt->cond) );
    g_mutex_unlock( &(intprt->m) );

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

    /* Spawn child process */
    ret = g_spawn_async_with_pipes( NULL, argvr, NULL,
            G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD, NULL,
            NULL, NULL,
            &in, &out, &err, NULL );
    if( ! ret ){
        g_error( "SPAWN FAILED" );
        return NULL;
    }

    struct interpreter* intprt = g_new(struct interpreter, 1);
    intprt->in = g_io_channel_unix_new( in );
    intprt->out = g_io_channel_unix_new( out );
    intprt->err = g_io_channel_unix_new( err );
    intprt->queue = queue;

    g_mutex_init( &(intprt->m) );
    g_cond_init( &(intprt->cond) );
    intprt->response = NULL;

    /* Add watches to channels */
    g_io_add_watch( intprt->out, G_IO_IN | G_IO_OUT | G_IO_HUP, (GIOFunc)out_watch, intprt);
    g_io_add_watch( intprt->err, G_IO_IN | G_IO_OUT | G_IO_HUP, (GIOFunc)err_watch, intprt);

    return intprt;
}


int main(){
    int i, kount;
    int num_processors = 1;//g_get_num_processors();
    g_print("Number processors: %d\n", num_processors);
    
    GMainContext *gcontext = g_main_context_default();

    g_print("Create queue\n");
    GAsyncQueue *q = g_async_queue_new();

    g_print("Push interpreters to queue\n");
    struct interpreter* intprt;
    for(i = 0; i < num_processors; i++){
        intprt = init_interpreter(q);
        g_async_queue_push(q, (gpointer)intprt);
    }
    g_print("Number of interpreters in queue: %d\n", g_async_queue_length(q));

    g_print("Create pool\n");
    GThreadPool *pool = g_thread_pool_new(
            (GFunc)pool_func,
            (gpointer)q,
            num_processors,
            TRUE, NULL);

    g_print("Push tasks to pool\n");
    int num_tasks = 2 * num_processors;
    struct task *tasks = g_new(struct task, num_tasks);
    for(i = 0; i < num_tasks; i++){
        tasks[i].data = i;
        tasks[i].result = NULL;
        g_mutex_init( &(tasks[i].m) );
        g_usleep(2000);
        g_thread_pool_push(pool, (gpointer)(&tasks[i]), NULL);
    }

    gboolean completed = FALSE;
    kount = 0;
    while( !completed ) {
        g_main_context_iteration(gcontext, FALSE);
        completed = TRUE;
        for(i = 0; i < num_tasks; i++){
            g_print("now %d ", i);
            g_mutex_lock (&(tasks[i].m));
            if( !tasks[i].result ){
                completed = FALSE;
                g_print(" waiting for %d task %d data %d\n", num_tasks - kount, i, tasks[i].data);
                fflush(stdout);
            }else if(tasks[i].data >= 0){
                kount++;
                g_print(" %d task %d data %d result: %s\n", kount, i, tasks[i].data,  tasks[i].result);
                fflush(stdout);
                tasks[i].data = -1;
            }else{
                g_print("\n");
            }
            g_mutex_unlock( &(tasks[i].m) );
        }
    }

    g_print("End main\n");
    return 0;
}

