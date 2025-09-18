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
    bool edge_swipe_active;
    bool edge_swipe_triggered;
    bool nav_dragging;
    lv_coord_t edge_swipe_start_x;
    lv_coord_t edge_swipe_reveal;
    lv_coord_t edge_swipe_range;
    lv_coord_t edge_swipe_threshold;
};

static void ui_root_hide_nav(ui_root_t *root, bool animate);
static void ui_root_show_nav(ui_root_t *root, bool animate);

static lv_obj_t *ui_root_nav_container(const ui_root_t *root)
{
    if (root == NULL || root->nav == NULL) {
        return NULL;
    }
    return ui_nav_rail_get_container(root->nav);
}

static void ui_root_update_nav_metrics(ui_root_t *root)
{
    if (root == NULL || root->nav == NULL) {
        return;
    }

    lv_coord_t range = -ui_nav_rail_get_hidden_offset(root->nav);
    if (range < 0) {
        range = 0;
    }
    root->edge_swipe_range = range;

    lv_coord_t threshold = range / 3;
    if (threshold < 24) {
        threshold = 24;
    }
    if (threshold > range) {
        threshold = range;
    }
    root->edge_swipe_threshold = threshold;
}

static void ui_root_nav_size_event_cb(lv_event_t *event)
{
    if (event == NULL) {
        return;
    }

    if (lv_event_get_code(event) != LV_EVENT_SIZE_CHANGED) {
        return;
    }

    ui_root_t *root = (ui_root_t *)lv_event_get_user_data(event);
    ui_root_update_nav_metrics(root);
}

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

    ui_root_t *root = (ui_root_t *)lv_event_get_user_data(event);
    if (root == NULL || root->nav == NULL) {
        return;
    }

    lv_event_code_t code = lv_event_get_code(event);
    lv_indev_t *indev    = lv_indev_get_act();
    switch (code) {
        case LV_EVENT_PRESSED:
            root->edge_swipe_active    = true;
            root->edge_swipe_triggered = false;
            root->nav_dragging         = false;
            root->edge_swipe_reveal    = 0;
            ui_root_update_nav_metrics(root);
            if (indev != NULL) {
                lv_point_t point;
                lv_indev_get_point(indev, &point);
                root->edge_swipe_start_x = point.x;
            } else {
                root->edge_swipe_start_x = 0;
            }
            if (!ui_nav_rail_is_visible(root->nav)) {
                lv_obj_t *nav_obj = ui_root_nav_container(root);
                if (nav_obj != NULL) {
                    lv_obj_clear_flag(nav_obj, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_move_foreground(nav_obj);
                    lv_obj_set_style_translate_x(nav_obj, ui_nav_rail_get_hidden_offset(root->nav), LV_PART_MAIN);
                }
                root->nav_dragging = true;
            }
            break;
        case LV_EVENT_PRESSING:
            if (!root->edge_swipe_active || indev == NULL) {
                break;
            }
            {
                lv_point_t point;
                lv_indev_get_point(indev, &point);
                if (root->nav_dragging) {
                    lv_coord_t delta = point.x - root->edge_swipe_start_x;
                    if (delta < 0) {
                        delta = 0;
                    }
                    lv_coord_t range = root->edge_swipe_range;
                    if (range <= 0) {
                        range = -ui_nav_rail_get_hidden_offset(root->nav);
                        if (range < 0) {
                            range = 0;
                        }
                        root->edge_swipe_range = range;
                    }
                    if (delta > range) {
                        delta = range;
                    }
                    root->edge_swipe_reveal = delta;
                    lv_obj_t *nav_obj       = ui_root_nav_container(root);
                    if (nav_obj != NULL) {
                        lv_coord_t hidden = ui_nav_rail_get_hidden_offset(root->nav);
                        lv_obj_set_style_translate_x(nav_obj, hidden + delta, LV_PART_MAIN);
                    }
                    if (root->edge_swipe_threshold > 0 && delta >= root->edge_swipe_threshold) {
                        root->edge_swipe_triggered = true;
                    }
                } else if ((point.x - root->edge_swipe_start_x) > root->edge_swipe_threshold) {
                    root->edge_swipe_triggered = true;
                    ui_root_show_nav(root, true);
                }
            }
            break;
        case LV_EVENT_RELEASED:
        case LV_EVENT_PRESS_LOST:
            if (root->edge_swipe_active) {
                bool should_show = root->edge_swipe_triggered;
                if (!should_show && root->edge_swipe_range > 0) {
                    should_show = root->edge_swipe_reveal > (root->edge_swipe_range / 2);
                }
                if (should_show) {
                    ui_root_show_nav(root, true);
                } else {
                    ui_root_hide_nav(root, true);
                }
            }
            root->edge_swipe_active    = false;
            root->edge_swipe_triggered = false;
            root->nav_dragging         = false;
            root->edge_swipe_reveal    = 0;
            break;
        default:
            break;
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
        lv_obj_set_size(root->gesture_zone, 96, LV_PCT(100));
        lv_obj_align(root->gesture_zone, LV_ALIGN_LEFT_MID, 0, 0);
        lv_obj_add_flag(root->gesture_zone, LV_OBJ_FLAG_IGNORE_LAYOUT);
        lv_obj_add_flag(root->gesture_zone, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(root->gesture_zone, LV_OBJ_FLAG_ADV_HITTEST);
        lv_obj_set_style_bg_opa(root->gesture_zone, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_add_event_cb(root->gesture_zone, ui_root_edge_gesture_cb, LV_EVENT_PRESSED, root);
        lv_obj_add_event_cb(root->gesture_zone, ui_root_edge_gesture_cb, LV_EVENT_PRESSING, root);
        lv_obj_add_event_cb(root->gesture_zone, ui_root_edge_gesture_cb, LV_EVENT_RELEASED, root);
        lv_obj_add_event_cb(root->gesture_zone, ui_root_edge_gesture_cb, LV_EVENT_PRESS_LOST, root);
    }

    lv_obj_t *nav_container = ui_root_nav_container(root);
    if (nav_container != NULL) {
        lv_obj_move_foreground(nav_container);
        lv_obj_add_event_cb(nav_container, ui_root_nav_size_event_cb, LV_EVENT_SIZE_CHANGED, root);
    }
    if (root->nav_scrim != NULL) {
        lv_obj_move_background(root->nav_scrim);
    }
    if (root->gesture_zone != NULL) {
        lv_obj_move_foreground(root->gesture_zone);
    }
    ui_nav_rail_hide(root->nav, false);
    lv_obj_add_event_cb(ui_nav_rail_get_container(root->nav), ui_root_nav_gesture_cb, LV_EVENT_GESTURE, root);
    ui_root_update_nav_metrics(root);
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
        lv_obj_move_background(root->nav_scrim);
    }

    ui_nav_rail_hide(root->nav, animate);
    if (root->gesture_zone != NULL) {
        lv_obj_clear_flag(root->gesture_zone, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(root->gesture_zone);
    }
    root->nav_dragging      = false;
    root->edge_swipe_reveal = 0;
    ui_root_update_nav_metrics(root);
}

static void ui_root_show_nav(ui_root_t *root, bool animate)
{
    if (root == NULL || root->nav == NULL) {
        return;
    }

    if (!ui_nav_rail_is_visible(root->nav)) {
        ui_nav_rail_show(root->nav, animate);
    }

    if (root->nav_scrim != NULL) {
        lv_obj_clear_flag(root->nav_scrim, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(root->nav_scrim);
    }

    lv_obj_t *nav_obj = ui_root_nav_container(root);
    if (nav_obj != NULL) {
        lv_obj_move_foreground(nav_obj);
    }
    if (root->gesture_zone != NULL) {
        lv_obj_add_flag(root->gesture_zone, LV_OBJ_FLAG_HIDDEN);
    }
    ui_root_update_nav_metrics(root);
}

ui_nav_page_t ui_root_get_active(const ui_root_t *root)
{
    if (root == NULL) {
        return UI_NAV_PAGE_DEFAULT;
    }
    return root->active;
}
