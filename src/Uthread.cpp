/* Uthread Class Implementation File
 * 161-Ultima 2.0
 *
 * Hunter Poole
 * 04-05-2026
 */

#include "Uthread.h"
#include "Kernel.h"

int uthread::create(uthread_t newthread, void *(*start_routine)(void *),
                    void *arg) {
  return KernelPtr->CreateTask(newthread, start_routine, arg);
}

int uthread::join(uthread_t th, void **thread_return) {
  return KernelPtr->JoinTask(th, thread_return);
}
