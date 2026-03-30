/* 161-Ultima 2.0 Phase 1 Demo file.
 *
 * Hunter Poole
 * 03-29-2026
 */

#include "Sched.h"
#include "Sema.h"
#include <cstring>
#include <ncurses.h>
#include <stdarg.h>
#include <unistd.h>

using namespace std;

Scheduler *my_scheduler = new Scheduler();
Semaphore *my_semaphore = new Semaphore("The Thing");

int Task1 = my_scheduler->create_task();
int Task2 = my_scheduler->create_task();
int Task3 = my_scheduler->create_task();

WINDOW *create_window(int height, int width, int starty, int startx) {
  WINDOW *Win;

  Win = newwin(height, width, starty, startx);
  scrollok(Win, TRUE);
  scroll(Win);
  box(Win, 0, 0);
  wrefresh(Win);

  return Win;
}

void write_window(WINDOW *Win, const char *text) {
  wprintw(Win, text);
  box(Win, 0, 0);
  wrefresh(Win);
}

void write_window(WINDOW *Win, int y, int x, const char *text) {
  mvwprintw(Win, y, x, text);
  box(Win, 0, 0);
  wrefresh(Win);
}

void display_help(WINDOW *Win) {
  wclear(Win);
  write_window(Win, 1, 1, "...Help...");
  write_window(Win, 2, 1, "1= Kill T1");
  write_window(Win, 3, 1, "2= Kill T2");
  write_window(Win, 4, 1, "3= Kill T3");
  write_window(Win, 5, 1, "c= clear screen");
  write_window(Win, 6, 1, "h= help screen");
  write_window(Win, 7, 1, "q= Quit");
}

void *fake_work(WINDOW *task1_window, WINDOW *task2_window,
                WINDOW *task3_window) {
  int current_task = my_scheduler->get_task_id();
  char buffer[256];
  WINDOW *current_window = nullptr;
  string TaskName;

  if (current_task == Task1) {
    current_window = task1_window;
    TaskName = "Task 1";
  } else if (current_task == Task2) {
    current_window = task2_window;
    TaskName = "Task 2";
  } else if (current_task == Task3) {
    current_window = task3_window;
    TaskName = "Task 3";
  }

  char task[7];
  strcpy(task, TaskName.c_str());

  for (int i = 0; i < 10; i++) {
    sprintf(buffer, " %s running\n", task);
    write_window(current_window, buffer);
    sleep(1);
  }

  sprintf(buffer, " %s yielding\n", task);
  write_window(current_window, buffer);
  sleep(1);
  my_scheduler->yield();

  return (NULL);
}

void *fake_work_with_semaphore() { return (NULL); }

int main() {
  Semaphore::set_scheduler(my_scheduler);

  initscr();

  WINDOW *Heading_Win = newwin(12, 80, 3, 2);
  box(Heading_Win, 0, 0);
  mvwprintw(Heading_Win, 2, 28, "161-ULTIMA 2.0 PHASE 1 DEMO");

  mvwprintw(Heading_Win, 4, 2, "Starting demo...");
  mvwprintw(Heading_Win, 5, 2, "Starting Task 1...");
  mvwprintw(Heading_Win, 6, 2, "Starting Task 2...");
  mvwprintw(Heading_Win, 7, 2, "Starting Task 3...");
  mvwprintw(Heading_Win, 9, 2, "Press 'q' or Ctrl-C to exit the program...");
  wrefresh(Heading_Win);

  WINDOW *Log_Win = create_window(10, 60, 30, 2);
  write_window(Log_Win, 1, 5, "...Log Window...\n");
  write_window(Log_Win, "...Main program started...\n");

  WINDOW *Console_Win = create_window(10, 20, 30, 62);
  write_window(Console_Win, 1, 1, "...Console...");
  write_window(Console_Win, 2, 1, "161-Ultima 2.0 #");

  WINDOW *Task1_Win = create_window(15, 25, 15, 2);
  write_window(Task1_Win, 6, 1, "Task 1 Window\n");
  WINDOW *Task2_Win = create_window(15, 25, 15, 30);
  write_window(Task2_Win, 6, 1, "Task 2 Window\n");
  WINDOW *Task3_Win = create_window(15, 25, 15, 57);
  write_window(Task3_Win, 6, 1, "Task 3 Window\n");

  for (int i = 0; i < 3; i++) {
    fake_work(Task1_Win, Task2_Win, Task3_Win);
  }

  // I/O processing
  cbreak();
  noecho();
  nodelay(Console_Win, TRUE);
  keypad(Console_Win, TRUE);

  char buffer[256];
  int input = -1;
}
