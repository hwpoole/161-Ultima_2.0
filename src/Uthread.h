/* Uthread Class Header File
 * 161-Ultima 2.0
 *
 * Uthread is a Facade over pthread.
 * It was created to repalce calls to the pthread library, such that working
 * with the Ultima 2.0 OS would be easy, and based off prior knowledge.
 *
 * Public methods:
 * 1. uthread()
 *    - A no-arg constructor.
 * 2. int create()
 *    - A call to create a uthread and dispatch it, with args.
 * 3. int join()
 *    - A call to join a uthead.
 * 4. uthread_t self()
 *    - Returns the uthread_t for the calling uthread.
 *
 * Hunter Poole
 * 04-05-2026
 */

#pragma once

#include "Kernel.h"
#include <pthread.h>

using namespace std;

// uthread_t is an alias for pthread_t.
typedef pthread_t uthread_t;

class uthread {
public:
  uthread() = default;

  int create(uthread_t *newthread, void *(*start_routine)(void *), void *arg);

  int join(uthread_t th, void **thread_return);

  uthread_t self();
};
