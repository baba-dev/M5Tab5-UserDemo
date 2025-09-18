cmake_minimum_required(VERSION 3.20)
project(host_tests CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

set(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")

add_library(app_core
    ${REPO_ROOT}/components/core/src/ringbuf.cpp
    ${REPO_ROOT}/components/core/src/settings.cpp
)
target_include_directories(app_core
    PUBLIC
        ${REPO_ROOT}/components/core/include
)

find_package(GTest CONFIG QUIET)
if(NOT GTest_FOUND)
  include(FetchContent)
  FetchContent_Declare(gtest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0)
  FetchContent_MakeAvailable(gtest)
endif()

add_executable(unit_tests
  unit/test_ringbuf.cpp
  unit/test_settings.cpp
)
target_link_libraries(unit_tests PRIVATE app_core GTest::gtest GTest::gtest_main)
add_test(NAME unit_tests COMMAND unit_tests)

add_subdirectory(ui)
