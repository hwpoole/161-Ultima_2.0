/* 161-Ultima 2.0 Phase 2 Demo file.
 *
 * Hunter Poole 04-05-2026
 */

#include "Kernel.h" // Singleton Monitor. Orchestrates all other Ultima classes.
#include "Sched.h"  // The Scheduler.
#include "Sema.h"   // The Semaphore.
#include "Uthread.h" // Our Uthread - a pthread wrapper for the Kernel.
#include <chrono>
#include <cstddef>
#include <ncurses.h>
#include <pthread.h>
#include <thread>
#include <unistd.h>

using namespace std;

//--------------Forward Declarations----------------

// Get or find The Kernel from Kernel.h
Kernel *KernelPtr = Kernel::Get_Instance();

// Get the scheduler's address from the Kernel.
Scheduler *SchedulerPtr = KernelPtr->Get_Scheduler();

// Ask the Kernel to make a Semaphore "Text".
Semaphore *SemaphorePtr = KernelPtr->Create_Semaphore("Text");

// Uthread wraps pthreads for use with Kernel. "Ultima Thread."
uthread ut;

// All windows asked for by methods.
WINDOW *Task1_Win;
WINDOW *Task2_Win;
WINDOW *Task3_Win;
WINDOW *Console_Win;
WINDOW *Log_Win;

/* struct TaskContext
 *
 * Holds a pointer to the Task's name and window.
 */
struct TaskContext {
  const char *name;
  WINDOW *win;
};

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
  pthread_mutex_unlock(&Kernel::CPULocker);
  this_thread::sleep_for(chrono::milliseconds(500));
  pthread_mutex_lock(&Kernel::CPULocker);
}

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
  wprintw(Win, text);
  box(Win, 0, 0);
  wrefresh(Win);

  Tick();
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
  mvwprintw(Win, y, x, text);
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
  write_window(Win, 4, 1, "3= Dump Sched/Sema");
  write_window(Win, 5, 1, "c= Clear screen");
  write_window(Win, 6, 1, "h= Help screen");
  write_window(Win, 7, 1, "q= Quit");
}

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
    // this_thread::sleep_for(chrono::milliseconds(500));
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
  int current_task = SchedulerPtr->get_task_id();
  char buffer[256];

  sprintf(buffer, " %s wants Sema\n", Context->name);
  write_window(Context->win, buffer);
  // this_thread::sleep_for(chrono::milliseconds(500));
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

  if (SchedulerPtr->get_state(current_task) == BLOCKED) {
    sprintf(buffer, " %s was blocked\n", Context->name);
    write_window(Context->win, buffer);
  }

  sprintf(buffer, " %s done w/ Sema\n", Context->name);
  write_window(Context->win, buffer);
  sprintf(buffer, " %s up() & yield\n", Context->name);
  write_window(Context->win, buffer);
  SemaphorePtr->up();

  return (NULL);
}

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
      write_window(Log_Win, " Scenario 1: One Task At A Time\n");

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
      int task_3_id = ut.create(&Task3, fake_work_with_sema, T3);

      for (int i = 0; i < 4; i++) {
        SchedulerPtr->yield();
      }

      write_window(Log_Win, " Scenario 2 Finished\n");
      SchedulerPtr->garbage_collect();

      break;
    }
    case 3: // Scenario 3 - Tasks communicate with Pipe.
    default:
      break;
    case ERR:
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

  WINDOW *Heading_Win = newwin(10, 80, 3, 2);
  box(Heading_Win, 0, 0);
  mvwprintw(Heading_Win, 2, 28, "161-ULTIMA 2.0 PHASE 2 DEMO");
  mvwprintw(Heading_Win, 4, 2, "Press 'h' to view the scenarios.");
  mvwprintw(Heading_Win, 5, 2, "Press 'q' or Crtl-C to exit the program.");
  wrefresh(Heading_Win);

  Log_Win = create_window(10, 60, 30, 2);
  write_window(Log_Win, 1, 5, "...Log Window...\n");

  Console_Win = create_window(10, 20, 30, 62);
  write_window(Console_Win, 1, 1, "...Console...\n");
  write_window(Console_Win, 2, 1, "161-Ultima 2.0 #\n");

  Task1_Win = create_window(15, 25, 15, 2);
  write_window(Task1_Win, 6, 1, "Thread 1\n");

  Task2_Win = create_window(15, 25, 15, 30);
  write_window(Task2_Win, 6, 1, "Thread 2\n");

  Task3_Win = create_window(15, 25, 15, 57);
  write_window(Task3_Win, 6, 1, "Thread 3\n");

  cbreak();
  noecho();
  nodelay(Console_Win, TRUE);
  keypad(Console_Win, TRUE);
  mousemask(ALL_MOUSE_EVENTS, NULL);

  // uthread_t is a thin wrapper over pthread_t for the Kernel.
  uthread_t console_task;
  ut.create(&console_task, console, NULL);

  SchedulerPtr->start();

  pthread_join(console_task, NULL);
  endwin();

  return 0;
}
