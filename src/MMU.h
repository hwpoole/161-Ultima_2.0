/* Memory Management Unit Header File
 * MMU.h
 *
 * Hunter Poole
 * 04-23-2026
 */

#pragma once

#include "CircularLinkedList.h"

class MMU {
private:
  struct Block {
    pthread_t owner;
    char Bytes[64];
  };

  CircularLinkedList<Block *> Memory;

public:
  MMU();
};
