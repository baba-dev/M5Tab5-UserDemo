/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define APP_CFG_VERSION 1U

#define APP_CFG_MAX_URL_LEN        128U
#define APP_CFG_MAX_TOKEN_LEN      128U
#define APP_CFG_MAX_TOPIC_LEN      128U
#define APP_CFG_MAX_NAME_LEN       64U
#define APP_CFG_MAX_USERNAME_LEN   64U
#define APP_CFG_MAX_PASSWORD_LEN   64U
#define APP_CFG_MAX_HOSTNAME_LEN   32U
#define APP_CFG_MAX_WIFI_SSID_LEN  32U
#define APP_CFG_MAX_WIFI_PASS_LEN  64U
#define APP_CFG_MAX_IP_LEN         16U
#define APP_CFG_MAX_TIMEZONE_LEN   32U
#define APP_CFG_MAX_NTP_SERVER_LEN 64U

    typedef enum
    {
        APP_CFG_UI_THEME_LIGHT = 0,
        APP_CFG_UI_THEME_DARK  = 1,
        APP_CFG_UI_THEME_AUTO  = 2,
    } app_cfg_ui_theme_t;

    typedef struct
    {
        bool enabled;
        char url[APP_CFG_MAX_URL_LEN];
        char token[APP_CFG_MAX_TOKEN_LEN];
    } app_cfg_home_assistant_t;

    typedef struct
    {
        bool enabled;
        char url[APP_CFG_MAX_URL_LEN];
        char camera_name[APP_CFG_MAX_NAME_LEN];
        bool snapshots_enabled;
    } app_cfg_frigate_t;

    typedef struct
    {
        bool enabled;
        char broker_uri[APP_CFG_MAX_URL_LEN];
        char client_id[APP_CFG_MAX_NAME_LEN];
        char username[APP_CFG_MAX_USERNAME_LEN];
        char password[APP_CFG_MAX_PASSWORD_LEN];
        bool use_tls;
        bool ha_discovery;
    } app_cfg_mqtt_t;

    typedef struct
    {
        app_cfg_ui_theme_t theme;
        uint8_t            brightness;             /* 0-100 percent */
        uint16_t           screen_timeout_seconds; /* Seconds */
    } app_cfg_ui_t;

    typedef struct
    {
        char ssid[APP_CFG_MAX_WIFI_SSID_LEN + 1U];
        char password[APP_CFG_MAX_WIFI_PASS_LEN + 1U];
        char hostname[APP_CFG_MAX_HOSTNAME_LEN + 1U];
        bool use_dhcp;
        char static_ip[APP_CFG_MAX_IP_LEN];
        char gateway[APP_CFG_MAX_IP_LEN];
        char netmask[APP_CFG_MAX_IP_LEN];
        char dns_primary[APP_CFG_MAX_IP_LEN];
        char dns_secondary[APP_CFG_MAX_IP_LEN];
        char timezone[APP_CFG_MAX_TIMEZONE_LEN];
        char ntp_server[APP_CFG_MAX_NTP_SERVER_LEN];
        bool sntp_sync_enabled;
    } app_cfg_network_t;

    typedef struct
    {
        bool child_lock;
        bool disable_wifi;
        bool allow_ota;
        bool diagnostics_opt_in;
    } app_cfg_safety_t;

    typedef struct
    {
        uint32_t                 cfg_ver;
        app_cfg_home_assistant_t home_assistant;
        app_cfg_frigate_t        frigate;
        app_cfg_mqtt_t           mqtt;
        app_cfg_ui_t             ui;
        app_cfg_network_t        network;
        app_cfg_safety_t         safety;
    } app_cfg_t;

    typedef esp_err_t (*app_cfg_migration_fn_t)(uint32_t from_version, app_cfg_t* cfg);

    typedef esp_err_t (*app_cfg_storage_read_fn_t)(void* ctx, void* buffer, size_t* length);
    typedef esp_err_t (*app_cfg_storage_write_fn_t)(void* ctx, const void* buffer, size_t length);
    typedef esp_err_t (*app_cfg_storage_erase_fn_t)(void* ctx);

    typedef struct
    {
        void*                      ctx;
        app_cfg_storage_read_fn_t  read;
        app_cfg_storage_write_fn_t write;
        app_cfg_storage_erase_fn_t erase;
    } app_cfg_storage_backend_t;

    void      app_cfg_set_defaults(app_cfg_t* cfg);
    esp_err_t app_cfg_validate(const app_cfg_t* cfg);
    esp_err_t app_cfg_load(app_cfg_t* cfg);
    esp_err_t app_cfg_save(app_cfg_t* cfg);
    esp_err_t app_cfg_reset(app_cfg_t* cfg);

    esp_err_t app_cfg_register_storage_backend(const app_cfg_storage_backend_t* backend);
    const app_cfg_storage_backend_t* app_cfg_get_storage_backend(void);
    esp_err_t                        app_cfg_storage_init_default(void);

    void app_cfg_register_migration_handler(app_cfg_migration_fn_t fn);

    /* NVS-backed storage helpers */
    esp_err_t app_cfg_use_nvs_namespace(const char* ns);
    esp_err_t app_cfg_erase_persisted(void);

#ifdef __cplusplus
}
#endif
