cmake_minimum_required(VERSION 3.20)
project(host_tests C CXX)

# ---- Build settings ----
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_UNITY_BUILD ON)
set(CMAKE_BUILD_TYPE Debug)
add_compile_options(-w)

# Build only UI page tests when ON (CI sets this); when OFF we also build core unit tests
option(ROMS_ONLY "Build only Rooms Page tests" ON)
option(BUILD_ROOMS_TESTS "Build Rooms Page snapshot test" OFF)

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

set(LV_BUILD_EXAMPLES 0 CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES 0 CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(lvgl
  GIT_REPOSITORY https://github.com/lvgl/lvgl.git
  GIT_TAG v9.1.0
)
FetchContent_MakeAvailable(lvgl)
if (TARGET lvgl)
  target_compile_definitions(lvgl PUBLIC LV_BUILD_EXAMPLES=0)
  # Avoid Unity on third-party to keep compile sane
  set_property(TARGET lvgl PROPERTY UNITY_BUILD OFF)
  target_link_libraries(lvgl PUBLIC lvgl_config)
endif()
if (TARGET lvgl_examples)
  set_target_properties(lvgl_examples PROPERTIES EXCLUDE_FROM_ALL TRUE EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif()
if (TARGET lvgl_demos)
  set_target_properties(lvgl_demos PROPERTIES EXCLUDE_FROM_ALL TRUE EXCLUDE_FROM_DEFAULT_BUILD TRUE)
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
# UI asset stubs
# -----------------------------
add_library(ui_assets_stub
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/stubs/assets/launcher_bg_stub.c
)
target_include_directories(ui_assets_stub PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/stubs
)
target_link_libraries(ui_assets_stub PUBLIC lvgl::lvgl lvgl_config)

# -----------------------------
# Rooms page UI under test (optional via BUILD_ROOMS_TESTS)
# -----------------------------
if (BUILD_ROOMS_TESTS)
  add_library(ui_rooms_under_test
    ${REPO_ROOT}/custom/ui/pages/ui_page_rooms.c
    ${REPO_ROOT}/custom/ui/pages/ui_rooms_model.c
    ${REPO_ROOT}/custom/ui/widgets/ui_room_card.c
    ${REPO_ROOT}/custom/ui/ui_theme.c
    ${REPO_ROOT}/custom/ui/ui_wallpaper.c
    ${REPO_ROOT}/custom/integration/rooms_provider.c
  )
  target_include_directories(ui_rooms_under_test PUBLIC
    ${REPO_ROOT}/custom/ui
    ${REPO_ROOT}/custom/ui/pages
    ${REPO_ROOT}/custom/ui/widgets
    ${REPO_ROOT}/custom/integration
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/stubs
  )
  target_link_libraries(ui_rooms_under_test PUBLIC lvgl::lvgl lvgl_config ui_assets_stub)
  # Some tests stub LVGL event bounds; keep compatible with our test doubles
  target_compile_definitions(ui_rooms_under_test PUBLIC LV_EVENT_LAST=2000)

  add_executable(rooms_page_test
    ui/test_rooms_snapshot.cpp
  )
  target_link_libraries(rooms_page_test PRIVATE
    ui_rooms_under_test
    GTest::gtest GTest::gtest_main
  )
  add_test(NAME rooms_page_test COMMAND rooms_page_test)
endif()

# -----------------------------
# Settings page UI under test (always built)
# -----------------------------
add_library(ui_settings_under_test
  ${REPO_ROOT}/custom/ui/pages/ui_page_settings.c
  ${REPO_ROOT}/custom/ui/ui_wallpaper.c
  ${REPO_ROOT}/custom/ui/ui_theme.c
)
target_include_directories(ui_settings_under_test PUBLIC
  ${REPO_ROOT}/custom/ui
  ${REPO_ROOT}/custom/ui/pages
  ${REPO_ROOT}/custom/integration
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/stubs
)
target_link_libraries(ui_settings_under_test PUBLIC lvgl::lvgl lvgl_config ui_assets_stub)

add_executable(settings_page_status_test
  ui/test_settings_status.cpp
)
target_link_libraries(settings_page_status_test PRIVATE
  ui_settings_under_test
  GTest::gtest GTest::gtest_main
)
add_test(NAME settings_page_status_test COMMAND settings_page_status_test)

# -----------------------------
# Media page UI under test
# -----------------------------
add_library(ui_media_under_test
  ${REPO_ROOT}/custom/ui/pages/ui_page_media.c
  ${REPO_ROOT}/custom/ui/widgets/ui_room_card.c
  ${REPO_ROOT}/custom/ui/ui_theme.c
  ${REPO_ROOT}/custom/ui/ui_wallpaper.c
)
target_include_directories(ui_media_under_test PUBLIC
  ${REPO_ROOT}/custom/ui
  ${REPO_ROOT}/custom/ui/pages
)
target_compile_definitions(ui_media_under_test PUBLIC LV_EVENT_LAST=2000)
target_link_libraries(ui_media_under_test PUBLIC lvgl::lvgl lvgl_config ui_assets_stub)

add_executable(media_page_test
  ui/media_page_test.c
)
target_link_libraries(media_page_test PRIVATE
  ui_media_under_test
)
add_test(NAME media_page_test COMMAND media_page_test)

# -----------------------------
# Core library + unit tests (optional when ROMS_ONLY=OFF)
# -----------------------------
if (NOT ROMS_ONLY)
  add_library(settings_core_under_test
    ${REPO_ROOT}/components/settings_core/src/app_cfg.c
    ${REPO_ROOT}/components/backup_server/src/backup_format.c
  )
  target_include_directories(settings_core_under_test PUBLIC
    ${REPO_ROOT}/components/settings_core/include
    ${REPO_ROOT}/components/backup_server/include
    ${CMAKE_CURRENT_SOURCE_DIR}/unit/stubs
  )

  add_library(weather_formatter_under_test
    ${REPO_ROOT}/custom/integration/weather_formatter.cpp
  )
  target_include_directories(weather_formatter_under_test PUBLIC
    ${REPO_ROOT}/custom
  )

  add_executable(unit_tests
    unit/test_app_cfg.cpp
    unit/test_weather_formatter.cpp
  )
  target_link_libraries(unit_tests PRIVATE
    settings_core_under_test
    weather_formatter_under_test
    GTest::gtest
    GTest::gtest_main
  )
  add_test(NAME unit_tests COMMAND unit_tests)
endif()
