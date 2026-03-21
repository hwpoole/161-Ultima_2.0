/* Semaphore Class
 * Ultima 2.0
 *
 * 03-20-2026
 */

#pragma once

#include <queue>

using namespace std;

class semaphore {
private:
  char resource_name[64];
  int sema_value;
  queue<char> *sema_queue;

public:
  bool down() { return false; }

  bool up() { return false; }

  void dump(int level) {}
};
