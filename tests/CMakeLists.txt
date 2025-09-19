cmake_minimum_required(VERSION 3.20)
project(host_tests C CXX)

# ---- Build settings ----
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_UNITY_BUILD ON)
set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-w)

# Build only Rooms Page tests when ON (CI sets this); when OFF we also build core unit tests
option(ROMS_ONLY "Build only Rooms Page tests" ON)

enable_testing()

# Repo root helper for shared sources/headers
set(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")

# -----------------------------
# LVGL (for UI tests, headless)
# -----------------------------
add_library(lvgl_config INTERFACE)
target_compile_definitions(lvgl_config INTERFACE LV_CONF_INCLUDE_SIMPLE)
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
  # Avoid Unity on third-party to keep compile sane
  set_property(TARGET lvgl PROPERTY UNITY_BUILD OFF)
  target_link_libraries(lvgl PUBLIC lvgl_config)
endif()

# -----------------------------
# GoogleTest (system first)
# -----------------------------
find_package(GTest QUIET)
if (NOT GTest_FOUND)
  message(STATUS "System GTest not found; falling back to FetchContent.")
  include(FetchContent)
  FetchContent_Declare(gtest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
  )
  FetchContent_MakeAvailable(gtest)
endif()

# -----------------------------
# Rooms page UI under test
# -----------------------------
add_library(ui_rooms_under_test
  ${REPO_ROOT}/custom/ui/pages/ui_page_rooms.c
  ${REPO_ROOT}/custom/ui/ui_wallpaper.c
)
target_include_directories(ui_rooms_under_test PUBLIC
  ${REPO_ROOT}/custom/ui
  ${REPO_ROOT}/custom/ui/pages
)
target_link_libraries(ui_rooms_under_test PUBLIC lvgl::lvgl lvgl_config)

add_executable(rooms_page_test
  ui/test_rooms_snapshot.cpp
)
target_link_libraries(rooms_page_test PRIVATE
  ui_rooms_under_test
  GTest::gtest GTest::gtest_main
)
add_test(NAME rooms_page_test COMMAND rooms_page_test)

# -----------------------------
# Core library + unit tests (optional when ROMS_ONLY=OFF)
# -----------------------------
if (NOT ROMS_ONLY)
  add_library(app_core
    ${REPO_ROOT}/components/core/src/ringbuf.cpp
    ${REPO_ROOT}/components/core/src/settings.cpp
  )
  target_include_directories(app_core PUBLIC
    ${REPO_ROOT}/components/core/include
  )

  add_library(settings_core_under_test
    ${REPO_ROOT}/components/settings_core/src/app_cfg.c
    ${REPO_ROOT}/components/backup_server/src/backup_format.c
  )
  target_include_directories(settings_core_under_test PUBLIC
    ${REPO_ROOT}/components/settings_core/include
    ${REPO_ROOT}/components/backup_server/include
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/stubs
  )

  add_executable(unit_tests
    unit/test_ringbuf.cpp
    unit/test_settings.cpp
    unit/test_app_cfg.cpp
  )
  target_link_libraries(unit_tests PRIVATE app_core settings_core_under_test GTest::gtest GTest::gtest_main)
  add_test(NAME unit_tests COMMAND unit_tests)
endif()
