#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"



// TASK 1.2
int sys_signal(void)
{
    int signum;
    sighandler_t handler;
    if(argint(0, &signum) < 0)
    {
        return -1;
    }
    if ((signum < 0) || (signum > 31)){
        return -1;
    }
    if (argptr(1, (void*)&handler, sizeof(sighandler_t)) < 0)         // argptr assigns the handler
    {
        return -1;
    }
    signal(signum, *handler);
    return 0;
}



// TASK 1.3
int sys_sigsend(void)
{
    int pid;
    int signum;
    if(argint(0, &pid) < 0)
    {
        return -1;
    }
    if(argint(1, &signum) < 0)
    {
        return -1;
    }
    if ((signum < 0) || (signum > 31)){         // Validity of signal id check
        return -1;
    }
    return sigsend(pid, signum);
}


// TASK 1.4
int sys_sigreturn(void)
{
    return sigreturn();
}


// TASK 1.5
int sys_alarm(void)
{
    int clockTick;
    if (argint(0, &clockTick) < 0)
    {
        return -1;
    }
    return alarm(clockTick);
}    


int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
