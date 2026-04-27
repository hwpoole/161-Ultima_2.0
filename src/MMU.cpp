/* Memory Management Unit Implementation File
 * Ultima 2.0
 *
 * This file houses the Implementation details for the MMU class and Block
 * constructor. It is recommended to view the header file for ways to use the
 * MMU class.
 *
 * Hunter Poole
 * 04-25-2026
 */

#include "MMU.h"
#include "Sema.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <ostream>
#include <pthread.h>
#include <sstream>

/* Block() {...}
 *
 * No-arg Block Constructor.
 *
 * Sets all Block values to defaults.
 */
MMU::Block::Block() {
  handle = -1;
  base = 0;
  limit = 0;
  read = 0;
  write = 0;
  owner = -1;
}

/* MMU() {...}
 *
 * No-arg MMU Constructor.
 *
 * 1. Fills Memory[] with '.'.
 * 2. Makes a new Block for the entirety of Memory[].
 * 3. Pushes it onto Blocks.
 */
MMU::MMU() {

  fill(begin(Memory), end(Memory), '.');
  Block *NewBlock = new Block;
  NewBlock->limit = 1023;
  Blocks.push_back(NewBlock);

  SemaphorePtr = Kernel::Get_Instance()->Create_Semaphore("Memory Semaphore");
  SchedulerPtr = Kernel::Get_Instance()->Get_Scheduler();
}

/* ~MMU() {...}
 *
 * Destructor. Removes all blocks from Blocks.
 *
 * 1. Get a block off of Blocks.
 * 2. Delete it.
 * 3. pop_back on blocks.
 */
MMU::~MMU() {
  while (!Blocks.empty()) {
    Block *DeadBlock;
    DeadBlock = Blocks.back();
    delete DeadBlock;
    Blocks.pop_back();
  }
  delete SemaphorePtr;
}

/* int Left() {...}
 *
 * Returns the amount of free memory.
 */
int MMU::Left() { return Free_Memory; }

/* int Largest() {...}
 *
 * Finds the largest block of free memory and returns its size.
 *
 * 1. Loop over Blocks for free blocks.
 *    1a. If the current block's size is larger than the known largest...
 *          Update the largest block to be the current block.
 * 2. Return size of the largest block.
 */
int MMU::Largest() {
  SemaphorePtr->down();
  Block *TheLargest = nullptr;

  int found = 0;
  for (Block *Current : Blocks) {
    if (Current->owner == -1) {
      if (found == 0) {
        TheLargest = Current;
        found++;
      } else if ((Current->limit - Current->base) >
                 (TheLargest->limit - TheLargest->base)) {
        TheLargest = Current;
      }
    }
  }

  SemaphorePtr->up();
  return (TheLargest->limit - TheLargest->base);
}

/* int Smallest() {...}
 *
 * Finds the smallest block of free memory and returns its size.
 *
 * 1. Loop over Blocks for free blocks.
 *    1. If the current block's size is smaller than the known smallest...
 *          Update the smallest block to the current block.
 * 2. Return the size of the smallest block.
 */
int MMU::Smallest() {
  SemaphorePtr->down();
  Block *TheSmallest = nullptr;

  int found = 0;
  for (Block *Current : Blocks) {
    if (Current->owner == -1) {
      if (found == 0) {
        TheSmallest = Current;
        found++;
      } else if ((Current->limit - Current->base) <
                 (TheSmallest->limit - TheSmallest->base)) {
        TheSmallest = Current;
      }
    }
  }

  SemaphorePtr->up();
  return (TheSmallest->limit - TheSmallest->base);
}

/* void Coalesce() {...}
 *
 * Rebuilds Blocks with all used blocks in order at the beginning, and all empty
 * space at the end. Modifies Memory[] to have all used space in the same order.
 *
 * 1. For each block:
 *    1a. If assigned...
 *        a. Get its size and an offset from the current address.
 *        b. Move its contents in memory.
 *        c. Update the block.
 *        d. Push it onto a new Block list.
 *    1b. Else...
 *        a. Delete the block.
 * 2. If the next address is not at the end of memory...
 *    2a. Make a new block for the empty contents.
 *    2b. Fill it with '.'
 *    2c. Push it onto a new Block list.
 * 3. Replace Blocks with the new Block list.
 */
