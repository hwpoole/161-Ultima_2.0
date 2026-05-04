/* 161-Ultima 2.0 Phase 2 Demo file.
 *
 * This Demo file feeatures five scenarios of consequence, for five features
 * of consequence in the Ultima 2.0 OS:
 *
 * 1. Strict Round Robin Scheduling.
 *
 * Three uthreads are spawned and dispatched to complete some "fake work." In a
 * cooperative system, they will not be interrupted until they make a call to
 * yield().
 *
 * 2. Semaphore and its queue.
 *
 * Three uthreads are spawned and dispatched to complete some "fake work"
 * requiring a Semaphore. Each tasks that can't get the Semaphore will be added
 * to a queue, and will gain access to the guarded resource strictly in the
 * order they were added to the queue.
 *
 * 3. Producer-Consumer problem with a Pipe.
 *
 * Two uthreads are spawned and dispatched - one as a producer and one as a
 * consumer. Each tries to get access to a Pipe, and locks a Semaphore to
 * prevent the other from messing with the Pipe while they are working on it.
 * Random chars are passed between uthreads.
 *
 * 4. Memory Management.
 *
 * Three uthreads are spawned and dispatched to
 *TODO: THIS
 *
 * 5. Ultima File System.
 *
 *
 * There are interesting features as follows:
 *
 * 1. A Scheduler.
 *
 * Strict round robin scheduler, uses a "clockhand" algorithm for scheduling.
 * As such, you will see the entire Process Table "rotate" to switch which task
 * is RUNNING. Only one task may run at a time.
 *
 * 2. A Semaphore.
 *
 * A binary Semaphore, who puts tasks into a queue when they are unable to get
 * access to the resource. Enforces strict FIFO access to the guarded resource.
 *
 * 3. A Pipe.
 *
 * A Pipe, who serves to transport chars between processes. Created with a
 * unique ID.
 *
 * 4. A Kernel.
 *
 * A pseudo-kernel for the purposes of orchestrating the Ultima 2.0 OS. It is a
 * Singleton Monitor. Ideally, this makes tracking and organizing access to
 * resources easier for the user-space programmer of the Ultima 2.0 OS.
 *
 * 5. Uthread.
 *
 * Uthread is a thin wrapper over pthread. It's just here to make pthreads
 * register themselves with the Kernel. Still spawns pthreads... we just call
 * them something different.
 *
 * Hunter Poole
 * 04-12-2026
 */

#include "Kernel.h" // Singleton Monitor. Orchestrates all other Ultima classes.
#include "MMU.h"    // Memory Management Unit.
#include "Sched.h"  // The Scheduler.
#include "Sema.h"   // The Semaphore.
#include "Uthread.h" // Our Uthread - a pthread wrapper for the Kernel.
#include "ufs.h"
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <ncurses.h>
#include <pthread.h>
#include <thread>
#include <unistd.h>

using namespace std;

//--------------Forward Declarations----------------

// Get "The Kernel" from Kernel.h
Kernel *KernelPtr = Kernel::Get_Instance();

// Get The Scheduler's address from the Kernel.
Scheduler *SchedulerPtr = KernelPtr->Get_Scheduler();

// Ask the Kernel to make a Semaphore "Text".
Semaphore *SemaphorePtr = KernelPtr->Create_Semaphore("Something special!");

// Get a pointer to the MMU.
MMU *MMUPtr = MMU::Get_Instance();

// Uthread wraps pthreads for use with Kernel. "Ultima Thread."
uthread ut;

// Pointer to a yet undefined Pipe.
Pipe *PipePtr;

// Get a pointer to the UFS
ufs *UFSPtr = ufs::Get_Instance();

// All windows asked for by methods.
WINDOW *Task1_Win;
WINDOW *Task2_Win;
WINDOW *Task3_Win;
WINDOW *Heading_Win;
WINDOW *Console_Win;
WINDOW *Log_Win;
WINDOW *Sched_Win;
WINDOW *Sema_Win;
WINDOW *Pipe_Win;
WINDOW *MMU_Win;
WINDOW *UFS_Win;

// Tick function for syncrhonization.
void Tick();

/* struct TaskContext
 *
 * Holds a pointer to the Task's name and window.
 */
