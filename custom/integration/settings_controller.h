/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <cstdint>
#include <memory>

namespace custom::integration
{

    class SettingsController
    {
    public:
        SettingsController();
        ~SettingsController();

        SettingsController(const SettingsController&)            = delete;
        SettingsController& operator=(const SettingsController&) = delete;

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
        class Impl;
        std::unique_ptr<Impl> impl_;
    };

}  // namespace custom::integration
