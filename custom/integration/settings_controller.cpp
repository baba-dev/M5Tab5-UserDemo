/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "integration/settings_controller.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <functional>
#include <new>
#include <sstream>
#include <string>
#include <vector>

#if !defined(ESP_PLATFORM)
#    include <condition_variable>
#    include <mutex>
#    include <queue>
#    include <thread>
#endif

#include "../app_trace.h"
#include "hal/hal.h"
#include "settings_core/app_cfg.h"
#include "settings_ui/settings_ui.h"
#include "ui/pages/ui_page_settings.h"

#if defined(ESP_PLATFORM)
#    include "backup_server/backup_format.h"
#    include "backup_server/backup_server.h"
#    include "cJSON.h"
#    include "connection_tester/connection_tester.h"
#    include "diag/diag.h"
#    include "esp_err.h"
#    include "esp_wifi.h"
#    include "freertos/FreeRTOS.h"
#    include "freertos/queue.h"
#    include "freertos/semphr.h"
#    include "freertos/task.h"
#    include "net_sntp/net_sntp.h"
#    include "ota_update/ota_update.h"
#endif

namespace custom::integration
{

    namespace
    {

        constexpr const char* kTag = "settings-controller";

        constexpr std::chrono::seconds kRefreshInterval{std::chrono::seconds(60)};

        constexpr uint8_t kMinBrightness = 1U;
        constexpr uint8_t kMaxBrightness = 100U;

        constexpr const char* kDefaultUpdateBaseUrl = "https://updates.m5stack.com/tab5";

#if defined(ESP_PLATFORM)
        constexpr UBaseType_t kTaskQueueLength     = 8U;
        constexpr uint32_t    kWorkerTaskStackSize = 4096U;
        constexpr UBaseType_t kWorkerTaskPriority  = 5U;
#endif

        std::string timestamp_string()
        {
            std::time_t now = std::time(nullptr);
            char        buffer[32];
#if defined(ESP_PLATFORM)
            tm tm_info;
            localtime_r(&now, &tm_info);
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm_info);
#else
            std::tm* tm_ptr = std::localtime(&now);
            if (tm_ptr != nullptr)
            {
                std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_ptr);
            }
            else
            {
                std::snprintf(buffer, sizeof(buffer), "%ld", static_cast<long>(now));
            }
#endif
            return std::string(buffer);
        }

#if defined(ESP_PLATFORM)
        std::string error_to_string(esp_err_t err)
        {
            const char* name = esp_err_to_name(err);
            return (name != nullptr) ? std::string(name) : std::string("unknown");
        }
