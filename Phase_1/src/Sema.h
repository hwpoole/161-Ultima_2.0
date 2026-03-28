/* Semaphore Class Header File
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
