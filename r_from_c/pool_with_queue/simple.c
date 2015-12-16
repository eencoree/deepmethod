#include<glib.h>
#include<unistd.h>

#include<stdlib.h>

gpointer func(gpointer data, gpointer user_data){
    GAsyncQueue * queue = (GAsyncQueue*)user_data;
    g_async_queue_ref(queue);

    int task = *((int*)g_async_queue_pop(queue)); 
    int val = *((int*)data);

    sleep(rand() % 3);
    g_print("task = %d, data = %d\n", task, val);

    g_async_queue_unref(queue);   

    return 0;
}

int main(){
    srand(0);

    int i;

    int num_processors = g_get_num_processors();
    g_print("Num processors: %d\n", g_get_num_processors());

    g_print("Create queue\n");
    GAsyncQueue *q = g_async_queue_new();

    g_print("Push task to queue\n");
    int tasks[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    for(i = 0; i < num_processors; i++){
        g_async_queue_push(q, (gpointer)(tasks + i));
    }
    g_print("Number tasks in queue: %d\n", g_async_queue_length(q));

    g_print("Create pool\n");
    GThreadPool *pool = g_thread_pool_new(
            (GFunc)func,
            (gpointer)q,
            g_get_num_processors(),
            TRUE, NULL);

    int vals[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    for(i = 0; i < 8; i++){
        g_thread_pool_push (pool, (gpointer)(vals+i), NULL);
    }

    g_thread_pool_free(pool, FALSE, TRUE);
    g_print("Free pool\n");

    g_async_queue_unref(q);
    g_print("Unref queue\n");

    g_print("End main\n");
    return 0;
}