#endif

    }  // namespace

    class SettingsController::Impl
    {
    public:
        Impl();
        ~Impl();

        void PublishInitialState();

        void RunConnectionTest(const char* tester_id);
        void SetDarkMode(bool enabled);
        void SetThemeVariant(const char* variant_id);
        void SetBrightness(uint8_t percent);
        void OpenDisplaySettings();
        void OpenNetworkSettings();
        void SyncTime();
        void CheckForUpdates();
        void StartOtaUpdate();
        void OpenDiagnostics();
        void ExportLogs();
        void BackupNow();
        void RestoreBackup();

    private:
        void        load_config();
        void        persist_config();
        void        apply_current_theme();
        std::string current_variant_id() const;

        void enqueue_task(std::function<void()> task);
#if defined(ESP_PLATFORM)
        static void WorkerTaskEntry(void* arg);
#else
        void worker_loop();
#endif
        void refresh_all_connections();
        void perform_connection_test(const std::string& tester_id);
        void test_wifi_connection();
        void test_home_assistant_connection();
        void test_cloud_connection();
        void post_connection_result(const char*               tester_id,
                                    ui_page_settings_status_t status,
                                    const std::string&        message);
        void post_update_status(const std::string& message);
        void post_diagnostics_status(const std::string& message);
        void post_backup_status(const std::string& message);

        std::string manifest_url() const;
        std::string firmware_url() const;

#if defined(ESP_PLATFORM)
        static void
        copy_string_field(const cJSON* object, const char* key, char* dest, size_t length);
        static void copy_bool_field(const cJSON* object, const char* key, bool& dest);
        static void copy_number_field(const cJSON* object, const char* key, int& dest);
        void        handle_restore_from_json(const std::string& json);
#endif

        app_cfg_t             config_{};
        bool                  config_loaded_ = false;
        settings_ui_runtime_t ui_runtime_{};

#if defined(ESP_PLATFORM)
        diag_handles_t diag_handles_{};
        bool           diag_running_ = false;
#endif

        std::atomic<bool> running_{false};
#if defined(ESP_PLATFORM)
        TaskHandle_t      worker_task_       = nullptr;
        QueueHandle_t     task_queue_        = nullptr;
        SemaphoreHandle_t refresh_semaphore_ = nullptr;
        std::atomic<bool> worker_active_{false};
#else
        std::thread                           worker_thread_;
        std::mutex                            mutex_;
        std::condition_variable               cv_;
        std::queue<std::function<void()>>     tasks_;
        std::chrono::steady_clock::time_point next_refresh_;
#endif

        std::string backup_path_;
        std::string logs_path_;
    };

    SettingsController::Impl::Impl()
    {
        load_config();

#if defined(ESP_PLATFORM)
        backup_path_ = "/sdcard/m5tab5_backup.json";
        logs_path_   = "/sdcard/m5tab5_logs.txt";
#else
        backup_path_ = "m5tab5_backup.json";
        logs_path_   = "m5tab5_logs.txt";
#endif

#if defined(ESP_PLATFORM)
        running_.store(true);
        refresh_semaphore_ = xSemaphoreCreateBinary();
        if (refresh_semaphore_ == nullptr)
        {
            APP_LOG_ERROR(kTag, "Failed to create refresh semaphore");
            running_.store(false);
        }

        if (running_.load())
        {
            task_queue_ = xQueueCreate(kTaskQueueLength, sizeof(std::function<void()>*));
            if (task_queue_ == nullptr)
            {
                APP_LOG_ERROR(kTag, "Failed to create task queue");
                running_.store(false);
            }
        }

        if (running_.load())
        {
            if (xTaskCreate(&SettingsController::Impl::WorkerTaskEntry,
                            "settings_ctrl",
                            kWorkerTaskStackSize,
                            this,
                            kWorkerTaskPriority,
                            &worker_task_)
                == pdPASS)
            {
                worker_active_.store(true);
            }
            else
            {
                APP_LOG_ERROR(kTag, "Failed to start worker task");
                worker_task_ = nullptr;
                running_.store(false);
            }
        }

        if (!running_.load())
        {
            if (task_queue_ != nullptr)
            {
                vQueueDelete(task_queue_);
                task_queue_ = nullptr;
            }
            if (refresh_semaphore_ != nullptr)
            {
                vSemaphoreDelete(refresh_semaphore_);
                refresh_semaphore_ = nullptr;
            }
            worker_active_.store(false);
        }
#else
        running_.store(true);
        next_refresh_  = std::chrono::steady_clock::now() + kRefreshInterval;
        worker_thread_ = std::thread(&SettingsController::Impl::worker_loop, this);
#endif
    }

    SettingsController::Impl::~Impl()
    {
#if defined(ESP_PLATFORM)
        running_.store(false);
        if (refresh_semaphore_ != nullptr)
        {
            xSemaphoreGive(refresh_semaphore_);
        }

        while (worker_active_.load())
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        if (task_queue_ != nullptr)
        {
            std::function<void()>* pending = nullptr;
            while (xQueueReceive(task_queue_, &pending, 0) == pdTRUE)
            {
                delete pending;
            }
            vQueueDelete(task_queue_);
            task_queue_ = nullptr;
        }

        if (refresh_semaphore_ != nullptr)
        {
            vSemaphoreDelete(refresh_semaphore_);
            refresh_semaphore_ = nullptr;
        }
#else
        {
            std::lock_guard<std::mutex> lock(mutex_);
            running_.store(false);
        }
        cv_.notify_all();
        if (worker_thread_.joinable())
        {
            worker_thread_.join();
        }
#endif

#if defined(ESP_PLATFORM)
        if (diag_running_)
        {
            diag_stop(&diag_handles_);
            diag_running_ = false;
        }
#endif

        ui_page_settings_set_actions(nullptr, nullptr);
    }

    void SettingsController::Impl::PublishInitialState()
    {
        enqueue_task(
            [this]()
            {
                apply_current_theme();
                ui_page_settings_set_brightness(config_.ui.brightness);
                refresh_all_connections();
            });
    }

    void SettingsController::Impl::RunConnectionTest(const char* tester_id)
    {
        if (tester_id == nullptr)
        {
            return;
        }
        std::string id(tester_id);
        enqueue_task([this, id]() { perform_connection_test(id); });
    }

    void SettingsController::Impl::SetDarkMode(bool enabled)
    {
        enqueue_task(
            [this, enabled]()
            {
                config_.ui.theme = enabled ? APP_CFG_UI_THEME_DARK : APP_CFG_UI_THEME_LIGHT;
                persist_config();
                apply_current_theme();
            });
    }

    void SettingsController::Impl::SetThemeVariant(const char* variant_id)
    {
        std::string id = (variant_id != nullptr) ? std::string(variant_id) : std::string();
        enqueue_task(
            [this, id]()
            {
                if (id == "system")
                {
                    config_.ui.theme = APP_CFG_UI_THEME_AUTO;
                }
                else if (id == "solar")
                {
                    config_.ui.theme = APP_CFG_UI_THEME_LIGHT;
                }
                else if (id == "midnight")
                {
                    config_.ui.theme = APP_CFG_UI_THEME_DARK;
                }
                persist_config();
                apply_current_theme();
            });
    }

    void SettingsController::Impl::SetBrightness(uint8_t percent)
    {
        enqueue_task(
            [this, percent]()
            {
                uint8_t clamped = std::clamp<uint8_t>(percent, kMinBrightness, kMaxBrightness);
                config_.ui.brightness = clamped;
                GetHAL()->setDisplayBrightness(clamped);
                persist_config();
                ui_page_settings_set_brightness(clamped);
            });
    }

    void SettingsController::Impl::OpenDisplaySettings()
    {
        enqueue_task(
            [this]()
            {
                GetHAL()->setDisplayBrightness(config_.ui.brightness);
                post_update_status("Display preferences applied");
            });
    }

    void SettingsController::Impl::OpenNetworkSettings()
    {
        enqueue_task(
            [this]()
            {
#if defined(ESP_PLATFORM)
                GetHAL()->startWifiAp();
                post_update_status("Wi-Fi provisioning hotspot started");
#else
                post_update_status("Network tools unavailable on host");
#endif
            });
    }

    void SettingsController::Impl::SyncTime()
    {
        enqueue_task(
            [this]()
            {
#if defined(ESP_PLATFORM)
                const char* server =
                    (config_.network.ntp_server[0] != '\0') ? config_.network.ntp_server : nullptr;
                esp_err_t err = net_sntp_start(server, true);
                if (err == ESP_OK)
                {
                    post_update_status("Time synchronised");
                }
                else
                {
                    APP_LOG_WARN(kTag, "SNTP sync failed: %s", error_to_string(err).c_str());
                    post_update_status("Time sync failed");
                }
                net_sntp_stop();
#else
                post_update_status("Time sync simulated");
#endif
            });
    }

    void SettingsController::Impl::CheckForUpdates()
    {
        enqueue_task(
            [this]()
            {
                post_update_status("Checking for updates...");
#if defined(ESP_PLATFORM)
                if (!config_.safety.allow_ota)
                {
                    post_update_status("OTA disabled by policy");
                    return;
                }
                std::string url         = manifest_url();
                int         status_code = 0;
                esp_err_t   err = connection_tester_http_get(url.c_str(), 5000, &status_code);
                if (err == ESP_OK && status_code >= 200 && status_code < 400)
                {
                    post_update_status("Update manifest reachable");
                }
                else
                {
                    if (err != ESP_OK)
                    {
                        APP_LOG_WARN(kTag, "Update check failed: %s", error_to_string(err).c_str());
                    }
                    post_update_status("No updates found");
                }
#else
                post_update_status("Update check simulated");
#endif
            });
    }

    void SettingsController::Impl::StartOtaUpdate()
    {
        enqueue_task(
            [this]()
            {
#if defined(ESP_PLATFORM)
                if (!config_.safety.allow_ota)
                {
                    post_update_status("OTA disabled by policy");
                    return;
                }
                std::string url = firmware_url();
                if (url.empty())
                {
                    post_update_status("No OTA endpoint configured");
                    return;
                }
                post_update_status("Preparing OTA...");
                auto callback = [](const ota_update_event_t* event, void* context)
                {
                    auto* self = static_cast<SettingsController::Impl*>(context);
                    if (self == nullptr || event == nullptr)
                    {
                        return;
                    }
                    switch (event->type)
                    {
                        case OTA_UPDATE_EVENT_START:
                            self->post_update_status("Starting OTA download...");
                            break;
                        case OTA_UPDATE_EVENT_PROGRESS:
                        {
                            size_t   total      = event->image_size;
                            size_t   downloaded = event->bytes_downloaded;
                            uint32_t percent =
                                (total > 0U) ? static_cast<uint32_t>((downloaded * 100ULL) / total)
                                             : 0U;
                            std::ostringstream status;
                            if (total > 0U)
                            {
                                status << "Downloading " << percent << "%";
                                if (total >= 1024U)
                                {
                                    status << " (" << (downloaded / 1024U) << '/' << (total / 1024U)
                                           << " KiB)";
                                }
                            }
                            else
                            {
                                status << "Downloading " << (downloaded / 1024U) << " KiB";
                            }
                            self->post_update_status(status.str());
                            break;
                        }
                        case OTA_UPDATE_EVENT_COMPLETED:
                            self->post_update_status("OTA completed");
                            break;
                        case OTA_UPDATE_EVENT_ERROR:
                        default:
                        {
                            std::string reason = error_to_string(event->error);
                            self->post_update_status("OTA failed: " + reason);
                            break;
                        }
                    }
                };
                esp_err_t err = ota_update_perform_with_callback(
                    url.c_str(), true, callback, static_cast<void*>(this));
                if (err != ESP_OK)
                {
                    APP_LOG_ERROR(kTag, "OTA failed: %s", error_to_string(err).c_str());
                }
#else
                post_update_status("OTA not supported on host");
#endif
            });
    }

    void SettingsController::Impl::OpenDiagnostics()
    {
        enqueue_task(
            [this]()
            {
#if defined(ESP_PLATFORM)
                if (!config_.safety.diagnostics_opt_in)
                {
                    post_diagnostics_status("Diagnostics disabled");
                    return;
                }
                if (diag_running_)
                {
                    diag_stop(&diag_handles_);
                    diag_running_ = false;
                    post_diagnostics_status("Restarting diagnostics...");
                }
                auto callback = [](const diag_event_t* event, void* context)
                {
                    auto* self = static_cast<SettingsController::Impl*>(context);
                    if (self == nullptr || event == nullptr)
                    {
                        return;
                    }
                    switch (event->type)
                    {
                        case DIAG_EVENT_STARTING:
                            self->post_diagnostics_status("Starting diagnostics...");
                            break;
                        case DIAG_EVENT_HTTP_READY:
                            self->post_diagnostics_status("Diagnostics HTTP ready");
                            break;
                        case DIAG_EVENT_MQTT_STARTED:
                            self->post_diagnostics_status("Diagnostics MQTT connected");
                            break;
                        case DIAG_EVENT_WARNING:
                            self->post_diagnostics_status("Diagnostics warning: "
                                                          + error_to_string(event->error));
                            break;
                        case DIAG_EVENT_ERROR:
                        default:
                            self->post_diagnostics_status("Diagnostics failed: "
                                                          + error_to_string(event->error));
                            break;
                    }
                };
                esp_err_t err =
                    diag_start(&config_, &diag_handles_, callback, static_cast<void*>(this));
                if (err == ESP_OK)
                {
                    diag_running_ = true;
                }
                else
                {
                    APP_LOG_ERROR(kTag, "Diagnostics failed: %s", error_to_string(err).c_str());
                }
#else
                post_diagnostics_status("Diagnostics not available on host");
#endif
            });
    }

    void SettingsController::Impl::ExportLogs()
    {
        enqueue_task(
            [this]()
            {
#if defined(ESP_PLATFORM)
                if (!GetHAL()->isSdCardMounted())
                {
                    post_diagnostics_status("Insert SD card to export logs");
                    return;
                }
#endif
                std::ofstream stream(logs_path_, std::ios::app);
                if (!stream.is_open())
                {
                    post_diagnostics_status("Failed to write logs");
                    return;
                }
                stream << '[' << timestamp_string() << "] Log export placeholder\n";
                stream.close();
                post_diagnostics_status("Logs saved to " + logs_path_);
            });
    }

    void SettingsController::Impl::BackupNow()
    {
        enqueue_task(
            [this]()
            {
#if defined(ESP_PLATFORM)
                if (!GetHAL()->isSdCardMounted())
                {
                    post_backup_status("Insert SD card to backup");
                    return;
                }
                post_backup_status("Preparing backup...");
                size_t needed = backup_server_calculate_json_size(&config_);
                if (needed == 0U)
                {
                    post_backup_status("Backup failed: invalid configuration");
                    return;
                }
                std::vector<char> buffer(needed);
                esp_err_t err = backup_server_write_json(&config_, buffer.data(), buffer.size());
                if (err != ESP_OK)
                {
                    APP_LOG_ERROR(kTag, "Backup encode failed: %s", error_to_string(err).c_str());
                    post_backup_status("Backup encode failed: " + error_to_string(err));
                    return;
                }
                std::ofstream output(backup_path_, std::ios::trunc);
                if (!output.is_open())
                {
                    post_backup_status("Cannot open backup file");
                    return;
                }
                output.write(buffer.data(), std::strlen(buffer.data()));
                output.close();
                post_backup_status("Backup saved to " + backup_path_);
#else
                post_backup_status("Backup not supported on host");
#endif
            });
    }

    void SettingsController::Impl::RestoreBackup()
    {
        enqueue_task(
            [this]()
            {
#if defined(ESP_PLATFORM)
                std::ifstream input(backup_path_);
                if (!input.is_open())
                {
                    post_backup_status("Backup file missing");
                    return;
                }
                std::stringstream buffer;
                buffer << input.rdbuf();
                input.close();
                std::string json = buffer.str();
                if (json.empty())
                {
                    post_backup_status("Backup file empty");
                    return;
                }
                handle_restore_from_json(json);
#else
                post_backup_status("Restore not supported on host");
#endif
            });
    }

    void SettingsController::Impl::load_config()
    {
        app_cfg_set_defaults(&config_);
#if defined(ESP_PLATFORM)
        esp_err_t err = app_cfg_load(&config_);
        if (err == ESP_OK || err == ESP_ERR_NOT_FOUND)
        {
            config_loaded_ = true;
        }
        else
        {
            APP_LOG_WARN(kTag, "Failed to load config: %s", error_to_string(err).c_str());
            config_loaded_ = true;
        }
#else
        config_loaded_ = true;
#endif
        settings_ui_apply(&config_, &ui_runtime_);
    }

    void SettingsController::Impl::persist_config()
    {
        (void)config_loaded_;
#if defined(ESP_PLATFORM)
        esp_err_t err = app_cfg_save(&config_);
        if (err != ESP_OK)
        {
            APP_LOG_WARN(kTag, "Failed to save config: %s", error_to_string(err).c_str());
        }
#endif
    }

    void SettingsController::Impl::apply_current_theme()
    {
        settings_ui_apply(&config_, &ui_runtime_);
        ui_page_settings_apply_theme_state(config_.ui.theme == APP_CFG_UI_THEME_DARK,
                                           current_variant_id().c_str());
    }

    std::string SettingsController::Impl::current_variant_id() const
    {
        switch (config_.ui.theme)
        {
            case APP_CFG_UI_THEME_DARK:
                return "midnight";
            case APP_CFG_UI_THEME_LIGHT:
                return "solar";
            case APP_CFG_UI_THEME_AUTO:
            default:
                return "system";
        }
    }

    void SettingsController::Impl::enqueue_task(std::function<void()> task)
    {
#if defined(ESP_PLATFORM)
        if (!running_.load() || task_queue_ == nullptr)
        {
            return;
        }

        auto* heap_task = new (std::nothrow) std::function<void()>(std::move(task));
        if (heap_task == nullptr)
        {
            APP_LOG_WARN(kTag, "Failed to allocate task");
            return;
        }

        if (xQueueSend(task_queue_, &heap_task, 0) != pdPASS)
        {
            APP_LOG_WARN(kTag, "Task queue full");
            delete heap_task;
            return;
        }

        if (refresh_semaphore_ != nullptr)
        {
            xSemaphoreGive(refresh_semaphore_);
        }
#else
        {
            std::lock_guard<std::mutex> lock(mutex_);
            tasks_.push(std::move(task));
        }
        cv_.notify_all();
#endif
    }

