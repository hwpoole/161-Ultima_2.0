/* ufs implementation file
 * Ultima 2.0
 *
 * This is the implementation file for the ufs class. It is recommended to view
 * the ufs.h header file for information on how to use this class.
 *
 * Hunter Poole
 * 05-02-2026
 */

#include "ufs.h"
#include "Kernel.h"
#include <algorithm>
#include <cstring>
#include <ctime>
#include <pthread.h>
#include <sstream>

/* INode() {...}
 *
 * A no-arg constructor for the INode struct.
 * Creates an I-Node with default values.
 * We purposefully do not give the I-Node a start block or blocks[] to prevent
 * accidental overwrites or "double counting" of a space.
 *
 * 1. Set the name as the null byte.
 * 2. Set size to 0.
 * 3. Set permission to 0666.
 * 4. Set creation and last modified to the current time.
 */
ufs::INode::INode() {
  strcpy(filename, "\0");

  size = 0;
  permission[0] = 0;
  permission[1] = 6;
  permission[2] = 6;
  permission[3] = 6;

  creation = clock();
  last_modified = creation;
}

/* INode(Name[8], Permission[4], Start, Blocks[4]) {...}
 *
 * Overloaded consructor for the INode struct.
 *
 * 1. Force last position in Name[8] to be the null byte.
 *    1a. Then, copy to filename.
 * 2. Set the current pthread as the owner.
 * 3. Set size, read, write to 0.
 * 4. Set open to false.
 * 5. Set permission with bounds checks (highest allowable is 7).
 * 6. Set creation and last modified to the current time.
 */
ufs::INode::INode(char Name[8], int Permission[4], unsigned int Blocks[4]) {
  Name[7] = '\0';
  strcpy(filename, Name);

  owner_TID = Scheduler_Ptr->get_task_id();
  size = 0;
  read = 0;
  write = 0;
  open = false;

  for (int i = 0; i < 4; i++) {
    permission[i] = (Permission[i] <= 7) ? Permission[i] : 7;
    blocks[i] = Blocks[i];
  }

  creation = clock();
  last_modified = creation;
}

/* ufs(Name, Block_Num, Block_Size, Init_Char) {...}
 *
 * A constructor for the Ultima File System class.
 *
 * 1. Set name to Name.
 * 2. Ensure given block count and size are positive and <= disk size.
 *    2a. If they are, use them.
 *    2b. If not, use defaults.
 * 3. Set init_char to Init_Char.
 */
ufs::ufs(string Name, int Block_Num, int Block_Size, char Init_Char) {
  fs_name = Name;

  if (Block_Num * Block_Size > 0 && Block_Num * Block_Size <= 2048) {
    blocks_count = Block_Num;
    block_size = Block_Size;
  } else {
    blocks_count = 16;
    block_size = 128;
  }

  init_char = Init_Char;
  Disk.assign((blocks_count * block_size), init_char);

  Scheduler_Ptr = Kernel::Get_Instance()->Get_Scheduler();
  Sempahore_Ptr = Kernel::Get_Instance()->Create_Semaphore("UFS Semaphore");
}

/* Get_Instance() {...}
 *
 * Gets the current instance of UFS or creates one.
 * Returns a ufs pointer.
 *
 * 1. Check if UFS_Ptr is null.
 *    1a. If so, call ufs() with defaults.
 * 2. Return UFS_Ptr.
 */
ufs *ufs::Get_Instance() {
  if (UFS_Ptr == nullptr) {
    UFS_Ptr = new ufs("Ultima File System", 16, 128, 'A');
  }

  return UFS_Ptr;
}

/* Get_Instance(Name, Number_Of_Blocks, Block_Size, Initialization_Char) {...}
 *
 * Gets the current instance of UFS or creates one with the specified values, if
 * it can.
 * Returns a ufs pointer.
 *
 * 1. Check if UFS_Ptr is null.
 *    1a. If so, call ufs() with specified values.
 * 2. Return UFS_Ptr.
 */
