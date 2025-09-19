/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "ui_page_settings.h"

#include <stdio.h>
#include <string.h>

#include "../ui_theme.h"
#include "../ui_wallpaper.h"

#define CONNECTION_TESTER_COUNT 3

typedef struct ui_page_settings_ctx_t ui_page_settings_ctx_t;

typedef struct
{
    const char*               id;
    const char*               label;
    lv_obj_t*                 button;
    lv_obj_t*                 pill;
    lv_obj_t*                 pill_label;
    ui_page_settings_status_t status;
    ui_page_settings_ctx_t*   ctx;
} connection_tester_t;

struct ui_page_settings_ctx_t
{
    lv_obj_t*                  page;
    lv_obj_t*                  content;
    ui_wallpaper_t*            wallpaper;
    connection_tester_t        testers[CONNECTION_TESTER_COUNT];
    lv_obj_t*                  dark_mode_switch;
    lv_obj_t*                  theme_dropdown;
    lv_obj_t*                  brightness_slider;
    lv_obj_t*                  brightness_value;
    lv_obj_t*                  display_settings_button;
    lv_obj_t*                  network_settings_button;
    lv_obj_t*                  sync_time_button;
    lv_obj_t*                  check_updates_button;
    lv_obj_t*                  start_ota_button;
    lv_obj_t*                  diagnostics_button;
    lv_obj_t*                  export_logs_button;
    lv_obj_t*                  backup_button;
    lv_obj_t*                  restore_button;
    lv_obj_t*                  ota_status_label;
    ui_page_settings_actions_t actions;
    void*                      actions_user_data;
    bool                       actions_bound;
    bool                       suppress_events;
};

static ui_page_settings_ctx_t* s_settings_ctx = NULL;

typedef struct
{
    const char* id;
    const char* label;
} theme_option_t;

static const theme_option_t k_theme_options[] = {
    {"system", "System Default"},
    {"solar", "Solar"},
    {"midnight", "Midnight"},
};

static const size_t kThemeOptionCount = sizeof(k_theme_options) / sizeof(k_theme_options[0]);

static const char* k_theme_options_list = "System Default\nSolar\nMidnight";

static const struct
{
    const char* id;
    const char* label;
} k_connection_testers[CONNECTION_TESTER_COUNT] = {
    {"wifi", "Wi-Fi"},
    {"ha", "Home Assistant"},
    {"cloud", "Cloud Relay"},
};

typedef struct
{
    char                      tester_id[32];
    ui_page_settings_status_t status;
    char                      message[64];
} connection_status_async_payload_t;

typedef struct
{
    char text[96];
} status_label_async_payload_t;

typedef struct
{
    bool dark_mode;
    char variant_id[32];
} theme_state_async_payload_t;

typedef struct
{
    uint8_t percent;
} brightness_async_payload_t;

static void ui_page_settings_delete_cb(lv_event_t* event);
static void connection_test_button_cb(lv_event_t* event);
static void dark_mode_switch_cb(lv_event_t* event);
static void theme_dropdown_cb(lv_event_t* event);
static void brightness_slider_cb(lv_event_t* event);
static void action_button_cb(lv_event_t* event);

static void apply_connection_status(connection_tester_t*      tester,
                                    ui_page_settings_status_t status,
                                    const char*               message)
{
    if (tester == NULL || tester->pill == NULL || tester->pill_label == NULL)
    {
        return;
    }

    lv_color_t  bg_color = ui_theme_color_outline();
    const char* fallback = "Unknown";

    switch (status)
    {
        case UI_PAGE_SETTINGS_STATUS_OK:
            bg_color = lv_color_hex(0x22c55e);
            fallback = "Online";
            break;
        case UI_PAGE_SETTINGS_STATUS_WARNING:
            bg_color = lv_color_hex(0xf97316);
            fallback = "Degraded";
            break;
        case UI_PAGE_SETTINGS_STATUS_ERROR:
            bg_color = lv_color_hex(0xef4444);
            fallback = "Offline";
            break;
        case UI_PAGE_SETTINGS_STATUS_UNKNOWN:
        default:
            bg_color = ui_theme_color_outline();
            fallback = "Unknown";
            break;
    }

    tester->status = status;

    lv_obj_set_style_bg_color(tester->pill, bg_color, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tester->pill, LV_OPA_COVER, LV_PART_MAIN);
    lv_label_set_text(tester->pill_label,
                      (message != NULL && message[0] != '\0') ? message : fallback);
}

