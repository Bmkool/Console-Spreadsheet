cmake_minimum_required(VERSION 3.13.4)

project(CETITEC)
set(CMAKE_CXX_STANDARD 17)

# Grab dependencies (GTest)
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        703bd9caab50b139428cea1aaff9974ebee5742e # release-1.10.0
)
FetchContent_MakeAvailable(googletest)

# Configure Unit Tests
set(TEST_SRC_FILES  unit-tests/Tests.cpp)
add_executable(tests ${TEST_SRC_FILES})
target_link_libraries(tests gtest_main)
target_compile_options(tests PRIVATE -Wall -Wextra -g)

# Configure Spreadsheet Application
set(SRC_FILES  src/Spreadsheet.cpp
               src/Main.cpp
)
add_executable(console_spreadsheet ${SRC_FILES})
target_compile_options(console_spreadsheet PRIVATE -Wall -Wextra -g)