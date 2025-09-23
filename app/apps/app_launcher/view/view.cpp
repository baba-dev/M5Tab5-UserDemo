/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "view.h"

#include <lvgl.h>
#include <mooncake_log.h>
#include <smooth_lvgl.h>
#include <string>

#include "hal/hal.h"
#include "integration/cctv_controller.h"
#include "integration/media_controller.h"
#include "integration/settings_controller.h"
#include "ui/pages/ui_page_settings.h"
#include "ui/ui_root.h"

using namespace launcher_view;
using custom::integration::CctvController;
using custom::integration::MediaController;
using custom::integration::SettingsController;

static const std::string _tag = "launcher-view";

void LauncherView::init()
{
    mclog::tagInfo(_tag, "init");

    if (_init_pending)
    {
        LV_LOG_WARN("Launcher init already pending");
        return;
    }

    ensure_controllers();

    _init_pending = true;
    if (lv_async_call(LauncherView::build_async_cb, this) != LV_RES_OK)
    {
        _init_pending = false;
        LV_LOG_WARN("Failed to schedule launcher build");
    }
}

void LauncherView::ensure_controllers()
{
    if (_settings_controller == nullptr)
    {
        _settings_controller = std::make_unique<SettingsController>();
    }
    if (_media_controller == nullptr)
    {
        _media_controller = std::make_unique<MediaController>();
    }
    if (_cctv_controller == nullptr)
    {
        _cctv_controller = std::make_unique<CctvController>();
    }
}

void LauncherView::build_async_cb(void* param)
{
    auto* view = static_cast<LauncherView*>(param);
    if (view == nullptr)
    {
        return;
    }
    view->build_ui();
}

void LauncherView::build_ui()
{
    _init_pending = false;

    destroy_ui();

    lv_obj_t* screen = lv_screen_active();
    if (screen == nullptr)
    {
        LV_LOG_WARN("No active screen for launcher view");
        return;
    }

    if (_settings_controller == nullptr)
    {
        LV_LOG_WARN("Settings controller missing; skipping UI build");
        return;
    }

    _screen       = screen;
    _touch_logged = false;

    lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);

    lv_obj_remove_event_cb_with_user_data(screen, LauncherView::pointer_event_cb, this);
    lv_obj_add_event_cb(screen, LauncherView::pointer_event_cb, LV_EVENT_PRESSED, this);
    lv_obj_add_event_cb(screen, LauncherView::pointer_event_cb, LV_EVENT_PRESSING, this);

    _ui_root = ui_root_create();
    if (_ui_root == nullptr)
    {
        LV_LOG_WARN("Failed to create launcher UI root");
        return;
    }

    ui_page_settings_actions_t actions{};
    actions.run_connection_test = [](const char* tester_id, void* user_data)
    {
        auto* controller = static_cast<SettingsController*>(user_data);
        if (controller != nullptr)
        {
            controller->RunConnectionTest(tester_id);
        }
    };
    actions.set_dark_mode = [](bool enabled, void* user_data)
    {
        auto* controller = static_cast<SettingsController*>(user_data);
        if (controller != nullptr)
        {
            controller->SetDarkMode(enabled);
        }
    };
    actions.set_theme_variant = [](const char* variant_id, void* user_data)
    {
        auto* controller = static_cast<SettingsController*>(user_data);
        if (controller != nullptr)
        {
            controller->SetThemeVariant(variant_id);
        }
    };
    actions.set_brightness = [](uint8_t percent, void* user_data)
    {
        auto* controller = static_cast<SettingsController*>(user_data);
        if (controller != nullptr)
        {
            controller->SetBrightness(percent);
        }
    };
    actions.open_display_settings = [](void* user_data)
    {
        auto* controller = static_cast<SettingsController*>(user_data);
        if (controller != nullptr)
        {
            controller->OpenDisplaySettings();
        }
    };
    actions.open_network_settings = [](void* user_data)
    {
        auto* controller = static_cast<SettingsController*>(user_data);
        if (controller != nullptr)
        {
            controller->OpenNetworkSettings();
        }
    };
    actions.sync_time = [](void* user_data)
    {
        auto* controller = static_cast<SettingsController*>(user_data);
        if (controller != nullptr)
        {
            controller->SyncTime();
        }
    };
    actions.check_for_updates = [](void* user_data)
    {
        auto* controller = static_cast<SettingsController*>(user_data);
        if (controller != nullptr)
        {
            controller->CheckForUpdates();
        }
    };
    actions.start_ota_update = [](void* user_data)
    {
        auto* controller = static_cast<SettingsController*>(user_data);
        if (controller != nullptr)
        {
            controller->StartOtaUpdate();
        }
    };
    actions.open_diagnostics = [](void* user_data)
    {
        auto* controller = static_cast<SettingsController*>(user_data);
        if (controller != nullptr)
        {
            controller->OpenDiagnostics();
        }
    };
    actions.export_logs = [](void* user_data)
    {
        auto* controller = static_cast<SettingsController*>(user_data);
        if (controller != nullptr)
        {
            controller->ExportLogs();
        }
    };
    actions.backup_now = [](void* user_data)
    {
        auto* controller = static_cast<SettingsController*>(user_data);
        if (controller != nullptr)
        {
            controller->BackupNow();
        }
    };
    actions.restore_backup = [](void* user_data)
    {
        auto* controller = static_cast<SettingsController*>(user_data);
        if (controller != nullptr)
        {
            controller->RestoreBackup();
        }
    };

    ui_page_settings_set_actions(&actions, _settings_controller.get());

    if (_heartbeat_timer != nullptr)
    {
        lv_timer_del(_heartbeat_timer);
        _heartbeat_timer = nullptr;
    }
    _heartbeat_timer = lv_timer_create(LauncherView::heartbeat_timer_cb, 2000, this);
    if (_heartbeat_timer == nullptr)
    {
        LV_LOG_WARN("Failed to create heartbeat timer");
    }

    if (_settings_controller != nullptr)
    {
        _settings_controller->PublishInitialState();
    }
    if (_media_controller != nullptr)
    {
        _media_controller->PublishInitialState();
    }
    if (_cctv_controller != nullptr)
    {
        _cctv_controller->PublishInitialState();
    }
}

