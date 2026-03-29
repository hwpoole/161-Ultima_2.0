/* Semaphore Class
 * Ultima 2.0
 *
 * This Semaphore class file houses the implementation details of Sema.h.
 * It is recommended you view Sema.h for information on how to use the Semaphore
 * class.
 *
 * Hunter Poole
 * 03-27-2026
 */

#include "Sema.h"
#include <iostream>
#include <pthread.h>
#include <queue>

/* Semaphore::Semaphore(const char *Name) {...}
 *
 * Constructor.
 *
 * Takes const char *Name as the name of the guarded resource.
 * Copies *Name into resource_name, with a null termination.
 *
 * Then, initializes the pthread mutex and condition variable.
 */
Semaphore::Semaphore(const char *Name) {
  resource_name = Name;
  pthread_mutex_init(&lock, nullptr);
  pthread_cond_init(&cond, nullptr);
}

/* Semaphore::~Semaphore() {...}
 *
 * Destructor.
 *
 * Destroys both pthread mutex and condition variable.
 */
Semaphore::~Semaphore() {
  pthread_mutex_destroy(&lock);
  pthread_cond_destroy(&cond);
}

/* void Semaphore::down() {...}
 *
 * 1. Locks the mutex.
 * 2. Pushes this thread onto the queue.
 * 3. Waits the current thread until the semaphore to be available & for this
 *    thread to be at the front of the FIFO queue.
 * 4. Pops the current thread off the queue.
 * 5. Decrements the semaphore, marking it unavaiable.
 * 6. Unlocks the mutex.
 *
 * This implementation varies greatly from what is given in Lab 10. Here is why:
 *
 * 1. If the semaphore is not available (sema_value < 1), we cannot take it.
 *    - We only take (decrement) the semaphore when it is available.
 *      Thus, we have no need to check & decrement the value before we take it.
 * 2. No need for print statements here, given the provided dump() method.
 * 3. We intend for the queue to be FIFO.
 *    - As such, no thread should continue to access the resource if it is not
 *      at the front of the queue.
 *    - Therefore, we add the current thread to the queue, wait if necessary,
 *      then pop the current thread from the queue when we have gained access to
 *      the guarded resource.
 * 4. Once we take our turn, we decrement the Semaphore to disallow
 *    other threads from interacting with the guarded resource, but not before
 *    we know we are able to get to the resource.
 */
void Semaphore::down() {
  pthread_mutex_lock(&lock);

  pthread_t this_thread = pthread_self();
  sema_queue.push(this_thread);

  while (sema_value <= 0 || sema_queue.front() != this_thread) {
    pthread_cond_wait(&cond, &lock);
  }

  sema_queue.pop();
  sema_value--;

  pthread_mutex_unlock(&lock);
}

/* void Semaphore::up() {...}
 *
 * 1. Locks the mutex.
 * 2. Increments the sema_value (marks Semaphore as available).
 * 3. Wakes *all* threads.
 *    - Since we don't yet have a way to track which thread is at the front of
 *      the queue.
 * 4. Unlocks the mutex.
 *
 * @TODO: Eliminate `pthread_cond_broadcast(&cond)` in favor of some solution
 *        that wakes *just* the thread at the front of the queue.
 */
void Semaphore::up() {
  pthread_mutex_lock(&lock);
  sema_value++;

  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&lock);
}

/* void Semaphore::dump() {...}
 *
 * dump() is a "pretty print" to display the contents and current state of the
 * Semaphore.
 *
 * 1. Locks the mutex.
 * 2. Prints to the screen.
 * 3. Copies the current queue into a print_queue.
 * 4. Reads, prints, appends, and pops on the print_queue.
 * 5. Unlocks the mutex.
 */
void Semaphore::dump() {
  pthread_mutex_lock(&lock);

  cout << "Resource: " << resource_name << endl;
  cout << "Sema_value: " << sema_value << endl;
  cout << "Sema_queue: ";

  queue<pthread_t> print_queue = sema_queue;

  while (!print_queue.empty()) {
    cout << (unsigned long)print_queue.front();
    print_queue.pop();
    cout << " --> ";
  }
  cout << endl;

  pthread_mutex_unlock(&lock);
}
