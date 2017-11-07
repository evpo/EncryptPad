#include <iostream>

#include "gtest/gtest.h"
#include "logger_init.h"

GTEST_API_ int main(int argc, char **argv) {
  std::cout << "Running main() from main.cpp\n";

  if(argc > 1)
      LibEncryptMsg::InitLogger(argv[1]);

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