ufs *ufs::Get_Instance(string Name, int Number_Of_Blocks, int Block_Size,
                       char Initialization_Char) {
  if (UFS_Ptr == nullptr) {
    UFS_Ptr = new ufs(Name, Number_Of_Blocks, Block_Size, Initialization_Char);
  }

  return UFS_Ptr;
}

/* Format() {...}
 *
 * Wipes the disk and resets all I-Nodes in the process.
 *
 * 1. Set the disk to be entirely null bytes.
 * 2. Delete all I-Nodes.
 * 3. Clears Nodes.
 */
void ufs::Format() {
  Sempahore_Ptr->down();
  for (int i = 0; i < Disk.size(); i++) {
    Disk[i] = '\0';
  }

  for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
    INode *TheNode = *it;
    delete TheNode;
  }

  Nodes.clear();
  Sempahore_Ptr->up();
}

/* Open(FileName[8], mode) {...}
 *
 * Opens a file on a few conditions:
 *
 * The file must exist.
 * The file must be accessible by the calling task.
 *
 * Returns -1 on error.
 *
 * 1. Find the node by name, if it exists.
 *    1a. Return -1 if we cannot find it.
 * 2. If we find a match,
 *    2a. If the permissions allow,
 *        1. Set the node to be open.
 *        2. Return 1
 * 3. Else,
 *    3a. Return -1
 */
int ufs::Open(char FileName[8], Mode mode) {
  INode *TheNode = nullptr;

  Sempahore_Ptr->down();
  for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
    TheNode = *it;

    if (strcmp(TheNode->filename, FileName) == 0) {
      if (TheNode->open == true) {
        Sempahore_Ptr->up();
        return -1;
      } else if (TheNode->owner_TID == pthread_self() ||
                 (TheNode->permission[3] >= 4 && mode == R) ||
                 (TheNode->permission[3] != 1 && TheNode->permission[3] != 4 &&
                  TheNode->permission[3] != 5 && mode == W)) {
        TheNode->open = true;
        *it = TheNode;

        Sempahore_Ptr->up();
        return 1;
      } else if (distance(it, Nodes.end()) == 2) {
        break;
      }
    }
  }

  Sempahore_Ptr->up();
  return -1;
}

/* Close(FileName[8]) {...}
 *
 * Closes the file, if it exists and permissions allow.
 * Returns -1 if it could not find the file.
 *
 * 1. Find the node by name, if it exists.
 *    1a. Return -1 if we cannot find it.
 * 2. If we find a match and it is open,
 *    2a. Check that the owner called for close.
 *        a. If so, close it.
 *        b. Reutn 1.
 * 3. Else,
 *    3a. Return -1.
 */
int ufs::Close(char FileName[8]) {
  INode *TheNode = nullptr;

  Sempahore_Ptr->down();
  for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
    TheNode = *it;

    if (strcmp(TheNode->filename, FileName) == 0 && TheNode->open == true) {
      if (TheNode->owner_TID == pthread_self()) {
        TheNode->open = false;
        *it = TheNode;

        Sempahore_Ptr->up();
        return 1;
      }
    } else if (distance(it, Nodes.end()) == 2) {
      break;
    }
  }

  Sempahore_Ptr->up();
  return -1;
}

/* Read_Char(FileName[8]) {...}
 *
 * Reads a char from the specified file's read pointer, if the file exists.
 *
 * 1. Find the node by name, if it exists.
 *    1a. Return -1 if we cannot find it.
 * 2. If we find a match and it is open,
 *    2a. Check for permission to read...
 *        a. If so, calculate the read pointer's disk position.
 *        b. Read a char from disk.
 *        c. Update the read pointer.
 *        d. Return the read char.
 * 3. Else,
 *    3a. Return -1.
 */
