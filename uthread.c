#include "types.h"
#include "stat.h"
#include "user.h"
#include "uthread.h"


void uthread_schedule();
void uthread_exit();


int nexttid = 0;                                            // next thread ID
int nextSemaphoreDesc = 0;                                  // Semaphore IDs
int numOfThreads = 0;                                       // Number of currently activated threads
int numOfSemaphores = 0;
int binSemaphoresInit = 0;                                  // Initialization flag of binary semaphores
static struct thread threadsTable[MAX_UTHREADS];
static struct binary_semaphore binSemTable[MAX_BSEM];       // Binary semaphores table
struct thread *currThread;                                  // Thread currently running                             



/***********************    Wrap the entry function, so that a thread_exit call will be carried out implicitly after the entry function finishes   ***********************/


void pseudo_entry_function(void (*start_func)(void *), void* arg)
{
    alarm(UTHREAD_QUANTA);
    start_func(arg);
    uthread_exit();
}


/*************************************    uthread_init    ******************************************/
// TASK 2.1: user thread initialization 

int uthread_init(){
    alarm(0);
    struct thread *t;

    // Initializes the process’ threads table
    for (t = threadsTable; t < &threadsTable[MAX_UTHREADS]; t++)
    {
        t->state = UNUSED;                              // Initialize threads table - set all threads to be UNUSED
        t->tid = -1;
    }
    
    // Creates the main thread
    t = &threadsTable[0];                               // Set 'main thread' as thread number 0
    t->tableIndex = 0;
    t->tid = nexttid++;                     
    t->pid = getpid();                                  // Get process PID
    t->state = RUNNING;                                 // Set state as RUNNING
    t->wakeupTick = 0;                                  // Initialize SLEEPING time
    numOfThreads++;                                     // Increment number of activated threads
    currThread = t;                                     // Assign current thread as the 'main thread'
    t->firstRun = 0;

    // Registers the SIGALRM handler to the uthread_schedule function
    signal(SIGALRM, (sighandler_t)uthread_schedule);

    // Executes the alarm system call with parameter UTHREAD_QUANTA=5
    alarm(UTHREAD_QUANTA);
    
    return 0;
}


/*************************************    uthread_create   *************************************/
// TASK 2.2 
/*  This function receives as arguments a pointer to the thread’s entry function and an argument for it. The
    function should allocate space for the thread, initialize it but not run it just yet. Once the thread’s fields
    are all initialized, the thread is inserted into the threads table. The function returns the identifier of the
    newly created thread or -1 in case of a failure.
*/

int uthread_create(void (*start_func)(void *), void *arg){
    alarm(0);
    int tIndex = 0;
    struct thread *t;
    char *sp;
    if (numOfThreads == MAX_UTHREADS)                       // Can't create any more threads
    {
        printf(1, "Reached maximum number of threads\n");
        return -1;
    }

    for (t = threadsTable; t < &threadsTable[MAX_UTHREADS]; t++) {
        if (t->state == UNUSED)
        {
            break;
        } 
        tIndex++;
        if(tIndex == MAX_UTHREADS)
        {
            tIndex = 0;
        }
    }
    t->tableIndex = tIndex;                                 // Store table Index
    t->firstRun = 1;
    t->tid = nexttid++;                                     // Set thread ID
    t->pid = getpid();                                      // Set process ID
    t->wakeupTick = 0;                                      // Initialize sleeping time counter
    t->thread_waiting_for_tid = -1;                         // Initalize thread id waiting for
    t->state = RUNNABLE;                                    // Set thread state as RUNNABLE
    t->thread_waiting_on_semaphore = -1;

    // Allocate user stack.
    t->ustack = malloc(USTACKSIZE);                         // MALLOCATE space for user stack (4096 size)
    sp = t->ustack + USTACKSIZE;                            // Get to top of stack

    sp -= 4;                                                // Allocate space for function arguments pointer
    *(uint *) sp = (uint)arg;                               // Push function arguments to stacks
    sp -= 4;                                                // Allocate space for thread's function address
    *(uint *) sp = (uint)start_func;                        // Push function address
    sp -= 4;                                                // Allocate space for fake return address
    *(uint *) sp = 0;                                       // Push fake return address

    t->tf.esp = (uint) sp;                                 // Initialize thread's esp to point on User Stack
    t->tf.ebp = (uint) sp;                                 // Initialize thread's ebp to pint on User Stack
    numOfThreads++;                                         // Increment number of activated threads     
    alarm(UTHREAD_QUANTA);                      
    return t->tid;
}




/***********************    TASK 2.3: inline asm function to switch thread stacks   ***********************/


