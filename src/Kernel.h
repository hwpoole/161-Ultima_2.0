/* Kernel Class Header File
 * 161-Ultima 2.0
 *
 * Seeing a need to coordinate resources between classes in a
 * "behind-the-scenes" way, this Kernel class was born.
 *
 * Hunter Poole
 * 04-05-2026
 */

#pragma once

class Kernel {
private:
  Kernel();

  ~Kernel() = default;

  // Delete the copy constructor.
  Kernel(const Kernel &) = delete;

  // Delete the assignment operator.
  Kernel &operator=(const Kernel &) = delete;
};