void MMU::Coalesce() {
  list<Block *> NewBlocks;
  int next_address = 0;

  for (Block *block : Blocks) {
    if (block->owner != -1) {
      int size = block->limit - block->base + 1;
      int offset = next_address - block->base;
      memmove(&Memory[next_address], &Memory[block->base], size);

      block->base = next_address;
      block->limit = next_address + size - 1;
      block->read += offset;
      block->write += offset;

      NewBlocks.push_back(block);
      next_address += size;
    } else {
      delete block;
    }
  }

  if (next_address < 1024) {
    Block *Empty = new Block();
    Empty->base = next_address;
    Empty->limit = 1023;

    memset(&Memory[next_address], '.', 1024 - next_address);
    NewBlocks.push_back(Empty);
  }

  Blocks = NewBlocks;
}

/* MMU *Get_Instance() {...}
 *
 * MMU is Singleton.
 * Returns the MMU_Ptr or makes a new one if nullptr.
 */
MMU *MMU::Get_Instance() {
  if (MMU_Ptr == nullptr) {
    MMU_Ptr = new MMU();
  }

  return MMU_Ptr;
}

/* int Alloc(int Size) {...}
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
int MMU::Alloc(int Size) {
  SemaphorePtr->down();
  // Check if there is enough free memory.
  if (Free_Memory < Size && Size <= 1024) {
    SemaphorePtr->down();
    SemaphorePtr->up();
    return -1;
  } else if (Size > 1024) {
    SemaphorePtr->up();
    return -1;
  }

  // Find an orphaned block.
  Block *Orphan = nullptr;
  for (Block *block : Blocks) {
    Orphan = block;

    // If found, break.
    if (Orphan->owner == -1) {
      break;
    }
  }

  // Check if we failed to find an orphaned block.
  if (Orphan->owner != -1) {
    SemaphorePtr->up();
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
    NewBlock->limit = NewBlock->base + (ceil((double)Size / 64.0) * 64) - 1;
    NewBlock->read = NewBlock->base;
    NewBlock->write = NewBlock->base;
    NewBlock->owner = SchedulerPtr->get_task_id();
    NewBlock->handle = Next_Handle++;

    auto it = find(begin(Blocks), end(Blocks), Orphan);
    int pos = distance(begin(Blocks), it);
    Orphan->base = NewBlock->limit + 1;
    *it = Orphan;
    Blocks.insert(it, NewBlock);

    Free_Memory -= (NewBlock->limit - NewBlock->base + 1);
    SemaphorePtr->up();
    return (Next_Handle - 1);
  } else {
    auto it = find(begin(Blocks), end(Blocks), Orphan);
    int pos = distance(begin(Blocks), it);
    Orphan->owner = SchedulerPtr->get_task_id();
    Orphan->handle = Next_Handle++;
    Orphan->read = Orphan->base;
    Orphan->write = Orphan->base;
    *it = Orphan;

    Free_Memory -= (Orphan->limit - Orphan->base + 1);
    SemaphorePtr->up();
    return (Next_Handle - 1);
  }
}

/* int Free(int Handle) {...}
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
 * 7 Update Free_Memory.
 * 8. Return 1 for success.
 */
int MMU::Free(int Handle) {
  SemaphorePtr->down();
  // 16 Maximum blocks: 1024 / 64 = 16.
  if (Handle < 0 || Handle > 16) {
    SemaphorePtr->up();
    return -1;
  }

  Block *DeadBlock = nullptr;
  for (Block *block : Blocks) {
    DeadBlock = block;

    if (DeadBlock->handle == Handle) {
      break;
    }
  }

  if (DeadBlock->handle != Handle) {
    SemaphorePtr->up();
    return -1;
  }

  for (int i = DeadBlock->base; i <= DeadBlock->limit; i++) {
    Memory[i] = '#';
  }

  // Update block to have no owner, no handle, and be empty.
  auto it = find(begin(Blocks), end(Blocks), DeadBlock);
  int pos = distance(begin(Blocks), it);
  DeadBlock->owner = -1;
  DeadBlock->handle = -1;
  DeadBlock->write = DeadBlock->base;
  DeadBlock->read = DeadBlock->base;
  *it = DeadBlock;
  Free_Memory += DeadBlock->limit - DeadBlock->base + 1;

  MMU::Coalesce();

  SemaphorePtr->up();
  return 1;
}

