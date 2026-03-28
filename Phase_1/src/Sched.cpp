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
#include <queue>

Scheduler::Scheduler() {}

Scheduler::~Scheduler() {}

int Scheduler::create_task() { return 0; }

void Scheduler::kill_task() {}

void Scheduler::yield() {}

void Scheduler::garbage_collect() {}

void Scheduler::dump() {}