#if !defined(ESP_PLATFORM)
    void SettingsController::Impl::worker_loop()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (running_.load())
        {
            if (!tasks_.empty())
            {
                auto task = std::move(tasks_.front());
                tasks_.pop();
                lock.unlock();
                task();
                lock.lock();
                next_refresh_ = std::chrono::steady_clock::now() + kRefreshInterval;
                continue;
            }

            auto now = std::chrono::steady_clock::now();
            if (now >= next_refresh_)
            {
                lock.unlock();
                refresh_all_connections();
                lock.lock();
                next_refresh_ = std::chrono::steady_clock::now() + kRefreshInterval;
                continue;
            }

            cv_.wait_until(
                lock, next_refresh_, [this]() { return !running_.load() || !tasks_.empty(); });
        }
    }
#else
    void SettingsController::Impl::WorkerTaskEntry(void* arg)
    {
        auto* self = static_cast<SettingsController::Impl*>(arg);
        if (self == nullptr)
        {
            vTaskDelete(nullptr);
            return;
        }

        if (self->task_queue_ == nullptr || self->refresh_semaphore_ == nullptr)
        {
            self->running_.store(false);
            self->worker_active_.store(false);
            self->worker_task_ = nullptr;
            vTaskDelete(nullptr);
            return;
        }

        self->worker_active_.store(true);

        const uint32_t refresh_ms = static_cast<uint32_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(kRefreshInterval).count());
        const TickType_t refresh_ticks = std::max<TickType_t>(1, pdMS_TO_TICKS(refresh_ms));

        while (self->running_.load())
        {
            std::function<void()>* task_ptr = nullptr;
            while (self->running_.load()
                   && xQueueReceive(self->task_queue_, &task_ptr, 0) == pdTRUE)
            {
                if (task_ptr == nullptr)
                {
                    self->running_.store(false);
                    break;
                }

                (*task_ptr)();
                delete task_ptr;
                task_ptr = nullptr;
            }

            if (!self->running_.load())
            {
                break;
            }

            if (xSemaphoreTake(self->refresh_semaphore_, refresh_ticks) == pdFALSE)
            {
                self->refresh_all_connections();
            }
        }

        std::function<void()>* leftover = nullptr;
        while (xQueueReceive(self->task_queue_, &leftover, 0) == pdTRUE)
        {
            delete leftover;
        }

        self->worker_active_.store(false);
        self->worker_task_ = nullptr;
        vTaskDelete(nullptr);
    }
