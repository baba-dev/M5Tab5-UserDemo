/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "integration/cctv_controller.h"

#include <algorithm>
#include <array>

#include "core/app_trace.h"

namespace custom::integration
{

    namespace
    {

        constexpr const char* kTag = "cctv-controller";

        constexpr std::array<const char*, 3> kQualityOptions = {
            "1080p60",
            "720p30",
            "360p15",
        };

        constexpr ui_page_cctv_camera_t kCameras[] = {
            {
                .camera_id       = "front_porch",
                .name            = "Front Porch",
                .status          = "Streaming · 1080p60",
                .stream_url      = "rtsp://frigate/front_porch/stream",
                .snapshot_url    = "https://frigate.local/api/front_porch.jpg",
                .audio_supported = true,
            },
            {
                .camera_id       = "garage",
                .name            = "Garage Bay",
                .status          = "Idle · Door closed",
                .stream_url      = "rtsp://frigate/garage/stream",
                .snapshot_url    = "https://frigate.local/api/garage.jpg",
                .audio_supported = false,
            },
            {
                .camera_id       = "backyard",
                .name            = "Backyard",
                .status          = "Streaming · 1440p30",
                .stream_url      = "rtsp://frigate/backyard/stream",
                .snapshot_url    = "https://frigate.local/api/backyard.jpg",
                .audio_supported = true,
            },
        };

        constexpr ui_page_cctv_event_t kEvents[] = {
            {
                .event_id     = "evt_front_1742",
                .camera_id    = "front_porch",
                .title        = "Front Porch",
                .description  = "Person detected · 2m ago",
                .timestamp    = "Today · 17:42",
                .clip_url     = "https://frigate.local/api/front_porch/event/evt_front_1742.mp4",
                .snapshot_url = "https://frigate.local/api/front_porch/event/evt_front_1742.jpg",
            },
            {
                .event_id     = "evt_drive_1737",
                .camera_id    = "garage",
                .title        = "Driveway",
                .description  = "Vehicle detected",
                .timestamp    = "Today · 17:37",
                .clip_url     = "https://frigate.local/api/garage/event/evt_drive_1737.mp4",
                .snapshot_url = "https://frigate.local/api/garage/event/evt_drive_1737.jpg",
            },
            {
                .event_id     = "evt_gate_1715",
                .camera_id    = "backyard",
                .title        = "Backyard Gate",
                .description  = "Package drop-off",
                .timestamp    = "Today · 17:15",
                .clip_url     = "https://frigate.local/api/backyard/event/evt_gate_1715.mp4",
                .snapshot_url = "https://frigate.local/api/backyard/event/evt_gate_1715.jpg",
            },
        };

        constexpr std::size_t CameraCount = sizeof(kCameras) / sizeof(kCameras[0]);
        constexpr std::size_t EventCount  = sizeof(kEvents) / sizeof(kEvents[0]);

    }  // namespace

    CctvController::CctvController()
    {
        page_ = ui_page_cctv_get_obj();
        if (page_ != nullptr)
        {
            lv_obj_add_event_cb(page_, PageEventCb, UI_PAGE_CCTV_EVENT_ACTION, this);
            lv_obj_add_event_cb(page_, PageEventCb, UI_PAGE_CCTV_EVENT_OPEN_CLIP, this);
        }
    }

    CctvController::~CctvController()
    {
        if (page_ != nullptr)
        {
            lv_obj_remove_event_cb_with_user_data(page_, PageEventCb, this);
        }
    }

    void CctvController::PublishInitialState()
    {
        PushState();
    }

    void CctvController::PageEventCb(lv_event_t* event)
    {
        if (event == nullptr)
        {
            return;
        }

        auto* controller = static_cast<CctvController*>(lv_event_get_user_data(event));
        if (controller == nullptr)
        {
            return;
        }

        lv_event_code_t code = lv_event_get_code(event);

        if (code == UI_PAGE_CCTV_EVENT_ACTION)
        {
            const auto* action =
                static_cast<const ui_page_cctv_action_event_t*>(lv_event_get_param(event));
            if (action != nullptr)
            {
                controller->HandleAction(*action);
            }
        }
        else if (code == UI_PAGE_CCTV_EVENT_OPEN_CLIP)
        {
            const auto* clip =
                static_cast<const ui_page_cctv_clip_event_t*>(lv_event_get_param(event));
            if (clip != nullptr)
            {
                controller->HandleClipRequest(*clip);
            }
        }
    }

    void CctvController::HandleAction(const ui_page_cctv_action_event_t& action)
    {
        if (CameraCount == 0)
        {
            return;
        }

        switch (action.action)
        {
            case UI_PAGE_CCTV_ACTION_PREVIOUS:
                if (CameraCount > 0)
                {
                    if (active_index_ == 0)
                    {
                        active_index_ = CameraCount - 1;
                    }
                    else
                    {
                        active_index_--;
                    }
                    PushState();
                }
                break;

            case UI_PAGE_CCTV_ACTION_NEXT:
                if (CameraCount > 0)
                {
                    active_index_ = (active_index_ + 1) % CameraCount;
                    PushState();
                }
                break;

            case UI_PAGE_CCTV_ACTION_QUALITY:
                quality_index_ = (quality_index_ + 1) % kQualityOptions.size();
                PushState();
                break;

            case UI_PAGE_CCTV_ACTION_TOGGLE_MUTE:
                muted_ = !muted_;
                PushState();
                break;

            case UI_PAGE_CCTV_ACTION_OPEN_GATE:
                APP_TRACEI(kTag,
                           "Requested gate open for camera %s",
                           action.camera_id != nullptr ? action.camera_id : "(none)");
                break;

            case UI_PAGE_CCTV_ACTION_TALK:
                APP_TRACEI(kTag,
                           "Initiating talkback on %s",
                           action.camera_id != nullptr ? action.camera_id : "(none)");
                break;

            case UI_PAGE_CCTV_ACTION_SNAPSHOT:
                APP_TRACEI(kTag,
                           "Snapshot requested for %s",
                           action.camera_id != nullptr ? action.camera_id : "(none)");
                break;

            case UI_PAGE_CCTV_ACTION_TIMELINE:
                APP_TRACEI(kTag, "Timeline requested (events=%u)", (unsigned int)EventCount);
                break;
        }
    }

    void CctvController::HandleClipRequest(const ui_page_cctv_clip_event_t& clip)
    {
        if (clip.event == nullptr)
        {
            return;
        }

        const char* clip_id = clip.event->event_id != nullptr ? clip.event->event_id : "(unknown)";
        APP_TRACEI(kTag, "Open clip %s", clip_id);
    }

    void CctvController::PushState()
    {
        if (page_ == nullptr)
        {
            return;
        }

        ui_page_cctv_state_t state = {
            .cameras       = kCameras,
            .camera_count  = CameraCount,
            .active_index  = std::min(active_index_, CameraCount > 0 ? CameraCount - 1 : 0),
            .events        = kEvents,
            .event_count   = EventCount,
            .quality_label = kQualityOptions[quality_index_],
            .muted         = muted_,
        };

        ui_page_cctv_set_state(&state);
    }

}  // namespace custom::integration
