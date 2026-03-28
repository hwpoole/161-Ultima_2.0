/* Scheduler Class Header File
 * Ultima 2.0
 *
 * This Scheduler class header file shows the private and public resources of
 * the Scheduler class.
 *
 * Of note to the public, who wish to use this class:
 *  1.
 *
 * Of note on the private resources:
 *  1.
 *
 * Hunter Poole
 * 03-28-2026
 */

#pragma once

using namespace std;

class Scheduler {
private:
  TCB process_table;

public:
  void create_task();

  void kill_task();

  void yield();

  void garbage_collect();

  void dump();
};
