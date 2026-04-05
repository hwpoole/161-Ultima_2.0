/* 161-Ultima 2.0 Phase 1 Demo file.
 *
 * This file is intended only the demonstrate the results of phase 1.
 * It is what I would consider to be "quite sloppy," and is maybe best deleted
 * after this phase is completed or heavily reworked.
 *
 * Hunter Poole
 * 03-29-2026
 */

#include "Sched.h"
#include "Sema.h"
#include <chrono>
#include <cstring>
#include <ncurses.h>
#include <stdarg.h>
#include <thread>
#include <unistd.h>

using namespace std;

//------------------Globals & forward declarations---------------
Scheduler *my_scheduler = new Scheduler();
Semaphore *my_semaphore = new Semaphore("The Resource");

// The Tasks used in this demo.
int Task1 = my_scheduler->create_task();
int Task2 = my_scheduler->create_task();
int Task3 = my_scheduler->create_task();

// The Task windows.
WINDOW *Task1_Win;
WINDOW *Task2_Win;
WINDOW *Task3_Win;
//-------------------End Globals & Forwards----------------------

/* A faked program counter for the tasks.
 * Was faster (and sillier) than changing the TCB and providing a getter/setter
 *
 * This is used entirely for the fake_work_with_semaphore() demo method.
 */
static int task_PCs[4] = {0, 0, 0, 0};

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

/* void *fake_work() {...}
 *
 * This method is a demo method to show off the strict round-robin scheduler in
 * particular.
 *
 * 1. Get the current task's task_id from the scheduler.
 * 2. Determine which Window that task relates to.
 * 3. Determine what that task is called in a string.
 * 4. "Run" that task 10 times.
 * 5. Yield the task.
 */
void *fake_work() {
  int current_task = my_scheduler->get_task_id();
  WINDOW *win = (current_task == Task1)   ? Task1_Win
                : (current_task == Task2) ? Task2_Win
                                          : Task3_Win;
  const char *name = (current_task == Task1)   ? "Task 1"
                     : (current_task == Task2) ? "Task 2"
                                               : "Task 3";
  char buffer[256];

  for (int i = 0; i < 5; i++) {
    sprintf(buffer, " %s running\n", name);
    write_window(win, buffer);
    this_thread::sleep_for(chrono::milliseconds(500));
  }

  sprintf(buffer, " %s yielding\n", name);
  write_window(win, buffer);
  this_thread::sleep_for(chrono::milliseconds(500));
  my_scheduler->yield();

  return (NULL);
}

/* void *fake_work_with_semaphore() {...}
 *
 * This method is a demo method to show off how the semaphore blocks tasks and
 * adds them to the queue. Of note, tasks do not run in *strict* FIFO on the
 * scheduler, but do have a *strict* FIFO to the semaphore.
 *
 * We use the task_PCs from earlier to fake a program counter for running this
 * method in a loop.
 *
 * 1. Get the current task's task id.
 * 2. Determine what window that relates to.
 * 3. Determine what that task is called.
 * 4. Make a big switch case.
 * 5. If program counter is 0:
 *    - Call down() on semaphore.
 *    - Increment PC.
 *    - Return null if blocked to skip yield.
 *    - Yield.
 * 6. If 1:
 *    - Declare access to critical region.
 *    - Increment PC.
 *    - Yield.
 * 7. If 2:
 *    - Release semaphore.
 *    - Increment PC.
 *    - Yield.
 * 8. If 3:
 *    - Declare finished.
 *    - Increment PC.
 * 9. Yield.
 */
