cmake_minimum_required(VERSION 3.16)
set(EXTRA_COMPONENT_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/../../components
    ${CMAKE_CURRENT_LIST_DIR}/../../dependencies/mooncake
    ${CMAKE_CURRENT_LIST_DIR}/../../dependencies/mooncake_log
    ${CMAKE_CURRENT_LIST_DIR}/../../dependencies/smooth_ui_toolkit
    ${CMAKE_CURRENT_LIST_DIR}/../../dependencies/lvgl
)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(m5tab5_userdemo)
