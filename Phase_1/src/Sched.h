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

#include <queue>
#include <string>

using namespace std;

const string READY = "READY";
const string RUNNING = "RUNNING";
const string BLOCKED = "BLOCKED";
const string DEAD = "DEAD";

struct TCB {
  int task_id;
  string state;
  clock_t start_time;
  TCB *next;
};

class Scheduler {
private:
  TCB *process_table;
  int current_task;
  long current_quantum;
  int next_available_task_id;

public:
  Scheduler();

  ~Scheduler();

  int create_task();

  void kill_task();

  void yield();

  void garbage_collect();

  void dump();
};
