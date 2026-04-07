/* Uthread Class Header File
 * 161-Ultima 2.0
 *
 * Uthread is a Facade over pthread.
 * It was created to repalce calls to the pthread library, such that working
 * with the Ultima 2.0 OS would be easy, and based off prior knowledge.
 *
 * Hunter Poole
 * 04-05-2026
 */

#pragma once

#include "Kernel.h"
#include <pthread.h>

using namespace std;

typedef pthread_t uthread_t;

class uthread {
private:
  Kernel *KernelPtr = Kernel::GetInstance();

public:
  int create(uthread_t newthread, void *(*start_routine)(void *), void *arg);

  int join(uthread_t th, void **thread_return);

  uthread_t self();
};
