project(ui_snapshot_tests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")
set(LV_CONF_PATH "${REPO_ROOT}/lv_conf.h" CACHE STRING "LVGL configuration" FORCE)
set(LV_CONF_BUILD_DISABLE_THORVG_INTERNAL ON CACHE BOOL "Disable bundled ThorVG" FORCE)
set(LV_CONF_BUILD_DISABLE_EXAMPLES ON CACHE BOOL "Skip LVGL examples" FORCE)
set(LV_CONF_BUILD_DISABLE_DEMOS ON CACHE BOOL "Skip LVGL demos" FORCE)

include(FetchContent)

FetchContent_Declare(lvgl
  GIT_REPOSITORY https://github.com/lvgl/lvgl.git
  GIT_TAG v9.2.0
)
FetchContent_MakeAvailable(lvgl)

add_library(lodepng_lib STATIC
  third_party/lodepng.cpp
)
target_include_directories(lodepng_lib PUBLIC ${CMAKE_CURRENT_LIST_DIR}/third_party)

add_executable(ui_snapshot_tests
  test_ui_snapshot.cpp
)

target_include_directories(ui_snapshot_tests
  PRIVATE
    ${REPO_ROOT}
)

target_compile_definitions(ui_snapshot_tests PRIVATE LV_CONF_INCLUDE_SIMPLE=1)

target_link_libraries(ui_snapshot_tests
  PRIVATE
    lvgl::lvgl
    lodepng_lib
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME ui_snapshot_tests COMMAND ui_snapshot_tests)