#endif

    void SettingsController::Impl::refresh_all_connections()
    {
        perform_connection_test("wifi");
        perform_connection_test("ha");
        perform_connection_test("cloud");
    }

    void SettingsController::Impl::perform_connection_test(const std::string& tester_id)
    {
        if (tester_id == "wifi")
        {
            test_wifi_connection();
        }
        else if (tester_id == "ha")
        {
            test_home_assistant_connection();
        }
        else if (tester_id == "cloud")
        {
            test_cloud_connection();
        }
        else
        {
            post_connection_result(
                tester_id.c_str(), UI_PAGE_SETTINGS_STATUS_UNKNOWN, "Unknown test");
        }
    }

    void SettingsController::Impl::test_wifi_connection()
    {
#if defined(ESP_PLATFORM) && CONFIG_APP_ENABLE_WIFI_HOSTED
        wifi_ap_record_t ap_info{};
        esp_err_t        err = esp_wifi_sta_get_ap_info(&ap_info);
        if (err == ESP_OK)
        {
            std::string ssid(reinterpret_cast<char*>(ap_info.ssid));
            post_connection_result("wifi", UI_PAGE_SETTINGS_STATUS_OK, ssid);
        }
        else if (err == ESP_ERR_WIFI_NOT_CONNECT)
        {
            post_connection_result("wifi", UI_PAGE_SETTINGS_STATUS_ERROR, "Disconnected");
        }
        else
        {
            post_connection_result("wifi", UI_PAGE_SETTINGS_STATUS_UNKNOWN, "Unavailable");
        }
#elif defined(ESP_PLATFORM)
        post_connection_result("wifi", UI_PAGE_SETTINGS_STATUS_UNKNOWN, "ESP-Hosted disabled");
#else
        post_connection_result("wifi", UI_PAGE_SETTINGS_STATUS_WARNING, "Not supported on host");
#endif
    }

    void SettingsController::Impl::test_home_assistant_connection()
    {
        if (!config_.home_assistant.enabled || config_.home_assistant.url[0] == '\0')
        {
            post_connection_result("ha", UI_PAGE_SETTINGS_STATUS_UNKNOWN, "Disabled");
            return;
        }
#if defined(ESP_PLATFORM)
        int       status_code = 0;
        esp_err_t err = connection_tester_http_get(config_.home_assistant.url, 5000, &status_code);
        if (err == ESP_OK && status_code >= 200 && status_code < 400)
        {
            post_connection_result("ha", UI_PAGE_SETTINGS_STATUS_OK, "Online");
        }
        else
        {
            post_connection_result("ha", UI_PAGE_SETTINGS_STATUS_ERROR, "Unreachable");
        }
#else
        post_connection_result("ha", UI_PAGE_SETTINGS_STATUS_OK, "Simulated");
#endif
    }

    void SettingsController::Impl::test_cloud_connection()
    {
        if (!config_.frigate.enabled || config_.frigate.url[0] == '\0')
        {
            post_connection_result("cloud", UI_PAGE_SETTINGS_STATUS_UNKNOWN, "Disabled");
            return;
        }
#if defined(ESP_PLATFORM)
        int       status_code = 0;
        esp_err_t err         = connection_tester_http_get(config_.frigate.url, 5000, &status_code);
        if (err == ESP_OK && status_code >= 200 && status_code < 400)
        {
            post_connection_result("cloud", UI_PAGE_SETTINGS_STATUS_OK, "Online");
        }
        else
        {
            post_connection_result("cloud", UI_PAGE_SETTINGS_STATUS_WARNING, "Check relay");
        }
#else
        post_connection_result("cloud", UI_PAGE_SETTINGS_STATUS_WARNING, "Simulated");
#endif
    }

    void SettingsController::Impl::post_connection_result(const char*               tester_id,
                                                          ui_page_settings_status_t status,
                                                          const std::string&        message)
    {
        ui_page_settings_set_connection_status(tester_id, status, message.c_str());
    }

    void SettingsController::Impl::post_update_status(const std::string& message)
    {
        ui_page_settings_set_update_status(message.c_str());
    }

    void SettingsController::Impl::post_diagnostics_status(const std::string& message)
    {
        ui_page_settings_set_diagnostics_status(message.c_str());
    }

    void SettingsController::Impl::post_backup_status(const std::string& message)
    {
        ui_page_settings_set_backup_status(message.c_str());
    }

    std::string SettingsController::Impl::manifest_url() const
    {
        std::string base;
        if (config_.home_assistant.url[0] != '\0')
        {
            base.assign(config_.home_assistant.url);
        }
        else
        {
            base.assign(kDefaultUpdateBaseUrl);
        }
        if (!base.empty() && base.back() == '/')
        {
            base.pop_back();
        }
        return base + "/ota/manifest.json";
    }

    std::string SettingsController::Impl::firmware_url() const
    {
        std::string base;
        if (config_.home_assistant.url[0] != '\0')
        {
            base.assign(config_.home_assistant.url);
        }
        else
        {
            base.assign(kDefaultUpdateBaseUrl);
        }
        if (!base.empty() && base.back() == '/')
        {
            base.pop_back();
        }
        return base + "/ota/firmware.bin";
    }

