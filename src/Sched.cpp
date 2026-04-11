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
#include "Kernel.h"
#include <ctime>
#include <pthread.h>
#include <signal.h>
#include <sstream>

/* Scheduler() {...}
 *
 * Constructor.
 *
 * 1. Set current task = -1.
 *    - Invalid value on purpose - no tasks yet.
 *    - next_available_task_id = 0;
 *    - Quantum is 300 by default.
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
 *    - Assign its task ID from next_available.
 *    - Set state READY.
 *    - Initialize thread_cond.
 * 2. Insert that TCB at the end of the queue.
 * 3. Increment next_available_task_id.
 * 4. Return created task's task_id.
 */
int Scheduler::create_task() {
  TCB *NewTask = new TCB();
  NewTask->task_id = next_available_task_id;
  NewTask->state = READY;
  NewTask->thread_cond = PTHREAD_COND_INITIALIZER;
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
 * 2. Get the TCB at the front of the queue.
 * 3. Set it's state to DEAD.
 * 4. Cancels the pthread.
 * 5. Call garbage_collect().
 */
void Scheduler::kill_task() {
  if (TCBList.is_empty()) {
    return;
  }

  TCB *current = TCBList.get_front();
  current->state = DEAD;
  pthread_cancel(current->thread_id);
  TCBList.set_value(current);

  garbage_collect();
}

/* void yield() {...}
 *
 * Allows the currently running task to voluntarily give up its CPU time to
 * another task. The scheduler will only switch tasks if the calling task
 * is BLOCKED or if it has exhausted its quantum.
 *
 * 1. Checks if the TCBList is empty.
 *    - If so, do nothing (return).
 * 2. Get the elapsed_time from the current task.
 * 3. Check if the current task is blocked or has exhausted its quantum.
 *    3a. If so, check if its state is RUNNING.
 *        3a-1. If so, set its state to READY.
 *    3b. If so, advance the TCBList and get the new TCB at the front of the
 *        queue.
 *    3c. Then, move to the next READY task.
 *    3d. Then, update that task's state to RUNNING.
 * 4. Update the current process_table and current_task.
 *
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
*/

void Scheduler::yield() {
  bool will_yeild = false;

  if (TCBList.is_empty()) {
    return;
  }

  TCB *current = TCBList.get_front();
  TCB *next = TCBList.get_front();
  clock_t elapsed_time = clock() - current->start_time;

  if (current->state == BLOCKED || elapsed_time >= current_quantum) {
    if (current->state == RUNNING) {
      current->state = READY;
      TCBList.set_value(current);
      will_yeild = true;
    }

    TCBList.advance();
    int counter = 0;
    while (next->state != READY && counter < TCBList.size() - 1) {
      TCBList.advance();
      next = TCBList.get_front();
      counter++;
    }

    if (next->state == READY && counter < TCBList.size() - 1) {
      will_yeild = true;
    }
  }

  if (will_yeild) {
    next->state = RUNNING;
    next->start_time = clock();
    TCBList.set_value(next);
    pthread_cond_signal(&next->thread_cond);

    while (current->state != RUNNING) {
      pthread_cond_wait(&current->thread_cond, &Kernel::CPULocker);
    }
  } else {
    TCBList.move_to_key(current);
  }
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

/* string get_state(int task_ID) {...}
 *
 * A getter method to get a specific task's state by its task_ID.
 *
 * 1. Saves the current position on the TCBList by key.
 * 2. Moves to the node that has a matching task_ID.
 * 3. Copies that TCB's STATE.
 * 4. Returns to starting position.
 * 5. Returns the found STATE.
 */
string Scheduler::get_state(int task_ID) {
  TCB *current = TCBList.get_front();
  TCB *temp = TCBList.get_front();

  string found_state;
  int counter = 0;
  while (temp->task_id != task_ID && counter < TCBList.size()) {
    TCBList.advance();
    temp = TCBList.get_front();
    counter++;
  }

  if (temp->task_id == task_ID) {
    found_state = temp->state;
  }

  if (temp != current) {
    TCBList.move_to_key(current);
  }

  return found_state;
}

/* int get_task_id() {...}
 *
 * A getter method for the current pthread's task_id from the TCBList.
 *
 * 1. Saves the current position in TCBList.
 * 2. Gets the current pthread_t.
 * 3. Finds the node with a matching pthread_t.
 * 4. Collects that node's task_id.
 * 5. Moves back to the starting position.
 * 6. Returns task_id.
 */
int Scheduler::get_task_id() {
  TCB *current = TCBList.get_front();
  TCB *found = current;
  pthread_t self = pthread_self();
  int count = 0;
  int task_id;

  while (self != found->thread_id && count < TCBList.size() - 1) {
    TCBList.advance();
    found = TCBList.get_front();
    count++;
  }

  if (count < TCBList.size() - 1) {
    task_id = found->task_id;
  }

  TCBList.move_to_key(current);
  return (task_id);
}

/* void set_pthread_t(int task_ID, pthread_t thread_ID) {...}
 *
 * A setter method for a given task_ID's pthread_t thread_ID.
 *
 * 1. Declares current TCB from the front of TCBList.
 * 2. Checks if head *does not* have the task ID.
 *    - Iterate through TCBList until it does.
 *    - Update temp TCB with thread_ID.
 *    - Save temp to TCBList.
 *    - Move back to starting point, "current."
 * 3. Else,
 *    - Updates the TCB 'current'
 *    - Then, sets the TCBList value for that.
 */
void Scheduler::set_pthread_t(int task_ID, pthread_t thread_ID) {
  TCB *current = TCBList.get_front();

  if (current->task_id != task_ID) {
    TCB *temp = current;

    int counter = 0;
    while (temp->task_id != task_ID && counter < TCBList.size()) {
      TCBList.advance();
      temp = TCBList.get_front();
      counter++;
    }

    temp->thread_id = thread_ID;
    TCBList.set_value(temp);
    TCBList.move_to_key(current);
  } else {
    current->thread_id = thread_ID;
    TCBList.set_value(current);
  }
}

/* pthread_t get_pthread_t(int task_ID) {...}
 *
 * A getter method for a given task_ID's pthred_t thread_ID.
 *
 * 1. Declares current TCB from the front of TCBList.
 * 2. Checks if head *does not* have the task ID.
 *    - Iterate through TCBList until it does.
 *    - Move back to starting point.
 *    - Return temp's thread_ID.
 * 3. Else,
 *    - Return current's thread_ID.
 */
pthread_t Scheduler::get_pthread_t(int task_ID) {
  TCB *current = TCBList.get_front();

  if (current->task_id != task_ID) {
    TCB *temp = current;
    int counter = 0;
    while (temp->task_id != task_ID && counter < TCBList.size()) {
      TCBList.advance();
      temp = TCBList.get_front();
      counter++;
    }

    TCBList.move_to_key(current);
    return temp->thread_id;
  } else {
    return current->thread_id;
  }
}

pthread_cond_t *Scheduler::get_cond_t(int task_ID) {
  TCB *current = TCBList.get_front();

  if (current->task_id != task_ID) {
    TCB *temp = current;
    int counter = 0;
    while (temp->task_id != task_ID && counter < TCBList.size() - 1) {
      TCBList.advance();
      temp = TCBList.get_front();
      counter++;
    }

    TCBList.move_to_key(current);
    return &temp->thread_cond;
  } else {
    return &current->thread_cond;
  }
}

/* void start() {...}
 *
 * Starts the scheduler and sets the first task's state to RUNNING.
 *
 * 1. Checks if the TCBList is not empty.
 * 2. Get the TCB of the task at the front of the queue.
 * 3. Start the clock on that task.
 * 4. Set that task's state to RUNNING.
 * 5. If the TCBList is empty, do nothing.
 */
void Scheduler::start() {
  if (!TCBList.is_empty()) {
    TCB *current = TCBList.get_front();
    current->start_time = clock();
    current->state = RUNNING;
    current_task = current->task_id;
    pthread_cond_signal(&current->thread_cond);
  }
}

/* void garbage_collect() {...}
 *
 * Collects and deletes all TCBs marked with state DEAD, and all nodes in
 * TCBList associated with DEAD TCBs.
 *
 * 1. Get TCBList size and save in ListSize.
 * 2. Iterate over the list and get each TCB.
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

/* string dump() {...}
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
 * 5. Return the resulting string.
 */
string Scheduler::dump() {
  stringstream ss;
  ss << " ---------- PROCESS TABLE ----------" << endl;
  ss << " Quantum = " << current_quantum << endl;
  ss << " Task-ID\t Elapsed Time\tState" << endl;

  for (int i = 0; i < TCBList.size(); i++) {
    TCB *current = TCBList.get_front();
    clock_t elapsed_time = clock() - current->start_time;
    char buffer[256];
    sprintf(buffer, " %6d\t%8d\t%s\n", current->task_id, (int)elapsed_time,
            current->state.c_str());
    ss << buffer;
    TCBList.advance();
  }
  return ss.str();
}
