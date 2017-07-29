#define MAX_UTHREADS 64
#define NUMSIG 32
#define USTACKSIZE 4096
#define UTHREAD_QUANTA 5
#define SIGALRM 14
#define MAX_BSEM 128          // Maximum number of binary semaphores

int uthread_init();
int uthread_create(void (*start_func)(void *), void*arg);
void uthread_exit();
int uthread_join(int tid);
int uthread_sleep(int ticks);
int uthread_self();

// Size of trapframe = 76 --> 19 uint (including ushort), each uint size of 4, thus: 4 * 19 = 76
struct trapframe {
  // registers as pushed by pusha

  uint edi;
  uint esi;
  uint ebp;
  uint oesp;      // useless & ignored
  uint ebx;
  uint edx;
  uint ecx;
  uint eax;

  // rest of trap frame
  ushort gs;
  ushort padding1;
  ushort fs;
  ushort padding2;
  ushort es;
  ushort padding3;
  ushort ds;
  ushort padding4;
  uint trapno;

  // below here defined by x86 hardware
  uint err;
  uint eip;
  ushort cs;
  ushort padding5;
  uint eflags;

  // below here only when crossing rings, such as from user to kernel
  uint esp;
  ushort ss;
  ushort padding6;
};


enum threadstate { UNUSED, SLEEPING, RUNNABLE, RUNNING, WAITING_FOR_THREAD, SLEEPING_ON_LOCK };

// Per-thread state
struct thread {
    struct trapframe tf;          // Trap frame for current syscall
    int tid;                      // Thread id
    int pid;                      // Thread's process id
    enum threadstate state;       // Thread state
    char *ustack;                 // Bottom of user stack for this thread
    int firstRun;                 // Indicator if thread's first time running
    int tableIndex;               // Index of thread's table position
    int wakeupTick;               // Wake up tick of sleeping thread
    int thread_waiting_for_tid;   // Thread waiting for ID
    int thread_waiting_on_semaphore;  
};


/***********************    TASK 3   ***********************/


enum semaphoreState { LOCKED, UNLOCKED, NOTUSED };

// Binary Semaphore structure
struct binary_semaphore {
    int desc;                     // Semaphore descriptor
    enum semaphoreState state;    // Semaphore's state
    int semTableIndex;            // Semaphore's index in the semaphores' table
};


// Counting Semaphore structure
struct counting_semaphore {
    int value;                    // Counting semaphore's counter
    struct binary_semaphore * S1;   // Binary semaphore number 1
    struct binary_semaphore * S2;   // BInary semaphore number 2
    enum semaphoreState state;    // Counting semaphore's state
};

//binary semaphore func declarations:
void bsem_up(int descriptor);
void bsem_down(int descriptor);
void bsem_free(int descriptor);
int bsem_alloc();

//counting semaphore func declarations:
void down(struct counting_semaphore * sem);
void up(struct counting_semaphore * sem);
struct counting_semaphore * counting_sem_alloc(int init_value);
void counting_sem_free(struct counting_semaphore *countSem);

