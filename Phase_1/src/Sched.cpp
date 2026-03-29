/* Scheduler Class
 * Ultima 2.0
 *
 * This Semaphore class file houses the implementation details of Sched.h.
 * It is recommended you view Sched.h for information on how to use the
 * Scheduler class.
 *
 * Hunter Poole
 * 03-28-2026
 */

#include "Sched.h"
#include "CircularLinkedList.h"
#include <ctime>

/* Scheduler() {...}
 *
 * Constructor.
 *
 * 1. Set current task = -1.
 *    - Invalid value on purpose - no tasks yet.
 *    - next_available_task_id = 0;
 *    - quantum is 300 by default.
 */
Scheduler::Scheduler() {
  current_task = -1;
  next_available_task_id = 0;
  current_quantum = 300;
}

/* ~Scheduler() {...}
 *
 * Destructor.
 *
 * While TCBList is not empty:
 *    - Get the TCB pointer at the front of the list.
 *    - Delete it.
 *    - Call TCB.remove_front();
 */
Scheduler::~Scheduler() {
  while (!TCBList.is_empty()) {
    TCB *DeadTask = TCBList.get_front();
    delete DeadTask;
    TCBList.remove_front();
  }
}

/* int create_task() {...}
 *
 * Creates a new task, and adds it to the end of the queue.
 *
 * 1. Create a NewTask TCB.
 *    - Assign it's task ID from next_available.
 *    - Set state READY.
 * 2. Insert that at end of queue.
 * 3. Increment next_available_task_id.
 * 4. Return created task's id.
 */
int Scheduler::create_task() {
  TCB *NewTask = new TCB();
  NewTask->task_id = next_available_task_id;
  NewTask->state = READY;
  TCBList.insert_end(NewTask);

  next_available_task_id++;
  return (NewTask->task_id);
}

/* void kill_task() {...}
 *
 * Kills the current task.
 *
 * 1. Checks if TCBList is empty.
 *    - If so, do nothing (return).
 * 2. Get the current front's TCB.
 * 3. Set it's state to DEAD.
 * 4. Call garbage_collect.
 */
void Scheduler::kill_task() {
  if (TCBList.is_empty()) {
    return;
  }

  TCB *current = TCBList.get_front();
  current->state = DEAD;
  TCBList.set_value(current);

  garbage_collect();
}

/* void yield() {...}
 *
 * Allows the currently running task to voluntarily give up its CPU time to
 * another task. However, the scheduler only switches tasks if the calling task
 * is BLOCKED or if it has used its time.
 *
 * 1. Checks if the TCBList is empty.
 *    - If so, do nothing (return).
 * 2. Get the elapsed_time from the current task.
 * 3. Check if current task is blocked or has used all quantum.
 *    3a. If so, check if state is RUNNING.
 *        3a-1. If so, set to READY.
 *    3b. If so, advance TCBList and get the new front.
 *    3c. Then, move to the next READY task.
 *    3d. Then, mark that task as running.
 * 4. Update the current process_table and current_task.
 */
void Scheduler::yield() {
  if (TCBList.is_empty()) {
    return;
  }

  TCB *current = TCBList.get_front();
  clock_t elapsed_time = clock() - current->start_time;

  if (current->state == BLOCKED || elapsed_time >= current_quantum) {
    if (current->state == RUNNING) {
      current->state = READY;
      TCBList.set_value(current);
    }

    TCBList.advance();
    current = TCBList.get_front();

    int counter = 0;
    while (current->state != READY && counter < TCBList.size() - 1) {
      TCBList.advance();
      current = TCBList.get_front();
      counter++;
    }

    if (current->state == READY && counter < TCBList.size() - 1) {
      current->state = RUNNING;
      current->start_time = clock();
      TCBList.set_value(current);
    } // Else... deadlock?
  }

  process_table = TCBList.get_front();
  current_task = process_table->task_id;
}

/* void set_quantum(long quantum) {...}
 *
 * A setter method for the current_quantum variable.
 */
