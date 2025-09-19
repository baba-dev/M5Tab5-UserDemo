/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <cstddef>

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

#include "ui/pages/ui_page_cctv.h"

namespace custom::integration
{

    class CctvController
    {
    public:
        CctvController();
        ~CctvController();

        CctvController(const CctvController&)            = delete;
        CctvController& operator=(const CctvController&) = delete;

        void PublishInitialState();

    private:
        static void PageEventCb(lv_event_t* event);

        void HandleAction(const ui_page_cctv_action_event_t& action);
        void HandleClipRequest(const ui_page_cctv_clip_event_t& clip);
        void PushState();

        lv_obj_t*   page_          = nullptr;
        std::size_t active_index_  = 0;
        std::size_t quality_index_ = 0;
        bool        muted_         = false;
    };

}  // namespace custom::integration