void LauncherView::destroy_ui()
{
    if (_heartbeat_timer != nullptr)
    {
        lv_timer_del(_heartbeat_timer);
        _heartbeat_timer = nullptr;
    }

    if (_screen != nullptr)
    {
        lv_obj_remove_event_cb_with_user_data(_screen, LauncherView::pointer_event_cb, this);
        _screen = nullptr;
    }

    _touch_logged = false;

    if (_ui_root != nullptr)
    {
        ui_page_settings_set_actions(nullptr, nullptr);
        ui_root_destroy(_ui_root);
        _ui_root = nullptr;
    }

    _init_pending = false;
}

void LauncherView::update() {}

LauncherView::~LauncherView()
{
    if (_ui_root != nullptr || _heartbeat_timer != nullptr || _screen != nullptr)
    {
        LvglLockGuard lock;
        destroy_ui();
    }

    _media_controller.reset();
    _cctv_controller.reset();
    _settings_controller.reset();
}

void LauncherView::pointer_event_cb(lv_event_t* event)
{
    if (event == nullptr)
    {
        return;
    }

    auto* view = static_cast<LauncherView*>(lv_event_get_user_data(event));
    if (view == nullptr)
    {
        return;
    }

    lv_event_code_t code = lv_event_get_code(event);
    if (code != LV_EVENT_PRESSED && code != LV_EVENT_PRESSING)
    {
        return;
    }

    if (view->_touch_logged && code == LV_EVENT_PRESSING)
    {
        return;
    }

    lv_indev_t* indev = lv_event_get_indev(event);
    lv_point_t  point = {0, 0};
    if (indev != NULL)
    {
        lv_indev_get_point(indev, &point);
    }

    LV_LOG_INFO("Touch event code=%d at (%d,%d) tick=%lu",
                (int)code,
                (int)point.x,
                (int)point.y,
                (unsigned long)lv_tick_get());

    view->_touch_logged = true;
}

void LauncherView::heartbeat_timer_cb(lv_timer_t* timer)
{
    if (timer == nullptr)
    {
        return;
    }

    LV_UNUSED(timer);
    LV_LOG_INFO("LVGL heartbeat tick=%lu", (unsigned long)lv_tick_get());
}
