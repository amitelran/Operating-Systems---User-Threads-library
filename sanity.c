
#include "types.h"
#include "stat.h"
#include "user.h"
#include "uthread.h"

#define N 100                                           /* Buffer size */
#define num_items_to_produce 1000                       /* Producer's number of items to add */
#define num_consumers 3                             
#define num_producers 1

void producer(void* arg);
void consumer(void* arg);

struct counting_semaphore *mutex;                       /* access control to critical section */
struct counting_semaphore *empty;                       /* counts empty buffer slots */
struct counting_semaphore *full;                        /* counts full slots */

int bin_sem;

int queue[N];                                           /* shared array of 100 integers */
int queue_next_index = 0;                               /* Minimal queue free index */
int num_of_consumed_items = 0;
int index_item_to_consume = 0;


/***********************    MAIN  ***********************/


int main(int argc, char *argv[])
{
    uthread_init();

    // Initialize Counting and Binary semaphores
    mutex = counting_sem_alloc(1);
    empty = counting_sem_alloc(N);
    full = counting_sem_alloc(0);
    
    bin_sem  = bsem_alloc();

    // Create producers and consumers
    int id1 = uthread_create(producer, (void *) 0);
    int id2 = uthread_create(consumer, (void *) 1);
    int id3 = uthread_create(consumer, (void *) 2);
    int id4 = uthread_create(consumer, (void *) 3);

    // Waiting for all threads finish before freeing semaphores.
    uthread_join(id1);
    uthread_join(id2);
    uthread_join(id3);
    uthread_join(id4);

    // Free semaphores
    counting_sem_free(mutex);
    counting_sem_free(full);
    counting_sem_free(empty);

    bsem_free (bin_sem);

    uthread_exit(); 
}



/***********************    PRODUCER   ***********************/



void producer(void* arg){
    for (int i = 1; i <= num_items_to_produce; i++)
    {
        down(empty);                               /* decrement count of empty */
        down(mutex);                               /* enter critical section */
        queue[queue_next_index] = i;                /* insert into buffer */
        queue_next_index = ((queue_next_index + 1) % N);
        up(mutex);                                 /* leave critical section */
        up(full);                                  /* increment count of full slots */
    }
}



/***********************    CONSUMER   ***********************/


void consumer(void *arg){
    int i = 0;
    while(1) 
    {   
        /* Check if the consumer reached the limit (no more items to consume) --> perform thread exit */
        if (num_of_consumed_items == num_items_to_produce){                 
            uthread_exit();
        }

        down(full);                                     /* decrement count of full */
        down(mutex);                                    /* enter critical section */

        i = queue[index_item_to_consume];
        index_item_to_consume = ((index_item_to_consume + 1) % N);
        num_of_consumed_items++;

        up(mutex);                                      /* leave critical section */
        up(empty);                                      /* update count of empty */
                                    
        uthread_sleep(i);

        bsem_down(bin_sem);
        printf(1, "\nThread %d slept for %d ticks\n", uthread_self(), i);
        bsem_up(bin_sem);
    }
}