/* int Read(int Handle) {...}
 *
 * Finds a block by its handle, then reads a char from memory at the read
 * pointer.
 *
 * 1. Check that handle is in range.
 * 2. Find a block with the matching handle.
 * 3. Error checking:
 *    3a. Check that we found the proper block by handle.
 *    3b. Check that read is less than the limit.
 *    3c. Check that this thread owns this block.
 * 4. Increment read pointer, read from memory, and return.
 */
int MMU::Read(int Handle) {
  SemaphorePtr->down();
  if (Handle < 0 || Handle > 16) {
    SemaphorePtr->up();
    return -1;
  }

  Block *TheBlock = nullptr;
  for (Block *block : Blocks) {
    TheBlock = block;

    if (TheBlock->handle == Handle) {
      break;
    }
  }

  if (TheBlock->handle != Handle) {
    SemaphorePtr->up();
    return -1;
  } else if (TheBlock->read > TheBlock->limit) {
    SemaphorePtr->up();
    return -1;
  } else if (TheBlock->owner != SchedulerPtr->get_task_id()) {
    SemaphorePtr->up();
    return -1;
    // TODO: Seg fault here
  }

  SemaphorePtr->up();
  return Memory[TheBlock->read++];
}

/* int Write(int Handle, char ch) {...}
 *
 * Finds a block by its handle, then writes a char into memory at the write
 * pointer.
 *
 * 1. Check that handle is in range.
 * 2. Find a block with the matching handle.
 * 3. Error checking:
 *    3a. Check that we found the proper block by handle.
 *    3b. Check that write is less than limit.
 *    3c. Check that this thread owns this block.
 * 4. Write the char to the write pointer, and increment write pointer.
 * 5. Return the char written as an int.
 */
int MMU::Write(int Handle, char ch) {
  SemaphorePtr->down();
  if (Handle < 0 || Handle > 16) {
    SemaphorePtr->up();
    return -1;
  }

  Block *TheBlock = nullptr;
  for (Block *block : Blocks) {
    TheBlock = block;

    if (TheBlock->handle == Handle) {
      break;
    }
  }

  if (TheBlock->handle != Handle) {
    SemaphorePtr->up();
    return -1;
  } else if (TheBlock->write > TheBlock->limit) {
    SemaphorePtr->up();
    return -1;
  } else if (TheBlock->owner != SchedulerPtr->get_task_id()) {
    SemaphorePtr->up();
    return -1;
    // TODO: Seg fault here
  }

  Memory[TheBlock->write++] = ch;
  SemaphorePtr->up();
  return ch;
}

/* string Read(int Handle, int offset, int size) {...}
 *
 * Finds and returns a string of size 'size' from a block by its handle, at an
 * offset from the current read pointer.
 *
 * 1. Check that handle is in range.
 * 2. Find a block with the matching handle.
 * 3. Error checking:
 *    3a. Check that we found the proper block by handle.
 *    3b. Check that this thread owns this block.
 *    3c. Check that the string is completely in-bounds of the limit.
 * 4. Build the return string from the chars read from Memory.
 * 5. Return the string.
 */
string MMU::Read(int Handle, int offset, int size) {
  SemaphorePtr->down();
  if (Handle < 0 || Handle > 16) {
    SemaphorePtr->up();
    return "";
  }

  Block *TheBlock = nullptr;
  for (Block *block : Blocks) {
    TheBlock = block;

    if (TheBlock->handle == Handle) {
      break;
    }
  }

  if (TheBlock->handle != Handle) {
    SemaphorePtr->up();
    return "";
  } else if (TheBlock->owner != SchedulerPtr->get_task_id()) {
    SemaphorePtr->up();
    return "";
    // TODO: Seg fault here
  } else if (TheBlock->base + offset + size > TheBlock->limit + 1) {
    SemaphorePtr->up();
    return "";
  }

  string return_string;
  for (int i = TheBlock->base + offset; i < TheBlock->base + offset + size;
       i++) {
    return_string += Memory[i];
  }

  SemaphorePtr->up();
  return return_string;
}

