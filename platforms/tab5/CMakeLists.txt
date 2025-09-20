cmake_minimum_required(VERSION 3.16)

set(_project_root ${CMAKE_CURRENT_LIST_DIR}/../..)
set(_dependency_root ${_project_root}/dependencies)
set(_third_party_deps
    mooncake
    mooncake_log
    smooth_ui_toolkit
    lvgl
)

set(_missing_deps)
foreach(_dep ${_third_party_deps})
    if(NOT EXISTS ${_dependency_root}/${_dep})
        list(APPEND _missing_deps ${_dep})
    endif()
endforeach()

if(_missing_deps)
    message(STATUS "Fetching managed dependencies: ${_missing_deps}")
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    execute_process(
        COMMAND ${Python3_EXECUTABLE} ${_project_root}/fetch_repos.py
        WORKING_DIRECTORY ${_project_root}
        RESULT_VARIABLE _fetch_result
    )
    if(NOT _fetch_result EQUAL 0)
        message(FATAL_ERROR "fetch_repos.py failed with exit code ${_fetch_result}")
    endif()

    set(_still_missing)
    foreach(_dep ${_third_party_deps})
        if(NOT EXISTS ${_dependency_root}/${_dep})
            list(APPEND _still_missing ${_dep})
        endif()
    endforeach()

    if(_still_missing)
        message(FATAL_ERROR "Missing dependencies after fetch: ${_still_missing}")
    endif()
endif()

set(EXTRA_COMPONENT_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/../../components
    ${_dependency_root}/mooncake
    ${_dependency_root}/mooncake_log
    ${_dependency_root}/smooth_ui_toolkit
    ${_dependency_root}/lvgl
)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(m5tab5_userdemo)

# Disable the linker flag added for non-contiguous SRAM regions to avoid
# discarding large .bss sections from the C++ runtime when linking the
# oversized factory app image.
idf_build_set_property(LINK_OPTIONS "-Wl,--no-enable-non-contiguous-regions" APPEND)
