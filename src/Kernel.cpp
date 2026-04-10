/* Kernel Class Implementation File
 *
 * Hunter Poole
 * 04-05-2026
 */

#include "Kernel.h"

Kernel::Kernel() {
  scheduler = new Scheduler();
  Semaphore::set_scheduler(scheduler);
  pthread_mutex_lock(&CPULocker);
}

void *Kernel::Bootstrap_Wrapper(void *context) {
  Context *ct = (Context *)context;
  ct->kernel->Bootstrap(ct);
  return (NULL);
}

void Kernel::Bootstrap(void *context) {
  pthread_mutex_lock(&CPULocker);

  Context *ct = (Context *)context;
  int Task = ct->task_id;
  auto routine = ct->start_routine;
  auto args = ct->arg;
  delete ct;

  while (scheduler->get_state(Task) != RUNNING) {
    pthread_cond_wait(scheduler->get_cond_t(Task), &CPULocker);
  }

  routine(args);
  scheduler->set_state(Task, DEAD);
  scheduler->yield();
  pthread_mutex_unlock(&CPULocker);
}

Kernel *Kernel::Get_Instance() {
  if (KernelPtr == nullptr) {
    KernelPtr = new Kernel();
  }
  return KernelPtr;
}

int Kernel::Create_Task(uthread_t newthread, void *(*start_routine)(void *),
                        void *arg) {
  int Task = scheduler->create_task();
  pthread_t Thread;

  Context *ct = new Context;
  ct->task_id = Task;
  ct->kernel = this;
  ct->start_routine = start_routine;
  ct->arg = arg;

  pthread_create(&Thread, NULL, &Bootstrap_Wrapper, ct);
  scheduler->set_pthread_t(Task, Thread);
}

Semaphore *Kernel::Create_Semaphore(const char *name) {
  pthread_mutex_lock(&CPULocker);
  Semaphore *temp = new Semaphore(name);
  Sema_Vector.push_back(temp);
  pthread_mutex_unlock(&CPULocker);

  return (temp);
}