struct TaskContext {
  const char *name;
  WINDOW *win;
};

//-----------------Window helpers--------------------------------------

/* WINDOW *create_window(int height, int width, int starty, int startx) {...}
 *
 * Helper method for creating windows from Lab 4.
 *
 * 1. Creates a WINDOW *Win pointer.
 * 2. Sets its atributes according to the method's arguments.
 * 3. Sets other desireable attributes.
 * 4. Returns Win.
 */
WINDOW *create_window(int height, int width, int starty, int startx) {
  WINDOW *Win;

  Win = newwin(height, width, starty, startx);
  scrollok(Win, TRUE);
  scroll(Win);
  box(Win, 0, 0);
  wrefresh(Win);

  return Win;
}

/* void write_window(WINDOW *Win, const char *text) {...}
 *
 * A helper method to write to the window from Lab 4.
 *
 * 1. Call wprintw on the window with the text.
 * 2. Calls box 0,0 on the window.
 * 3. Refreshes the window.
 * 4. Calls Tick()
 */
void write_window(WINDOW *Win, const char *text) {
  wprintw(Win, "%s", text);
  box(Win, 0, 0);
  wrefresh(Win);

  Tick();
}

/* void write_window_fast(WINDOW *Win, const char *text) {...}
 *
 * A special implementation of write_window that does *not* call Tick().
 * Otherwise, write_window() calls Tick(), and Tick() calls write_window().
 * Infinite recursion -> segmentation fault.
 */
void write_window_fast(WINDOW *Win, const char *text) {
  wprintw(Win, "%s", text);
  box(Win, 0, 0);
  wrefresh(Win);
}

/* void write_window_start(WINDOW *Win, int y, int x, const char *text) {...}
 *
 * A helper method to write to the window from Lab 4.
 * An overloaded version of the above method.
 *
 * 1. Call mvwprintw on the Window with x,y cords and text.
 * 2. Calls box 0,0 on the window.
 * 3. Refreshes the window.
 */
void write_window(WINDOW *Win, int y, int x, const char *text) {
  mvwprintw(Win, y, x, "%s", text);
  box(Win, 0, 0);
  wrefresh(Win);
}

/* void display_help(WINDOW *Win) {...}
 *
 * A helper method to display the help menu from Lab 4.
 * In this version, we display two demo scenarios to show off the scheduler and
 * semaphore.
 *
 * 1. Clears the window (likely Console_Win)
 * 2. Writes a series of commands and their consequences to the Console_Win.
 */
void display_help(WINDOW *Win) {
  wclear(Win);
  write_window(Win, 1, 1, "...Help...");
  write_window(Win, 2, 1, "1= Scenario 1");
  write_window(Win, 3, 1, "2= Scenario 2");
  write_window(Win, 4, 1, "3= Scenario 3");
  write_window(Win, 5, 1, "4= Scenario 4");
  write_window(Win, 6, 1, "p= Pause");
  write_window(Win, 7, 1, "c= Clear screen");
  write_window(Win, 8, 1, "h= Help screen");
  write_window(Win, 9, 1, "q= Quit");
}

/* void write_defaults() {...}
 *
 * A method to clear and re-write all windows that are written to.
 *
 * 1. Clear every Window.
 * 2. Write back it's default text.
 */
void write_defaults() {
  wclear(Log_Win);
  wclear(Console_Win);
  wclear(Task1_Win);
  wclear(Task2_Win);
  wclear(Task3_Win);
  wclear(Sched_Win);
  wclear(Sema_Win);
  wclear(Pipe_Win);
  wclear(MMU_Win);

  write_window(Log_Win, 1, 5, "...Log Window...\n");
  write_window(Console_Win, 1, 1, "...Console...\n");
  write_window(Console_Win, 2, 1, "161-Ultima 2.0 #\n");
  write_window(Task1_Win, 6, 1, "Thread 1\n");
  write_window(Task2_Win, 6, 1, "Thread 2\n");
  write_window(Task3_Win, 6, 1, "Thread 3\n");
  write_window(Sched_Win, 1, 3, "...Scheduler Window...\n");
  write_window(Sema_Win, 1, 3, "...Semaphore Window...\n");
  write_window(Pipe_Win, 1, 3, "...Pipe Window...\n");
  write_window(MMU_Win, 1, 3, "...MMU Window...\n");
}

