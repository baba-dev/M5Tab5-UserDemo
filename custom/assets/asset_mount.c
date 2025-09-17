/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "assets/asset_mount.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>

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

#define SD_MOUNT_POINT      "/sdcard"
#define INTERNAL_ASSET_ROOT "/custom/assets"
#define BG_RELATIVE         "/bg"
#define FALLBACK_1          INTERNAL_ASSET_ROOT BG_RELATIVE "/1.png"
#define FALLBACK_2          INTERNAL_ASSET_ROOT BG_RELATIVE "/2.png"

static const char *k_tag = "assets";

static bool s_fs_initialized = false;
static bool s_sd_ok          = false;
static bool s_warned_missing = false;

static bool path_exists(const char *path)
{
    if (path == NULL) {
        return false;
    }
    struct stat st;
    return stat(path, &st) == 0;
}

static const char *pick_file(const char *relative, const char *fallback)
{
    static char path_buffer[128];

    if (s_sd_ok) {
        int written = snprintf(path_buffer, sizeof(path_buffer), "%s%s/%s", SD_MOUNT_POINT, BG_RELATIVE, relative);
        if (written > 0 && written < (int)sizeof(path_buffer) && path_exists(path_buffer)) {
            return path_buffer;
        }
    }

    int written = snprintf(path_buffer, sizeof(path_buffer), "%s%s/%s", INTERNAL_ASSET_ROOT, BG_RELATIVE, relative);
    if (written > 0 && written < (int)sizeof(path_buffer) && path_exists(path_buffer)) {
        return path_buffer;
    }

    if (!s_warned_missing) {
        s_warned_missing = true;
        ASSET_LOGW(k_tag, "Falling back to built-in wallpaper path: %s", fallback);
    }
    return fallback;
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
    sdmmc_host_t host                        = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config          = SDMMC_SLOT_CONFIG_DEFAULT();
    esp_vfs_fat_sdmmc_mount_config_t mnt_cfg = {
        .format_if_mount_failed = false,
        .max_files              = 4,
        .allocation_unit_size   = 16 * 1024,
    };
    sdmmc_card_t *card = NULL;
    esp_err_t err      = esp_vfs_fat_sdmmc_mount(SD_MOUNT_POINT, &host, &slot_config, &mnt_cfg, &card);
    if (err == ESP_OK) {
        s_sd_ok = true;
        ASSET_LOGI(k_tag, "SD mounted at %s", SD_MOUNT_POINT);
    } else {
        ASSET_LOGW(k_tag, "SD not mounted (err=0x%x). Proceeding without SD.", (unsigned int)err);
    }
#else
    ASSET_LOGI(k_tag, "Desktop build: using host filesystem without SD mount");
#endif
}

const char *assets_path_1(void)
{
    return pick_file("1.png", FALLBACK_1);
}

const char *assets_path_2(void)
{
    return pick_file("2.png", FALLBACK_2);
}
