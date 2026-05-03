/* ufs header file.
 * Ultima 2.0
 *
 * This ufs class header file shows the private and public resources of the ufs
 * class
 *
 * Hunter Poole
 * 05-02-2026
 */

#pragma once

#include <string>

using namespace std;

class ufs {
private:
  struct INode {
    char filename[8];
    int owner_TID, start_block, size, permission[4];
    unsigned int blocks[4];
    time_t creation, last_modified;

    INode();

    INode(char Name[8], int Permission[4], int Start, unsigned int Blocks[4]);
  };

  static inline ufs *UFS_Ptr = nullptr;
  string fs_name;
  char init_char;
  int block_size, blocks_count, next_handle;

  enum Mode { R, W };

  ufs(string Name, int Block_Num, int Block_Size, char Init_Char);

public:
  static ufs *Get_Instance();

  static ufs *Get_Instance(string Name, int Number_Of_Blocks, int Block_Size,
                           char Initialization_Char);

  ufs(const ufs &) = delete;

  ufs &operator=(const ufs &) = delete;

  void Format();

  int Open(char FileName[8], Mode mode);

  int Close(char FileName[8]);

  int Read_Char(char FileName[8]);

  int Write_Char(char FileName[8]);

  int Create_File(char FileName[8], int Size, int Permission[4]);

  int Delete_File(char FileName[8]);

  int Change_Permissions(char FileName[8], int Permission[4]);

  void Dir();

  void Task_Dir();

  string Dump();
};
