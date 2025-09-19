/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "backup_server/backup_format.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
    char*  buffer;
    size_t length;
    size_t used;
} json_writer_t;

static esp_err_t writer_append_char(json_writer_t* writer, char ch)
{
    if (!writer)
    {
        return ESP_ERR_INVALID_ARG;
    }
    if (!writer->buffer)
    {
        writer->used += 1U;
        return ESP_OK;
    }
    if (writer->length == 0U || writer->used + 1U >= writer->length)
    {
        return ESP_ERR_INVALID_SIZE;
    }
    writer->buffer[writer->used++] = ch;
    writer->buffer[writer->used]   = '\0';
    return ESP_OK;
}

static esp_err_t writer_append(json_writer_t* writer, const char* text)
{
    if (!writer || !text)
    {
        return ESP_ERR_INVALID_ARG;
    }
    size_t len = strlen(text);
    if (!writer->buffer)
    {
        writer->used += len;
        return ESP_OK;
    }
    if (writer->used + len >= writer->length)
    {
        return ESP_ERR_INVALID_SIZE;
    }
    memcpy(writer->buffer + writer->used, text, len);
    writer->used += len;
    writer->buffer[writer->used] = '\0';
    return ESP_OK;
}

static esp_err_t writer_append_bool(json_writer_t* writer, bool value)
{
    return writer_append(writer, value ? "true" : "false");
}

static esp_err_t writer_append_number(json_writer_t* writer, unsigned long value)
{
    char scratch[24];
    int  len = snprintf(scratch, sizeof(scratch), "%lu", value);
    if (len < 0)
    {
        return ESP_FAIL;
    }
    return writer_append(writer, scratch);
}

static esp_err_t writer_append_string(json_writer_t* writer, const char* value)
{
    esp_err_t err = writer_append_char(writer, '"');
    if (err != ESP_OK)
    {
        return err;
    }
    if (!value)
    {
        value = "";
    }
    while (*value)
    {
        const char* escape     = NULL;
        char        escaped[3] = {'\\', '\0', '\0'};
        switch (*value)
        {
            case '\\':
            case '"':
                escaped[1] = *value;
                escape     = escaped;
                break;
            case '\n':
                escaped[1] = 'n';
                escape     = escaped;
                break;
            case '\r':
                escaped[1] = 'r';
                escape     = escaped;
                break;
            case '\t':
                escaped[1] = 't';
                escape     = escaped;
                break;
            case '\b':
                escaped[1] = 'b';
                escape     = escaped;
                break;
            case '\f':
                escaped[1] = 'f';
                escape     = escaped;
                break;
            default:
                err = writer_append_char(writer, *value);
                if (err != ESP_OK)
                {
                    return err;
                }
                break;
        }
        if (escape)
        {
            err = writer_append(writer, escape);
            if (err != ESP_OK)
            {
                return err;
            }
        }
        ++value;
    }
    return writer_append_char(writer, '"');
}

static esp_err_t writer_append_field_name(json_writer_t* writer, const char* name)
{
    esp_err_t err = writer_append_char(writer, '"');
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, name);
    if (err != ESP_OK)
    {
        return err;
    }
    return writer_append(writer, "\":");
}

static esp_err_t
writer_append_string_field(json_writer_t* writer, const char* name, const char* value)
{
    esp_err_t err = writer_append_field_name(writer, name);
    if (err != ESP_OK)
    {
        return err;
    }
    return writer_append_string(writer, value);
}

static esp_err_t writer_append_bool_field(json_writer_t* writer, const char* name, bool value)
{
    esp_err_t err = writer_append_field_name(writer, name);
    if (err != ESP_OK)
    {
        return err;
    }
    return writer_append_bool(writer, value);
}

static esp_err_t
writer_append_number_field(json_writer_t* writer, const char* name, unsigned long value)
{
    esp_err_t err = writer_append_field_name(writer, name);
    if (err != ESP_OK)
    {
        return err;
    }
    return writer_append_number(writer, value);
}

