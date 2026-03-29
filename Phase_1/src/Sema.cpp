/* Semaphore Class
 * Ultima 2.0
 *
 * This Semaphore class file houses the implementation details of Sema.h.
 * It is recommended you view Sema.h for information on how to use the Semaphore
 * class.
 *
 * Hunter Poole
 * 03-27-2026
 */

#include "Sema.h"
#include <iostream>
#include <queue>

// Allocate memory for static variable Scheduler *scheduler.
Scheduler *Semaphore::scheduler = nullptr;

/* Semaphore::Semaphore(const char *Name) {...}
 *
 * Constructor.
 *
 * Takes const char *Name as the name of the guarded resource.
 * Copies *Name into resource_name.
 */
Semaphore::Semaphore(const char *Name) { resource_name = Name; }

/* Semaphore::~Semaphore() {...}
 *
 * Destructor.
 */
Semaphore::~Semaphore() {}

/* void Semaphore::down() {...}
 *
 * Marks the semaphore unavailable if it is available.
 * Otherwise, the current task is marked as BLOCKED, and joins the queue to wait
 * for access to the guarded resource.
 *
 * 1. Gets the current task's task_id.
 * 2. If the semaphore is available, mark it as unavailable.
 * 3. Else,
 *    - Push current task onto the sema_queue.
 *    - Call the scheduler to set its state to BLOCKED.
 *    - Call the scheduler to yield.
 */
void Semaphore::down() {
  int this_task = scheduler->get_task_id();

  if (sema_value > 0) {
    sema_value--;
  } else {
    sema_queue.push(this_task);
    scheduler->set_state(this_task, BLOCKED);
    scheduler->yield();
  }
}

/* void Semaphore::up() {...}
 *
 * Hands the guarded resource off *strictly the next task in the queue*, or
 * marks it available if there are no tasks in the queue.
 *
 * When we select the next task to use the guarded resource via checking the
 * queue, there is no need to mark the semaphore as available, since we know we
 * have at least one task that has been moved to a READY state, which will run
 * soon, and *has already* requested access to the guarded resource. We may not
 * guarantee this task runs next, but we may guarantee the semaphore will remain
 * unavailable until that task call up(), at which point the process repeats.
 *
 * In this way, we enforce the FIFO nature (or intent) of the sema_queue with
 * regards to resource access.
 *
 * 1. Checks if the sema_queue is empty.
 *    - If so, gets the next task's task_id.
 *    - Pops that task from the queue.
 *    - Sets its state to READY in the scheduler.
 * 2. Else,
 *    - Increment sema_value++
 */
void Semaphore::up() {
  if (!sema_queue.empty()) {
    int next_task = sema_queue.front();
    sema_queue.pop();
    scheduler->set_state(next_task, READY);
  } else {
    sema_value++;
  }
}

/* void Semaphore::dump() {...}
 *
 * dump() is a "pretty print" to display the contents and current state of the
 * Semaphore.
 *
 * 1. Prints to the screen.
 * 2. Copies the current queue into a print_queue.
 * 3. Reads, prints, appends, and pops on the print_queue.
 */
void Semaphore::dump() {
  cout << "Resource: " << resource_name << endl;
  cout << "Sema_value: " << sema_value << endl;
  cout << "Sema_queue: ";

  queue<int> print_queue = sema_queue;

  while (!print_queue.empty()) {
    cout << print_queue.front();
    print_queue.pop();
    cout << " --> ";
  }
  cout << endl;
}
