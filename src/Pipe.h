/* Manpreet Kaur
 * 4-10-2026
 */

#ifndef PIPE_H // Prevents multiple inclusion of this header file
#define PIPE_H

#include <iostream> // For cout (used in dump function)
using namespace std;

#define PIPE_SIZE 32 // Maximum size of the pipe buffer (as per assignment)

class Pipe {
private:
  int pfd; // Pipe File Descriptor (unique ID for each pipe)

  char buffer[PIPE_SIZE]; // Fixed-size buffer to store data (like a queue)

  int read_ptr;  // Points to position from where data will be read
  int write_ptr; // Points to position where new data will be written

  int count; // Tracks how many elements are currently in buffer
             // Needed to detect FULL and EMPTY conditions

  int writer_task; // Stores Task ID of the writer (-1 means none assigned)
  int reader_task; // Stores Task ID of the reader (-1 means none assigned)

public:
  // Constructor
  // Initializes pipe with a unique ID and sets default values
  Pipe(int id);

  // Assigns a task as writer
  // Only ONE writer allowed
  // Returns true if successful, false if writer already exists
  bool open_for_write(int task_id);

  // Assigns a task as reader
  // Only ONE reader allowed
  // Returns true if successful, false if reader already exists
  bool open_for_read(int task_id);

  // Removes writer from pipe (sets it back to -1)
  void close_write();

  // Removes reader from pipe (sets it back to -1)
  void close_read();

  // Writes a single character into the pipe
  // Returns 0 if success
  // Returns -1 if buffer is FULL or no writer assigned
  int write(char data);

  // Reads a single character from the pipe
  // Returns ASCII value of character if success
  // Returns -1 if buffer is EMPTY or no reader assigned
  int read();

  // Prints current state of the pipe (for debugging/demo)
  void dump();
};

#endif
