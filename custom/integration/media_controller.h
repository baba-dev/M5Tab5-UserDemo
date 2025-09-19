/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <cstddef>
#include <cstdint>

#ifdef __has_include
#    if __has_include("lvgl.h")
#        ifndef LV_LVGL_H_INCLUDE_SIMPLE
#            define LV_LVGL_H_INCLUDE_SIMPLE
#        endif
#    endif
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#    include "lvgl.h"
#else
#    include "lvgl/lvgl.h"
#endif

#include "ui/pages/ui_page_media.h"

namespace custom::integration
{

    class MediaController
    {
    public:
        MediaController();
        ~MediaController();

        MediaController(const MediaController&)            = delete;
        MediaController& operator=(const MediaController&) = delete;

        void PublishInitialState();

    private:
        static void PageEventCb(lv_event_t* event);

        void HandleEvent(const ui_page_media_event_t& event);
        void PushNowPlaying();
        void PushScenes();

        lv_obj_t*    page_           = nullptr;
        std::size_t  track_index_    = 0;
        bool         playing_        = true;
        std::uint8_t volume_percent_ = 40U;
    };

}  // namespace custom::integration