/* void Tick() {...}
 *
 * Slows everything down to "ticks."
 *
 * 1. Unlock the CPULocker.
 * 2. Sleep for 500ms.
 * 3. Lock the CPULocker.
 */
void Tick() {
  // write to all windows here.
  if (SchedulerPtr != nullptr) {
    write_window_fast(Sched_Win, SchedulerPtr->dump().c_str());
  }

  if (SemaphorePtr != nullptr) {
    write_window_fast(Sema_Win, SemaphorePtr->dump().c_str());
  }

  if (PipePtr != nullptr) {
    write_window_fast(Pipe_Win, PipePtr->dump().c_str());
  }

  if (MMUPtr != nullptr) {
    write_window_fast(MMU_Win, MMUPtr->Dump_Blocks().c_str());
  }

  pthread_mutex_unlock(&Kernel::CPULocker);
  this_thread::sleep_for(chrono::milliseconds(500));
  pthread_mutex_lock(&Kernel::CPULocker);
}

//----------------Scenario Methods----------------------------------------

/* void *fake_work(void *args) {...}
 *
 * For Scenario 1.
 * Each Task pretends to be doing meaningful work in its own window.
 * When the task is done, it is fully done, and it yields for the next task.
 *
 * 1. Extract all arguments into a TaskContext struct *Context.
 * 2. Make a buffer.
 * 3. Do this five times:
 *    - SemaphorePtr->down()
 *    - Print that we are "running"
 *    - SemaphorePtr->up();
 *    - Sleep for 500ms
 * 4. Lock Semaphore.
 * 5. Print that we are yielding.
 * 6. Sleep for 500ms.
 * 7. Yield.
 */
void *fake_work(void *args) {
  TaskContext *Context = (TaskContext *)args;
  char buffer[256];

  // Get semaphore, print, release it, and sleep.
  for (int i = 0; i < 5; i++) {
    SemaphorePtr->down();
    sprintf(buffer, " %s running\n", Context->name);
    write_window(Context->win, buffer);
    SemaphorePtr->up();
  }

  // Prepare to yield. Kernel yields for us on return.
  SemaphorePtr->down();
  sprintf(buffer, " %s yielding\n", Context->name);
  write_window(Context->win, buffer);
  SemaphorePtr->up();

  return (NULL);
}

/* void *fake_work_with_sema(void *args) {...}
 *
 * For Scenario 2.
 * Each task declares it wants the Semaphore, and access to a guarded resource.
 * They all make a mad dash to try and acquire it, only to be blocked and forced
 * to wait.
 *
 * 1. Extract all arguments into a TaskContext struct *Context.
 */
void *fake_work_with_sema(void *args) {
  TaskContext *Context = (TaskContext *)args;
  char buffer[256];

  sprintf(buffer, " %s wants Sema\n", Context->name);
  write_window(Context->win, buffer);
  SemaphorePtr->down();

  sprintf(buffer, " %s HAS Sema\n", Context->name);
  write_window(Context->win, buffer);
  sprintf(buffer, " %s yields\n", Context->name);
  write_window(Context->win, buffer);
  SchedulerPtr->yield();

  for (int i = 0; i < 5; i++) {
    sprintf(buffer, " %s is working\n", Context->name);
    write_window(Context->win, buffer);
  }

  sprintf(buffer, " %s done w/ Sema\n", Context->name);
  write_window(Context->win, buffer);
  sprintf(buffer, " %s up() & yield\n", Context->name);
  write_window(Context->win, buffer);
  SemaphorePtr->up();

  return (NULL);
}

/* void *producer(void *args) {...}
 *
 * This is a producer for the producer-consumer problem.
 * Features some odd yields for the sake of a demo.
 *
 * 1.  Extract args into TaskContext *Context.
 * 2.  Get a buffer.
 * 3.  Get a Pipe of ID = 1.
 * 4.  Lock the Semaphore.
 * 5.  Call yield (Consumer will yield from locked Semaphore).
 * 6.  Close the read end of the Pipe.
 * 7.  Open the write end of the Pipe for this task.
 * 8.  Write 5 random chars to the pipe.
 * 9.  Release Semaphore.
 * 10. Yield.
 */
