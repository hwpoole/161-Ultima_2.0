/* Semaphore Class
 * Ultima 2.0
 *
 * 03-27-2026
 */

#include "Sema.h"
#include <pthread.h>
#include <queue>

Semaphore::Semaphore(char Name[64]) : resource_name() {
  sema_value = 1;
  pthread_mutex_init(&lock, nullptr);
  pthread_cond_init(&cond, nullptr);
}

Semaphore::~Semaphore() {
  pthread_mutex_destroy(&lock);
  pthread_cond_destroy(&cond);
}

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

void Semaphore::up() {
  pthread_mutex_lock(&lock);
  sema_value++;

  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&lock);
}

void dump(int level) {}
