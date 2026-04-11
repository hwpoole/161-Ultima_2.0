/* Kernel Class Implementation File
 *
 * Hunter Poole
 * 04-05-2026
 */

#include "Kernel.h"
#include "Uthread.h"
#include <pthread.h>

Kernel::Kernel() {
  scheduler = new Scheduler();
  scheduler->set_quantum(1);
  Semaphore::set_scheduler(scheduler);
}

void *Kernel::Bootstrap_Wrapper(void *context) {
  Context *ct = (Context *)context;
  ct->kernel->Bootstrap(ct);
  return (NULL);
}

void Kernel::Bootstrap(void *context) {
  Context *ct = (Context *)context;
  int Task = ct->task_id;
  auto routine = ct->start_routine;
  auto args = ct->arg;
  delete ct;

  pthread_mutex_lock(&CPULocker);
  while (scheduler->get_state(Task) != RUNNING) {
    pthread_cond_wait(scheduler->get_cond_t(Task), &CPULocker);
  }

  routine(args);

  scheduler->set_state(Task, DEAD);
  scheduler->yield();
  // pthread_mutex_unlock(&CPULocker);
}

Kernel *Kernel::Get_Instance() {
  if (KernelPtr == nullptr) {
    KernelPtr = new Kernel();
  }
  return KernelPtr;
}

int Kernel::Create_Task(uthread_t *newthread, void *(*start_routine)(void *),
                        void *arg) {
  int task_id = scheduler->create_task();

  Context *ct = new Context;
  ct->kernel = this;
  ct->task_id = task_id;
  ct->start_routine = start_routine;
  ct->arg = arg;

  int result = pthread_create(newthread, NULL, &Bootstrap_Wrapper, ct);
  scheduler->set_pthread_t(task_id, *newthread);

  return (result);
}

int Kernel::Join_Task(uthread_t th, void **thread_return) { return (0); }

Semaphore *Kernel::Create_Semaphore(const char *name) {
  pthread_mutex_lock(&CPULocker);
  Semaphore *temp = new Semaphore(name);
  Sema_Vector.push_back(temp);
  pthread_mutex_unlock(&CPULocker);

  return (temp);
}

Scheduler *Kernel::Get_Scheduler() { return scheduler; }
