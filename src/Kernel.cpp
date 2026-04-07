/* Kernel Class Implementation File
 *
 * Hunter Poole
 * 04-05-2026
 */

#include "Kernel.h"

Kernel::Kernel() {
  scheduler = new Scheduler();
  Semaphore::set_scheduler(scheduler);
}

Kernel *Kernel::GetInstance() {
  if (KernelPtr == nullptr) {
    KernelPtr = new Kernel();
  }
  return KernelPtr;
}