void *fake_work_with_semaphore() {
  int current_task = my_scheduler->get_task_id();
  WINDOW *win = (current_task == Task1)   ? Task1_Win
                : (current_task == Task2) ? Task2_Win
                                          : Task3_Win;
  const char *name = (current_task == Task1)   ? "Task 1"
                     : (current_task == Task2) ? "Task 2"
                                               : "Task 3";
  char buffer[256];
  int &pc = task_PCs[current_task];

  switch (pc) {
  case 0:
    sprintf(buffer, " %s wants Sema\n", name);
    write_window(win, buffer);
    my_semaphore->down();

    pc++;

    if (my_scheduler->get_state(current_task) == BLOCKED) {
      sprintf(buffer, " %s was blocked\n", name);
      write_window(win, buffer);
      this_thread::sleep_for(chrono::milliseconds(500));
      return (NULL);
    }

    this_thread::sleep_for(chrono::milliseconds(500));
    my_scheduler->yield();
    break;
  case 1:
    sprintf(buffer, " %s in crit region\n", name);
    write_window(win, buffer);
    pc++;

    this_thread::sleep_for(chrono::milliseconds(500));
    my_scheduler->yield();
    break;
  case 2:
    sprintf(buffer, " %s releases Sema\n", name);
    write_window(win, buffer);
    my_semaphore->up();
    pc++;

    this_thread::sleep_for(chrono::milliseconds(500));
    my_scheduler->yield();
    break;
  case 3:
    sprintf(buffer, " %s finished\n", name);
    write_window(win, buffer);
    pc++;

    this_thread::sleep_for(chrono::milliseconds(500));
    break;
  }

  my_scheduler->yield();
  return (NULL);
}

/* int main() {...}
 *
 * main method to string all of this together.
 *
 * 1. Registers the scheduler with the semaphore class.
 * 2. Sets quantum *very* low for the sake of the demo to force yields to
 *    happen on command.
 * 3. Initializes the screen.
 * 4. Creates and prints the Heading window.
 * 5. Creates and prints the log window.
 * 6. Creates and prints the console window.
 * 7. Creates and prints the task windows.
 * 8. Starts the scheduler.
 * 9. Sets up I/O processing for ncurses.
 * 10.Runs I/O processing in a while loop.
 * 11.Big switch case for I/O options.
 */
