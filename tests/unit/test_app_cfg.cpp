/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include <algorithm>
#include <cstring>
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "backup_server/backup_format.h"
#include "settings_core/app_cfg.h"

namespace
{

    struct InMemoryStorage
    {
        std::vector<uint8_t> blob;
    };

    static esp_err_t StorageRead(void* ctx, void* buffer, size_t* length)
    {
        if (!ctx || !length)
        {
            return ESP_ERR_INVALID_ARG;
        }
        auto* storage = static_cast<InMemoryStorage*>(ctx);
        if (storage->blob.empty())
        {
            return ESP_ERR_NOT_FOUND;
        }
        if (!buffer)
        {
            *length = storage->blob.size();
            return ESP_OK;
        }
        size_t to_copy = std::min(*length, storage->blob.size());
        std::memcpy(buffer, storage->blob.data(), to_copy);
        *length = to_copy;
        return ESP_OK;
    }

    static esp_err_t StorageWrite(void* ctx, const void* buffer, size_t length)
    {
        if (!ctx || !buffer || length == 0U)
        {
            return ESP_ERR_INVALID_ARG;
        }
        auto* storage = static_cast<InMemoryStorage*>(ctx);
        storage->blob.assign(static_cast<const uint8_t*>(buffer),
                             static_cast<const uint8_t*>(buffer) + length);
        return ESP_OK;
    }

    static esp_err_t StorageErase(void* ctx)
    {
        if (!ctx)
        {
            return ESP_ERR_INVALID_ARG;
        }
        static_cast<InMemoryStorage*>(ctx)->blob.clear();
        return ESP_OK;
    }

    class AppCfgTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            storage_.blob.clear();
            backend_.ctx   = &storage_;
            backend_.read  = &StorageRead;
            backend_.write = &StorageWrite;
            backend_.erase = &StorageErase;
            ASSERT_EQ(ESP_OK, app_cfg_register_storage_backend(&backend_));
        }

        InMemoryStorage           storage_;
        app_cfg_storage_backend_t backend_{};
    };

    TEST_F(AppCfgTest, SaveAndLoadRoundTrip)
    {
        app_cfg_t cfg;
        app_cfg_set_defaults(&cfg);
        cfg.home_assistant.enabled = true;
        std::strncpy(cfg.home_assistant.url, "https://demo.local", sizeof(cfg.home_assistant.url));
        cfg.mqtt.enabled = true;
        std::strncpy(cfg.mqtt.broker_uri, "mqtt://broker", sizeof(cfg.mqtt.broker_uri));

        ASSERT_EQ(ESP_OK, app_cfg_save(&cfg));

        app_cfg_t loaded;
        ASSERT_EQ(ESP_OK, app_cfg_load(&loaded));
        EXPECT_TRUE(loaded.home_assistant.enabled);
        EXPECT_STREQ(loaded.home_assistant.url, "https://demo.local");
        EXPECT_TRUE(loaded.mqtt.enabled);
        EXPECT_STREQ(loaded.mqtt.broker_uri, "mqtt://broker");
        EXPECT_EQ(loaded.cfg_ver, APP_CFG_VERSION);
    }

    TEST_F(AppCfgTest, ValidationRejectsOutOfRangeBrightness)
    {
        app_cfg_t cfg;
        app_cfg_set_defaults(&cfg);
        cfg.ui.brightness = 0U;
        EXPECT_EQ(ESP_ERR_INVALID_ARG, app_cfg_validate(&cfg));
        cfg.ui.brightness = 101U;
        EXPECT_EQ(ESP_ERR_INVALID_ARG, app_cfg_validate(&cfg));
    }

    TEST_F(AppCfgTest, BackupSerializationProducesJson)
    {
        app_cfg_t cfg;
        app_cfg_set_defaults(&cfg);
        cfg.mqtt.enabled = true;
        std::strncpy(cfg.mqtt.username, "user", sizeof(cfg.mqtt.username));
        std::strncpy(cfg.network.hostname, "tab5", sizeof(cfg.network.hostname));
        cfg.safety.diagnostics_opt_in = true;

        size_t json_size = backup_server_calculate_json_size(&cfg);
        ASSERT_GT(json_size, 0U);
        std::vector<char> buffer(json_size, '\0');
        ASSERT_EQ(ESP_OK, backup_server_write_json(&cfg, buffer.data(), buffer.size()));
        std::string json(buffer.data());

        EXPECT_NE(std::string::npos, json.find("\"mqtt\":{"));
        EXPECT_NE(std::string::npos, json.find("\"enabled\":true"));
        EXPECT_NE(std::string::npos, json.find("\"diagnostics_opt_in\":true"));
        EXPECT_NE(std::string::npos, json.find("\"hostname\":\"tab5\""));
    }

}  // namespace
