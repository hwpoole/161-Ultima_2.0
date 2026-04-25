#include "MMU2.h"
#include <cmath>
#include <pthread.h>
#include <vector>

MMU::Block::Block() {
  base = 0;
  limit = 0;
  empty = true;
  owner = -1;
}

MMU::MMU() {
  fill(begin(Memory), end(Memory), '.');
  Block *NewBlock = new Block;
  NewBlock->limit = 1023;
  Blocks.push_back(NewBlock);
}

MMU::~MMU() {
  while (!Blocks.empty()) {
    Block *DeadBlock;
    DeadBlock = Blocks.back();
    delete DeadBlock;
    Blocks.pop_back();
  }
}

/* int Mem_Alloc(int Size) {...}
 *
 * Finds a free block and assigns it to the requesting task.
 * Returns a handle to that block.
 *
 * 1. Check if enough free memory exists to satisfy the request.
 *    1a. If not, return -1.
 * 2. Find an orphaned block.
 *    2a. If none, return -1.
 * 3. Check that orphaned block's size.
 *    3a. If that size > 64, cut it into some multiple of 64 >= Size.
 *        a. Create a new block for that free space.
 *        b. Insert this new block into the list in front of the orphaned block.
 *        c. Return the handle to that block.
 *    3b. Else,
 *        a. Assign that block to the requesting task.
 *        b. Return the handle to that block.
 */
int MMU::Mem_Alloc(int Size) {
  // Check if there is enough free memory.
  if (Free_Memory <= Size) {
    return -1;
  }

  // Find an orphaned block.
  Block *Orphan;
  for (Block *block : Blocks) {
    Orphan = block;

    // If found, break.
    if (Orphan->owner == -1) {
      break;
    }
  }

  // Check if we failed to find an orphaned block.
  if (Orphan->owner != -1 || !Orphan->empty) {
    return -1;
  }

  /* Large if-statement.
   *
   * 1. If the block is larger than 64 bytes...
   *    1a. Make a new block.
   *        a. Set its base to the Orphan base.
   *        b. Set its limit to 64x the rounded-up number of 64-byte blocks
   *              needed. The Free_Memory check guards this from going outside
   *              the bounds of Memory[].
   *        c. Set the new block's owner.
   *        d. Set the new block's handle.
   *    1b. Find the Oprhan's position in the list.
   *        a. Update Orphan's base to start after the new block.
   *        b. Save Oprhan back to list.
   *        c. Insert the new block before the Oprhan.
   *    1c. Remove the amount of memory allocated from Free_Memory.
   *    1d. Return a handle to the new block.
   * 2. If the block is not larger than 64 bytes...
   *    2a. Find the Oprhan block's position.
   *        a. Update the Orphan's owner to this thread.
   *        b. Set the Orphan's handle.
   *        c. Save Orphan back to the list.
   *    2b. Return a handle to the new block.
   */
  if ((Orphan->limit - Orphan->base) > 63) {
    Block *NewBlock = new Block;
    NewBlock->base = Orphan->base;
    NewBlock->limit = NewBlock->base + (ceil(Size / 64) * 64) - 1;
    NewBlock->owner = pthread_self();
    NewBlock->handle = Next_Handle++;

    auto it = find(begin(Blocks), end(Blocks), Orphan);
    int pos = distance(begin(Blocks), it);
    Orphan->base = NewBlock->limit + 1;
    *it = Orphan;
    Blocks.insert(it, NewBlock);

    Free_Memory = Free_Memory - (NewBlock->limit - NewBlock->base);
    return (Next_Handle - 1);
  } else {
    auto it = find(begin(Blocks), end(Blocks), Orphan);
    int pos = distance(begin(Blocks), it);
    Orphan->owner = pthread_self();
    Orphan->handle = Next_Handle++;
    *it = Orphan;

    return (Next_Handle - 1);
  }
}

/* int Mem_Free(int Handle) {...}
 *
 * Finds a block of memory by its handle and deletes its contents.
 * Then, marks the block as having no owner, no handle, and being empty.
 * Returns -1 on failure, 1 on success.
 *
 * 1. If handle is out-of-bounds...
 *    1a. Return -1.
 * 2. Check the list of blocks for a matching handle.
 * 3. If no matching handle found...
 *    3a. Return -1.
 * 4. Loop over the block's range (base to limit) in memory to write '#'.
 * 5. Update the block to have no owner, no handle, and be empty.
 * 6. Decrement Next_Handle.
 * 7. Return 1 for success.
 */
int MMU::Mem_Free(int Handle) {
  // 16 Maximum blocks: 1024 / 64 = 16.
  if (Handle < 0 || Handle > 16) {
    return -1;
  }

  Block *DeadBlock;
  for (Block *block : Blocks) {
    DeadBlock = block;

    if (DeadBlock->handle == Handle) {
      break;
    }
  }

  if (DeadBlock->handle != Handle) {
    return -1;
  }

  for (int i = DeadBlock->base; i < DeadBlock->limit; i++) {
    Memory[i] = '#';
  }

  // Update block to have no owner, no handle, and be empty.
  auto it = find(begin(Blocks), end(Blocks), DeadBlock);
  int pos = distance(begin(Blocks), it);
  DeadBlock->owner = -1;
  DeadBlock->handle = -1;
  DeadBlock->empty = true;
  *it = DeadBlock;
  Next_Handle--;

  return 1;
}
