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
  empty = true;
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

/* int Mem_Read(int Handle) {...}
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
int MMU::Mem_Read(int Handle) {
  if (Handle < 0 || Handle > 16) {
    return -1;
  }

  Block *TheBlock;
  for (Block *block : Blocks) {
    TheBlock = block;

    if (TheBlock->handle == Handle) {
      break;
    }
  }

  if (TheBlock->handle != Handle) {
    return -1;
  } else if (TheBlock->read >= TheBlock->limit) {
    return -1;
  } else if (TheBlock->owner != pthread_self()) {
    return -1;
    // TODO: Seg fault here
  }

  return Memory[TheBlock->read++];
}

/* int Mem_Write(int Handle, char ch) {...}
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
int MMU::Mem_Write(int Handle, char ch) {
  if (Handle < 0 || Handle > 16) {
    return -1;
  }

  Block *TheBlock;
  for (Block *block : Blocks) {
    TheBlock = block;

    if (TheBlock->handle == Handle) {
      break;
    }
  }

  if (TheBlock->handle != Handle) {
    return -1;
  } else if (TheBlock->write >= TheBlock->limit) {
    return -1;
  } else if (TheBlock->owner != pthread_self()) {
    return -1;
    // TODO: Seg fault here
  }

  Memory[TheBlock->write++] = ch;
  return ch;
}

/* string Mem_Read(int Handle, int offset, int size) {...}
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
string MMU::Mem_Read(int Handle, int offset, int size) {
  if (Handle < 0 || Handle > 16) {
    return "";
  }

  Block *TheBlock;
  for (Block *block : Blocks) {
    TheBlock = block;

    if (TheBlock->handle == Handle) {
      break;
    }
  }

  if (TheBlock->handle != Handle) {
    return "";
  } else if (TheBlock->owner != pthread_self()) {
    return "";
    // TODO: Seg fault here
  } else if (TheBlock->base + offset + size > TheBlock->limit) {
    return "";
  }

  string return_string;
  for (int i = TheBlock->base + offset; i < TheBlock->base + offset + size;
       i++) {
    return_string.append(&Memory[i]);
  }

  return return_string;
}

/* int Mem_Write(int Handle, int offset, char *text)
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
int MMU::Mem_Write(int Handle, int offset, char *text) {
  if (Handle < 0 || Handle > 16) {
    return -1;
  }

  Block *TheBlock;
  for (Block *block : Blocks) {
    TheBlock = block;

    if (TheBlock->handle == Handle) {
      break;
    }
  }

  if (TheBlock->handle != Handle) {
    return -1;
  } else if (TheBlock->owner != pthread_self()) {
    return -1;
    // TODO: Seg fault here
  } else if (TheBlock->base + offset + strlen(text) > TheBlock->limit) {
    return -1;
  }

  for (int i = 0; i < strlen(text); i++) {
    Memory[TheBlock->base + offset + i] = text[i];
  }

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
  if (Handle < 0 || Handle > 16) {
    return "";
  }

  Block *TheBlock;
  for (Block *block : Blocks) {
    TheBlock = block;

    if (TheBlock->handle == Handle) {
      break;
    }
  }

  if (TheBlock->handle != Handle) {
    return "";
  }

  stringstream ss;
  int limit = 10;

  ss << " ---------- Block " << Handle << " ---------- " << endl;
  ss << " Owner: " << TheBlock->owner << endl;
  ss << " Base: " << TheBlock->base << endl;
  ss << " Limit: " << TheBlock->limit << endl;
  ss << " Read: " << TheBlock->read << endl;
  ss << " Write: " << TheBlock->write << endl;
  ss << " Empty: " << TheBlock->empty << "\n" << endl;

  for (int i = TheBlock->base; i < TheBlock->limit; i++) {
    ss << Memory[i];
    if ((i + 1) % limit == 0) {
      ss << endl;
    }
  }

  return ss.str();
}

/* string Dump_Blocks() {...}
 *
 * Dumps all blocks, in the order they appear in Blocks.
 *
 * 1. For each block, dump all contents into a string stream.
 * 2. Return string stream as string.
 */
string MMU::Dump_Blocks() {
  stringstream ss;
  int limit = 10;

  Block *TheBlock;
  for (Block *block : Blocks) {
    TheBlock = block;

    ss << " ---------- Block " << TheBlock->handle << " ---------- " << endl;
    ss << " Owner: " << TheBlock->owner << endl;
    ss << " Base: " << TheBlock->base << endl;
    ss << " Limit: " << TheBlock->limit << endl;
    ss << " Read: " << TheBlock->read << endl;
    ss << " Write: " << TheBlock->write << endl;
    ss << " Empty: " << TheBlock->empty << "\n" << endl;

    for (int i = TheBlock->base; i < TheBlock->limit; i++) {
      ss << Memory[i];
      if ((i + 1) % limit == 0) {
        ss << endl;
      }
    }
    ss << "\n" << endl;
  }

  return ss.str();
}