/* int Write(int Handle, int offset, char *text)
 *
 * Writes text to an offset within a specific block, identified by its handle.
 *
 * 1. Check that handle is in range.
 * 2. Find a block with the matching handle.
 * 3. Error checking:
 *    3a. Check that we found the proper block by handle.
 *    3b. Check that this thread owns this block.
 *    3c. Check that the string is completely in-bounds of the limit.
 * 4. Write text to memory at the offset.
 * 5. Return 1.
 */
int MMU::Write(int Handle, int offset, char *text) {
  SemaphorePtr->down();
  if (Handle < 0 || Handle > 16) {
    SemaphorePtr->up();
    return -1;
  }

  Block *TheBlock = nullptr;
  for (Block *block : Blocks) {
    TheBlock = block;

    if (TheBlock->handle == Handle) {
      break;
    }
  }

  if (TheBlock->handle != Handle) {
    SemaphorePtr->up();
    return -1;
  } else if (TheBlock->owner != SchedulerPtr->get_task_id()) {
    SemaphorePtr->up();
    return -1;
    // TODO: Seg fault here
  } else if (TheBlock->base + offset + strlen(text) > TheBlock->limit + 1) {
    SemaphorePtr->up();
    return -1;
  }

  for (int i = 0; i < strlen(text); i++) {
    Memory[TheBlock->base + offset + i] = text[i];
  }

  SemaphorePtr->up();
  return 1;
}

/* string Dump_A_Block(int Handle) {...}
 *
 * Dumps a specific block identified by its handle.
 *
 * 1. Check that handle is in range.
 * 2. Find a block with the matching handle.
 * 3. Check that we found the proper block by handle.
 * 4. Make a string stream, and define line length 'limit'.
 * 5. Dump all contents into string stream.
 * 6. Return string stream as string.
 */
string MMU::Dump_A_Block(int Handle) {
  SemaphorePtr->down();
  if (Handle < 0 || Handle > 16) {
    SemaphorePtr->up();
    return "";
  }

  Block *TheBlock = nullptr;
  for (Block *block : Blocks) {
    TheBlock = block;

    if (TheBlock->handle == Handle) {
      break;
    }
  }

  if (TheBlock->handle != Handle) {
    SemaphorePtr->up();
    return "";
  }

  stringstream ss;
  int limit = 10;

  ss << " ---------- Block " << Handle << " ---------- " << endl;
  ss << " Owner: " << TheBlock->owner << endl;
  ss << " Base: " << TheBlock->base << endl;
  ss << " Limit: " << TheBlock->limit << endl;
  ss << " Read: " << TheBlock->read << endl;
  ss << " Write: " << TheBlock->write << "\n" << endl;

  for (int i = TheBlock->base; i <= TheBlock->limit; i++) {
    ss << Memory[i];
    if ((i + 1) % limit == 0) {
      ss << endl;
    }
  }

  SemaphorePtr->up();
  return ss.str();
}

/* string Dump_Blocks() {...}
 *
 * Dumps all blocks, in the order they appear in Blocks.
 *
 * 1. Dump the memory semaphore.
 * 2. For each block, dump all contents.
 * 3. Return as string.
 */
string MMU::Dump_Blocks() {
  SemaphorePtr->down();
  stringstream ss;
  int limit = 90;

  ss << " ---------- Memory Semaphore ---------- " << endl;
  ss << SemaphorePtr->dump() << endl;
  Block *TheBlock = nullptr;
  for (Block *block : Blocks) {
    TheBlock = block;

    if (TheBlock->handle == -1) {
      ss << " Free space: " << Free_Memory << "\n" << endl;
    } else {

      ss << " ---------- Block " << TheBlock->handle << " ---------- " << endl;
      ss << " Owner: " << TheBlock->owner << endl;
      ss << " Base: " << TheBlock->base << endl;
      ss << " Limit: " << TheBlock->limit << endl;
      ss << " Read: " << TheBlock->read << endl;
      ss << " Write: " << TheBlock->write << endl;

      int size = TheBlock->limit - TheBlock->base + 1;
      int found = 0;
      for (int i = TheBlock->base; i <= TheBlock->limit; i++) {
        ss << Memory[i];
        if ((i - TheBlock->base + 1) % limit == 0) {
          ss << endl;
        }
        found++;
      }
      ss << "\n Computed Size (Limit - Base + 1): " << size << endl;
      ss << " Found size (counted while printing): " << found << "\n" << endl;
    }
  }

  SemaphorePtr->up();
  return ss.str();
}
