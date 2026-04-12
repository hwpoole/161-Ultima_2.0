/* Kernel Class Implementation File
 *
 * This file houses the implementation details for Kernel.h
 *
 * Hunter Poole
 * 04-05-2026
 */

#include "Kernel.h" // The Kernel we are implementing.
#include "Uthread.h"
#include <pthread.h>

/* Kernel() {...}
 *
 * A no-arg constructor.
 *
 * 1. Creates a new scheduler.
 * 2. Sets its quantum LOW for the demo.
 *    - Thus, tasks always yield when calling yield.
 * 3. Registers the scheduler with the Semaphore class.
 */
Kernel::Kernel() {
  scheduler = new Scheduler();
  scheduler->set_quantum(0);
  Semaphore::set_scheduler(scheduler);
}

/* void Bootstrap_Wrapper(void *context) {...}
 *
 * A static void wrapper method for pthread_create.
 * pthread_create usese C-style programming, and will *not* allow a direct call
 * Bootstrap().
 *
 * 1. Extract args into Context *ct.
 * 2. Call Bootstrap(ct).
 *    - Uses ct->kernel to make the implicit "this->bootstrap()" included in
 *      public methods into an explicit call.
 * 3. Returns NULL.
 */
void *Kernel::Bootstrap_Wrapper(void *context) {
  Context *ct = (Context *)context;
  ct->kernel->Bootstrap(ct);
  return (NULL);
}

/* void Bootstrap(void *context) {...}
 *
 * Creates and manages "The Scheduler Waiting Room" for each task.
 * Marks a task DEAD and yields it when done.
 *
 * 1. Extract args into Context *ct.
 * 2. Get all needed info from ct.
 * 3. Delete ct.
 * 4. Lock CPULocker.
 * 5. pthread_cond_wait loop until scheduler marks task as RUNNING.
 * 6. Run the provided start_routine with provided args.
 * 7. Set the task's state as DEAD.
 * 8. Yield.
 */
void Kernel::Bootstrap(void *context) {
  Context *ct = (Context *)context;
  int task_id = ct->task_id;
  auto routine = ct->start_routine;
  auto args = ct->arg;
  delete ct;

  pthread_mutex_lock(&CPULocker);
  while (scheduler->get_state(task_id) != RUNNING) {
    pthread_cond_wait(scheduler->get_cond_t(task_id), &CPULocker);
  }

  routine(args);

  scheduler->set_state(task_id, DEAD);
  scheduler->yield();
  pthread_mutex_unlock(&CPULocker);
}

/* Kernel *Get_Instance() {...}
 *
 * Returns a pointer to the Kernel.
 * If the current KernelPtr is null, it creates a new Kernel.
 *
 * This is the only way to create or get a reference to the Kernel.
 * Thus ensuring only one Kernel is active at a time.
 *
 * 1. If KernelPtr is null...
 *    - Initialize it with new Kernel();
 * 2. Else...
 *    - Return the current KernelPtr.
 */
Kernel *Kernel::Get_Instance() {
  if (KernelPtr == nullptr) {
    KernelPtr = new Kernel();
  }
  return KernelPtr;
}

/* int Create_Task(uthread_t *newthread, void *(*start_routine)(void *), void
 *                  *arg) {...}
 *
 * A uthread's entry point into management by the Kernel.
 *
 * 1. Create a new task and get its task_id.
 * 2. Create a Context *ct for pass to Bootstrap_Wrapper.
 * 3. Create a pthread for Bootstrap_Wrapper with ct, and get its result code.
 * 4. Register that pthread with the scheduler.
 * 5. Return the pthread result code.
 */
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

/* int Join_Task(uthread_t th, void **thread_return) {...}
 *
 * A join method, similar to pthread_join.
 *
 * TODO: This.
 */
int Kernel::Join_Task(uthread_t th, void **thread_return) { return (0); }

/* Semaphore *Create_Semaphore(const char *name) {...}
 *
 * A method to create and register a Semaphore with the Kernel.
 *
 * 1. Locks the CPULocker.
 * 2. Creates a new Semaphore with provided name.
 * 3. Pushes it back onto Sema_Vector.
 * 4. Unlocks CPULocker.
 * 5. Returns the pointer to the Semaphore.
 */
Semaphore *Kernel::Create_Semaphore(const char *name) {
  pthread_mutex_lock(&CPULocker);
  Semaphore *temp = new Semaphore(name);
  Sema_Vector.push_back(temp);
  pthread_mutex_unlock(&CPULocker);

  return (temp);
}

/* Scheduler *Get_Scheduler() {...}
 *
 * A getter method for the Kernel's registered scheduler.
 *
 * 1. Return a pointer to the scheduler.
 */
Scheduler *Kernel::Get_Scheduler() { return scheduler; }

/* Pipe *Create_Pipe(int id) {...}
 *
 * A method to create and register a Pipe with the Kernel.
 *
 * 1. Locks the CPULocker.
 * 2. Creates a new Pipe with provided id.
 * 3. Pushes it back onto Pipe_Vector.
 * 4. Unlocks CPULocker.
 * 5. Returns the pointer to the Pipe.
 */
Pipe *Kernel::Create_Pipe(int id) {
  // pthread_mutex_lock(&CPULocker);
  Pipe *temp = new Pipe(id);
  Pipe_Vector.push_back(temp);
  // pthread_mutex_unlock(&CPULocker);

  return (temp);
}

/* Pipe *Get_Pipe(int id) {...}
 *
 * A method to get a Pipe from the Kernel by its id.
 *
 * 1. Locks the CPULocker.
 * 2. Searches Pipe_Vector for the pipe matching the id.
 * 3. If found, returns a pointer to the pipe.
 * 4. Else, returns a new pipe from Create_Pipe().
 */
Pipe *Kernel::Get_Pipe(int id) {
  // pthread_mutex_lock(&CPULocker);
  if (!Pipe_Vector.empty()) {
    Pipe *temp = Pipe_Vector.front();

    for (int i = 0; i < Pipe_Vector.size() && temp->pfd != id; i++) {
      temp = Pipe_Vector.at(i);
    }

    if (temp->pfd == id) {
      return (temp);
    }
  } else {
    return (Kernel::Create_Pipe(id));
  }

  return (NULL);
}
