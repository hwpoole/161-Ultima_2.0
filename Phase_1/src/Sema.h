/* Semaphore Class Header File
 * Ultima 2.0
 *
 * This Semaphore header file shows the private and public resources of the
 * Semaphore class.
 *
 * Of note to the public, who wish to use this class:
 *
 *  1. Semaphore(cost char *Name);
 *      - A constructor. Takes the name of the resource the Semaphore manages.
 *  2. ~Semaphore();
 *      - A destructor.
 *  3. set_scheduler(Scheduler *s);
 *      - Registers the Scheduler with the Semaphore class.
 *  4. void down();
 *      - Call this to attempt to lock and access the guarded resource.
 *      - Threads that can't get access to the resource will wait in a FIFO
 *        queue until they are called on.
 *      - This is a blocking wait, but is not a busy wait.
 *  5. void up();
 *      - Call this to unlock the guarded resource for use by other threads.
 *  6. void dump();
 *      - Call this to see the current contents and/or state of the Semaphore.
 *
 *
 * Of note on the private resources:
 *
 * 1. string resource_name;
 *    - Stores the name of the resource the Semaphore manages.
 *    - This is set *once* via the constructor.
 * 2. int sema_value = 1;
 *    - Any valid binary Semaphore should start its life as "available."
 *      As in, you may not simultaneously create and lock a Semaphore.
 *      Thus, it is "up" and available for use from the get-go.
 * 3. queue<int> sema_queue;
 *    - Stores the task_ids for the tasks waiting for access on the
 *      guarded resource.
 *    - Is a FIFO queue. Tasks may not proceed unless they are in the front.
 *
 * Hunter Poole
 * 03-28-2026
 */

#pragma once

#include "Sched.h"
#include <queue>
#include <string>

using namespace std;

class Semaphore {
private:
  string resource_name;
  int sema_value = 1;
  queue<int> sema_queue;
  static Scheduler *scheduler;

public:
  Semaphore(const char *Name);

  ~Semaphore();

  static void set_scheduler(Scheduler *s) { scheduler = s; }

  void down();

  void up();

  void dump();
};