void *producer(void *args) {
  TaskContext *Context = (TaskContext *)args;
  char buffer[256];

  sprintf(buffer, " %s is Producer\n", Context->name);
  write_window(Context->win, buffer);
  sprintf(buffer, " %s makes Pipe 1\n", Context->name);
  write_window(Context->win, buffer);
  PipePtr = KernelPtr->Get_Pipe(1);

  for (int i = 0; i < 5; i++) {
    sprintf(buffer, " %s locks Sema\n", Context->name);
    write_window(Context->win, buffer);
    SemaphorePtr->down();
    SchedulerPtr->yield();

    sprintf(buffer, " %s closes Read\n", Context->name);
    write_window(Context->win, buffer);
    PipePtr->close_read();

    sprintf(buffer, " %s opens Write\n", Context->name);
    write_window(Context->win, buffer);
    PipePtr->open_for_write(SchedulerPtr->get_task_id());

    for (int j = 0; j < 5; j++) {
      char rand_char = ('a' + rand() % 26);
      sprintf(buffer, " %s writes '%c'\n", Context->name, rand_char);
      write_window(Context->win, buffer);
      PipePtr->write(rand_char);
    }

    sprintf(buffer, " %s releases Sema\n", Context->name);
    write_window(Context->win, buffer);
    SemaphorePtr->up();

    sprintf(buffer, " %s yields\n", Context->name);
    write_window(Context->win, buffer);

    SchedulerPtr->yield();
  }

  return (NULL);
}

/* void *consumer(void *args) {...}
 *
 * This is a consumer for the producer-consumer problem.
 * Features some odd yields for the sake of a demo.
 *
 * 1.  Extract args into TaskContext *Context
 * 2.  Get a buffer.
 * 3.  Get a Pipe of ID = 1.
 * 4.  Lock the Semaphore (will fail and yield).
 * 5.  Close the write end of the Pipe.
 * 6.  Open the read end of the Pipe.
 * 7.  Read all chars in the Pipe.
 * 8.  Release Semaphore.
 * 9.  Yield.
 * 10. Reset the Pipe for the next run
 *     - LAST step in the Scenario.
 */
void *consumer(void *args) {
  TaskContext *Context = (TaskContext *)args;
  char buffer[256];

  sprintf(buffer, " %s is Consumer\n", Context->name);
  write_window(Context->win, buffer);
  sprintf(buffer, " %s gets Pipe 1\n", Context->name);
  write_window(Context->win, buffer);
  PipePtr = KernelPtr->Get_Pipe(1);

  for (int i = 0; i < 5; i++) {
    sprintf(buffer, " %s wants Sema\n", Context->name);
    write_window(Context->win, buffer);
    SemaphorePtr->down();

    sprintf(buffer, " %s gets Sema\n", Context->name);
    write_window(Context->win, buffer);

    sprintf(buffer, " %s closes Write\n", Context->name);
    write_window(Context->win, buffer);
    PipePtr->close_write();

    sprintf(buffer, " %s opens Read\n", Context->name);
    write_window(Context->win, buffer);
    PipePtr->open_for_read(SchedulerPtr->get_task_id());

    while (!PipePtr->is_empty()) {
      char read = PipePtr->read();
      sprintf(buffer, " %s read '%c'\n", Context->name, read);
      write_window(Context->win, buffer);
    }

    sprintf(buffer, " %s releases Sema\n", Context->name);
    write_window(Context->win, buffer);
    SemaphorePtr->up();

    sprintf(buffer, " %s yields\n", Context->name);
    write_window(Context->win, buffer);
    SchedulerPtr->yield();
  }

  PipePtr->reset(1);
  return (NULL);
}

/* void tasks_try_to_read_without_alloc(void *args) {...}
 *
 * A helper method for Scenario 4.
 *
 * In this sub-scenario, each task will try to read memory, without first
 * allocating it. We expect them to fail.
 *
 * We recall that we made a mistake in programming this sub-scenario, and print
 * that out for the user...
 *
 * 1. Extract args into TaskContext *Context.
 * 2. Get a buffer.
 * 3. Declare ints read, handle.
 * 4. Try to read.
 * 5. Check if Read() return value is -1.
 *    5a. If so, we successfully failed.
 *    5b. Print that.
 * 6. Else...
 *    6a. We failed at failing.
 *    6b. Print that.
 * 7. Yield.
 */