static esp_err_t backup_server_encode(const app_cfg_t* cfg, json_writer_t* writer)
{
    if (!cfg || !writer)
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = writer_append_char(writer, '{');
    if (err != ESP_OK)
    {
        return err;
    }

    err = writer_append_number_field(writer, "cfg_ver", cfg->cfg_ver);
    if (err != ESP_OK)
    {
        return err;
    }

    err = writer_append(writer, ",\"home_assistant\":{");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_bool_field(writer, "enabled", cfg->home_assistant.enabled);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_string_field(writer, "url", cfg->home_assistant.url);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_string_field(writer, "token", cfg->home_assistant.token);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_char(writer, '}');
    if (err != ESP_OK)
    {
        return err;
    }

    err = writer_append(writer, ",\"frigate\":{");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_bool_field(writer, "enabled", cfg->frigate.enabled);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_string_field(writer, "url", cfg->frigate.url);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_string_field(writer, "camera", cfg->frigate.camera_name);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_bool_field(writer, "snapshots", cfg->frigate.snapshots_enabled);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_char(writer, '}');
    if (err != ESP_OK)
    {
        return err;
    }

    err = writer_append(writer, ",\"mqtt\":{");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_bool_field(writer, "enabled", cfg->mqtt.enabled);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_string_field(writer, "broker", cfg->mqtt.broker_uri);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_string_field(writer, "client_id", cfg->mqtt.client_id);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_string_field(writer, "username", cfg->mqtt.username);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_string_field(writer, "password", cfg->mqtt.password);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_bool_field(writer, "use_tls", cfg->mqtt.use_tls);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_bool_field(writer, "ha_discovery", cfg->mqtt.ha_discovery);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_char(writer, '}');
    if (err != ESP_OK)
    {
        return err;
    }

    err = writer_append(writer, ",\"ui\":{");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_number_field(writer, "theme", (unsigned long)cfg->ui.theme);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_number_field(writer, "brightness", (unsigned long)cfg->ui.brightness);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_number_field(
        writer, "screen_timeout", (unsigned long)cfg->ui.screen_timeout_seconds);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_char(writer, '}');
    if (err != ESP_OK)
    {
        return err;
    }

    err = writer_append(writer, ",\"network\":{");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_string_field(writer, "ssid", cfg->network.ssid);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_string_field(writer, "password", cfg->network.password);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_string_field(writer, "hostname", cfg->network.hostname);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_bool_field(writer, "use_dhcp", cfg->network.use_dhcp);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_string_field(writer, "static_ip", cfg->network.static_ip);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_string_field(writer, "gateway", cfg->network.gateway);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_string_field(writer, "netmask", cfg->network.netmask);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_string_field(writer, "dns_primary", cfg->network.dns_primary);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_string_field(writer, "dns_secondary", cfg->network.dns_secondary);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_string_field(writer, "timezone", cfg->network.timezone);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_string_field(writer, "ntp_server", cfg->network.ntp_server);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_bool_field(writer, "sntp_sync", cfg->network.sntp_sync_enabled);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_char(writer, '}');
    if (err != ESP_OK)
    {
        return err;
    }

    err = writer_append(writer, ",\"safety\":{");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_bool_field(writer, "child_lock", cfg->safety.child_lock);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_bool_field(writer, "disable_wifi", cfg->safety.disable_wifi);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_bool_field(writer, "allow_ota", cfg->safety.allow_ota);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append(writer, ",");
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_bool_field(writer, "diagnostics_opt_in", cfg->safety.diagnostics_opt_in);
    if (err != ESP_OK)
    {
        return err;
    }
    err = writer_append_char(writer, '}');
    if (err != ESP_OK)
    {
        return err;
    }

    return writer_append_char(writer, '}');
}

size_t backup_server_calculate_json_size(const app_cfg_t* cfg)
{
    json_writer_t writer = {
        .buffer = NULL,
        .length = SIZE_MAX,
        .used   = 0U,
    };
    if (backup_server_encode(cfg, &writer) != ESP_OK)
    {
        return 0U;
    }
    return writer.used + 1U;
}

esp_err_t backup_server_write_json(const app_cfg_t* cfg, char* buffer, size_t length)
{
    if (!cfg || !buffer || length == 0U)
    {
        return ESP_ERR_INVALID_ARG;
    }

    json_writer_t writer = {
        .buffer = buffer,
        .length = length,
        .used   = 0U,
    };

    esp_err_t err = backup_server_encode(cfg, &writer);
    if (err != ESP_OK)
    {
        return err;
    }
    if (writer.used >= length)
    {
        return ESP_ERR_INVALID_SIZE;
    }
    buffer[writer.used] = '\0';
    return ESP_OK;
}
