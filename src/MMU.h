/* Memory Management Unit Header File
 * Ultima 2.0
 *
 * The Memory Management Unit is responsible for managing the memory requested
 * by tasks.
 *
 * Of note to the public:
 *  1.  MMU()
 *      - A no-arg constructor.
 *  2.
 *
 * Hunter Poole
 * 04-25-2026
 */

#pragma once

#include <list>
#include <pthread.h>
#include <string>

using namespace std;

class MMU {
private:
  static inline MMU *MMU_Ptr = nullptr;
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

  MMU();

  ~MMU();

public:
  static MMU *Get_Instance();

  int Mem_Alloc(int Size);

  int Mem_Free(int Handle);

  int Mem_Read(int Handle);

  int Mem_Write(int Handle, char ch);

  string Mem_Read(int Handle, int offset, int size);

  int Mem_Write(int Handle, int offset, char *text);

  string Dump_A_Block(int Handle);

  string Dump_Blocks();
};
