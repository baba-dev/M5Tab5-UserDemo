/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include <string.h>
#include <nvs.h>

#include "settings_core/app_cfg.h"

#ifndef ESP_STATIC_ASSERT
#    define ESP_STATIC_ASSERT(CONDITION, MESSAGE) _Static_assert((CONDITION), #MESSAGE)
#endif

#define APP_CFG_MIN_BRIGHTNESS  1U
#define APP_CFG_MAX_BRIGHTNESS  100U
#define APP_CFG_MIN_TIMEOUT_SEC 5U
#define APP_CFG_MAX_TIMEOUT_SEC 7200U

static const app_cfg_storage_backend_t* s_backend           = NULL;
static app_cfg_migration_fn_t           s_migration_handler = NULL;

static size_t safe_strlen(const char* value, size_t max_len)
{
    if (!value)
    {
        return 0U;
    }
    size_t len = strnlen(value, max_len + 1U);
    return (len > max_len) ? max_len + 1U : len;
}

static void copy_string(char* dest, size_t dest_len, const char* src)
{
    if (!dest || dest_len == 0U)
    {
        return;
    }
    if (!src)
    {
        dest[0] = '\0';
        return;
    }
    size_t max_copy = dest_len - 1U;
    size_t length   = strnlen(src, max_copy);
    memcpy(dest, src, length);
    dest[length] = '\0';
}

static bool is_string_valid(const char* value, size_t max_len)
{
    return safe_strlen(value, max_len) <= max_len;
}

const app_cfg_storage_backend_t* app_cfg_default_backend(void) __attribute__((weak));
const app_cfg_storage_backend_t* app_cfg_default_backend(void)
{
    return NULL;
}

void app_cfg_set_defaults(app_cfg_t* cfg)
{
    if (!cfg)
    {
        return;
    }

    memset(cfg, 0, sizeof(*cfg));
    cfg->cfg_ver                = APP_CFG_VERSION;
    cfg->home_assistant.enabled = false;
    copy_string(
        cfg->home_assistant.url, sizeof(cfg->home_assistant.url), "https://homeassistant.local");
    copy_string(cfg->home_assistant.token, sizeof(cfg->home_assistant.token), "");

    cfg->frigate.enabled = false;
    copy_string(cfg->frigate.url, sizeof(cfg->frigate.url), "https://frigate.local");
    copy_string(cfg->frigate.camera_name, sizeof(cfg->frigate.camera_name), "front-door");
    cfg->frigate.snapshots_enabled = true;

    cfg->mqtt.enabled = false;
    copy_string(cfg->mqtt.broker_uri, sizeof(cfg->mqtt.broker_uri), "mqtts://broker.local:8883");
    copy_string(cfg->mqtt.client_id, sizeof(cfg->mqtt.client_id), "m5tab5");
    copy_string(cfg->mqtt.username, sizeof(cfg->mqtt.username), "");
    copy_string(cfg->mqtt.password, sizeof(cfg->mqtt.password), "");
    cfg->mqtt.use_tls      = true;
    cfg->mqtt.ha_discovery = true;

    cfg->ui.theme                  = APP_CFG_UI_THEME_AUTO;
    cfg->ui.brightness             = 80U;
    cfg->ui.screen_timeout_seconds = 60U;

    copy_string(cfg->network.ssid, sizeof(cfg->network.ssid), "");
    copy_string(cfg->network.password, sizeof(cfg->network.password), "");
    copy_string(cfg->network.hostname, sizeof(cfg->network.hostname), "m5tab5");
    cfg->network.use_dhcp = true;
    copy_string(cfg->network.static_ip, sizeof(cfg->network.static_ip), "");
    copy_string(cfg->network.gateway, sizeof(cfg->network.gateway), "");
    copy_string(cfg->network.netmask, sizeof(cfg->network.netmask), "");
    copy_string(cfg->network.dns_primary, sizeof(cfg->network.dns_primary), "");
    copy_string(cfg->network.dns_secondary, sizeof(cfg->network.dns_secondary), "");
    copy_string(cfg->network.timezone, sizeof(cfg->network.timezone), "Etc/UTC");
    copy_string(cfg->network.ntp_server, sizeof(cfg->network.ntp_server), "pool.ntp.org");
    cfg->network.sntp_sync_enabled = true;

    cfg->safety.child_lock         = false;
    cfg->safety.disable_wifi       = false;
    cfg->safety.allow_ota          = true;
    cfg->safety.diagnostics_opt_in = false;
}

