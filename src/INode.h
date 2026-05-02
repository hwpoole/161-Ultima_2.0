/* INode header file.
 * Ultima 2.0
 *
 * This INode class header file shows the private and public resources of the
 * INode class.
 */

#include <time.h>

using namespace std;

class INode {
private:
  char filename[8], permission[4];
  int owner_task_id, starting_block, size;
  unsigned int blocks[4];
  time_t creation, last_modified;

public:
};
