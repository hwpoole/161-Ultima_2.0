/* Scheduler Class Header File
 * Ultima 2.0
 *
 * This Scheduler class header file shows the private and public resources of
 * the Scheduler class and its struct, TCB.
 *
 * Of note to the public, who wish to use this class:
 *  1.  Scheduler()
 *      - A no-arg constructor.
 *  2.  ~Scheduler()
 *      - A destructor.
 *  3.  int create_task()
 *      - Creates a new task at the end of the queue.
 *  4.  void kill_task()
 *      - Kills the current task and calls garbage_collect().
 *  5.  void yield()
 *      - Allows the calling task to give up its time, if the scheduler agrees
 *        to do so.
 *      - BLOCKED tasks or tasks that have used their quantum WILL be yielded.
 *      - Other tasks will not be.
 *  6.  void set_quantum()
 *      - A setter for the current_quantum variable.
 *  7.  long get_quantum()
 *      - A getter for the current quantum.
 *  8.  void set_state(int task_ID, string STATE)
 *      - Sets a specific task's STATE by task_ID.
 *  9.  string get_state(int task_ID)
 *      - A getter for the state of a task by task_ID.
 *  10. int get_task_id()
 *      - A getter for the current task's task_id.
 *  11. void start()
 *      - Starts the scheduler and runs the first task, if tasks are present.
 *  12. void garbage_collect()
 *      - Collects and deletes all TCBs whole state is DEAD.
 *  13. void dump()
 *      - A "pretty print" method for the current scheduler state.
 *
 * Of note on the private resources:
 *  1.  TCB *process_table;
 *      - A pointer to a TCB struct.
 *  2.  int current_task;
 *      - The task_id of the currently selected and possibly RUNNING task.
 *  3.  long current_quantum;
 *      - The current quantum limit that tasks must adhere to.
 *  4.  int next_available_task_id;
 *      - The task_id to be assigned to the next new task, if one is created.
 *  5.  CircularLinkedList<TCB *> TCBList
 *      - TCBList holds the tasks in a circuler linked listed for the scheduler.
 *
 * Hunter Poole
 * 03-28-2026
 */

#pragma once

#include "CircularLinkedList.h"
#include <pthread.h>
#include <string>

using namespace std;

// Allowable states for tasks.
const string READY = "READY";
const string RUNNING = "RUNNING";
const string BLOCKED = "BLOCKED";
const string DEAD = "DEAD";

// Task Control Block.
struct TCB {
  int task_id;
  string state;
  clock_t start_time;
  pthread_t thread_id;
};

class Scheduler {
private:
  TCB *process_table;
  int current_task;
  long current_quantum;
  int next_available_task_id;
  CircularLinkedList<TCB *> TCBList;

public:
  Scheduler();

  ~Scheduler();

  int create_task();

  void kill_task();

  void yield();

  void set_quantum(long quantum);

  long get_quantum();

  void set_state(int task_ID, string state);

  string get_state(int task_ID);

  int get_task_id();

  void set_pthread_t(int task_ID, pthread_t thread_ID);

  pthread_t get_pthread_t(int task_ID);

  void start();

  void garbage_collect();

  string dump();
};