int main() {
  // Register scheduler with semaphore and set quantum LOW To force yields.
  Semaphore::set_scheduler(my_scheduler);
  my_scheduler->set_quantum(1);

  // Initializes the ncurses screen.
  initscr();

  // Create and print Heading_Win.
  WINDOW *Heading_Win = newwin(12, 80, 3, 2);
  box(Heading_Win, 0, 0);
  mvwprintw(Heading_Win, 2, 28, "161-ULTIMA 2.0 PHASE 1 DEMO");
  mvwprintw(Heading_Win, 4, 2, "Starting demo...");
  mvwprintw(Heading_Win, 5, 2, "Starting Task 1...");
  mvwprintw(Heading_Win, 6, 2, "Starting Task 2...");
  mvwprintw(Heading_Win, 7, 2, "Starting Task 3...");
  mvwprintw(Heading_Win, 9, 2, "Press 'q' or Ctrl-C to exit the program...");
  wrefresh(Heading_Win);

  // Create and print Log_Win.
  WINDOW *Log_Win = create_window(10, 60, 30, 2);
  write_window(Log_Win, 1, 5, "...Log Window...\n");
  write_window(Log_Win, "...Main program started...\n");

  // Create and print Console_Win.
  WINDOW *Console_Win = create_window(10, 20, 30, 62);
  write_window(Console_Win, 1, 1, "...Console...");
  write_window(Console_Win, 2, 1, "161-Ultima 2.0 #");

  // Create and print all Task<x>_Win.
  Task1_Win = create_window(15, 25, 15, 2);
  write_window(Task1_Win, 6, 1, "Task 1 Window\n");
  Task2_Win = create_window(15, 25, 15, 30);
  write_window(Task2_Win, 6, 1, "Task 2 Window\n");
  Task3_Win = create_window(15, 25, 15, 57);
  write_window(Task3_Win, 6, 1, "Task 3 Window\n");

  // Call to start the scheduler.
  my_scheduler->start();

  // I/O processing
  cbreak();
  noecho();
  nodelay(Console_Win, TRUE);
  keypad(Console_Win, TRUE);

  char buffer[256];
  int input = -1;

  // Collect input until user quits.
  while (input != 'q') {
    input = wgetch(Console_Win);

    switch (input) {
    case '1': // Scenario 1.
      write_window(Log_Win, " Scenario 1: One Task At A Time\n");
      for (int i = 0; i < 4; i++) {
        fake_work();
      }
      write_window(Log_Win, " Scenario 1 Finished\n");
      break;
    case '2': // Scenario 2.
      write_window(Log_Win,
                   " Scenario 2: Each Task wants the critical region\n");
      for (int i = 0; i < 30; i++) {
        fake_work_with_semaphore();
      }
      write_window(Log_Win, " Scenario 2 Finished\n");

      for (int i = 0; i < 3; i++) {
        task_PCs[i] = 0;
      }
      break;
    case '3': // Scenario 3 - Dump Sched/Sema
      wclear(Log_Win);
      write_window(Log_Win, " Scenario 3: Scheduler Dump\n");
      write_window(Log_Win, my_scheduler->dump().c_str());
      wrefresh(Log_Win);
      sleep(3);

      wclear(Log_Win);
      write_window(Log_Win, " Scenario 3: Semaphore Dump\n");
      write_window(Log_Win, my_semaphore->dump().c_str());
      wrefresh(Log_Win);
      sleep(3);

      wclear(Log_Win);
      write_window(Log_Win, 1, 5, "...Log Window...\n");
      write_window(Log_Win, " Scenario 3 Finished\n");
      break;
    case 'c': // Clear windows.
      wclear(Console_Win);
      wrefresh(Console_Win);

      wclear(Task1_Win);
      box(Task1_Win, 0, 0);
      wrefresh(Task1_Win);
      write_window(Task1_Win, 6, 1, "Task 1 Window\n");

      wclear(Task2_Win);
      box(Task2_Win, 0, 0);
      wrefresh(Task2_Win);
      write_window(Task2_Win, 6, 1, "Task 2 Window\n");

      wclear(Task3_Win);
      box(Task3_Win, 0, 0);
      wrefresh(Task3_Win);
      write_window(Task3_Win, 6, 1, "Task 3 Window\n");

      wclear(Log_Win);

      write_window(Log_Win, 1, 5, "...Log Window...\n");
      write_window(Log_Win, "...Main program started...\n");
      write_window(Console_Win, 1, 1, "161-Ultima 2.0 # ");
      break;
    case 'h': // Display help.
      display_help(Console_Win);
      write_window(Console_Win, 8, 1, "161-Ultima 2.0 # ");
      break;
    case 'q': // q for quit!
      write_window(Log_Win, " Quitting...\n");
      write_window(Log_Win, " Call kill_task on all tasks.\n");

      /*
      for (int i = 0; i < 3; i++) {
        int this_task = my_scheduler->get_task_id();
        my_scheduler->kill_task();
        if (this_task == Task1) {
          write_window(Task1_Win, " Killed Task 1\n");
        } else if (this_task == Task2) {
          write_window(Task2_Win, " Killed Task 2\n");
        } else if (this_task == Task3) {
          write_window(Task3_Win, " Killed Task 3\n");
        }
      }
      */

      break;
    case ERR: // Gracefully handle errors.
      break;
    default: // Default behavior on unregistered inputs.
      sprintf(buffer, " %c", input);
      write_window(Console_Win, buffer);
      write_window(Console_Win, " -Invalid Command\n");
      write_window(Log_Win, buffer);
      write_window(Log_Win, " -Invalid Command\n");
      write_window(Console_Win, " 161-Ultima 2.0 #");
    }
  }

  write_window(Log_Win, " All tasks have now ended...\n");
  write_window(Log_Win, " Demo ended...\n");

  sleep(5);
  getch();
  endwin();
  return (0);
}
