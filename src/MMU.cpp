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