void tasks_try_to_read_without_alloc(void *args) {
  TaskContext *Context = (TaskContext *)args;
  char buffer[256];
  int read, handle;

  sprintf(buffer, " %s wants to read\n", Context->name);
  write_window(Context->win, buffer);
  read = MMUPtr->Read(handle);
  if (read == -1) {
    sprintf(buffer, " ERROR! Can't read\n I forgot to Alloc()...\n\n");
    write_window(Context->win, buffer);
  } else {
    sprintf(buffer, " %s read %c\n", Context->name, read);
    write_window(Context->win, buffer);
  }

  SchedulerPtr->yield();
}

/* void tasks_alloc_10000(void *args) {...}
 *
 * A helper method for Scenario 4.
 *
 * In this sub-scenario, the programmer of each task is unsure of how many bytes
 * they will need, so they just try and allocate 10,000 bytes, figuring that
 * will be enough for their purposes.
 *
 * 1. Extract args into TaskContext *Context.
 * 2. Get a buffer.
 * 3. Declare int handle.
 * 4. Try to alloc 10,000 bytes.
 * 5. Check return value of Alloc(10000)
 *    5a. If -1, we failed successfully.
 *    5b. Print that.
 * 6. Else...
 *    6a. We failed at failing.
 *    6b. Print that.
 * 7. Yield.
 */
void tasks_alloc_10000(void *args) {
  TaskContext *Context = (TaskContext *)args;
  char buffer[256];
  int handle;

  sprintf(buffer, " %s Alloc()'s 10,000\n", Context->name);
  write_window(Context->win, buffer);
  handle = MMUPtr->Alloc(10000);
  if (handle == -1) {
    sprintf(buffer, " ERROR! Can't Alloc()\n Guess that's too much?\n\n");
    write_window(Context->win, buffer);
  } else {
    sprintf(buffer, " %s was successful!\n", Context->name);
    write_window(Context->win, buffer);
  }

  SchedulerPtr->yield();
}

/* void kill_time_for_inspection(void *args) {...}
 *
 * A helper method for Scenario 4.
 *
 * This method just exists to *visibly* kill time.
 * In this way, we let the user know we are waiting for them and giving them a
 * second to look around.
 *
 * 1. Extract args into TaskContext *Context.
 * 2. Get a buffer.
 * 3. Do this five times:
 *    3a. Write " pause for inspection...\n"
 * 4. Yield.
 * 5. Write " Continue tests...\n\n"
 */
void kill_time_for_inspection(void *args) {
  TaskContext *Context = (TaskContext *)args;
  char buffer[256];

  // Allow user time to inspect memory condition.
  write_window(Context->win, "\n");
  for (int i = 0; i < 5; i++) {
    write_window(Context->win, " pause for inspection...\n");
  }
  write_window(Context->win, "\n");
  SchedulerPtr->yield();
  write_window(Context->win, " Continue tests...\n\n");
}

/* void tasks_read_bad_blocks(void *args) {...}
 *
 * A helper method for Scenario 4.
 *
 * In this sub-scenario, each task will try to read a block by a known bad
 * handle. We expect them to fail.
 *
 * 1. Extract args into TaskContext *Context.
 * 2. Get a buffer.
 * 3. Declare int read.
 * 4. Read by a known bad handle.
 * 5. Check the return value of Read().
 *    5a. If -1...
 *        a. We failed successfully.
 *        b. Print that.
 *    5b. Else,
 *        a. We failed at failing.
 *        b. Print that.
 * 6. Yield.
 */
void tasks_read_bad_blocks(void *args) {
  TaskContext *Context = (TaskContext *)args;
  char buffer[256];
  int read;

  sprintf(buffer, " %s reads another thread's block\n", Context->name);
  read = MMUPtr->Read(-1);
  write_window(Context->win, buffer);
  if (read == -1) {
    write_window(Context->win, " Failure.\n\n");
  } else {
    sprintf(buffer, " Success! Read %c\n\n", read);
    write_window(Context->win, buffer);
  }
  SchedulerPtr->yield();
}

