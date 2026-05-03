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
 * 2. Get the current pthread as the owner.
 * 3. Set size to 0.
 * 4. Set permission to 0666.
 * 5. Set creation and last modified to the current time.
 */
ufs::INode::INode() {
  strcpy(filename, "\0");

  owner_TID = pthread_self();
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
 * 6. Set blocks owned.
 */
ufs::INode::INode(char Name[8], int Permission[4], int Start,
                  unsigned int Blocks[4]) {
  Name[8] = '\0';
  strcpy(filename, Name);

  owner_TID = pthread_self();
  start_block = Start;
  size = 0;

  for (int i = 0; i < 3; i++) {
    permission[i] = (Permission[i] <= 7) ? Permission[i] : 7;
    blocks[i] = Blocks[i];
  }

  creation = clock();
  last_modified = creation;
}

/*
 */
ufs::ufs(string Name, int Block_Num, int Block_Size, char Init_Char) {}

/*
 */
ufs *ufs::Get_Instance() {}
/*
 */
ufs *ufs::Get_Instance(string Name, int Number_Of_Blocks, int Block_Size,
                       char Initialization_Char) {}
/*
 */
void ufs::Format() {}

/*
 */
int ufs::Open(char FileName[8], Mode mode) {}

/*
 */
int ufs::Close(char FileName[8]) {}

/*
 */
int ufs::Read_Char(char FileName[8]) {}

/*
 */
int ufs::Write_Char(char FileName[8]) {}

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
