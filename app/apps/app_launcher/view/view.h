/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <memory>

#include "integration/cctv_controller.h"
#include "integration/media_controller.h"
#include "integration/settings_controller.h"

struct ui_root_t;

namespace launcher_view
{

    class LauncherView
    {
    public:
        void init();
        void update();
        ~LauncherView();

    private:
        ui_root_t*                                               _ui_root = nullptr;
        std::unique_ptr<custom::integration::SettingsController> _settings_controller;
        std::unique_ptr<custom::integration::MediaController>    _media_controller;
        std::unique_ptr<custom::integration::CctvController>     _cctv_controller;
    };

}  // namespace launcher_view
