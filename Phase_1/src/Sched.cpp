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

Scheduler::~Scheduler() {}

int Scheduler::create_task() {
  process_table->task_id = next_available_task_id;
  process_table->state = READY;
  TCBList.insert_end(process_table);

  next_available_task_id++;
  return (next_available_task_id - 1);
}

void Scheduler::kill_task() {}

void Scheduler::yield() {}

void Scheduler::garbage_collect() {}

void Scheduler::dump() {}
