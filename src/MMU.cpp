/* Memory Management Unit Implementation File
 * MMU.cpp
 *
 * Hunter Poole
 * 04-23-2026
 */

#include "MMU.h"

/* Block() {...}
 *
 * No-arg constructor.
 *
 * Primary benefit of using this is to have Bytes be initialized to '.'
 *
 * 1. Sets owner to NULL.
 * 2. Fills all bytes with '.'
 */
MMU::Block::Block() {
  owner = NULL;
  fill(begin(Bytes), end(Bytes), '.');
}

/* MMU() {...}
 *
 * No-arg constructor.
 *
 * We use 16 blocks of 64 bytes to create 1024 bytes of RAM for Ultima 2.0.
 * Since we are allocating these blocks "on boot," we don't yet have an owner
 * for them.
 *
 * Do this 16x:
 *  1. Create a new Block pointer.
 *  2. Insert that block into Memory.
 */
MMU::MMU() {
  for (int i = 0; i < 15; i++) {
    Block *NewBlock = new Block();
    Memory.insert_front(NewBlock);
    Free.push_back(true);
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

/* Mem_Alloc(int size) {...}
 *
 * Assigns memory blocks to the requesting task based on the size argument.
 * Finds empty blocks in Memory and assigns them to the task until there are no
 * more empty blocks OR we have allocated all that the requesting task has asked
 * for.
 *
 * 1. Get an Empty block for comparison.
 * 2. Get the start position.
 * 3. Find an empty block, if one exists.
 * 4. Make an int 'allocated' to track allocated bytes.
 * 5. While we have more bytes to allocate, and empty blocks exist...
 *    5a. Move to an empty block.
 *    5b. Set that block's owner as this thread via pthread_self().
 *    5c. Increment allocated by 64 (block size).
 *    5d. Decrement size by 64.
 *    5e. Determine if there is another empty block.
 * 6. If the start position is not the current head and still exists...
 *    6a. Move back to the start position.
 * 7. Return the number of allocated bytes.
 */
int MMU::Mem_Alloc(int size) {
  Block *FindEmpty;
  Block *StartBlock = Memory.get_front();
  int pos = Memory.get_pos(FindEmpty);
  int allocated = 0;

  while (size > 0 && pos != -1) {
    Memory.move_to_key(FindEmpty);

    FindEmpty->owner = pthread_self();
    Memory.set_value(FindEmpty);
    allocated += 64;
    size -= 64;
    FindEmpty->owner = NULL;

    pos = Memory.get_pos(FindEmpty);
  }

  // Possible that start pos was empty and allocated to the requesting task.
  if (StartBlock != Memory.get_front() && Memory.get_pos(StartBlock) != -1) {
    Memory.move_to_key(StartBlock);
  }

  return allocated;
}
