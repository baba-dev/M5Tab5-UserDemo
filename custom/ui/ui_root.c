/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "ui_root.h"

#include "pages/ui_page_cctv.h"
#include "pages/ui_page_media.h"
#include "pages/ui_page_rooms.h"
#include "pages/ui_page_weather.h"

struct ui_root_t {
    lv_obj_t *screen;
    ui_nav_rail_t *nav;
    lv_obj_t *pages[UI_NAV_PAGE_COUNT];
    ui_nav_page_t active;
    lv_obj_t *nav_scrim;
    lv_obj_t *gesture_zone;
};

static void ui_root_hide_nav(ui_root_t *root, bool animate);

static void ui_root_nav_changed(ui_nav_rail_t *rail, ui_nav_page_t page, void *user_data)
{
    LV_UNUSED(rail);
    ui_root_t *root = (ui_root_t *)user_data;
    if (root == NULL) {
        return;
    }

    ui_root_show_page(root, page);
    ui_root_hide_nav(root, true);
}

static void ui_root_scrim_event_cb(lv_event_t *event)
{
    if (event == NULL) {
        return;
    }

    if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
        return;
    }

    ui_root_t *root = (ui_root_t *)lv_event_get_user_data(event);
    ui_root_hide_nav(root, true);
}

static void ui_root_edge_gesture_cb(lv_event_t *event)
{
    if (event == NULL) {
        return;
    }

    if (lv_event_get_code(event) != LV_EVENT_GESTURE) {
        return;
    }

    ui_root_t *root = (ui_root_t *)lv_event_get_user_data(event);
    if (root == NULL || root->nav == NULL) {
        return;
    }

    lv_indev_t *indev = lv_indev_get_act();
    if (indev == NULL) {
        return;
    }

    if (lv_indev_get_gesture_dir(indev) == LV_DIR_RIGHT) {
        if (!ui_nav_rail_is_visible(root->nav)) {
            ui_nav_rail_show(root->nav, true);
            if (root->nav_scrim != NULL) {
                lv_obj_clear_flag(root->nav_scrim, LV_OBJ_FLAG_HIDDEN);
                lv_obj_move_foreground(root->nav_scrim);
            }
            lv_obj_move_foreground(ui_nav_rail_get_container(root->nav));
        }
    }
}

static void ui_root_nav_gesture_cb(lv_event_t *event)
{
    if (event == NULL) {
        return;
    }

    if (lv_event_get_code(event) != LV_EVENT_GESTURE) {
        return;
    }

    ui_root_t *root = (ui_root_t *)lv_event_get_user_data(event);
    if (root == NULL) {
        return;
    }

    lv_indev_t *indev = lv_indev_get_act();
    if (indev != NULL && lv_indev_get_gesture_dir(indev) == LV_DIR_LEFT) {
        ui_root_hide_nav(root, true);
    }
}