esp_err_t app_cfg_validate(const app_cfg_t* cfg)
{
    if (!cfg)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (cfg->ui.brightness < APP_CFG_MIN_BRIGHTNESS || cfg->ui.brightness > APP_CFG_MAX_BRIGHTNESS)
    {
        return ESP_ERR_INVALID_ARG;
    }
    if (cfg->ui.screen_timeout_seconds < APP_CFG_MIN_TIMEOUT_SEC
        || cfg->ui.screen_timeout_seconds > APP_CFG_MAX_TIMEOUT_SEC)
    {
        return ESP_ERR_INVALID_ARG;
    }
    if (cfg->ui.theme > APP_CFG_UI_THEME_AUTO)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (!is_string_valid(cfg->home_assistant.url, APP_CFG_MAX_URL_LEN)
        || !is_string_valid(cfg->home_assistant.token, APP_CFG_MAX_TOKEN_LEN))
    {
        return ESP_ERR_INVALID_ARG;
    }
    if (!is_string_valid(cfg->frigate.url, APP_CFG_MAX_URL_LEN)
        || !is_string_valid(cfg->frigate.camera_name, APP_CFG_MAX_NAME_LEN))
    {
        return ESP_ERR_INVALID_ARG;
    }
    if (!is_string_valid(cfg->mqtt.broker_uri, APP_CFG_MAX_URL_LEN)
        || !is_string_valid(cfg->mqtt.client_id, APP_CFG_MAX_NAME_LEN)
        || !is_string_valid(cfg->mqtt.username, APP_CFG_MAX_USERNAME_LEN)
        || !is_string_valid(cfg->mqtt.password, APP_CFG_MAX_PASSWORD_LEN))
    {
        return ESP_ERR_INVALID_ARG;
    }
    if (!is_string_valid(cfg->network.ssid, APP_CFG_MAX_WIFI_SSID_LEN)
        || !is_string_valid(cfg->network.password, APP_CFG_MAX_WIFI_PASS_LEN)
        || !is_string_valid(cfg->network.hostname, APP_CFG_MAX_HOSTNAME_LEN)
        || !is_string_valid(cfg->network.static_ip, APP_CFG_MAX_IP_LEN)
        || !is_string_valid(cfg->network.gateway, APP_CFG_MAX_IP_LEN)
        || !is_string_valid(cfg->network.netmask, APP_CFG_MAX_IP_LEN)
        || !is_string_valid(cfg->network.dns_primary, APP_CFG_MAX_IP_LEN)
        || !is_string_valid(cfg->network.dns_secondary, APP_CFG_MAX_IP_LEN)
        || !is_string_valid(cfg->network.timezone, APP_CFG_MAX_TIMEZONE_LEN)
        || !is_string_valid(cfg->network.ntp_server, APP_CFG_MAX_NTP_SERVER_LEN))
    {
        return ESP_ERR_INVALID_ARG;
    }

    return ESP_OK;
}

