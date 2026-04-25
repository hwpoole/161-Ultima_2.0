#pragma once

#include "CircularLinkedList.h"
#include <list>

using namespace std;

class MMU {
private:
  char Memory[1024];
  int Free_Memory = 1024;
  int Next_Handle = 0;

  struct Block {
    int handle;
    int base, limit;
    bool empty;
    pthread_t owner;

    Block();
  };

  list<Block *> Blocks;

public:
  MMU();

  ~MMU();

  int Mem_Alloc(int Size);

  int Mem_Free(int Handle);
};
