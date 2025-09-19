cmake_minimum_required(VERSION 3.10)
project(MC_APP_BOILERPLATE VERSION 0.1.0 LANGUAGES C CXX)

set(PROJECT_EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/components" CACHE PATH
    "Common ESP-IDF components" FORCE)

include(platforms/desktop/CMakeLists.txt)
