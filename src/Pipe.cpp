/* Manpreet Kaur
 * 4-11-2026
 */
#include "Pipe.h" // Includes class definition from header
#include <sstream>

// Constructor
Pipe::Pipe(int id) {
  pfd = id; // Assign unique pipe ID

  read_ptr = 0;  // Reading starts at index 0
  write_ptr = 0; // Writing starts at index 0
  count = 0;     // Buffer is empty initially

  writer_task = -1; // No writer assigned yet
  reader_task = -1; // No reader assigned yet
}

// open_for_write()
bool Pipe::open_for_write(int task_id) {

  if (writer_task != -1) { // If already assigned
    return false;          // Reject new writer
  }

  writer_task = task_id; // Assign writer
  return true;
}

// open_for_read()
bool Pipe::open_for_read(int task_id) {

  if (reader_task != -1) { // If already assigned
    return false;          // Reject new reader
  }

  reader_task = task_id; // Assign reader
  return true;
}

// close functions
void Pipe::close_write() {
  writer_task = -1; // Remove writer
}

void Pipe::close_read() {
  reader_task = -1; // Remove reader
}

// write()
int Pipe::write(char data) {

  // 1. Check if writer exists
  if (writer_task == -1) {
    cout << "Error: No writer assigned\n";
    return -1;
  }

  // 2. Check if buffer is FULL
  if (count == PIPE_SIZE) {
    cout << "Error: Buffer FULL\n";
    return -1;
  }

  // 3. Insert data at write_ptr
  buffer[write_ptr] = data;

  // 4. Move write pointer (circular logic)
  write_ptr = (write_ptr + 1) % PIPE_SIZE;

  // 5. Increase count
  count++;

  return 0; // Success
}

// read()
int Pipe::read() {

  // 1. Check if reader exists
  if (reader_task == -1) {
    cout << "Error: No reader assigned\n";
    return -1;
  }

  // 2. Check if buffer is EMPTY
  if (count == 0) {
    cout << "Error: Buffer EMPTY\n";
    return -1;
  }

  // 3. Get data from buffer
  char data = buffer[read_ptr];

  // 4. Move read pointer (circular)
  read_ptr = (read_ptr + 1) % PIPE_SIZE;

  // 5. Decrease count
  count--;

  return data; // Return character
}

// is_empty()
bool Pipe::is_empty() { return (count == 0 ? true : false); }

// dump()
//  Return pipe state as a string
string Pipe::dump() {
  stringstream ss;

  ss << "\n===== PIPE STATE =====\n";
  ss << "Pipe ID: " << pfd << "\n";

  ss << "Buffer: ";
  for (int i = 0; i < count; i++) {
    int index = (read_ptr + i) % PIPE_SIZE;
    ss << buffer[index] << " ";
  }
  ss << "\n";

  ss << "Read Ptr: " << read_ptr << "\n";
  ss << "Write Ptr: " << write_ptr << "\n";
  ss << "Count: " << count << "\n";
  ss << "Writer Task: " << writer_task << "\n";
  ss << "Reader Task: " << reader_task << "\n";
  ss << "======================\n";

  return ss.str();
}
