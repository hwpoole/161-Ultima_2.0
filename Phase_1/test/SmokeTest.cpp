/* SmokeTest
 * Testing prototype (demo) for ULTIMA 2.0
 * CSCI-C435 Operating Systems 1
 *
 * Hunter William Poole
 * 3-14-2026
 *
 * SmokeTest.cpp
 * This is an exceedingly simple test suite, presented as a prototype of how
 * CMake and GoogleTest would interact in this project.
 *
 * Currently, it tests that tests work: you may run this program as a test,
 * using ctest, and you will find that (1 == 1) is TRUE.
 */

#include "gtest/gtest.h"

TEST(DemoTest, OneEqualsOne) { EXPECT_EQ(1, 1); }
