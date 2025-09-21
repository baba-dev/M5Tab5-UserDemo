/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "asset_mount.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#if defined(ESP_PLATFORM)
#if defined(__has_include)
#if __has_include("sdkconfig.h")
#include "sdkconfig.h"
#endif
#endif
#endif

#ifdef __has_include
#if __has_include("lvgl.h")
#ifndef LV_LVGL_H_INCLUDE_SIMPLE
#define LV_LVGL_H_INCLUDE_SIMPLE
#endif
#endif
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#if LV_USE_FS_IF

static lv_fs_res_t posix_close(lv_fs_drv_t *drv, void *file_p);
static lv_fs_res_t posix_read(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br);
static lv_fs_res_t posix_write(lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw);
static lv_fs_res_t posix_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence);
static lv_fs_res_t posix_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p);

static lv_fs_res_t posix_close(lv_fs_drv_t *drv, void *file_p)
{
    LV_UNUSED(drv);
    if (file_p == NULL) {
        return LV_FS_RES_INV_PARAM;
    }
    FILE *f = (FILE *)file_p;
    return fclose(f) == 0 ? LV_FS_RES_OK : LV_FS_RES_FS_ERR;
}

static lv_fs_res_t posix_read(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br)
{
    LV_UNUSED(drv);
    if (file_p == NULL || buf == NULL) {
        return LV_FS_RES_INV_PARAM;
    }
    FILE *f   = (FILE *)file_p;
    size_t rd = fread(buf, 1, btr, f);
    if (br != NULL) {
        *br = (uint32_t)rd;
    }
    if (rd < btr && ferror(f)) {
        clearerr(f);
        return LV_FS_RES_FS_ERR;
    }
    return LV_FS_RES_OK;
}

static lv_fs_res_t posix_write(lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw)
{
    LV_UNUSED(drv);
    if (file_p == NULL || buf == NULL) {
        return LV_FS_RES_INV_PARAM;
    }
    FILE *f   = (FILE *)file_p;
    size_t wr = fwrite(buf, 1, btw, f);
    if (bw != NULL) {
        *bw = (uint32_t)wr;
    }
    if (wr < btw) {
        return LV_FS_RES_FS_ERR;
    }
    return LV_FS_RES_OK;
}

static lv_fs_res_t posix_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence)
{
    LV_UNUSED(drv);
    if (file_p == NULL) {
        return LV_FS_RES_INV_PARAM;
    }
    FILE *f = (FILE *)file_p;

    int origin = SEEK_SET;
    switch (whence) {
        case LV_FS_SEEK_CUR:
            origin = SEEK_CUR;
            break;
        case LV_FS_SEEK_END:
            origin = SEEK_END;
            break;
        case LV_FS_SEEK_SET:
        default:
            origin = SEEK_SET;
            break;
    }

    return fseek(f, (long)pos, origin) == 0 ? LV_FS_RES_OK : LV_FS_RES_FS_ERR;
}

static lv_fs_res_t posix_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p)
{
    LV_UNUSED(drv);
    if (file_p == NULL || pos_p == NULL) {
        return LV_FS_RES_INV_PARAM;
    }
    FILE *f  = (FILE *)file_p;
    long pos = ftell(f);
    if (pos < 0) {
        return LV_FS_RES_FS_ERR;
    }
    *pos_p = (uint32_t)pos;
    return LV_FS_RES_OK;
}

static void *posix_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode)
{
    LV_UNUSED(drv);
    const char *flags = NULL;
    bool rd           = (mode & LV_FS_MODE_RD) != 0U;
    bool wr           = (mode & LV_FS_MODE_WR) != 0U;

    if (rd && wr) {
        flags = "rb+";
    } else if (wr) {
        flags = "wb";
    } else {
        flags = "rb";
    }

    return fopen(path, flags);
}

static void lv_fs_if_init(void)
{
#if defined(LV_FS_IF_POSIX) && (LV_FS_IF_POSIX != '\0')
    static bool registered = false;
    static lv_fs_drv_t drv;
    if (registered) {
        return;
    }

    lv_fs_drv_init(&drv);
    drv.letter   = LV_FS_IF_POSIX;
    drv.open_cb  = posix_open;
    drv.close_cb = posix_close;
    drv.read_cb  = posix_read;
    drv.write_cb = posix_write;
    drv.seek_cb  = posix_seek;
    drv.tell_cb  = posix_tell;

    lv_fs_drv_register(&drv);
    registered = true;
#endif
}
#endif

#if defined(ESP_PLATFORM)
#include "driver/sdmmc_host.h"
#include "esp_err.h"
#include "esp_log.h"
#if defined(__has_include)
#if __has_include("esp_vfs_fat.h")
#include "esp_vfs_fat.h"
#endif
#else
#include "esp_vfs_fat.h"
#endif
#include "sdmmc_cmd.h"
#endif

#if defined(ESP_PLATFORM)
#define ASSET_LOGI(tag, fmt, ...) ESP_LOGI(tag, fmt, ##__VA_ARGS__)
#define ASSET_LOGW(tag, fmt, ...) ESP_LOGW(tag, fmt, ##__VA_ARGS__)
#else
#define ASSET_LOGI(tag, fmt, ...) LV_LOG_INFO(fmt, ##__VA_ARGS__)
#define ASSET_LOGW(tag, fmt, ...) LV_LOG_WARN(fmt, ##__VA_ARGS__)
#endif

static const char *k_tag = "assets";

static bool s_fs_initialized = false;

void assets_fs_init(void)
{
    if (s_fs_initialized) {
        return;
    }
    s_fs_initialized = true;

#if LV_USE_FS_IF
    lv_fs_if_init();
#else
    LV_LOG_WARN("LV_USE_FS_IF disabled; asset loading from FS unavailable");
#endif

#if defined(ESP_PLATFORM)
    ASSET_LOGW(k_tag, "SD card mounting disabled; using internal assets only");
#else
    ASSET_LOGI(k_tag, "Desktop build: using host filesystem without SD mount");
#endif
}
