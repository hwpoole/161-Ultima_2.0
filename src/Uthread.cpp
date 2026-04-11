/* Uthread Class Implementation File
 * 161-Ultima 2.0
 *
 * This file houses the implementation details for the Uthread class.
 *
 * Hunter Poole
 * 04-05-2026
 */

#include "Uthread.h"
#include "Kernel.h"

/* int create(uthread_t *newthread, void *(*start_routine)(void *), void *arg)
 * {...}
 *
 * This method creates a uthread, and registers it with the Kernel.
 * The Kernel class will orchestrate the thread's movement into the provided
 * start_routine, using the provided args.
 *
 * 1. Pass all provided information to the Kernel.
 */
int uthread::create(uthread_t *newthread, void *(*start_routine)(void *),
                    void *arg) {
  return Kernel::Get_Instance()->Create_Task(newthread, start_routine, arg);
}

/* int join(uthread_t th, void **thread_return) {...}
 *
 * This method is a call to join a uthread, analogous to pthread_join().
 *
 * 1. Pass all provided information to the Kernel.
 */
int uthread::join(uthread_t th, void **thread_return) {
  return Kernel::Get_Instance()->Join_Task(th, thread_return);
}
