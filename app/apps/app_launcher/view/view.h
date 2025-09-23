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

typedef struct _lv_obj_t   lv_obj_t;
typedef struct _lv_timer_t lv_timer_t;
typedef struct _lv_event_t lv_event_t;
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
        void ensure_controllers();
        void build_ui();
        void destroy_ui();

        static void build_async_cb(void* param);
        static void pointer_event_cb(lv_event_t* event);
        static void heartbeat_timer_cb(lv_timer_t* timer);

        ui_root_t*                                               _ui_root = nullptr;
        std::unique_ptr<custom::integration::SettingsController> _settings_controller;
        std::unique_ptr<custom::integration::MediaController>    _media_controller;
        std::unique_ptr<custom::integration::CctvController>     _cctv_controller;
        lv_timer_t*                                              _heartbeat_timer = nullptr;
        lv_obj_t*                                                _screen          = nullptr;
        bool                                                     _init_pending    = false;
        bool                                                     _touch_logged    = false;
    };

}  // namespace launcher_view
