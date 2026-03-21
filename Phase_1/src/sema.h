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
  queue<int> *sema_queue;

public:
  bool down() {
    if (sema_value == 1) {
      sema_value = 0;
      return true;
    } else {
      return false;
    }
  }

  bool up() {
    if (sema_value == 0) {
      sema_value = 1;
      return true;
    } else {
      return false;
    }
  }

  void dump(int level) {}
};
