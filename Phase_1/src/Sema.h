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
  queue<int> *sema_queue;
  pthread_mutex_t lock;
  pthread_cond_t cond;

public:
  Semaphore(int initialValue) : sema_value(initialValue) {
    pthread_mutex_init(&lock, nullptr);
    pthread_cond_init(&cond, nullptr);
  }

  ~Semaphore() {
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
  }

  void down() {}

  void up() {}

  void dump(int level) {}
};