void Scheduler::set_quantum(long quantum) { current_quantum = quantum; }

/* long get_quantum() {...}
 *
 * A getter method for the current_quantum variable.
 */
long Scheduler::get_quantum() { return (current_quantum); }

/* void set_state(int task_ID, string STATE) {...}
 *
 * A setter method to set a specific task's state by its task_ID.
 *
 * 1. Saves the current position on the TCBList by key.
 * 2. Moves to the node that has a matching task_ID.
 * 3. Sets that node's TCB state accordingly.
 * 4. Reverts to the starting position, if different than current position.
 */
void Scheduler::set_state(int task_ID, string STATE) {
  TCB *current = TCBList.get_front();
  TCB *temp = TCBList.get_front();

  int counter = 0;
  while (temp->task_id != task_ID && counter < TCBList.size()) {
    TCBList.advance();
    temp = TCBList.get_front();
    counter++;
  }

  if (temp->task_id == task_ID) {
    temp->state = STATE;
    TCBList.set_value(temp);
  }

  if (temp != current) {
    TCBList.move_to_key(current);
  }
}

/* int get_task_id() {...}
 *
 * A getter method for the current task_id from the front of the TCBList.
 */
int Scheduler::get_task_id() {
  TCB *current = TCBList.get_front();
  return (current->task_id);
}

/* void start() {...}
 *
 * Starts the scheduler and sets the first task to RUNNING.
 *
 * 1. Checks if the TCBList is not empty.
 * 2. Start the clock on it.
 * 3. Set it's state to RUNNING.
 * 4. If the TCBList is empty, do nothing.
 */
void Scheduler::start() {
  if (!TCBList.is_empty()) {
    TCB *current = TCBList.get_front();
    current->start_time = clock();
    current->state = RUNNING;
    current_task = current->task_id;
  }
}

/* void garbage_collect() {...}
 *
 * Collects and delets all TCBs marked with state DEAD, and all nodes in TCBList
 * associated with DEAD TCBs.
 *
 * 1. Get TCBList size and save in ListSize.
 * 2. Iterate over the list and collect the current TCB.
 * 3. If any TCB has state DEAD...
 *    3a. Call remove_front.
 *    3b. Delete that TCB.
 * 4. Else, advance.
 * 5. If TCBList is not empty...
 *    5a. Update the current process_table.
 *    5b. Update the current_task.
 * 6. Else...
 *    6a. Give process_table a nullptr.
 *    6b. Set current_task to -1, as in constructor.
 */
void Scheduler::garbage_collect() {
  int ListSize = TCBList.size();
  for (int i = 0; i < ListSize; i++) {
    TCB *current = TCBList.get_front();
    if (current->state == DEAD) {
      TCBList.remove_front();
      delete current;
    } else {
      TCBList.advance();
    }
  }

  if (!TCBList.is_empty()) {
    process_table = TCBList.get_front();
    current_task = process_table->task_id;
  } else {
    process_table = nullptr;
    current_task = -1;
  }
}

/* void dump() {...}
 *
 * "Pretty prints" a table containing the current state of the process table and
 * TCBList.
 *
 * 1. Print top of table and column labels.
 * 2. Iterate over TCBList.
 * 3. For each item in TCBList:
 *    - Get the TCB.
 *    - Get the elapsed_time.
 *    - Print the task_id, elapsed_time, and state.
 * 4. Advance again to return to starting position.
 */
void Scheduler::dump() {
  cout << "---------- PROCESS TABLE ----------" << endl;
  cout << "Quantum = " << current_quantum << endl;
  cout << "Task-ID\t Elapsed Time\tState" << endl;

  for (int i = 0; i < TCBList.size() - 1; i++) {
    TCB *current = TCBList.get_front();
    clock_t elapsed_time = clock() - current->start_time;
    printf("%6d\t%8d\t%s", current->task_id, elapsed_time,
           current->state.c_str());
    TCBList.advance();
  }
  TCBList.advance();
}
