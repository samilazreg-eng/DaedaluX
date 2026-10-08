// The main of every test executable.

#include "TestEnvironment.hpp"

#include <gtest/gtest.h>

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  installTestEnvironment();
  return RUN_ALL_TESTS();
}
