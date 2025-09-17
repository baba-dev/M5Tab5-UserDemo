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
};

static void ui_root_nav_changed(ui_nav_rail_t *rail, ui_nav_page_t page, void *user_data)
{
    LV_UNUSED(rail);
    ui_root_t *root = (ui_root_t *)user_data;
    if (root == NULL) {
        return;
    }

    ui_root_show_page(root, page);
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

    lv_obj_move_foreground(ui_nav_rail_get_container(root->nav));
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

ui_nav_page_t ui_root_get_active(const ui_root_t *root)
{
    if (root == NULL) {
        return UI_NAV_PAGE_DEFAULT;
    }
    return root->active;
}