int ufs::Read_Char(char FileName[8]) {
  INode *TheNode = nullptr;

  Sempahore_Ptr->down();
  for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
    TheNode = *it;

    if (strcmp(TheNode->filename, FileName) == 0 && TheNode->open == true) {
      if (TheNode->owner_TID == pthread_self() || TheNode->permission[3] >= 4) {
        int Block_Index = TheNode->read / block_size;
        int Offset = TheNode->read % block_size;
        int Disk_Pos = (TheNode->blocks[Block_Index] * block_size) + Offset;

        char Read = Disk[Disk_Pos];
        TheNode->read++;
        *it = TheNode;
        Sempahore_Ptr->up();
        return Read;
      }
    } else if (distance(it, Nodes.end()) == 2) {
      break;
    }
  }

  Sempahore_Ptr->up();
  return -1;
}

/* Write_Char(FileName[8]) {...}
 *
 * Writes a char to the specified file's write pointer, if the file exists.
 *
 * 1. Find the node by name, if it exists.
 *    1a. Return -1 if we cannot find it.
 * 2. If we find a match an it is open,
 *    2a. Check for permission to write...
 *        a. If so, calculate the write pointer's disk position.
 *        b. Write a char to disk.
 *        c. Increment size and write pointers.
 *        d. Return the written char.
 * 3. Else,
 *    3a. Return -1.
 */
int ufs::Write_Char(char FileName[8], char Char) {
  INode *TheNode = nullptr;

  Sempahore_Ptr->down();
  for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
    TheNode = *it;

    if (strcmp(TheNode->filename, FileName) == 0 && TheNode->open == true) {
      if (TheNode->owner_TID == pthread_self() ||
          (TheNode->permission[3] != 1 && TheNode->permission[3] != 4 &&
           TheNode->permission[3] != 5)) {
        int Block_Index = TheNode->write / block_size;
        int Offset = TheNode->write % block_size;
        int Disk_Pos = (TheNode->blocks[Block_Index] * block_size) + Offset;

        if (Disk_Pos <= Disk.size()) {
          Disk[Disk_Pos] = Char;
          TheNode->size++;
          TheNode->write++;
          *it = TheNode;
          Sempahore_Ptr->up();
          return Char;
        } else {
          break;
        }
      }
    } else if (distance(it, Nodes.end()) == 2) {
      break;
    }
  }

  Sempahore_Ptr->up();
  return -1;
}

/* Create_File(FileName[8], Size, Permission[4]) {...}
 *
 * Creates a file with the specified Name, Size, and Permissions, if able.
 *
 * 1. Check that a file by that name doesn't already exist.
 * 2. Check that we have space for a new file.
 *    1a. If none, return -1.
 * 3. Find free blocks in Blocks_Map.
 *    2a. If a block is free, assign it.
 * 4. Create the new I-Node, and push onto Nodes.
 * 5. Return 1.
 */
int ufs::Create_File(char FileName[8], int Size, int Permission[4]) {
  INode *TheNode = nullptr;

  Sempahore_Ptr->down();
  for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
    TheNode = *it;
    if (strcmp(TheNode->filename, FileName) == 0) {
      Sempahore_Ptr->up();
      return -1;
    }
  }

  int Blocks_Needed = (Size + block_size - 1) / block_size;
  if (Blocks_Needed > 4 || Nodes.size() >= 16) {
    Sempahore_Ptr->up();
    return -1;
  }

  int found = 0;
  unsigned int assigned[4];
  for (int i = 0; i < blocks_count && found < Blocks_Needed; i++) {
    if (!Blocks_Map.test(i)) {
      Blocks_Map.set(i);
      assigned[found++] = i;
    }
  }

  INode *DoneNode = new INode(FileName, Permission, assigned);
  Nodes.push_back(DoneNode);

  Sempahore_Ptr->up();
  return 1;
}

/* Delete_File(FileName[8]) {...}
 *
 * Deletes a file with the specified name, if permissions allow.
 *
 * 1. Find the node by name, if it exists.
 *    1a. Return -1 if we cannot find it, or permissions do not allow us to
 *        delete it.
 * 2. Wipe all contents held by that node on the disk.
 * 3. Delete the node.
 * 4. Return 1.
 */