#if defined(ESP_PLATFORM)

    void SettingsController::Impl::copy_string_field(const cJSON* object,
                                                     const char*  key,
                                                     char*        dest,
                                                     size_t       length)
    {
        if (object == nullptr || dest == nullptr || length == 0U)
        {
            return;
        }
        const cJSON* item = cJSON_GetObjectItemCaseSensitive(object, key);
        if (cJSON_IsString(item) && item->valuestring != nullptr)
        {
            std::strncpy(dest, item->valuestring, length - 1U);
            dest[length - 1U] = '\0';
        }
    }

    void SettingsController::Impl::copy_bool_field(const cJSON* object, const char* key, bool& dest)
    {
        if (object == nullptr)
        {
            return;
        }
        const cJSON* item = cJSON_GetObjectItemCaseSensitive(object, key);
        if (cJSON_IsBool(item))
        {
            dest = cJSON_IsTrue(item);
        }
    }

    void
    SettingsController::Impl::copy_number_field(const cJSON* object, const char* key, int& dest)
    {
        if (object == nullptr)
        {
            return;
        }
        const cJSON* item = cJSON_GetObjectItemCaseSensitive(object, key);
        if (cJSON_IsNumber(item))
        {
            dest = static_cast<int>(item->valuedouble);
        }
    }

    void SettingsController::Impl::handle_restore_from_json(const std::string& json)
    {
        cJSON* root = cJSON_Parse(json.c_str());
        if (root == nullptr)
        {
            post_backup_status("Invalid backup file");
            return;
        }

        app_cfg_t restored;
        app_cfg_set_defaults(&restored);

        const cJSON* home_assistant = cJSON_GetObjectItemCaseSensitive(root, "home_assistant");
        if (cJSON_IsObject(home_assistant))
        {
            copy_bool_field(home_assistant, "enabled", restored.home_assistant.enabled);
            copy_string_field(home_assistant,
                              "url",
                              restored.home_assistant.url,
                              sizeof(restored.home_assistant.url));
            copy_string_field(home_assistant,
                              "token",
                              restored.home_assistant.token,
                              sizeof(restored.home_assistant.token));
        }

        const cJSON* frigate = cJSON_GetObjectItemCaseSensitive(root, "frigate");
        if (cJSON_IsObject(frigate))
        {
            copy_bool_field(frigate, "enabled", restored.frigate.enabled);
            copy_string_field(frigate, "url", restored.frigate.url, sizeof(restored.frigate.url));
            copy_string_field(frigate,
                              "camera",
                              restored.frigate.camera_name,
                              sizeof(restored.frigate.camera_name));
            copy_bool_field(frigate, "snapshots", restored.frigate.snapshots_enabled);
        }

        const cJSON* mqtt = cJSON_GetObjectItemCaseSensitive(root, "mqtt");
        if (cJSON_IsObject(mqtt))
        {
            copy_bool_field(mqtt, "enabled", restored.mqtt.enabled);
            copy_string_field(
                mqtt, "broker", restored.mqtt.broker_uri, sizeof(restored.mqtt.broker_uri));
            copy_string_field(
                mqtt, "client_id", restored.mqtt.client_id, sizeof(restored.mqtt.client_id));
            copy_string_field(
                mqtt, "username", restored.mqtt.username, sizeof(restored.mqtt.username));
            copy_string_field(
                mqtt, "password", restored.mqtt.password, sizeof(restored.mqtt.password));
            copy_bool_field(mqtt, "use_tls", restored.mqtt.use_tls);
            copy_bool_field(mqtt, "ha_discovery", restored.mqtt.ha_discovery);
        }

        const cJSON* ui = cJSON_GetObjectItemCaseSensitive(root, "ui");
        if (cJSON_IsObject(ui))
        {
            int theme_value      = static_cast<int>(restored.ui.theme);
            int brightness_value = restored.ui.brightness;
            int timeout_value    = restored.ui.screen_timeout_seconds;
            copy_number_field(ui, "theme", theme_value);
            copy_number_field(ui, "brightness", brightness_value);
            copy_number_field(ui, "screen_timeout", timeout_value);
            restored.ui.theme                  = static_cast<app_cfg_ui_theme_t>(theme_value);
            restored.ui.brightness             = std::clamp(brightness_value,
                                                static_cast<int>(kMinBrightness),
                                                static_cast<int>(kMaxBrightness));
            restored.ui.screen_timeout_seconds = static_cast<uint16_t>(std::max(timeout_value, 5));
        }

        const cJSON* network = cJSON_GetObjectItemCaseSensitive(root, "network");
        if (cJSON_IsObject(network))
        {
            copy_string_field(
                network, "ssid", restored.network.ssid, sizeof(restored.network.ssid));
            copy_string_field(
                network, "password", restored.network.password, sizeof(restored.network.password));
            copy_string_field(
                network, "hostname", restored.network.hostname, sizeof(restored.network.hostname));
            copy_bool_field(network, "use_dhcp", restored.network.use_dhcp);
            copy_string_field(network,
                              "static_ip",
                              restored.network.static_ip,
                              sizeof(restored.network.static_ip));
            copy_string_field(
                network, "gateway", restored.network.gateway, sizeof(restored.network.gateway));
            copy_string_field(
                network, "netmask", restored.network.netmask, sizeof(restored.network.netmask));
            copy_string_field(network,
                              "dns_primary",
                              restored.network.dns_primary,
                              sizeof(restored.network.dns_primary));
            copy_string_field(network,
                              "dns_secondary",
                              restored.network.dns_secondary,
                              sizeof(restored.network.dns_secondary));
            copy_string_field(
                network, "timezone", restored.network.timezone, sizeof(restored.network.timezone));
            copy_string_field(network,
                              "ntp_server",
                              restored.network.ntp_server,
                              sizeof(restored.network.ntp_server));
            copy_bool_field(network, "sntp_sync", restored.network.sntp_sync_enabled);
        }

        const cJSON* safety = cJSON_GetObjectItemCaseSensitive(root, "safety");
        if (cJSON_IsObject(safety))
        {
            copy_bool_field(safety, "child_lock", restored.safety.child_lock);
            copy_bool_field(safety, "disable_wifi", restored.safety.disable_wifi);
            copy_bool_field(safety, "allow_ota", restored.safety.allow_ota);
            copy_bool_field(safety, "diagnostics_opt_in", restored.safety.diagnostics_opt_in);
        }

        cJSON_Delete(root);

        if (app_cfg_validate(&restored) != ESP_OK)
        {
            post_backup_status("Backup validation failed");
            return;
        }

        config_ = restored;
        persist_config();
        apply_current_theme();
        GetHAL()->setDisplayBrightness(config_.ui.brightness);
        ui_page_settings_set_brightness(config_.ui.brightness);
        refresh_all_connections();
        post_backup_status("Backup restored");
    }

