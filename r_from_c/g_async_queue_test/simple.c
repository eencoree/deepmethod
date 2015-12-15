#include<stdio.h>
#include<glib.h>

struct queue_item{
    int data;
};


int main(){
    GAsyncQueue *q = g_async_queue_new();
    g_async_queue_ref(q);

    struct queue_item i;
    i.data = 5;
    printf("push to queue value: %d\n", i.data);

    g_async_queue_push (q, ( (gpointer) (&i)));

    struct queue_item *d = g_async_queue_pop (q);
    printf("pop value from queue: %d\n", d->data);
    
    g_async_queue_unref (q);

    return 0;
}
