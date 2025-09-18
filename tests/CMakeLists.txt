cmake_minimum_required(VERSION 3.20)
project(host_tests C CXX)

# Fast compile: unity + low optimization
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_UNITY_BUILD ON)
set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-w)

# Option to build only the rooms page test
option(ROMS_ONLY "Build only Rooms Page tests" ON)

# ---- Minimal LVGL: headless, small ----
add_library(lvgl_config INTERFACE)
target_compile_definitions(lvgl_config INTERFACE
  LV_CONF_INCLUDE_SIMPLE
)
target_include_directories(lvgl_config INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

include(FetchContent)
FetchContent_Declare(lvgl
  GIT_REPOSITORY https://github.com/lvgl/lvgl.git
  GIT_TAG v9.1.0
)
FetchContent_MakeAvailable(lvgl)
if (TARGET lvgl)
  set_property(TARGET lvgl PROPERTY UNITY_BUILD OFF)
  target_link_libraries(lvgl PUBLIC lvgl_config)
endif()

# Your UI code under test (keep this minimal!)
add_library(ui_rooms_under_test
  ../custom/ui/pages/ui_page_rooms.c
  ../custom/ui/ui_wallpaper.c
)
target_include_directories(ui_rooms_under_test PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../custom/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/../custom/ui/pages
)
target_link_libraries(ui_rooms_under_test PUBLIC lvgl::lvgl lvgl_config)

# ---- GoogleTest from system packages ----
find_package(GTest QUIET)
if (NOT GTest_FOUND)
  message(STATUS "System GTest not found; falling back to FetchContent (slower).")
  include(FetchContent)
  FetchContent_Declare(gtest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
  )
  FetchContent_MakeAvailable(gtest)
endif()

# ---- Rooms page test ----
add_executable(rooms_page_test
  ui/test_rooms_snapshot.cpp
)
target_link_libraries(rooms_page_test PRIVATE
  ui_rooms_under_test
  GTest::gtest GTest::gtest_main
)

enable_testing()
add_test(NAME rooms_page_test COMMAND rooms_page_test)
