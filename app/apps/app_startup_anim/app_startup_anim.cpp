/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "app_startup_anim.h"
#include <hal/hal.h>
#include <mooncake.h>
#include <mooncake_log.h>
#include <assets/assets.h>

using namespace mooncake;
using namespace smooth_ui_toolkit;
using namespace smooth_ui_toolkit::lvgl_cpp;

#define FIRMWARE_VERSION "V1.0.1"

AppStartupAnim::AppStartupAnim()
{
    // 配置 App 信息
    setAppInfo().name = "AppStartupAnim";
}

void AppStartupAnim::onCreate()
{
    mclog::tagInfo(getAppInfo().name, "on create");
}

void AppStartupAnim::onOpen()
{
    mclog::tagInfo(getAppInfo().name, "on open");

    LvglLockGuard lock;

    lv_obj_t* screen          = lv_screen_active();
    _cached_screen_bg_color   = lv_obj_get_style_bg_color(screen, LV_PART_MAIN);
    _cached_screen_bg_opa     = lv_obj_get_style_bg_opa(screen, LV_PART_MAIN);
    _cached_screen_scrollable = lv_obj_has_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    _screen_state_cached      = true;

    lv_obj_set_style_bg_color(screen, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    _logo_tab = std::make_unique<Image>(lv_screen_active());
    _logo_tab->setAlign(LV_ALIGN_TOP_MID);
    _logo_tab->setSrc(&logo_tab);
    _logo_tab->setPos(-46, 785);

    _logo_5 = std::make_unique<Image>(lv_screen_active());
    _logo_5->setAlign(LV_ALIGN_TOP_MID);
    _logo_5->setSrc(&logo_5);
    _logo_5->setPos(700, 308);

    _label_version = std::make_unique<Label>(lv_screen_active());
    _label_version->setTextColor(lv_color_hex(0xCCCCCC));
    _label_version->setTextFont(&lv_font_montserrat_24);
    _label_version->align(LV_ALIGN_CENTER, 580, 320);
    _label_version->setText(FIRMWARE_VERSION);

    _anim_logo_tab_y.pause();
    _anim_logo_tab_y.teleport(785);
    _anim_logo_tab_y.springOptions().visualDuration = 0.6;
    _anim_logo_tab_y.springOptions().bounce         = 0.2;

    _anim_logo_tab_opa.pause();
    _anim_logo_tab_opa.teleport(0);
    _anim_logo_tab_opa.easingOptions().easingFunction = ease::linear;
    _anim_logo_tab_opa.easingOptions().duration       = 0.6;

    _anim_logo_5_x.pause();
    _anim_logo_5_x.teleport(700);
    _anim_logo_5_x.springOptions().visualDuration = 0.4;
    _anim_logo_5_x.springOptions().bounce         = 0.2;

    _time_count = GetHAL()->millis();
}

void AppStartupAnim::onRunning()
{
    if (_anime_state == AnimState_StartupDelay) {
        if (GetHAL()->millis() - _time_count > 400) {
            _anime_state = AnimState_LogoTabMoveUp;
            _anim_logo_tab_y.play();
            _anim_logo_tab_y = 309;
            _anim_logo_tab_opa.play();
            _anim_logo_tab_opa = 255;
        }
        return;
    }

    else if (_anime_state == AnimState_LogoTabMoveUp) {
        if (_anim_logo_tab_y.done()) {
            _anime_state = AnimState_Logo5MoveLeft;
            _anim_logo_5_x.play();
            _anim_logo_5_x = 82;
        }
    }

    else if (_anime_state == AnimState_Logo5MoveLeft) {
        if (_anim_logo_5_x.done()) {
            _anime_state = AnimState_FinalDelay;
            _time_count  = GetHAL()->millis();
            GetHAL()->startWifiAp();
        }
        if (!_is_sfx_played) {
            if (_anim_logo_5_x.directValue() < 90) {
                GetHAL()->setSpeakerVolume(80);
                GetHAL()->playStartupSfx();
                _is_sfx_played = true;
            }
        }
    }

    else if (_anime_state == AnimState_FinalDelay) {
        if (GetHAL()->millis() - _time_count > 1000) {
            close();
        }
        return;
    }

    LvglLockGuard lock;

    _logo_tab->setY(_anim_logo_tab_y);
    _logo_tab->setOpa(_anim_logo_tab_opa);
    _logo_5->setX(_anim_logo_5_x);
}

void AppStartupAnim::onClose()
{
    mclog::tagInfo(getAppInfo().name, "on close");

    LvglLockGuard lock;

    _logo_tab.reset();
    _logo_5.reset();
    _label_version.reset();

    if (_screen_state_cached) {
        lv_obj_t* screen = lv_screen_active();
        lv_obj_set_style_bg_color(screen, _cached_screen_bg_color, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(screen, _cached_screen_bg_opa, LV_PART_MAIN);
        if (_cached_screen_scrollable) {
            lv_obj_add_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
        } else {
            lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
        }
        _screen_state_cached = false;
    }

    GetHAL()->setSpeakerVolume(60);
}
