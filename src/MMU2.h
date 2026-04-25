#pragma once

#include <list>
#include <pthread.h>
#include <string>

using namespace std;

class MMU {
private:
  char Memory[1024];
  int Free_Memory = 1024;
  int Next_Handle = 0;

  struct Block {
    int handle, base, limit, read, write;
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

  int Mem_Read(int Handle);

  int Mem_Write(int Handle, char ch);

  string Mem_Read(int Handle, int offset, int size);

  int Mem_Write(int Handle, int offset, char *text);
};