#endif

    SettingsController::SettingsController() : impl_(std::make_unique<Impl>()) {}

    SettingsController::~SettingsController() = default;

    void SettingsController::PublishInitialState()
    {
        impl_->PublishInitialState();
    }

    void SettingsController::RunConnectionTest(const char* tester_id)
    {
        impl_->RunConnectionTest(tester_id);
    }

    void SettingsController::SetDarkMode(bool enabled)
    {
        impl_->SetDarkMode(enabled);
    }

    void SettingsController::SetThemeVariant(const char* variant_id)
    {
        impl_->SetThemeVariant(variant_id);
    }

    void SettingsController::SetBrightness(uint8_t percent)
    {
        impl_->SetBrightness(percent);
    }

    void SettingsController::OpenDisplaySettings()
    {
        impl_->OpenDisplaySettings();
    }

    void SettingsController::OpenNetworkSettings()
    {
        impl_->OpenNetworkSettings();
    }

    void SettingsController::SyncTime()
    {
        impl_->SyncTime();
    }

    void SettingsController::CheckForUpdates()
    {
        impl_->CheckForUpdates();
    }

    void SettingsController::StartOtaUpdate()
    {
        impl_->StartOtaUpdate();
    }

    void SettingsController::OpenDiagnostics()
    {
        impl_->OpenDiagnostics();
    }

    void SettingsController::ExportLogs()
    {
        impl_->ExportLogs();
    }

    void SettingsController::BackupNow()
    {
        impl_->BackupNow();
    }

    void SettingsController::RestoreBackup()
    {
        impl_->RestoreBackup();
    }

}  // namespace custom::integration
