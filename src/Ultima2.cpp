/* 161-Ultima 2.0 Phase 2 Demo file.
 *
 * Hunter Poole 04-05-2026
 */

#include "Sched.h"
#include "Sema.h"
#include <ncurses.h>

using namespace std;

//--------------Forward Declarations----------------
Scheduler *scheduler = new Scheduler();
Semaphore *semaphore = new Semaphore("Text");

int Task1 = scheduler->create_task();
int Task2 = scheduler->create_task();
int Task3 = scheduler->create_task();

WINDOW *Task1_Win;
WINDOW *Task2_Win;
WINDOW *Task3_Win;

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

void *fake_work() { return (NULL); }

void *fake_work_with_sema() { return (NULL); }

int main() {
  // Register scheduler with semaphore and set quantum LOW to force yields.
  Semaphore::set_scheduler(scheduler);
  scheduler->set_quantum(1);

  // Create all tasks with the scheduler.
  int Task1 = scheduler->create_task();
  int Task2 = scheduler->create_task();
  int Task3 = scheduler->create_task();

  // Create all pthread_t
  pthread_t Thread1, Thread2, Thread3;

  // Register each Thread with its Task.
  scheduler->set_pthread_t(Task1, Thread1);
  scheduler->set_pthread_t(Task2, Thread2);
  scheduler->set_pthread_t(Task3, Thread3);

  // Initializes the ncurses screen.
  initscr();

  return 0;
}
