/* Memory Management Unit Header File
 * MMU.h
 *
 * Hunter Poole
 * 04-23-2026
 */

#pragma once

#include "CircularLinkedList.h"
#include <vector>

using namespace std;

class MMU {
private:
  struct Block {
    pthread_t owner;
    char Bytes[64];

    Block();
  };

  CircularLinkedList<Block *> Memory;
  vector<bool> Free;

public:
  MMU();

  ~MMU();

  int Mem_Alloc(int size);
};