/* void tasks_alloc_and_write_and_read(void *args) {...}
 *
 * A helper method for Scenario 4.
 *
 * In this sub-scenario, each task will alloc 64 bytes, write a random char to
 * them, read them, attempt to read someone else's memory, then free their
 * blocks.
 *
 * 1.  Extract args into TaskContext *Context.
 * 2.  Get a buffer.
 * 3.  Declare ints read, write, handle, free_mem.
 * 4.  Alloc 64, and check the return value.
 *     4a. If anything but -1, Alloc() gave us a block.
 *         a. Print success.
 *     4b. Else,
 *         a. Print failure.
 * 5.  Yield.
 * 6.  Write a random char to the memory block by handle.
 * 7.  Check the return value of Write().
 *     7a. If anything but -1, Write() was successful.
 *         a. Print that.
 *     7b. Else,
 *         a. Print failure.
 * 8.  Yield.
 * 9.  Read a char from memory by handle.
 * 10. Check the return value of Read().
 *     10a. If anything but -1, Read() was successful.
 *          a. Print that.
 *     10b. Else,
 *          a. Print failure.
 * 11. Call tasks_read_bad_blocks(args);
 * 12. Free memory.
 * 13. Check return value of Free().
 *     13a. If anything but -1, Free() was successful.
 *          a. Print that.
 *     13b. Else,
 *          b. Print failure.
 * 14. Yield.
 */
void tasks_alloc_and_write_and_read(void *args) {
  TaskContext *Context = (TaskContext *)args;
  char buffer[256];
  int read, write, handle, free_mem;

  sprintf(buffer, " %s Alloc()'s 64\n", Context->name);
  handle = MMUPtr->Alloc(64);
  write_window(Context->win, buffer);
  if (handle != -1) {
    write_window(Context->win, " Success!\n");
  } else {
    write_window(Context->win, " Failure.\n");
  }

  SchedulerPtr->yield();

  char rand_char = ('a' + rand()) % 26;
  sprintf(buffer, " %s writes %c\n", Context->name, rand_char);
  write = MMUPtr->Write(handle, rand_char);
  write_window(Context->win, buffer);
  if (write != -1) {
    write_window(Context->win, " Success!\n");
  } else {
    write_window(Context->win, " Failure.\n");
  }

  SchedulerPtr->yield();

  sprintf(buffer, " %s reads memory\n", Context->name);
  read = MMUPtr->Read(handle);
  write_window(Context->win, buffer);
  if (read != -1) {
    sprintf(buffer, " Success! Read %c\n\n", read);
    write_window(Context->win, buffer);
  } else {
    sprintf(buffer, " Failure.\n\n");
    write_window(Context->win, buffer);
  }

  SchedulerPtr->yield();
  tasks_read_bad_blocks(args);

  sprintf(buffer, " %s frees memory\n", Context->name);
  free_mem = MMUPtr->Free(handle);
  write_window(Context->win, buffer);
  if (free_mem != -1) {
    write_window(Context->win, " Success.\n\n");
  } else {
    write_window(Context->win, " Failure.\n\n");
  }

  SchedulerPtr->yield();
}

/* void tasks_compete_for_800(void *args) {...}
 *
 * A helper method for Scenario 4.
 *
 * In this sub-scenario, each task races against the others to allocate 800
 * bytes, which we know is most of our available memory...
 * Since we are working with a strictly cooperative scheduler, we know that
 * Task1 will go first and get it, then Task2 and Task 3 will be blocked by the
 * Semaphore. After Task1 is done, Task2 will grab 800 bytes, and so on.
 *
 * However, this sub-scenario is not designed with that intimate knowledge
 * Scheduler. In this case, all tasks must call down on the Semaphore and wait
 * their turn.
 *
 * 1.  Extract args into TaskContext *Context.
 * 2.  Get a buffer.
 * 3.  Declare ints handle, read, write, free_mem.
 * 4.  Write the sub-scenario's goals.
 *     4a. Yield.
 * 5.  Set handle = -1 to let While loop run.
 * 6.  Call down() on Semaphore.
 * 7.  While handle is -1, try to allocate 800 bytes.
 *     7a. Then, yield.
 * 8.  If handle is not -1, we have the block.
 *     8a. Write that we succeeded.
 *     8b. Print 5 chars to the block, and inform the user.
 *     8c. Read all chars just to clear the block.
 *     8d. Free the memory.
 *     8e. Check the return value of Free().
 *         a. If anything other than -1, Free() succeeded.
 *            1. Print that.
 *         b. Else,
 *            1. Print failure.
 * 9.  Else...
 *     9a. Print failure.
 * 10. Call up() on Semaphore.
 * 11. yield.
 */