static lv_obj_t* create_page_container(lv_obj_t* parent)
{
    lv_obj_t* page = lv_obj_create(parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_scroll_dir(page, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(page, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(page, LV_OBJ_FLAG_CLICKABLE);
    return page;
}

static lv_obj_t* create_content_container(lv_obj_t* page)
{
    lv_obj_t* content = lv_obj_create(page);
    lv_obj_remove_style_all(content);
    lv_obj_set_size(content, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_left(content, 192, LV_PART_MAIN);
    lv_obj_set_style_pad_right(content, 48, LV_PART_MAIN);
    lv_obj_set_style_pad_top(content, 48, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(content, 48, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(content, 32, LV_PART_MAIN);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
    return content;
}

static lv_obj_t* create_section_card(lv_obj_t* parent, const char* title, const char* subtitle)
{
    lv_obj_t* card = lv_obj_create(parent);
    lv_obj_remove_style_all(card);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_style_radius(card, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(card, ui_theme_color_surface(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(card, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_border_color(card, ui_theme_color_outline(), LV_PART_MAIN);
    lv_obj_set_style_border_opa(card, LV_OPA_40, LV_PART_MAIN);
    lv_obj_set_style_border_width(card, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_all(card, 32, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(card, 20, LV_PART_MAIN);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    if (title != NULL)
    {
        lv_obj_t* title_label = lv_label_create(card);
        lv_label_set_text(title_label, title);
        lv_obj_set_style_text_font(title_label, &lv_font_montserrat_28, LV_PART_MAIN);
        lv_obj_set_style_text_color(title_label, ui_theme_color_on_surface(), LV_PART_MAIN);
    }

    if (subtitle != NULL)
    {
        lv_obj_t* subtitle_label = lv_label_create(card);
        lv_label_set_text(subtitle_label, subtitle);
        lv_obj_set_style_text_font(subtitle_label, &lv_font_montserrat_18, LV_PART_MAIN);
        lv_obj_set_style_text_color(subtitle_label, ui_theme_color_muted(), LV_PART_MAIN);
        lv_label_set_long_mode(subtitle_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(subtitle_label, LV_PCT(100));
    }

    return card;
}

static lv_obj_t* create_setting_row(lv_obj_t* parent, const char* label_text)
{
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_remove_style_all(row);
    lv_obj_set_width(row, LV_PCT(100));
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(row, 16, LV_PART_MAIN);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    if (label_text != NULL)
    {
        lv_obj_t* label = lv_label_create(row);
        lv_label_set_text(label, label_text);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_20, LV_PART_MAIN);
        lv_obj_set_style_text_color(label, ui_theme_color_on_surface(), LV_PART_MAIN);
        lv_obj_set_width(label, LV_PCT(40));
    }

    return row;
}

static lv_obj_t* create_action_button(lv_obj_t* parent, const char* title, const char* description)
{
    lv_obj_t* button = lv_btn_create(parent);
    lv_obj_remove_style_all(button);
    lv_obj_set_style_radius(button, 18, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, ui_theme_color_surface(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(button, LV_OPA_70, LV_PART_MAIN);
    lv_obj_set_style_border_color(button, ui_theme_color_outline(), LV_PART_MAIN);
    lv_obj_set_style_border_width(button, 1, LV_PART_MAIN);
    lv_obj_set_style_border_opa(button, LV_OPA_40, LV_PART_MAIN);
    lv_obj_set_style_pad_all(button, 20, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(button, 8, LV_PART_MAIN);
    lv_obj_set_style_text_align(button, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_flex_flow(button, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(button, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(button, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_width(button, LV_PCT(45));

    lv_obj_t* title_label = lv_label_create(button);
    lv_label_set_text(title_label, title != NULL ? title : "Action");
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(title_label, ui_theme_color_on_surface(), LV_PART_MAIN);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(title_label, LV_PCT(100));

    if (description != NULL)
    {
        lv_obj_t* desc_label = lv_label_create(button);
        lv_label_set_text(desc_label, description);
        lv_obj_set_style_text_font(desc_label, &lv_font_montserrat_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(desc_label, ui_theme_color_muted(), LV_PART_MAIN);
        lv_label_set_long_mode(desc_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(desc_label, LV_PCT(100));
    }

    return button;
}

static void build_connectivity_section(ui_page_settings_ctx_t* ctx)
{
    lv_obj_t* card =
        create_section_card(ctx->content, "Connectivity", "Run quick checks for each integration.");

    lv_obj_t* row = lv_obj_create(card);
    lv_obj_remove_style_all(row);
    lv_obj_set_width(row, LV_PCT(100));
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(row, 18, LV_PART_MAIN);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    for (size_t i = 0; i < CONNECTION_TESTER_COUNT; i++)
    {
        connection_tester_t* tester = &ctx->testers[i];
        tester->id                  = k_connection_testers[i].id;
        tester->label               = k_connection_testers[i].label;
        tester->ctx                 = ctx;

        lv_obj_t* button = lv_btn_create(row);
        lv_obj_remove_style_all(button);
        lv_obj_set_width(button, 260);
        lv_obj_set_style_radius(button, 18, LV_PART_MAIN);
        lv_obj_set_style_bg_color(button, lv_color_hex(0x1b2430), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(button, LV_OPA_80, LV_PART_MAIN);
        lv_obj_set_style_border_width(button, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(button, 20, LV_PART_MAIN);
        lv_obj_set_style_pad_gap(button, 12, LV_PART_MAIN);
        lv_obj_set_flex_flow(button, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(
            button, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_clear_flag(button, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_event_cb(button, connection_test_button_cb, LV_EVENT_CLICKED, tester);
        tester->button = button;

        lv_obj_t* title = lv_label_create(button);
        lv_label_set_text(title, tester->label);
        lv_obj_set_style_text_font(title, &lv_font_montserrat_22, LV_PART_MAIN);
        lv_obj_set_style_text_color(title, lv_color_hex(0xe6edf3), LV_PART_MAIN);

        lv_obj_t* pill = lv_obj_create(button);
        lv_obj_remove_style_all(pill);
        lv_obj_set_width(pill, LV_SIZE_CONTENT);
        lv_obj_set_style_radius(pill, 999, LV_PART_MAIN);
        lv_obj_set_style_pad_hor(pill, 16, LV_PART_MAIN);
        lv_obj_set_style_pad_ver(pill, 6, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(pill, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_width(pill, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_gap(pill, 0, LV_PART_MAIN);
        lv_obj_clear_flag(pill, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* pill_label = lv_label_create(pill);
        lv_obj_set_style_text_font(pill_label, &lv_font_montserrat_16, LV_PART_MAIN);
        lv_obj_set_style_text_color(pill_label, lv_color_hex(0x0f172a), LV_PART_MAIN);

        tester->pill       = pill;
        tester->pill_label = pill_label;
        apply_connection_status(tester, UI_PAGE_SETTINGS_STATUS_UNKNOWN, "Unknown");
    }
}

static void build_theme_section(ui_page_settings_ctx_t* ctx)
{
    lv_obj_t* card =
        create_section_card(ctx->content, "Theme & Display", "Adjust the on-device appearance.");

    lv_obj_t* dark_row    = create_setting_row(card, "Dark mode");
    lv_obj_t* dark_switch = lv_switch_create(dark_row);
    lv_obj_add_event_cb(dark_switch, dark_mode_switch_cb, LV_EVENT_VALUE_CHANGED, ctx);
    lv_obj_set_style_bg_color(dark_switch, ui_theme_color_outline(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(dark_switch, LV_OPA_40, LV_PART_MAIN);
    lv_obj_set_style_bg_color(
        dark_switch, ui_theme_color_accent(), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(dark_switch, LV_OPA_COVER, LV_PART_INDICATOR | LV_STATE_CHECKED);
    ctx->dark_mode_switch = dark_switch;

    lv_obj_t* theme_row = create_setting_row(card, "Accent palette");
    lv_obj_t* dropdown  = lv_dropdown_create(theme_row);
    lv_dropdown_set_options_static(dropdown, k_theme_options_list);
    lv_obj_set_width(dropdown, 200);
    lv_obj_add_event_cb(dropdown, theme_dropdown_cb, LV_EVENT_VALUE_CHANGED, ctx);
    ctx->theme_dropdown = dropdown;

    lv_obj_t* brightness_row = create_setting_row(card, "Brightness");
    lv_obj_t* slider         = lv_slider_create(brightness_row);
    lv_slider_set_range(slider, 5, 100);
    lv_slider_set_value(slider, 75, LV_ANIM_OFF);
    lv_obj_set_width(slider, 260);
    lv_obj_add_event_cb(slider, brightness_slider_cb, LV_EVENT_VALUE_CHANGED, ctx);
    ctx->brightness_slider = slider;

    lv_obj_t* value_label = lv_label_create(brightness_row);
    lv_label_set_text(value_label, "75%");
    lv_obj_set_style_text_font(value_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_style_text_color(value_label, ui_theme_color_on_surface(), LV_PART_MAIN);
    ctx->brightness_value = value_label;
}

static void build_network_section(ui_page_settings_ctx_t* ctx)
{
    lv_obj_t* card =
        create_section_card(ctx->content, "Network & Time", "Manage connectivity utilities.");

    lv_obj_t* row = lv_obj_create(card);
    lv_obj_remove_style_all(row);
    lv_obj_set_width(row, LV_PCT(100));
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(row, 18, LV_PART_MAIN);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    ctx->network_settings_button =
        create_action_button(row, "Wi-Fi settings", "Join or edit known networks.");
    ctx->sync_time_button = create_action_button(row, "Sync clock", "Update RTC from NTP or GPS.");
    ctx->display_settings_button =
        create_action_button(row, "Display tools", "Calibrate touch and color.");

    lv_obj_add_event_cb(ctx->network_settings_button, action_button_cb, LV_EVENT_CLICKED, ctx);
    lv_obj_add_event_cb(ctx->sync_time_button, action_button_cb, LV_EVENT_CLICKED, ctx);
    lv_obj_add_event_cb(ctx->display_settings_button, action_button_cb, LV_EVENT_CLICKED, ctx);
}

static void build_updates_section(ui_page_settings_ctx_t* ctx)
{
    lv_obj_t* card =
        create_section_card(ctx->content, "Updates", "Keep firmware and apps current.");

    ctx->ota_status_label = lv_label_create(card);
    lv_label_set_text(ctx->ota_status_label, "Idle");
    lv_obj_set_style_text_font(ctx->ota_status_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_style_text_color(ctx->ota_status_label, ui_theme_color_muted(), LV_PART_MAIN);
    lv_obj_set_width(ctx->ota_status_label, LV_PCT(100));

    lv_obj_t* row = lv_obj_create(card);
    lv_obj_remove_style_all(row);
    lv_obj_set_width(row, LV_PCT(100));
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(row, 18, LV_PART_MAIN);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    ctx->check_updates_button =
        create_action_button(row, "Check for updates", "Query OTA servers for new builds.");
    ctx->start_ota_button =
        create_action_button(row, "Start OTA", "Apply the latest downloaded firmware.");

    lv_obj_add_event_cb(ctx->check_updates_button, action_button_cb, LV_EVENT_CLICKED, ctx);
    lv_obj_add_event_cb(ctx->start_ota_button, action_button_cb, LV_EVENT_CLICKED, ctx);
}

static void build_diagnostics_section(ui_page_settings_ctx_t* ctx)
{
    lv_obj_t* card =
        create_section_card(ctx->content, "Diagnostics", "Capture logs and run system tests.");

    lv_obj_t* row = lv_obj_create(card);
    lv_obj_remove_style_all(row);
    lv_obj_set_width(row, LV_PCT(100));
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(row, 18, LV_PART_MAIN);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    ctx->diagnostics_button =
        create_action_button(row, "Run diagnostics", "Perform onboard hardware checks.");
    ctx->export_logs_button =
        create_action_button(row, "Export logs", "Save recent logs to external storage.");

    lv_obj_add_event_cb(ctx->diagnostics_button, action_button_cb, LV_EVENT_CLICKED, ctx);
    lv_obj_add_event_cb(ctx->export_logs_button, action_button_cb, LV_EVENT_CLICKED, ctx);
}

static void build_backup_section(ui_page_settings_ctx_t* ctx)
{
    lv_obj_t* card = create_section_card(
        ctx->content, "Backup & Restore", "Safeguard and recover your configuration.");

    lv_obj_t* row = lv_obj_create(card);
    lv_obj_remove_style_all(row);
    lv_obj_set_width(row, LV_PCT(100));
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(row, 18, LV_PART_MAIN);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    ctx->backup_button =
        create_action_button(row, "Backup now", "Generate a snapshot of current settings.");
    ctx->restore_button =
        create_action_button(row, "Restore backup", "Apply a saved snapshot to this device.");

    lv_obj_add_event_cb(ctx->backup_button, action_button_cb, LV_EVENT_CLICKED, ctx);
    lv_obj_add_event_cb(ctx->restore_button, action_button_cb, LV_EVENT_CLICKED, ctx);
}

static void connection_status_async_cb(void* param)
{
    connection_status_async_payload_t* payload = (connection_status_async_payload_t*)param;
    ui_page_settings_ctx_t*            ctx     = s_settings_ctx;
    if (payload == NULL || ctx == NULL || ctx->page == NULL || !lv_obj_is_valid(ctx->page))
    {
        if (payload != NULL)
        {
            lv_free(payload);
        }
        return;
    }

    for (size_t i = 0; i < CONNECTION_TESTER_COUNT; i++)
    {
        connection_tester_t* tester = &ctx->testers[i];
        if (tester->id != NULL && strcmp(tester->id, payload->tester_id) == 0)
        {
            apply_connection_status(tester, payload->status, payload->message);
            break;
        }
    }

    lv_free(payload);
}

static void status_label_async_cb(void* param)
{
    status_label_async_payload_t* payload = (status_label_async_payload_t*)param;
    ui_page_settings_ctx_t*       ctx     = s_settings_ctx;
    if (payload == NULL || ctx == NULL || ctx->ota_status_label == NULL || ctx->page == NULL
        || !lv_obj_is_valid(ctx->page))
    {
        if (payload != NULL)
        {
            lv_free(payload);
        }
        return;
    }

    lv_label_set_text(ctx->ota_status_label, payload->text);
    lv_free(payload);
}

static void theme_state_async_cb(void* param)
{
    theme_state_async_payload_t* payload = (theme_state_async_payload_t*)param;
    ui_page_settings_ctx_t*      ctx     = s_settings_ctx;
    if (payload == NULL || ctx == NULL || ctx->page == NULL || !lv_obj_is_valid(ctx->page))
    {
        if (payload != NULL)
        {
            lv_free(payload);
        }
        return;
    }

    ctx->suppress_events = true;

    if (ctx->dark_mode_switch != NULL)
    {
        if (payload->dark_mode)
        {
            lv_obj_add_state(ctx->dark_mode_switch, LV_STATE_CHECKED);
        }
        else
        {
            lv_obj_clear_state(ctx->dark_mode_switch, LV_STATE_CHECKED);
        }
    }

    if (ctx->theme_dropdown != NULL && payload->variant_id[0] != '\0')
    {
        for (uint32_t i = 0; i < kThemeOptionCount; i++)
        {
            if (strcmp(k_theme_options[i].id, payload->variant_id) == 0)
            {
                lv_dropdown_set_selected(ctx->theme_dropdown, i);
                break;
            }
        }
    }

    ctx->suppress_events = false;
    lv_free(payload);
}

static void brightness_async_cb(void* param)
{
    brightness_async_payload_t* payload = (brightness_async_payload_t*)param;
    ui_page_settings_ctx_t*     ctx     = s_settings_ctx;
    if (payload == NULL || ctx == NULL || ctx->page == NULL || !lv_obj_is_valid(ctx->page))
    {
        if (payload != NULL)
        {
            lv_free(payload);
        }
        return;
    }

    ctx->suppress_events = true;

    if (ctx->brightness_slider != NULL)
    {
        lv_slider_set_value(ctx->brightness_slider, payload->percent, LV_ANIM_OFF);
    }

    if (ctx->brightness_value != NULL)
    {
        char buffer[8];
        lv_snprintf(buffer, sizeof(buffer), "%u%%", (unsigned)payload->percent);
        lv_label_set_text(ctx->brightness_value, buffer);
    }

    ctx->suppress_events = false;
    lv_free(payload);
}

static void ui_page_settings_delete_cb(lv_event_t* event)
{
    if (event == NULL)
    {
        return;
    }

    ui_page_settings_ctx_t* ctx = (ui_page_settings_ctx_t*)lv_event_get_user_data(event);
    if (ctx == NULL)
    {
        return;
    }

    if (ctx->wallpaper != NULL)
    {
        ui_wallpaper_detach(ctx->wallpaper);
        ctx->wallpaper = NULL;
    }

    if (s_settings_ctx == ctx)
    {
        s_settings_ctx = NULL;
    }

    lv_free(ctx);
}

static void connection_test_button_cb(lv_event_t* event)
{
    if (event == NULL)
    {
        return;
    }

    connection_tester_t* tester = (connection_tester_t*)lv_event_get_user_data(event);
    if (tester == NULL || tester->ctx == NULL)
    {
        return;
    }

    ui_page_settings_ctx_t* ctx = tester->ctx;
    if (ctx->actions_bound && ctx->actions.run_connection_test != NULL)
    {
        ctx->actions.run_connection_test(tester->id, ctx->actions_user_data);
    }
}

static void dark_mode_switch_cb(lv_event_t* event)
{
    if (event == NULL)
    {
        return;
    }

    ui_page_settings_ctx_t* ctx = (ui_page_settings_ctx_t*)lv_event_get_user_data(event);
    if (ctx == NULL || ctx->suppress_events)
    {
        return;
    }

    if (ctx->actions_bound && ctx->actions.set_dark_mode != NULL)
    {
        bool enabled = lv_obj_has_state(lv_event_get_target(event), LV_STATE_CHECKED);
        ctx->actions.set_dark_mode(enabled, ctx->actions_user_data);
    }
}

static void theme_dropdown_cb(lv_event_t* event)
{
    if (event == NULL)
    {
        return;
    }

    ui_page_settings_ctx_t* ctx = (ui_page_settings_ctx_t*)lv_event_get_user_data(event);
    if (ctx == NULL || ctx->suppress_events)
    {
        return;
    }

    if (ctx->actions_bound && ctx->actions.set_theme_variant != NULL)
    {
        uint16_t index = lv_dropdown_get_selected(lv_event_get_target(event));
        if (index < kThemeOptionCount)
        {
            ctx->actions.set_theme_variant(k_theme_options[index].id, ctx->actions_user_data);
        }
    }
}

static void update_brightness_label(ui_page_settings_ctx_t* ctx, uint16_t value)
{
    if (ctx == NULL || ctx->brightness_value == NULL)
    {
        return;
    }

    char buffer[8];
    lv_snprintf(buffer, sizeof(buffer), "%u%%", (unsigned)value);
    lv_label_set_text(ctx->brightness_value, buffer);
}

static void brightness_slider_cb(lv_event_t* event)
{
    if (event == NULL)
    {
        return;
    }

    ui_page_settings_ctx_t* ctx = (ui_page_settings_ctx_t*)lv_event_get_user_data(event);
    if (ctx == NULL)
    {
        return;
    }

    lv_obj_t* slider = lv_event_get_target(event);
    uint16_t  value  = (uint16_t)lv_slider_get_value(slider);
    update_brightness_label(ctx, value);

    if (ctx->suppress_events)
    {
        return;
    }

    if (ctx->actions_bound && ctx->actions.set_brightness != NULL)
    {
        ctx->actions.set_brightness((uint8_t)value, ctx->actions_user_data);
    }
}

static void action_button_cb(lv_event_t* event)
{
    if (event == NULL)
    {
        return;
    }

    ui_page_settings_ctx_t* ctx = (ui_page_settings_ctx_t*)lv_event_get_user_data(event);
    if (ctx == NULL || !ctx->actions_bound)
    {
        return;
    }

    lv_obj_t* target = lv_event_get_target(event);

    if (target == ctx->network_settings_button)
    {
        if (ctx->actions.open_network_settings != NULL)
        {
            ctx->actions.open_network_settings(ctx->actions_user_data);
        }
    }
    else if (target == ctx->sync_time_button)
    {
        if (ctx->actions.sync_time != NULL)
        {
            ctx->actions.sync_time(ctx->actions_user_data);
        }
    }
    else if (target == ctx->display_settings_button)
    {
        if (ctx->actions.open_display_settings != NULL)
        {
            ctx->actions.open_display_settings(ctx->actions_user_data);
        }
    }
    else if (target == ctx->check_updates_button)
    {
        if (ctx->actions.check_for_updates != NULL)
        {
            ctx->actions.check_for_updates(ctx->actions_user_data);
        }
    }
    else if (target == ctx->start_ota_button)
    {
        if (ctx->actions.start_ota_update != NULL)
        {
            ctx->actions.start_ota_update(ctx->actions_user_data);
        }
    }
    else if (target == ctx->diagnostics_button)
    {
        if (ctx->actions.open_diagnostics != NULL)
        {
            ctx->actions.open_diagnostics(ctx->actions_user_data);
        }
    }
    else if (target == ctx->export_logs_button)
    {
        if (ctx->actions.export_logs != NULL)
        {
            ctx->actions.export_logs(ctx->actions_user_data);
        }
    }
    else if (target == ctx->backup_button)
    {
        if (ctx->actions.backup_now != NULL)
        {
            ctx->actions.backup_now(ctx->actions_user_data);
        }
    }
    else if (target == ctx->restore_button)
    {
        if (ctx->actions.restore_backup != NULL)
        {
            ctx->actions.restore_backup(ctx->actions_user_data);
        }
    }
}

lv_obj_t* ui_page_settings_create(lv_obj_t* parent)
{
    if (parent == NULL)
    {
        return NULL;
    }

    ui_page_settings_ctx_t* ctx =
        (ui_page_settings_ctx_t*)lv_malloc(sizeof(ui_page_settings_ctx_t));
    if (ctx == NULL)
    {
        return NULL;
    }

    lv_memset(ctx, 0, sizeof(ui_page_settings_ctx_t));

    lv_obj_t* page = create_page_container(parent);
    ctx->page      = page;

    ui_wallpaper_t* wallpaper = ui_wallpaper_attach(page);
    ctx->wallpaper            = wallpaper;

    ctx->content = create_content_container(page);

    build_connectivity_section(ctx);
    build_theme_section(ctx);
    build_network_section(ctx);
    build_updates_section(ctx);
    build_diagnostics_section(ctx);
    build_backup_section(ctx);

    lv_obj_add_event_cb(page, ui_page_settings_delete_cb, LV_EVENT_DELETE, ctx);

    s_settings_ctx = ctx;

    return page;
}

void ui_page_settings_set_actions(const ui_page_settings_actions_t* actions, void* user_data)
{
    ui_page_settings_ctx_t* ctx = s_settings_ctx;
    if (ctx == NULL)
    {
        return;
    }

    if (actions != NULL)
    {
        ctx->actions           = *actions;
        ctx->actions_bound     = true;
        ctx->actions_user_data = user_data;
    }
    else
    {
        lv_memset(&ctx->actions, 0, sizeof(ctx->actions));
        ctx->actions_bound     = false;
        ctx->actions_user_data = NULL;
    }
}

void ui_page_settings_set_connection_status(const char*               tester_id,
                                            ui_page_settings_status_t status,
                                            const char*               message)
{
    if (tester_id == NULL)
    {
        return;
    }

    connection_status_async_payload_t* payload =
        (connection_status_async_payload_t*)lv_malloc(sizeof(connection_status_async_payload_t));
    if (payload == NULL)
    {
        return;
    }

    lv_memset(payload, 0, sizeof(*payload));
    lv_snprintf(payload->tester_id, sizeof(payload->tester_id), "%s", tester_id);
    payload->status = status;
    if (message != NULL)
    {
        lv_snprintf(payload->message, sizeof(payload->message), "%s", message);
    }

    lv_async_call(connection_status_async_cb, payload);
}

void ui_page_settings_set_update_status(const char* status_text)
{
    status_label_async_payload_t* payload =
        (status_label_async_payload_t*)lv_malloc(sizeof(status_label_async_payload_t));
    if (payload == NULL)
    {
        return;
    }

    lv_memset(payload, 0, sizeof(*payload));
    if (status_text != NULL)
    {
        lv_snprintf(payload->text, sizeof(payload->text), "%s", status_text);
    }

    lv_async_call(status_label_async_cb, payload);
}

void ui_page_settings_apply_theme_state(bool dark_mode_enabled, const char* variant_id)
{
    theme_state_async_payload_t* payload =
        (theme_state_async_payload_t*)lv_malloc(sizeof(theme_state_async_payload_t));
    if (payload == NULL)
    {
        return;
    }

    payload->dark_mode = dark_mode_enabled;
    if (variant_id != NULL)
    {
        lv_snprintf(payload->variant_id, sizeof(payload->variant_id), "%s", variant_id);
    }
    else
    {
        payload->variant_id[0] = '\0';
    }

    lv_async_call(theme_state_async_cb, payload);
}

void ui_page_settings_set_brightness(uint8_t percent)
{
    brightness_async_payload_t* payload =
        (brightness_async_payload_t*)lv_malloc(sizeof(brightness_async_payload_t));
    if (payload == NULL)
    {
        return;
    }

    payload->percent = percent;
    lv_async_call(brightness_async_cb, payload);
}
