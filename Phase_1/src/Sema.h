/* Semaphore Class Header File
 * Ultima 2.0
 *
 * This Semaphore header file shows the private and public resources of the
 * Semaphore class.
 *
 * Of note to the public, who wish to use this class:
 *
 *  1. Semaphore(cost char *Name);
 *      - A constructor. Takes the name of the resource the Semaphore manages.
 *  2. ~Semaphore();
 *      - A destructor.
 *  3. void down();
 *      - Call this to attempt to lock and access the guarded resource.
 *      - Threads that can't get access to the resource will wait in a FIFO
 *        queue until they are called on.
 *      - This is a blocking wait, but is not a busy wait.
 *  4. void up();
 *      - Call this to unlock the guarded resource for use by other threads.
 *  5. void dump();
 *      - Call this to see the current contents and/or state of the Semaphore.
 *
 * 03-27-2026
 */

#pragma once

#include <pthread.h>
#include <queue>

using namespace std;

/* Semaphore class.
 *
 * Of note on the private resources:
 *
 * 1. char resource_name[64];
 *    - Stores the name of the resource the Semaphore manages.
 *    - This is set *once* via the constructor.
 * 2. int sema_value = 1;
 *    - Any valid binary Semaphore should start its life as "available."
 *      As in, you may not simultaneously create and lock a Semaphore.
 *      Thus, it is "up" and available for use from the get-go.
 * 3. queue<pthread_t> sema_queue;
 *    - Stores the pthread IDs for the pthreads waiting for access on the
 *      guarded resource.
 *    - Is a FIFO queue. Threads may not proceed unless they are in the front.
 * 4. pthread_mutex_t lock;
 *    - A mutex to lock the guarded resource.
 * 5. pthread_cond_t cond;
 *    - A condition on which all threads will wait.
 *    - Used to signal that a thread is good to go.
 */

class Semaphore {
private:
  char resource_name[64];
  int sema_value = 1;
  queue<pthread_t> sema_queue;
  pthread_mutex_t lock;
  pthread_cond_t cond;

public:
  Semaphore(const char *Name);

  ~Semaphore();

  void down();

  void up();

  void dump();
};
