/* 161-Ultima 2.0 Phase 2 Demo file.
 *
 * Hunter Poole 04-05-2026
 */

#include "Kernel.h"
#include "Sched.h"
#include "Sema.h"
#include "Uthread.h"
#include <cstddef>
#include <ncurses.h>
#include <pthread.h>
#include <thread>
#include <unistd.h>

using namespace std;

//--------------Forward Declarations----------------
Kernel *KernelPtr = Kernel::Get_Instance();
Scheduler *SchedulerPtr = KernelPtr->Get_Scheduler();
Semaphore *SemaphorePtr = KernelPtr->Create_Semaphore("Text");
uthread ut;

WINDOW *Task1_Win;
WINDOW *Task2_Win;
WINDOW *Task3_Win;
WINDOW *Console_Win;
WINDOW *Log_Win;

struct TaskContext {
  const char *name;
  WINDOW *win;
};

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
 */
void write_window(WINDOW *Win, const char *text) {
  wprintw(Win, text);
  box(Win, 0, 0);
  wrefresh(Win);
}

/* void write_window(WINDOW *Win, int y, int x, const char *text) {...}
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

void *fake_work(void *args) {
  TaskContext *Context = (TaskContext *)args;
  char buffer[256];

  for (int i = 0; i < 5; i++) {
    SemaphorePtr->down();
    sprintf(buffer, " %s running\n", Context->name);
    write_window(Context->win, buffer);
    SemaphorePtr->up();
    this_thread::sleep_for(chrono::milliseconds(500));
  }

  SemaphorePtr->down();
  sprintf(buffer, " %s yielding\n", Context->name);
  write_window(Context->win, buffer);
  this_thread::sleep_for(chrono::milliseconds(500));
  SemaphorePtr->up();

  return (NULL);
}

void *fake_work_with_sema() {
  int current_task = SchedulerPtr->get_task_id();
  WINDOW *win;
  const char *name;
  char buffer[256];

  sprintf(buffer, " %s wants Sema\n", name);
  write_window(win, buffer);
  SemaphorePtr->down();

  if (SchedulerPtr->get_state(current_task) == BLOCKED) {
    sprintf(buffer, " %s was blocked\n", name);
    write_window(win, buffer);
    this_thread::sleep_for(chrono::milliseconds(500));
  }

  return (NULL);
}

void *console(void *arg) {
  char buffer[256];
  int input = -1;

  uthread_t Task1, Task2, Task3;
  TaskContext *T1 = new TaskContext;
  T1->name = "Task1";
  T1->win = Task1_Win;

  TaskContext *T2 = new TaskContext;
  T2->name = "Task2";
  T2->win = Task2_Win;

  TaskContext *T3 = new TaskContext;
  T3->name = "Task3";
  T3->win = Task3_Win;

  while (input != 'q') {
    input = wgetch(Console_Win);

    switch (input) {
    case '1': // Scenario 1
      write_window(Log_Win, " Scenario 1: One Task At A Time\n");

      ut.create(&Task1, fake_work, T1);
      ut.create(&Task2, fake_work, T2);
      ut.create(&Task3, fake_work, T3);
      SchedulerPtr->yield();
      write_window(Log_Win, " Scenario 1 Finished\n");
      SchedulerPtr->garbage_collect();
      break;
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

  uthread_t console_task;
  ut.create(&console_task, console, NULL);

  SchedulerPtr->start();

  pthread_join(console_task, NULL);
  endwin();

  /*
   cbreak();
   noecho();
   nodelay(Console_Win, TRUE);
   keypad(Console_Win, TRUE);
   mousemask(ALL_MOUSE_EVENTS, NULL);

   char buffer[256];
   int input = -1;

   while (input != 'q') {
     input = wgetch(Console_Win);

     switch (input) {
     case '1': // Scenario 1.
       write_window(Log_Win, " Scenario 1: One Task At A Time\n");

       uthread_t Task1, Task2, Task3;
       ut.create(&Task1, fake_work, NULL);
       ut.create(&Task2, fake_work, NULL);
       ut.create(&Task3, fake_work, NULL);
       SchedulerPtr->start();

       write_window(Log_Win, " Scenario 1 Finished\n");
       break;
     default:
       break;
     }
   }

   */
  return 0;
}
