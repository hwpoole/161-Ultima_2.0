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

Scheduler::Scheduler() {
  current_task = -1;
  next_available_task_id = 0;
  current_quantum = 300;
}

Scheduler::~Scheduler() {
  while (!TCBList.is_empty()) {
    TCB *DeadTask = TCBList.get_front();
    delete DeadTask;
    TCBList.remove_front();
  }
}

int Scheduler::create_task() {
  TCB *NewTask = new TCB();
  NewTask->task_id = next_available_task_id;
  NewTask->state = READY;
  TCBList.insert_end(NewTask);

  next_available_task_id++;
  return (next_available_task_id - 1);
}

void Scheduler::kill_task() {
  if (TCBList.is_empty()) {
    return;
  }

  TCB *current = TCBList.get_front();

  TCBList.move_to_key(process_table);
  TCB *TaskToKill = TCBList.get_front();
  TaskToKill->state = DEAD;
  TCBList.set_value(TaskToKill);

  TCBList.move_to_key(current);
  garbage_collect();
}

void Scheduler::yield() {
  if (TCBList.is_empty()) {
    return;
  }

  TCB *current = TCBList.get_front();
  int counter = 0;
  clock_t elapsed_time = clock() - current->start_time;

  if (elapsed_time >= current_quantum) {
    if (current->state == RUNNING) {
      current->state = READY;
      TCBList.set_value(current);
    }

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
  } // Else... we don't yield. Quantum not used.

  TCBList.advance();
  process_table = TCBList.get_front();
  current_task = process_table->task_id;
}

void Scheduler::set_quantum(long quantum) { current_quantum = quantum; }

long Scheduler::get_quantum() { return (current_quantum); }

void Scheduler::set_state(int task_ID, string STATE) {
  TCB *current = TCBList.get_front();
  TCB *temp = TCBList.get_front();

  while (temp->task_id != task_ID) {
    TCBList.advance();
    temp = TCBList.get_front();
  }

  temp->state = STATE;
  TCBList.set_value(temp);

  if (temp != current) {
    TCBList.move_to_key(current);
  }
}

void Scheduler::start() {
  TCB *current = TCBList.get_front();
  current->start_time = clock();
  current->state = RUNNING;
  current_task = current->task_id;
}

void Scheduler::garbage_collect() {
  for (int i = 0; i < TCBList.size() - 1; i++) {
    TCB *current = TCBList.get_front();
    if (current->state == DEAD) {
      delete current;
    }
    TCBList.advance();
  }
  TCBList.advance();
}

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
