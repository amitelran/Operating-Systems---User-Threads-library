
#include "types.h"
#include "user.h"
#include "stat.h"
#include "uthread.h"

void tthread1(void *arg) {
  int i;
  //int arg1 = (int) arg;
  //bsem_down(arg1);
  struct counting_semaphore *countSem;
  countSem = (struct counting_semaphore *) arg;
  down(countSem);
  for(i = 0; i < 70; i++) 
  {
    printf(1, " 111111111111 %d \n",i);
  }
  up(countSem);
   counting_sem_free(countSem);
  //bsem_up(arg1);
 // bsem_free(arg1);
  
}

void tthread2(void *arg) {
 int i;
  //int arg1 = (int) arg;
  //bsem_down(arg1);
  struct counting_semaphore *countSem;
  countSem = (struct counting_semaphore *) arg;
  down(countSem);
  for(i = 0; i < 70; i++) 
  {
    printf(1, " 2222 %d \n",i);
  }
  up(countSem);
  counting_sem_free(countSem);
  //bsem_up(arg1);
 // bsem_free(arg1);
}

void tthread3(void *arg) {
 int i;
  //int arg1 = (int) arg;
  //bsem_down(arg1);
  struct counting_semaphore *countSem;
  countSem = (struct counting_semaphore *) arg;
  down(countSem);
  for(i = 0; i < 70; i++) 
  {
    printf(1, " 3333 %d \n",i);
  }
  up(countSem);
 
  //bsem_up(arg1);
 // bsem_free(arg1);
}


void tthread4(void *arg) {
 int i;
  //int arg1 = (int) arg;
  //bsem_down(arg1);
  struct counting_semaphore *countSem;
  countSem = (struct counting_semaphore *) arg;
  down(countSem);
  for(i = 0; i < 70; i++) 
  {
    printf(1, " 4444 %d \n",i);
  }
  up(countSem);
 
  //bsem_up(arg1);
 // bsem_free(arg1);
}

void tthread5(void *arg) {
 int i;
  //int arg1 = (int) arg;
  //bsem_down(arg1);
 uthread_sleep(100);
  for(i = 0; i < 20; i++) 
  {
    printf(1, " 5555 %d \n",i);
  }
  //bsem_up(arg1);
 // bsem_free(arg1);
}


void tthread6(void *arg) {
 int i;

  //int arg1 = (int) arg;
  //bsem_down(arg1);
 uthread_sleep(100);
  for(i = 0; i < 20; i++) 
  {
     
    printf(1, " 6666 %d \n",i);
  }
 
  //bsem_up(arg1);
 // bsem_free(arg1);
}

void tthread7(void *arg) {
 int i;
  //int arg1 = (int) arg;
  //bsem_down(arg1);
 // struct counting_semaphore *countSem;
 // countSem = (struct counting_semaphore *) arg;
 // down(countSem);

  for(i = 0; i < 20; i++) 
  {
    uthread_sleep(8);
    printf(1, " 7777 %d \n",i);
  }
  //up(countSem);
 
  //bsem_up(arg1);
 // bsem_free(arg1);
  }


  void tthread8(void *arg) {
 int i;
  //int arg1 = (int) arg;
  //bsem_down(arg1);
 // struct counting_semaphore *countSem;
 // countSem = (struct counting_semaphore *) arg;
 // down(countSem);
 uthread_sleep(101);
 printf(1, "tthread8 will join on thread id %d\n", (int) arg);
   uthread_join((int) arg);

  for(i = 0; i < 20; i++) 
  {
    printf(1, " 8888 %d \n",i);
  }
  //up(countSem);
 
  //bsem_up(arg1);
 // bsem_free(arg1);
  }

    void tthread9(void *arg) {
 int i;
 printf(1, "tthread9 will join on thread id %d\n", (int) arg);
 uthread_join((int) arg);
  //int arg1 = (int) arg;
  //bsem_down(arg1);
 // struct counting_semaphore *countSem;
 // countSem = (struct counting_semaphore *) arg;
 // down(countSem);
 uthread_sleep(101);
  for(i = 0; i < 20; i++) 
  {
    printf(1, " 9999 %d \n",i);
  }
  //up(countSem);
 
  //bsem_up(arg1);
 // bsem_free(arg1);
  }



/*
int main(int argc, char *argv[]) {
    int id1,id2=0,id3=0;
    //,id4,id5;
    struct counting_semaphore *countSem;
    printf(1,"\n-=-=-=-=-=-=-=-=-=-=-TEXT USER LEVEL THREAD-=-=-=-=-=-=-=-=-=-=-\n");
    uthread_init();

     countSem = counting_sem_alloc(3);
    id1 = uthread_create(tthread1, (void*)countSem);
    id2 = uthread_create(tthread2, (void*)countSem);
     id3 = uthread_create(tthread3, (void*)countSem);
   uthread_create(tthread4, (void*)countSem);
     uthread_create(tthread5, (void*)countSem);
      uthread_create(tthread6, (void*)countSem);
    uthread_create(tthread7, (void*)countSem);



    //uthread_create(tthread4, (void*)sema);
   // uthread_create(tthread5, (void*)sema);
    //uthread_create(tthread5, (void*)sema);
   // uthread_create(tthread5, (void*)sema);
   // uthread_create(tthread5, (void*)sema);
   // uthread_join(id1);
    //uthread_join(id2);
    //uthread_join(id3);
    //uthread_join(id4);
   // counting_sem_free(countSem);
    counting_sem_free(countSem);
    printf(1, " %d %d %d %d %d\n",id1,id2 , id3);
    uthread_exit();
}

*/
int main(int argc, char *argv[]) 
{   
  int id1,id2=0,id3=0,id4 =0;
     uthread_init();
    id1 = uthread_create(tthread5, (void*)1);
    id2 = uthread_create(tthread6, (void*)2);
    id3 = uthread_create(tthread7, (void*)3);
    id4 = uthread_create(tthread8, (void*)id2);
     uthread_create(tthread9, (void*)id3);

    uthread_sleep(100);
  //uthread_join(id1);
  
    printf(1, " %d %d %d %d %d\n",id1,id2 , id3, id4);
    uthread_exit();
}