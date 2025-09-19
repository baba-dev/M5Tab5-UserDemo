/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include <string.h>

#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "settings_core/app_cfg.h"

#define APP_CFG_NVS_NAMESPACE "app_cfg"
#define APP_CFG_NVS_KEY       "blob"

static const char* TAG = "app_cfg";

typedef struct
{
    char namespace_name[16];
    bool initialized;
} app_cfg_nvs_context_t;

static esp_err_t app_cfg_nvs_read(void* ctx, void* buffer, size_t* length);
static esp_err_t app_cfg_nvs_write(void* ctx, const void* buffer, size_t length);
static esp_err_t app_cfg_nvs_erase(void* ctx);
static esp_err_t ensure_nvs_ready(app_cfg_nvs_context_t* context);

static app_cfg_nvs_context_t s_nvs_context = {
    .namespace_name = APP_CFG_NVS_NAMESPACE,
    .initialized    = false,
};
static const app_cfg_storage_backend_t s_nvs_backend = {
    .ctx   = &s_nvs_context,
    .read  = app_cfg_nvs_read,
    .write = app_cfg_nvs_write,
    .erase = app_cfg_nvs_erase,
};

static esp_err_t ensure_nvs_ready(app_cfg_nvs_context_t* context)
{
    if (!context)
    {
        return ESP_ERR_INVALID_ARG;
    }
    if (context->initialized)
    {
        return ESP_OK;
    }

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGW(TAG, "Erasing NVS partition for app_cfg (err=0x%x)", (unsigned int)err);
        esp_err_t erase_err = nvs_flash_erase();
        if (erase_err != ESP_OK)
        {
            return erase_err;
        }
        err = nvs_flash_init();
    }
    if (err == ESP_OK)
    {
        context->initialized = true;
    }
    return err;
}

static esp_err_t app_cfg_nvs_read(void* ctx, void* buffer, size_t* length)
{
    if (!ctx || !length)
    {
        return ESP_ERR_INVALID_ARG;
    }

    app_cfg_nvs_context_t* context = (app_cfg_nvs_context_t*)ctx;
    esp_err_t              err     = ensure_nvs_ready(context);
    if (err != ESP_OK)
    {
        return err;
    }

    nvs_handle_t handle = 0;
    err                 = nvs_open(context->namespace_name, NVS_READONLY, &handle);
    if (err != ESP_OK)
    {
        return err;
    }

    err = nvs_get_blob(handle, APP_CFG_NVS_KEY, buffer, length);
    nvs_close(handle);
    return err;
}

static esp_err_t app_cfg_nvs_write(void* ctx, const void* buffer, size_t length)
{
    if (!ctx || !buffer || length == 0U)
    {
        return ESP_ERR_INVALID_ARG;
    }

    app_cfg_nvs_context_t* context = (app_cfg_nvs_context_t*)ctx;
    esp_err_t              err     = ensure_nvs_ready(context);
    if (err != ESP_OK)
    {
        return err;
    }

    nvs_handle_t handle = 0;
    err                 = nvs_open(context->namespace_name, NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        return err;
    }

    err = nvs_set_blob(handle, APP_CFG_NVS_KEY, buffer, length);
    if (err == ESP_OK)
    {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    return err;
}

static esp_err_t app_cfg_nvs_erase(void* ctx)
{
    if (!ctx)
    {
        return ESP_ERR_INVALID_ARG;
    }

    app_cfg_nvs_context_t* context = (app_cfg_nvs_context_t*)ctx;
    esp_err_t              err     = ensure_nvs_ready(context);
    if (err != ESP_OK)
    {
        return err;
    }

    nvs_handle_t handle = 0;
    err                 = nvs_open(context->namespace_name, NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        return err;
    }

    err = nvs_erase_key(handle, APP_CFG_NVS_KEY);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        err = ESP_OK;
    }
    if (err == ESP_OK)
    {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    return err;
}

const app_cfg_storage_backend_t* app_cfg_default_backend(void)
{
    if (ensure_nvs_ready(&s_nvs_context) != ESP_OK)
    {
        return NULL;
    }
    return &s_nvs_backend;
}

esp_err_t app_cfg_use_nvs_namespace(const char* ns)
{
    if (!ns)
    {
        return ESP_ERR_INVALID_ARG;
    }
    size_t len = strnlen(ns, sizeof(s_nvs_context.namespace_name));
    if (len == 0U || len >= sizeof(s_nvs_context.namespace_name))
    {
        return ESP_ERR_INVALID_SIZE;
    }
    memcpy(s_nvs_context.namespace_name, ns, len);
    s_nvs_context.namespace_name[len] = '\0';
    s_nvs_context.initialized         = false;
    return ESP_OK;
}

esp_err_t app_cfg_erase_persisted(void)
{
    return app_cfg_nvs_erase(&s_nvs_context);
}
