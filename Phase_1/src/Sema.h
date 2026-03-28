/* Semaphore Class
 * Ultima 2.0
 *
 * 03-20-2026
 */

#pragma once

#include <pthread.h>
#include <queue>

using namespace std;

class Semaphore {
private:
  char resource_name[64];
  int sema_value;
  queue<pthread_t> sema_queue;
  pthread_mutex_t lock;
  pthread_cond_t cond;

public:
  Semaphore(char Name[64]) : resource_name() {
    sema_value = 1;
    pthread_mutex_init(&lock, nullptr);
    pthread_cond_init(&cond, nullptr);
  }

  ~Semaphore() {
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
  }

  void down() {
    pthread_mutex_lock(&lock);

    if (sema_value >= 1) {
      sema_value--;
    } else {
      pthread_t this_thread = pthread_self();
      sema_queue.push(this_thread);

      do {
        pthread_cond_wait(&cond, &lock);
      } while (sema_value < 0);
    }
    pthread_mutex_unlock(&lock);
  }

  void up() {
    pthread_mutex_lock(&lock);

    if (sema_value <= 0 && !sema_queue.empty()) {
      sema_queue.pop();
      pthread_cond_signal(&cond);
    } else {
      sema_value++;
      pthread_mutex_unlock(&lock);
    }
  }

  void dump(int level) {}
};