void switch_running_thread(struct thread *next_Thread)
{   
    // Still on alarm(0) from uthread_schedule
    uint sp;
    uint * derefSp;
    struct trapframe* tf = 0;
    asm("movl %%esp, %0\n": "=r" (sp));

    // Finding the trapframe (searching for the MAGIC number we inserted at the end of the trapframe, and go to start of trapframe)
    derefSp = (uint *) sp;
    while (*derefSp != 0xABCDEF) 
    {
        sp += 4;
        derefSp = (uint *) sp;
    }   
    sp -= sizeof(struct trapframe);
    derefSp = (uint *) sp;
    tf = (struct trapframe*) derefSp;

    // Backup current thread's trapframe
    memmove((void *)&currThread->tf, (void *) sp, sizeof(struct trapframe));
    currThread = next_Thread;

    // Switching the context
    if (currThread->firstRun)                         // If running for the first time: backup trapframe, else: backup ebp & esp
    {
        currThread->firstRun = 0;
        tf->eip = (uint) pseudo_entry_function;
        tf->ebp = currThread->tf.ebp;
        tf->esp = currThread->tf.esp;
    }
    else                                                   // Not running for the first time: insert trapframe 
    {  
        memmove((void *)sp, (void *) &currThread->tf, sizeof(struct trapframe));
    }
    alarm(UTHREAD_QUANTA);
}



/***********************    uthread_schedule   ***********************/
// TASK 2.3: This function should choose (schedule) the next thread from the threads table, according to the round robin scheduling policy


void uthread_schedule()
{     
    alarm(0);   
    int noOneIsRunnable = 0;                                   
    struct thread *t;
    struct thread *minimal_sleeping_thread;
    int minimal_wakeup_tick = 0;

    // Check if there are activated threads to run, if none --> perform exit
    if (numOfThreads == 0)
    {
        printf(1, "uthread_schedule: No threads to run, performing process exit\n");
        exit();
    }
    
    // Set currThread from RUNNING to RUNNABLE
    if (currThread->state == RUNNING)
    {
        currThread->state = RUNNABLE;
    }

    // Update SLEEPING threads
    for (t = threadsTable; t < &threadsTable[MAX_UTHREADS]; t++)
    {
        if (t->state == SLEEPING)
        {
            // Wake up SLEEPING threads past their wakeuping tick
            if (t->wakeupTick <= uptime())
            {
                t->wakeupTick = 0;
                t->state = RUNNABLE;
            }
        }
    }

    // Find the next thread to run:
    t = currThread;
    t++;
    if (t == &threadsTable[MAX_UTHREADS])                   // If reached end of threads table, set to look from start of table
    {
        t = threadsTable;
    }

    // After current thread found, start looking for next thread to run from next thread in array
    while(t < &threadsTable[MAX_UTHREADS])
    {   
        // If we looped all over the table, reached currThread, and currThread is not RUNNING (sleeping or waiting)--> all other threads are sleeping
        // We'll call system call sleep, in order to sleep until minimal sleeping thread will wake up
        if (noOneIsRunnable > MAX_UTHREADS)
        {
            if ((minimal_wakeup_tick - uptime()) > 0){
                sleep(minimal_wakeup_tick - uptime());
            }
            minimal_sleeping_thread->wakeupTick = 0;
            minimal_sleeping_thread->state = RUNNABLE;
            t = minimal_sleeping_thread;
            break;
        }

        if (minimal_wakeup_tick == 0)                       // If minimal wake up tick has not been initialized --> insert value into it anyway (either will be 0, or higher)
        {                              
            minimal_wakeup_tick = t->wakeupTick;
            minimal_sleeping_thread = t;
        }
        else if ((t->wakeupTick > 0) && (t->wakeupTick < minimal_wakeup_tick)){              // If reached this condition, then minimal wakeup tick has been initialized
            minimal_wakeup_tick = t->wakeupTick;                                             // Update if found lower wakeup tick
            minimal_sleeping_thread = t;
        }

        if (t->state == RUNNABLE)                           // Break if found a RUNNABLE thread
        {
            break;
        }
        t++;   
        noOneIsRunnable++;                                  // Increment t to next thread in array 
        if (t == &threadsTable[MAX_UTHREADS])               // If reached end of array, set t to beginning of array and loop from start of array (Round Robin)
        {
            t = threadsTable;
        }
    }

    t->state = RUNNING;
    switch_running_thread(t);                               // Call inline assembly function to switch between thread stacks

}



/***********************    uthread_exit   ***********************/
// TASK 2.4: Terminates the calling thread and transfers control to some other thread (similar to uthread_schedule)


