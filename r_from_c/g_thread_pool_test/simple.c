#include<glib.h>
#include<unistd.h>

gpointer func(gpointer data, gpointer user_data){
    int val = *((int*)data);
    sleep(8 - val);
    g_print("hey, data = %d\n", val);
    return 0;
}

int main(){
    g_print("Num processors: %d\n", g_get_num_processors());

    g_print("Create pool\n");
    GThreadPool *pool = g_thread_pool_new ((GFunc)func, NULL, g_get_num_processors(), TRUE, NULL);

    int i = 0;
    int vals[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    for(; i < 8; i++){
        g_thread_pool_push (pool, (gpointer)(vals+i), NULL);
    }

    g_thread_pool_free(pool, FALSE, TRUE);
    g_print("Free pool\n");

    g_print("End main\n");
    return 0;
}
