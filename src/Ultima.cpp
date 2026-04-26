/* 161-Ultima 2.0 Phase 2 Demo file.
 *
 * This Demo file feeatures three scenarios of consequence, for three features
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
 * Three utrheads are spawned and dispatched to complete some "fake work"
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
  wprintw(Win, text);
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
  wprintw(Win, text);
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

      for (int i = 0; i < 4; i++) {
        SchedulerPtr->yield();
      }

      write_window(Log_Win, " Scenario 2 Finished\n");
      SchedulerPtr->garbage_collect();

      break;
    }
    case '3': { // Scenario 3 - Tasks communicate with Pipe.
      write_window(Log_Win, " Scenario 3: Threads use Pipe\n");

      ut.create(&Task1, producer, T1);
      ut.create(&Task2, consumer, T2);

      // for (int i = 0; i < 4; i++) {
      //   SchedulerPtr->yield();
      // }

      SchedulerPtr->garbage_collect();

      write_window(Log_Win, " Scenario 3 Finished\n");
      break;
    }
    case '4': { // Scenario 4 - Tasks use Memory Management Unit.
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

  Heading_Win = newwin(10, 80, 3, 2);
  box(Heading_Win, 0, 0);
  mvwprintw(Heading_Win, 2, 28, "161-ULTIMA 2.0 PHASE 2 DEMO");
  mvwprintw(Heading_Win, 4, 2, "Press 'h' to view the scenarios.");
  mvwprintw(Heading_Win, 5, 2, "Press 'q' or Crtl-C to exit the program.");
  wrefresh(Heading_Win);

  Log_Win = create_window(10, 60, 28, 2);
  Console_Win = create_window(10, 20, 28, 62);
  Task1_Win = create_window(15, 27, 13, 2);
  Task2_Win = create_window(15, 26, 13, 29);
  Task3_Win = create_window(15, 27, 13, 55);
  Sched_Win = create_window(8, 40, 3, 82);
  Sema_Win = create_window(5, 40, 11, 82);
  Pipe_Win = create_window(9, 40, 16, 82);
  MMU_Win = create_window(9, 40, 20, 82);
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