static esp_err_t app_cfg_read_blob(app_cfg_t* cfg, bool* migrated)
{
    if (!cfg)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (migrated)
    {
        *migrated = false;
    }

    const app_cfg_storage_backend_t* backend = app_cfg_get_storage_backend();
    if (!backend || !backend->read)
    {
        return ESP_ERR_INVALID_STATE;
    }

    size_t    length = 0U;
    esp_err_t err    = backend->read(backend->ctx, NULL, &length);
    if (err == ESP_ERR_NOT_FOUND || err == ESP_ERR_NVS_NOT_FOUND)
    {
        return ESP_ERR_NOT_FOUND;
    }
    if (err != ESP_OK)
    {
        return err;
    }
    if (length == 0U)
    {
        return ESP_ERR_INVALID_SIZE;
    }
    if (length > sizeof(app_cfg_t))
    {
        length = sizeof(app_cfg_t);
    }

    app_cfg_t loaded;
    memset(&loaded, 0, sizeof(loaded));
    err = backend->read(backend->ctx, &loaded, &length);
    if (err != ESP_OK)
    {
        return err;
    }

    if (loaded.cfg_ver > APP_CFG_VERSION)
    {
        return ESP_ERR_INVALID_VERSION;
    }

    if (loaded.cfg_ver < APP_CFG_VERSION)
    {
        if (!s_migration_handler)
        {
            return ESP_ERR_INVALID_VERSION;
        }
        *cfg         = loaded;
        cfg->cfg_ver = loaded.cfg_ver;
        err          = s_migration_handler(loaded.cfg_ver, cfg);
        if (err != ESP_OK)
        {
            return err;
        }
        cfg->cfg_ver = APP_CFG_VERSION;
        if (migrated)
        {
            *migrated = true;
        }
        return ESP_OK;
    }

    *cfg         = loaded;
    cfg->cfg_ver = APP_CFG_VERSION;
    return ESP_OK;
}

esp_err_t app_cfg_load(app_cfg_t* cfg)
{
    if (!cfg)
    {
        return ESP_ERR_INVALID_ARG;
    }

    app_cfg_set_defaults(cfg);

    esp_err_t err = app_cfg_storage_init_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
    {
        return err;
    }

    bool migrated = false;
    err           = app_cfg_read_blob(cfg, &migrated);
    if (err == ESP_ERR_NOT_FOUND)
    {
        return ESP_ERR_NOT_FOUND;
    }
    if (err != ESP_OK)
    {
        return err;
    }

    cfg->cfg_ver = APP_CFG_VERSION;
    err          = app_cfg_validate(cfg);
    if (err != ESP_OK)
    {
        return err;
    }

    if (migrated)
    {
        (void)app_cfg_save(cfg);
    }
    return ESP_OK;
}

esp_err_t app_cfg_save(app_cfg_t* cfg)
{
    if (!cfg)
    {
        return ESP_ERR_INVALID_ARG;
    }

    cfg->cfg_ver  = APP_CFG_VERSION;
    esp_err_t err = app_cfg_validate(cfg);
    if (err != ESP_OK)
    {
        return err;
    }

    const app_cfg_storage_backend_t* backend = app_cfg_get_storage_backend();
    if (!backend || !backend->write)
    {
        return ESP_ERR_INVALID_STATE;
    }

    err = backend->write(backend->ctx, cfg, sizeof(*cfg));
    if (err != ESP_OK)
    {
        return err;
    }
    return ESP_OK;
}

esp_err_t app_cfg_reset(app_cfg_t* cfg)
{
    if (!cfg)
    {
        return ESP_ERR_INVALID_ARG;
    }

    app_cfg_set_defaults(cfg);
    const app_cfg_storage_backend_t* backend = app_cfg_get_storage_backend();
    if (backend && backend->erase)
    {
        backend->erase(backend->ctx);
    }
    return app_cfg_save(cfg);
}

esp_err_t app_cfg_register_storage_backend(const app_cfg_storage_backend_t* backend)
{
    if (!backend || !backend->read || !backend->write)
    {
        return ESP_ERR_INVALID_ARG;
    }
    s_backend = backend;
    return ESP_OK;
}

const app_cfg_storage_backend_t* app_cfg_get_storage_backend(void)
{
    if (s_backend)
    {
        return s_backend;
    }
    const app_cfg_storage_backend_t* backend = app_cfg_default_backend();
    if (backend)
    {
        s_backend = backend;
    }
    return s_backend;
}

esp_err_t app_cfg_storage_init_default(void)
{
    if (s_backend)
    {
        return ESP_ERR_INVALID_STATE;
    }
    const app_cfg_storage_backend_t* backend = app_cfg_default_backend();
    if (!backend)
    {
        return ESP_ERR_NOT_FOUND;
    }
    s_backend = backend;
    return ESP_OK;
}

void app_cfg_register_migration_handler(app_cfg_migration_fn_t fn)
{
    s_migration_handler = fn;
}

ESP_STATIC_ASSERT(sizeof(app_cfg_t) < 2048, app_cfg_struct_must_remain_small);
