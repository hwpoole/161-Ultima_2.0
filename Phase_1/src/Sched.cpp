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

void Scheduler::kill_task() {}

void Scheduler::yield() {
  if (TCBList.is_empty()) {
    return;
  }

  TCBList.advance();
  process_table = TCBList.get_front();
  current_task = process_table->task_id;
}

void Scheduler::garbage_collect() {}

void Scheduler::dump() {}