static void ui_root_hide_all_overlays(ui_root_t *root)
{
    for (uint32_t i = 0; i < UI_NAV_PAGE_COUNT; i++) {
        if (i == UI_NAV_PAGE_DEFAULT) {
            continue;
        }
        if (root->pages[i] != NULL) {
            lv_obj_add_flag(root->pages[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void ui_root_create_pages(ui_root_t *root)
{
    root->pages[UI_NAV_PAGE_ROOMS]   = ui_page_rooms_create(root->screen);
    root->pages[UI_NAV_PAGE_CCTV]    = ui_page_cctv_create(root->screen);
    root->pages[UI_NAV_PAGE_WEATHER] = ui_page_weather_create(root->screen);
    root->pages[UI_NAV_PAGE_MEDIA]   = ui_page_media_create(root->screen);

    for (uint32_t i = 0; i < UI_NAV_PAGE_COUNT; i++) {
        if (root->pages[i] == NULL) {
            continue;
        }
        lv_obj_move_foreground(root->pages[i]);
        lv_obj_add_flag(root->pages[i], LV_OBJ_FLAG_HIDDEN);
    }
}

ui_root_t *ui_root_create(void)
{
    ui_root_t *root = (ui_root_t *)lv_malloc(sizeof(ui_root_t));
    if (root == NULL) {
        return NULL;
    }
    lv_memset(root, 0, sizeof(ui_root_t));

    root->screen = lv_screen_active();

    root->nav = ui_nav_rail_create(root->screen, ui_root_nav_changed, root);
    if (root->nav == NULL) {
        lv_free(root);
        return NULL;
    }

    ui_root_create_pages(root);

    root->nav_scrim = lv_obj_create(root->screen);
    if (root->nav_scrim != NULL) {
        lv_obj_remove_style_all(root->nav_scrim);
        lv_obj_set_size(root->nav_scrim, LV_PCT(100), LV_PCT(100));
        lv_obj_add_flag(root->nav_scrim, LV_OBJ_FLAG_IGNORE_LAYOUT);
        lv_obj_add_flag(root->nav_scrim, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(root->nav_scrim, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_bg_color(root->nav_scrim, lv_color_hex(0x04060a), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(root->nav_scrim, LV_OPA_40, LV_PART_MAIN);
        lv_obj_add_event_cb(root->nav_scrim, ui_root_scrim_event_cb, LV_EVENT_CLICKED, root);
    }

    root->gesture_zone = lv_obj_create(root->screen);
    if (root->gesture_zone != NULL) {
        lv_obj_remove_style_all(root->gesture_zone);
        lv_obj_set_size(root->gesture_zone, 56, 160);
        lv_obj_align(root->gesture_zone, LV_ALIGN_TOP_LEFT, 0, 0);
        lv_obj_add_flag(root->gesture_zone, LV_OBJ_FLAG_IGNORE_LAYOUT);
        lv_obj_add_flag(root->gesture_zone, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(root->gesture_zone, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_add_event_cb(root->gesture_zone, ui_root_edge_gesture_cb, LV_EVENT_GESTURE, root);
    }

    lv_obj_move_foreground(ui_nav_rail_get_container(root->nav));
    if (root->nav_scrim != NULL) {
        lv_obj_move_background(root->nav_scrim);
    }
    if (root->gesture_zone != NULL) {
        lv_obj_move_foreground(root->gesture_zone);
    }
    ui_nav_rail_hide(root->nav, false);
    lv_obj_add_event_cb(ui_nav_rail_get_container(root->nav), ui_root_nav_gesture_cb, LV_EVENT_GESTURE, root);
    ui_root_hide_all_overlays(root);
    root->active = UI_NAV_PAGE_DEFAULT;

    return root;
}

void ui_root_destroy(ui_root_t *root)
{
    if (root == NULL) {
        return;
    }

    for (uint32_t i = 0; i < UI_NAV_PAGE_COUNT; i++) {
        if (root->pages[i] != NULL) {
            lv_obj_del(root->pages[i]);
            root->pages[i] = NULL;
        }
    }

    if (root->nav != NULL) {
        ui_nav_rail_destroy(root->nav);
        root->nav = NULL;
    }

    if (root->gesture_zone != NULL) {
        lv_obj_del(root->gesture_zone);
        root->gesture_zone = NULL;
    }

    if (root->nav_scrim != NULL) {
        lv_obj_del(root->nav_scrim);
        root->nav_scrim = NULL;
    }

    lv_free(root);
}

void ui_root_show_page(ui_root_t *root, ui_nav_page_t page)
{
    if (root == NULL || page >= UI_NAV_PAGE_COUNT) {
        return;
    }

    if (page == UI_NAV_PAGE_DEFAULT) {
        ui_root_hide_all_overlays(root);
    } else {
        for (uint32_t i = 0; i < UI_NAV_PAGE_COUNT; i++) {
            if (i == UI_NAV_PAGE_DEFAULT) {
                continue;
            }
            lv_obj_t *candidate = root->pages[i];
            if (candidate == NULL) {
                continue;
            }
            if (i == (uint32_t)page) {
                lv_obj_clear_flag(candidate, LV_OBJ_FLAG_HIDDEN);
                lv_obj_move_foreground(candidate);
            } else {
                lv_obj_add_flag(candidate, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }

    lv_obj_move_foreground(ui_nav_rail_get_container(root->nav));
    ui_nav_rail_set_active(root->nav, page);
    root->active = page;
}

static void ui_root_hide_nav(ui_root_t *root, bool animate)
{
    if (root == NULL || root->nav == NULL) {
        return;
    }

    if (root->nav_scrim != NULL) {
        lv_obj_add_flag(root->nav_scrim, LV_OBJ_FLAG_HIDDEN);
    }

    ui_nav_rail_hide(root->nav, animate);
}

ui_nav_page_t ui_root_get_active(const ui_root_t *root)
{
    if (root == NULL) {
        return UI_NAV_PAGE_DEFAULT;
    }
    return root->active;
}