void uthread_exit(){
    alarm(0);                                      
    printf(1, "uthread_exit: Thread %d is exiting\n", currThread->tid);
    struct thread *t;
    currThread->state = UNUSED;                    // Thread is unused  
    numOfThreads--;    
    
    // Wake up all waiting threads waiting for currThread
    for (t = threadsTable; t < &threadsTable[MAX_UTHREADS]; t++)
    {   
        if (t->state == WAITING_FOR_THREAD)
        {
            if(t->thread_waiting_for_tid == currThread->tid)
            {
                t->state = RUNNABLE;
                t->thread_waiting_for_tid = -1;
            }
        }
    }
    // If not the main thread --> free user stack mallocation
    if(currThread->tid != 0)
    { 
        free(currThread->ustack);
    }
    // If main thread --> free all USED threads user stacks, and exit process
    if(numOfThreads > 0)
    {
        sigsend(currThread->pid, SIGALRM);    
    }
    else
    {
        exit();
    }
}



/***********************    TASK 2.5: uthread_self   ***********************/
// Returns the thread id associated with the calling thread.


int uthread_self(){
    return currThread->tid;
}



/***********************    TASK 2.6: uthread_join    ***********************/
// Waits for the thread specified by tid to terminate. If already terminated/not exists, returns immediately


int uthread_join(int tid){
    alarm(0);
    struct thread *t;
    int foundTID = 0;

    // Checking if tid is valid:
    for (t = threadsTable; t < &threadsTable[MAX_UTHREADS]; t++)
    {
        if (t->tid == tid)
        {
            foundTID = 1;
            if (t->state == UNUSED)
            {
                return -1;
            }
            break;
        }
    }    
    if (!foundTID)
    {
        return -1;
    }
    currThread->state = WAITING_FOR_THREAD;
    currThread->thread_waiting_for_tid = tid;   
    sigsend(currThread->pid, SIGALRM);
    return 0;
}



/***********************    TASK 2.7: uthread_sleep   ***********************/
// Suspends the execution of the current thread for at least ticks


int uthread_sleep(int ticks)
{
    if (ticks < 1)
    {
        return -1;
    }
    alarm(0);
    currThread->state = SLEEPING;                           // Set thread state as SLEEPING
    currThread->wakeupTick = ticks + uptime();              // Set thread's sleeping time counter (current tick + given ticks)
    sigsend(currThread->pid, SIGALRM);                      // Send SIGALRM signal to switch threads
    return 0;
}



/***********************    TASK 3: Initialize binary semaphores table   ***********************/


void bsemTable_init()
{
    alarm(0);
    struct binary_semaphore *binSem;
    for (binSem = binSemTable; binSem < &binSemTable[MAX_BSEM]; binSem++)
    {
        binSem->state = NOTUSED;
    }
}



/***********************    TASK 3.1: allocate a new binary semaphore and return its descriptor   ***********************/


int bsem_alloc()
{
    alarm(0);
    struct binary_semaphore *binSem;

    // If binary semaphores table has yet to be initialized --> set all table spots as UNUSED
    if (!binSemaphoresInit)                                           
    {
        bsemTable_init();  
        binSemaphoresInit = 1;
    }
    if (numOfSemaphores == MAX_BSEM)                                  // Check number of active semaphores
    {
        return -1;
    }
    for (binSem = binSemTable; binSem < &binSemTable[MAX_BSEM]; binSem++)
    {
        if (binSem->state == NOTUSED)
        {
            break;
        }
    }
    binSem->desc = nextSemaphoreDesc++;                                 // Set binary semaphore descriptor
    binSem->state = UNLOCKED;                                           // Initial semaphore state is UNLOCKED
    numOfSemaphores++;                                                  // Increment semaphores counter
    return binSem->desc;
}



/***********************    TASK 3.1: free binary semaphore of given descriptor   ***********************/


void bsem_free(int descriptor)
{
    alarm(0);
    struct binary_semaphore *binSem;
    struct thread *t;

    // Find the corresponding semaphore
    for (binSem = binSemTable; binSem < &binSemTable[MAX_BSEM]; binSem++)       
    {
        if (binSem->desc == descriptor)
        {
            break;
        }
    }

    // Check if there are any threads sleeping on corresponding semaphore
    for (t = threadsTable; t < &threadsTable[MAX_UTHREADS]; t++)
    {
        if ((t->state == SLEEPING_ON_LOCK) && (t->thread_waiting_on_semaphore == descriptor))
        {
            printf(1, "bsem_free: Binary semaphore is LOCKED, cannot release the semaphore at the current time. Try again later\n");
            return;
        }
    }

    // If found the semaphore, and no thread is sleeping on it, then release the semaphore
    binSem->state = NOTUSED;
    numOfSemaphores--;
}



/***********************    TASK 3.1: attempt to acquire the semaphore   ***********************/



