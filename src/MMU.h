/* Memory Management Unit Header File
 * Ultima 2.0
 *
 * The Memory Management Unit is responsible for managing the memory requested
 * by tasks.
 *
 * Of note to the public:
 *  1. MMU *Get_Instance()
 *      - Returns a pointer to the MMU.
 *  2. int Alloc(int Size)
 *      - Allocates memory for the requesting task.
 *        Returns a handle to the memory.
 *        Or -1 on failure.
 *  3. int Free(int Handle)
 *      - Frees a block by its handle.
 *        Returns -1 on failure, 1 on success.
 *  4. int Read(int Handle)
 *      - Reads a block by its handle at the current read pointer.
 *        Returns -1 on failure or a char as an int.
 *  5. int Write(int Handle, char ch)
 *      - Writes to a block by its handle at the current write pointer.
 *        Returns -1 on failure or the written char as an int.
 *  6. string Read(int Handle, int offset, int size)
 *      - Reads a series of char from block Handle, at position 'offset', for
 *        length 'size'.
 *        Returns an empty string on failure.
 *  7. int Write(int Handle, int offset, char *text)
 *      - Writes *text to a block Handle at position 'offset'.
 *        Returns -1 on failure, 1 on success.
 *  8. string Dump_A_Block(int Handle)
 *      - Dumps a specific block by its Handle.
 *        Returns an empty string on failure.
 *        Returns the block's contents on success.
 *  9. string Dump_Blocks()
 *      - Dumps all blocks.
 *
 * Hunter Poole
 * 04-25-2026
 */

#pragma once

#include "Kernel.h"
#include <list>
#include <pthread.h>
#include <string>

using namespace std;

/* class MMU
 *
 * This class has serves a purpose described above, with public members as
 * described above, and private members as described below:
 *
 * Variables:
 * 1. static inline MMU *MMU_Ptr;
 *    - A pointer to the MMU.
 *    - MMU is Singleton. Only one may exist. Controlled by MMU *Get_Instance().
 * 2. char Memory[1024];
 *    - The bytes of memory available to Ultima 2.0
 * 3. int Free_Memory;
 *    - A counter for the available memory.
 * 4. int Next_Handle;
 *    - A counter for which handle is available for assignment.
 * 5. list<Block *> Blocks;
 *    - A list that contains each block.
 *      This is the linked list for free/used memory blocks.
 *
 * Methods:
 * 1. MMU();
 *    - A no-arg constructor.
 * 2. ~MMU();
 *    - A no-arg destructor.
 * 3. int Left();
 *    - Returns Free_Memory.
 * 4. int Largest();
 *    - Returns the size of the largest free block.
 * 5. int Smallest();
 *    - Returns the size of the smallest free block.
 * 6. void Coalesce();
 *    - Condenses all used memory to the beginning of Memory[],
 *      and all free memory to the end of Memory[].
 *      Will update Blocks to match.
 */
class MMU {
private:
  static inline MMU *MMU_Ptr = nullptr;
  Kernel *KernelPtr;
  Semaphore *SemaphorePtr;
  char Memory[1024];
  int Free_Memory = 1024;
  int Next_Handle = 0;

  /* struct Block
   *
   * The actual memory Block to be assigned and referenced by tasks.
   * In this case, it does not *hold* memory, but holds indices/"pointers" to
   * memory locations.
   *
   * We assign Blocks to tasks, and Blocks give tasks the ability to manipulate
   * memory.
   *
   * Variables:
   * 1. int handle;
   *    - The block's handle. A unique identifier.
   * 2. int base;
   *    - The lowest position in memory this block has access to.
   * 3. int limit.
   *    - The highest position in memory this block has access to.
   * 4. int read;
   *    - A read pointer, for reading from memory sequentially.
   * 5. int write;
   *    - A write pointer, for writing to memory sequentially.
   * 6. bool empty;
   *    - A bool for if the block is empty or not.
   * 7. pthread_t owner;
   *    - Stores the ID of the owning task.
   *      Allows comparison checks and prevents unauthorized access.
   *
   * Methods:
   * 1. Block();
   *    - A no-arg constructor.
   */
  struct Block {
    int handle, base, limit, read, write;
    bool empty;
    pthread_t owner;

    Block();
  };

  list<Block *> Blocks;

  MMU();

  ~MMU();

  int Left();

  int Largest();

  int Smallest();

  void Coalesce();

public:
  static MMU *Get_Instance();

  // Delete the copy constructor.
  MMU(const MMU &) = delete;

  // Delete the assignment operator.
  MMU &operator=(const MMU &) = delete;

  int Alloc(int Size);

  int Free(int Handle);

  int Read(int Handle);

  int Write(int Handle, char ch);

  string Read(int Handle, int offset, int size);

  int Write(int Handle, int offset, char *text);

  string Dump_A_Block(int Handle);

  string Dump_Blocks();
};
