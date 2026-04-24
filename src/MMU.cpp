/* Memory Management Unit Implementation File
 * MMU.cpp
 *
 * Hunter Poole
 * 04-23-2026
 */

#include "MMU.h"

/* MMU() {...}
 *
 * No-arg constructor.
 *
 * We use 16 blocks of 64 bytes to create 1024 bytes of RAM for Ultima 2.0.
 * Since we are allocating these blocks "on boot," we don't yet have an owner
 * for them.
 *
 * Nested for-loops. Do this 16x.
 *  1. Create a new Block pointer.
 *     2. Initialize all bytes to '.'
 *  3. Insert that block into Memory.
 */
MMU::MMU() {
  for (int i = 0; i < 15; i++) {
    Block *NewBlock = new Block();
    for (int j = 0; j < 63; j++) {
      NewBlock->Bytes[j] = '.';
    }
    Memory.insert_front(NewBlock);
  }
}

/* ~MMU() {...}
 *
 * Destructor.
 *
 * Gets all memory blocks and deletes them.
 * Then, calls to remove nodes from Memory CLL.
 *
 * 1. While memory is not empty...
 *    1a. Get the block from the front of the CLL.
 *    1b. Delete the block.
 *    1c. Remove the node from the front of the CLL.
 */
MMU::~MMU() {
  while (!Memory.is_empty()) {
    Block *DeadBlock = Memory.get_front();
    delete DeadBlock;
    Memory.remove_front();
  }
}
