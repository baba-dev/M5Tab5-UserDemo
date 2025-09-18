/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "asset_mount.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

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
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#endif

#if defined(ESP_PLATFORM)
#define ASSET_LOGI(tag, fmt, ...) ESP_LOGI(tag, fmt, ##__VA_ARGS__)
#define ASSET_LOGW(tag, fmt, ...) ESP_LOGW(tag, fmt, ##__VA_ARGS__)
#else
#define ASSET_LOGI(tag, fmt, ...) LV_LOG_INFO(fmt, ##__VA_ARGS__)
#define ASSET_LOGW(tag, fmt, ...) LV_LOG_WARN(fmt, ##__VA_ARGS__)
#endif

#define SD_ASSET_ROOT       "/sdcard/custom/assets"
#define INTERNAL_ASSET_ROOT "/custom/assets"
#define BG_RELATIVE         "/bg"

static const char *k_tag = "assets";

static bool s_fs_initialized = false;
static bool s_sd_ok          = false;
static bool s_warned_missing = false;

static const char *drive_prefix(void)
{
#if defined(LV_FS_IF_POSIX) && (LV_FS_IF_POSIX != '\0')
    static const char prefix[] = {LV_FS_IF_POSIX, ':', '\0'};
    return prefix;
#elif defined(LV_FS_MEMFS_LETTER) && (LV_FS_MEMFS_LETTER != '\0')
    static const char prefix[] = {LV_FS_MEMFS_LETTER, ':', '\0'};
    return prefix;
#else
    return "";
#endif
}

static bool path_exists(const char *path)
{
    if (path == NULL) {
        return false;
    }

    lv_fs_file_t file;
    lv_fs_res_t res = lv_fs_open(&file, path, LV_FS_MODE_RD);
    if (res == LV_FS_RES_OK) {
        lv_fs_close(&file);
        return true;
    }
    return false;
}

static bool format_path(char *buffer, size_t size, const char *root, const char *relative)
{
    const char *prefix = drive_prefix();
    int length         = snprintf(buffer, size, "%s%s%s/%s", prefix, root, BG_RELATIVE, relative);
    return length > 0 && length < (int)size;
}

static const char *pick_file(const char *relative, char *buffer, size_t size)
{
    if (buffer == NULL || size == 0) {
        return NULL;
    }

    if (s_sd_ok && format_path(buffer, size, SD_ASSET_ROOT, relative) && path_exists(buffer)) {
        return buffer;
    }

    if (!format_path(buffer, size, INTERNAL_ASSET_ROOT, relative)) {
        return NULL;
    }

    if (!path_exists(buffer) && !s_warned_missing) {
        s_warned_missing = true;
        ASSET_LOGW(k_tag, "Falling back to built-in wallpaper path: %s", buffer);
    }

    return buffer;
}

void assets_fs_init(void)
{
    if (s_fs_initialized) {
        return;
    }
    s_fs_initialized = true;

#if LV_USE_FS_IF
    lv_fs_if_init();
#else
    LV_LOG_WARN("LV_USE_FS_IF disabled; wallpaper loading from FS unavailable");
#endif

#if defined(ESP_PLATFORM)
    ASSET_LOGW(k_tag, "SD card mounting disabled; using internal assets only");
#else
    ASSET_LOGI(k_tag, "Desktop build: using host filesystem without SD mount");
#endif
}

const char *assets_path_1(void)
{
    static char path_buffer[128];
    return pick_file("1.png", path_buffer, sizeof(path_buffer));
}

const char *assets_path_2(void)
{
    static char path_buffer[128];
    return pick_file("2.png", path_buffer, sizeof(path_buffer));
}