void tasks_compete_for_800(void *args) {
  TaskContext *Context = (TaskContext *)args;
  char buffer[256];
  int handle, read, write, free_mem;

  write_window(Context->win, " Compete for space.\n");
  SchedulerPtr->yield();

  sprintf(buffer, " %s wants 800 bytes.\n", Context->name);
  write_window(Context->win, buffer);

  handle = -1;
  SemaphorePtr->down();
  while (handle == -1) {
    handle = MMUPtr->Alloc(800);
    SchedulerPtr->yield();
  }
  write_window(Context->win, buffer);
  if (handle != -1) {
    write_window(Context->win, " Success!\n");

    for (int i = 0; i < 5; i++) {
      MMUPtr->Write(handle, ('a' + rand() % 26));
      write_window(Context->win, " Working...\n");
    }

    for (int i = 0; i < 5; i++) {
      MMUPtr->Read(handle);
    }
    write_window(Context->win, "\n");

    sprintf(buffer, " %s Frees memory\n", Context->name);
    free_mem = MMUPtr->Free(handle);
    write_window(Context->win, buffer);
    if (free_mem != -1) {
      write_window(Context->win, " Success!\n\n");
    } else {
      write_window(Context->win, " Failure.\n\n");
    }
  } else {
    write_window(Context->win, " Failure.\n\n");
  }
  SemaphorePtr->up();
  SchedulerPtr->yield();
}

/* void *use_mmu(void *arg) {...}
 *
 * This is a demo method to use the Memory Management Unit.
 * This method immediately calls the four helper methods, which are the
 * sub-scenarios for this scenario.
 *
 * 1. Read without alloc. (fails)
 * 2. Alloc 10,000 bytes. (fails).
 * 3. Alloc and write/read. (works - but can only read your own block).
 * 4. Compete for space. (works - but tasks are put in queue for space).
 */
void *use_mmu(void *args) {

  tasks_try_to_read_without_alloc(args);

  tasks_alloc_10000(args);

  tasks_alloc_and_write_and_read(args);

  wclear(MMU_Win);

  tasks_compete_for_800(args);

  return (NULL);
}

//---------------Orchestration----------------------------------------

/* void *console(void *arg) {...}
 *
 * This method exists to put the console window in its own Uthread.
 * That way, we can run the Ultima OS demo off of the Ultima OS structures.
 *
 * 1. Make a buffer.
 * 2. Prepare for input.
 * 3. Build all uthread tasks and TaskContext structs.
 * 4. Loop input catch-and-respond until 'q' is entered.
 *    - '1' for scenario 1.
 *    - '2' for scenario 2.
 *    - '3' for scenario 3.
 *    - '4' for scenario 4.
 *    - 'p' for Pause.
 *        - 'r' for Resume.
 *    - 'h' for Help.
 *    - 'c' for Clear.
 *    - 'q' for Quit.
 * 5. Delete all TaskContext structs.
 */
