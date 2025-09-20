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
             esp_http_server chmorgan__esp-audio-player mooncake mooncake_log
            smooth_ui_toolkit power_monitor_ina226 esp_video esp_cam_sensor
             sensor_bmi270 espressif__usb_host_hid usb json
    EMBED_TXTFILES "../audio/canon_in_d.mp3" "../audio/startup_sfx.mp3" "../audio/shutdown_sfx.mp3")

