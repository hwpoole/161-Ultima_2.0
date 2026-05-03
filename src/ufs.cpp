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
#include <cstddef>
#include <cstring>
#include <ctime>
#include <pthread.h>

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
 * 3. Set start block to Start.
 * 4. Set size to 0.
 * 5. Set permission with bounds checks (highest allowable is 7).
 * 6. Set read and write pointers.
 * 7. Set creation and last modified to the current time.
 */
ufs::INode::INode(char Name[8], int Permission[4], int Start, int Read,
                  int Write, unsigned int Blocks[4]) {
  Name[7] = '\0';
  strcpy(filename, Name);

  owner_TID = pthread_self();
  start_block = Start;
  size = 0;

  for (int i = 0; i < 4; i++) {
    permission[i] = (Permission[i] <= 7) ? Permission[i] : 7;
    blocks[i] = Blocks[i];
  }

  read = Read;
  write = Write;

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
 * 2. Replace all I-Nodes with default I-Nodes.
 */
void ufs::Format() {
  for (int i = 0; i < Disk.size(); i++) {
    Disk[i] = '\0';
  }

  for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
    INode *TheNode = *it;
    delete TheNode;

    *it = new INode();
  }
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

  for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
    TheNode = *it;

    if (TheNode->filename == FileName) {
      if (TheNode->open == true) {
        return -1;
      } else if (TheNode->owner_TID == pthread_self()) {
        TheNode->open = true;
        *it = TheNode;

        return 1;
      } else if (TheNode->permission[3] >= 4 && mode == R) {
        TheNode->open = true;
        *it = TheNode;

        return 1;
      } else if (TheNode->permission[3] != 1 && TheNode->permission[3] != 4 &&
                 TheNode->permission[3] != 5 && mode == W) {
        TheNode->open = true;
        *it = TheNode;

        return 1;
      }
    } else if (distance(it, Nodes.end()) == 2) {
      return -1;
    }
  }

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

  for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
    TheNode = *it;

    if (TheNode->filename == FileName && TheNode->open == true) {
      if (TheNode->owner_TID == pthread_self()) {
        TheNode->open = false;
        *it = TheNode;

        return 1;
      }
    } else if (distance(it, Nodes.end()) == 2) {
      return -1;
    }
  }

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
 *        a. If so, read and update the read pointer.
 *        b. Return the read char.
 * 3. Else,
 *    3a. Return -1.
 */
int ufs::Read_Char(char FileName[8]) {
  INode *TheNode = nullptr;

  for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
    TheNode = *it;

    if (TheNode->filename == FileName && TheNode->open == true) {
      if (TheNode->owner_TID == pthread_self() || TheNode->permission[3] >= 4) {
        char Read = Disk[TheNode->read++];
        *it = TheNode;
        return Read;
      }
    } else if (distance(it, Nodes.end()) == 2) {
      return -1;
    }
  }

  return -1;
}

/* Write_Char(FileName[8]) {...}
 *
 * Writes a char to the specified file's write pointer, if the file exists.
 *
 *
 */
int ufs::Write_Char(char FileName[8], char Char) {
  INode *TheNode = nullptr;

  for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
    TheNode = *it;

    if (TheNode->filename == FileName && TheNode->open == true) {
      if (TheNode->owner_TID == pthread_self() ||
          (TheNode->permission[3] != 1 && TheNode->permission[3] != 4 &&
           TheNode->permission[3] != 5)) {
        Disk[TheNode->write++] = Char;
        *it = TheNode;
        return Char;
      }
    } else if (distance(it, Nodes.end()) == 2) {
      return -1;
    }
  }

  return -1;
}

/*
 */
int ufs::Create_File(char FileName[8], int Size, int Permission[4]) {}

/*
 */
int ufs::Delete_File(char FileName[8]) {}

/*
 */
int ufs::Change_Permissions(char FileName[8], int Permission[4]) {}

/*
 */
void ufs::Dir() {}

/*
 */
void ufs::Task_Dir() {}

/*
 */
string ufs::Dump() {}