void *console(void *args) {
  char buffer[256];
  int input = -1;

  uthread_t Task1, Task2, Task3;
  TaskContext *T1 = new TaskContext;
  T1->name = "Thread 1";
  T1->win = Task1_Win;

  TaskContext *T2 = new TaskContext;
  T2->name = "Thread 2";
  T2->win = Task2_Win;

  TaskContext *T3 = new TaskContext;
  T3->name = "Thread 3";
  T3->win = Task3_Win;

  while (input != 'q') {
    input = wgetch(Console_Win);

    switch (input) {
    case '1': // Scenario 1 - each task all the way through.
      write_window(Log_Win, " Scenario 1: One thread at a time\n");

      ut.create(&Task1, fake_work, T1);
      ut.create(&Task2, fake_work, T2);
      ut.create(&Task3, fake_work, T3);
      SchedulerPtr->yield();

      write_window(Log_Win, " Scenario 1 Finished\n");
      SchedulerPtr->garbage_collect();

      break;
    case '2': { // Scenario 2 - tasks get blocked by Semaphore.
      write_window(Log_Win, " Scenario 2: All want the Semaphore\n");

      ut.create(&Task1, fake_work_with_sema, T1);
      ut.create(&Task2, fake_work_with_sema, T2);
      ut.create(&Task3, fake_work_with_sema, T3);

      break;
    }
    case '3': { // Scenario 3 - Tasks communicate with Pipe.
      write_window(Log_Win, " Scenario 3: Threads use Pipe\n");

      ut.create(&Task1, producer, T1);
      ut.create(&Task2, consumer, T2);

      break;
    }
    case '4': { // Scenario 4 - Tasks use Memory Management Unit.
      write_window(Log_Win, " Scenario 4: Threads use MMU\n");

      ut.create(&Task1, use_mmu, T1);
      ut.create(&Task2, use_mmu, T2);
      ut.create(&Task3, use_mmu, T3);

      break;
    }
    case 'p': { // PAUSE the Scenarios
      write_window(Log_Win, " PAUSED: 'r' TO RESUME\n");
      while (input != 'r') {
        input = wgetch(Console_Win);
      }
      write_window(Log_Win, " RESUMED\n");
    }
    case 'c': { // CLEAR the windows
      write_defaults();
      SchedulerPtr->garbage_collect();
      break;
    }
    case 'q': { // q for quit
      endwin();
      exit(0);
    }
    case 'h':
      display_help(Console_Win);
    case ERR:
      SchedulerPtr->yield();
      break;
    default:
      sprintf(buffer, " %c", input);
      write_window_fast(Console_Win, buffer);
      write_window_fast(Console_Win, " -Invalid Command\n");
      write_window_fast(Log_Win, buffer);
      write_window_fast(Log_Win, " -Invalid Command\n");
      write_window_fast(Console_Win, " 161-Ultima 2.0 #");
      SchedulerPtr->yield();
      break;
    }
  }

  delete T1;
  delete T2;
  delete T3;

  return (NULL);
}

/* int main() {...}
 *
 * Main method for the Ultima demo.
 *
 * 1.  Initializes ncurses screen.
 * 2.  Builds and prints to the heading window.
 * 3.  Builds and prints to the log window.
 * 4.  Builds and prints to the console window.
 * 5.  Builds and prints all task windows.
 * 6.  Set input flags and features.
 * 7.  Create console_task and dispatch it to console().
 * 8.  Start the scheduler.
 * 9.  Wait for console_task to finish and join.
 * 10. Kill the ncurses window.
 */
int main() {
  // Initializes the ncurses screen.
  initscr();

  Heading_Win = newwin(10, 81, 3, 2);
  box(Heading_Win, 0, 0);
  mvwprintw(Heading_Win, 2, 28, "161-ULTIMA 2.0 PHASE 2 DEMO");
  mvwprintw(Heading_Win, 4, 2, "Press 'h' to view the scenarios.");
  mvwprintw(Heading_Win, 5, 2, "Press 'q' or Crtl-C to exit the program.");
  wrefresh(Heading_Win);

  Log_Win = create_window(10, 61, 28, 2);
  Console_Win = create_window(10, 20, 28, 63);
  Task1_Win = create_window(15, 27, 13, 2);
  Task2_Win = create_window(15, 27, 13, 29);
  Task3_Win = create_window(15, 27, 13, 56);
  Sched_Win = create_window(8, 40, 3, 83);
  Sema_Win = create_window(5, 40, 11, 83);
  Pipe_Win = create_window(9, 40, 16, 83);
  MMU_Win = create_window(35, 92, 25, 83);
  write_defaults();

  cbreak();
  noecho();
  nodelay(Console_Win, TRUE);
  keypad(Console_Win, TRUE);
  mousemask(ALL_MOUSE_EVENTS, NULL);

  // uthread_t is a thin wrapper over pthread_t for the Kernel.
  uthread_t console_task;
  ut.create(&console_task, console, NULL);

  // *Immediately* start scheduler, since the Console is run off a uthread.
  SchedulerPtr->start();

  // Wait for console to die, kill windows, return and exit.
  pthread_join(console_task, NULL);
  endwin();

  return 0;
}