int ufs::Delete_File(char FileName[8]) {
  INode *TheNode = nullptr;
  _List_iterator<ufs::INode *> NodeIterator = Nodes.begin();

  Sempahore_Ptr->down();
  for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
    TheNode = *it;

    if (strcmp(TheNode->filename, FileName) == 0) {
      if (TheNode->owner_TID != pthread_self() &&
          (TheNode->permission[3] == 1 || TheNode->permission[3] == 4 ||
           TheNode->permission[3] == 5)) {
        Sempahore_Ptr->up();
        return -1;
      }
    } else if (distance(it, Nodes.end()) == 2) {
      Sempahore_Ptr->up();
      return -1;
    }

    NodeIterator = find(begin(Nodes), end(Nodes), TheNode);
  }

  for (int i = 0; i < 4; i++) {
    int Disk_Pos = TheNode->blocks[i] * block_size;
    for (int j = Disk_Pos; j < Disk_Pos + block_size; j++) {
      Disk[j] = '$';
    }
  }

  Nodes.erase(NodeIterator);
  delete TheNode;

  Sempahore_Ptr->up();
  return 1;
}

/* Change_Permissions(FileName[8], Permission[4]) {...}
 *
 * Changes a file's permissions by name.
 *
 * 1. Find the node by name, if it exists.
 *    1a. If found, set its permissions as given.
 *        a. Return 1.
 *    1b. Else, return -1.
 * 2. Return -1.
 */
int ufs::Change_Permissions(char FileName[8], int Permission[4]) {
  INode *TheNode = nullptr;

  Sempahore_Ptr->down();
  for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
    TheNode = *it;

    if (strcmp(TheNode->filename, FileName) == 0) {
      for (int i = 0; i < 4; i++) {
        TheNode->permission[i] = Permission[i];
      }

      Sempahore_Ptr->up();
      return 1;
    } else if (distance(it, Nodes.end()) == 2) {
      break;
    }
  }

  Sempahore_Ptr->up();
  return -1;
}

/* Dir() {...}
 *
 * Shows the directory's contents.
 */
string ufs::Dir() {
  stringstream ss;
  INode *TheNode = nullptr;

  ss << " Permissions   Size   Date Modified   Name" << endl;

  Sempahore_Ptr->down();
  for (INode *Node : Nodes) {
    TheNode = Node;
    string Name = TheNode->filename;

    for (int i = 0; i < 4; i++) {
      ss << TheNode->permission[i];
    }

    ss << " " << TheNode->size;
    ss << " " << TheNode->last_modified;
    ss << " " << Name << endl;
  }

  Sempahore_Ptr->up();
  return ss.str();
}

/* Task_Dir() {...}
 *
 * Shows just the contents of the director that this task owns.
 */
string ufs::Task_Dir() {
  stringstream ss;
  INode *TheNode = nullptr;
  Sempahore_Ptr->down();

  for (INode *Node : Nodes) {
  }

  return ss.str();
}

/* Dump() {...}
 *
 * Pretty-prints the contents of UFS for viewing in the Ultima demo.
 */
string ufs::Dump() {
  stringstream ss;

  INode *TheNode = nullptr;

  Sempahore_Ptr->down();
  for (INode *Node : Nodes) {
    TheNode = Node;
    string Name = TheNode->filename;

    ss << " ---------- File: " << Name << " ---------- " << endl;
    ss << " Blocks used: ";

    for (int i = 0; i < 4; i++) {
      if (TheNode->blocks[i] > 0 && TheNode->blocks[i] <= 16) {
        ss << (int)TheNode->blocks[i] << ", ";
      }
    }
    ss << endl;

    ss << " Size: " << TheNode->size << endl;
    ss << " Start block: " << TheNode->blocks[0] << endl;
    ss << " Is open: " << TheNode->open << endl;
    ss << " Permissions: ";

    for (int i = 0; i < 4; i++) {
      if (TheNode->permission[i] >= 0 && TheNode->permission[i] <= 7) {
        ss << (int)TheNode->permission[i] << ", ";
      }
    }
    ss << endl;

    ss << " Owner: " << TheNode->owner_TID << endl;
    ss << " Creation: " << TheNode->creation << endl;
    ss << " Modified: " << TheNode->last_modified << endl;
  }

  Sempahore_Ptr->up();
  return ss.str();
}
