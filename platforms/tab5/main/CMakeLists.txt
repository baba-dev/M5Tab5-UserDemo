file(GLOB_RECURSE APP_LAYER_SRCS
    ../../../app/*.c
    ../../../app/*.cc
    ../../../app/*.cpp
    ../../../custom/*.c
    ../../../custom/*.cc
    ../../../custom/*.cpp
)

set(APP_LAYER_INCS
    ../../../app
    ../../../custom
)

file(GLOB_RECURSE MY_HAL_SRCS
    ./hal/*.c
    ./hal/*.cc
    ./hal/*.cpp
)

idf_component_register(
    SRCS "app_main.cpp" ${APP_LAYER_SRCS} ${MY_HAL_SRCS}
    INCLUDE_DIRS "." ${APP_LAYER_INCS}
    REQUIRES backup_server connection_tester diag net_sntp ota_update settings_core
             settings_ui m5stack_tab5 esp_wifi esp_netif esp_event nvs_flash
             esp_http_server
    EMBED_TXTFILES "../audio/canon_in_d.mp3" "../audio/startup_sfx.mp3" "../audio/shutdown_sfx.mp3")

set(CUSTOM_ASSETS_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../custom/assets")
get_filename_component(CUSTOM_ASSETS_DIR "${CUSTOM_ASSETS_DIR}" REALPATH)

set(CUSTOM_ASSETS_IMAGE_ROOT "${CMAKE_CURRENT_BINARY_DIR}/spiffs_custom_assets")
set(CUSTOM_ASSETS_STAMP "${CUSTOM_ASSETS_IMAGE_ROOT}/.staged")
file(GLOB_RECURSE CUSTOM_ASSET_FILES CONFIGURE_DEPENDS LIST_DIRECTORIES false
     "${CUSTOM_ASSETS_DIR}/*")

add_custom_command(
    OUTPUT "${CUSTOM_ASSETS_STAMP}"
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${CUSTOM_ASSETS_IMAGE_ROOT}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CUSTOM_ASSETS_IMAGE_ROOT}/custom"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CUSTOM_ASSETS_DIR}"
                                    "${CUSTOM_ASSETS_IMAGE_ROOT}/custom/assets"
    COMMAND ${CMAKE_COMMAND} -E touch "${CUSTOM_ASSETS_STAMP}"
    DEPENDS ${CUSTOM_ASSET_FILES}
    COMMENT "Staging custom assets for SPIFFS image"
)

spiffs_create_partition_image(storage "${CUSTOM_ASSETS_IMAGE_ROOT}" FLASH_IN_PROJECT
                              DEPENDS "${CUSTOM_ASSETS_STAMP}")
