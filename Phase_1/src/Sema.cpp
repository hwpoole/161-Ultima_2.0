/* Semaphore Class
 * Ultima 2.0
 *
 * 03-27-2026
 */

#include "Sema.h"
#include <cstring>
#include <pthread.h>
#include <queue>

Semaphore::Semaphore(const char *Name) {
  strncpy(resource_name, Name, 63);
  resource_name[63] = '\0';
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