void bsem_down(int descriptor)
{ 
    int caught_semaphore = 0;
    struct binary_semaphore *binSem;
    while (!caught_semaphore)
    {
        alarm(0);
        for (binSem = binSemTable; binSem < &binSemTable[MAX_BSEM]; binSem++)       // Find the corresponding semaphore
        {
            if (binSem->desc == descriptor)
            {
                break;
            }
        }
        if (binSem->state == LOCKED){
            currThread->state = SLEEPING_ON_LOCK;
            currThread->thread_waiting_on_semaphore = descriptor;
            sigsend(currThread->pid, SIGALRM);
            //this is not busy wait because the thread's state is not 'runnable', hence it would not be picked by the scheduler
        }
        else
        {
            binSem->state = LOCKED;
            alarm(UTHREAD_QUANTA);
            caught_semaphore = 1;
        }
    }
}



/***********************    TASK 3.1: releases the semaphore   ***********************/



void bsem_up(int descriptor)
{
    alarm(0);
    struct binary_semaphore *binSem;
    for (binSem = binSemTable; binSem < &binSemTable[MAX_BSEM]; binSem++)       // Find the corresponding semaphore
    {
        if (binSem->desc == descriptor)
        {
            binSem->state = UNLOCKED;
            break;
        }
    }

    struct thread *t;
    for (t = threadsTable; t < &threadsTable[MAX_UTHREADS]; t++)
    {
        if ((t->state == SLEEPING_ON_LOCK) && (t->thread_waiting_on_semaphore == descriptor))
        {
            t->state = RUNNABLE;
            break;
        }
    }
    alarm(UTHREAD_QUANTA);
}



/***********************    TASK 3.2: Counting semaphore allocation   ***********************/



struct counting_semaphore * counting_sem_alloc(int init_value)
{
    alarm(0);
    struct counting_semaphore *countSem;
    struct binary_semaphore *binSem; //iterator
    if (init_value < 0)
    {
        printf(1, "Counting semaphore given initial value is non-positive\n");
        return 0;
    }

    countSem = malloc(sizeof (struct counting_semaphore)); // allocate memmory to the semaphore
    countSem->value = init_value;                          // value = init_value
    int s1_desc = bsem_alloc();                            // S1->state = UNLOCKED
    int s2_desc = bsem_alloc();                            // S2->state = UNLOCKED



    // Get corresponding binary semaphores
    for (binSem = binSemTable; binSem < &binSemTable[MAX_BSEM]; binSem++)
    {
        if (binSem->desc == s1_desc)
        {
            countSem->S1 = binSem;
        }
        else if (binSem->desc == s2_desc){
            countSem->S2 = binSem;
        }
    }

    if (init_value == 0){                           // S2 = min(1, init_value) --> if init_value is 0, S2->state = LOCKED
        countSem->S2->state = LOCKED;
    }
    countSem->state = UNLOCKED;
    return countSem;
}



/***********************    TASK 3.2: Counting semaphore free   ***********************/



void counting_sem_free(struct counting_semaphore *countSem)
{
    alarm(0);
    struct thread *t;
    for (t = threadsTable; t < &threadsTable[MAX_UTHREADS]; t++)
    {
        if ((t->state == SLEEPING_ON_LOCK) && ((t->thread_waiting_on_semaphore == countSem->S2->desc) || (t->thread_waiting_on_semaphore == countSem->S1->desc)))
        {
            printf(1, "counting_sem_free: Counting semaphore is LOCKED, Thread %d cannot release the semaphore at the current time. Try again later\n", currThread->tid);
            return;
        }
    }
    // Free counting semaphores' binary semaphores
    bsem_free(countSem->S1->desc);
    bsem_free(countSem->S2->desc);
    countSem->state = NOTUSED;
    free(countSem);
    alarm(UTHREAD_QUANTA);
}



/***********************    TASK 3.2: If representing value is non-negative --> decrement by 1   ***********************/



void down(struct counting_semaphore *sem)
{   
    bsem_down(sem->S2->desc);           // down(S2)
    bsem_down(sem->S1->desc);           // down(S1)
    sem->value--;
    if (sem->value > 0){
        bsem_up(sem->S2->desc);         // up(S2)
    }
    bsem_up(sem->S1->desc);             // up(S1)
}



/***********************    TASK 3.2: Increment value of counting semaphore by 1   ***********************/


void up(struct counting_semaphore *sem)
{
    bsem_down(sem->S1->desc);           // down(S1)
    sem->value++;                       // S.value++
    if (sem->value == 1){
        bsem_up(sem->S2->desc);         // up(S2)
    }
    bsem_up(sem->S1->desc);             // up(S1)
}
