/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "integration/media_controller.h"

#include <algorithm>
#include <array>

#include "app_trace.h"

namespace custom::integration
{

    namespace
    {

        constexpr const char* kTag = "media-controller";

        struct Track
        {
            const char* media_id;
            const char* title;
            const char* artist;
            const char* source;
        };

        constexpr std::array<Track, 4> kTracks = {
            Track{
                "media.spotify.kitchen", "Coffee Shop Jazz", "Lo-Fi Ensemble", "Spotify · Kitchen"},
            Track{"media.local.turntable", "Analog Sunshine", "Night Drive", "Turntable · Aux"},
            Track{"media.assist.request", "What's the weather?", "Assist", "Assist Prompt"},
            Track{"media.home_theater", "Atmos Demo", "Living Room", "Home Theater"},
        };

        constexpr std::array<ui_page_media_scene_t, 4> kScenes = {
            ui_page_media_scene_t{"scene.morning_mix", "Morning"},
            ui_page_media_scene_t{"scene.movie_time", "Movie"},
            ui_page_media_scene_t{"scene.night_relax", "Night"},
            ui_page_media_scene_t{"scene.party_mode", "Party"},
        };

    }  // namespace

    MediaController::MediaController()
    {
        page_ = ui_page_media_get_obj();
        if (page_ != nullptr)
        {
            lv_obj_add_event_cb(page_, PageEventCb, UI_PAGE_MEDIA_EVENT_COMMAND, this);
        }
    }

    MediaController::~MediaController()
    {
        if (page_ != nullptr)
        {
            lv_obj_remove_event_cb_with_user_data(page_, PageEventCb, this);
        }
    }

    void MediaController::PublishInitialState()
    {
        PushScenes();
        PushNowPlaying();
    }

    void MediaController::PageEventCb(lv_event_t* event)
    {
        if (event == nullptr)
        {
            return;
        }

        auto* controller = static_cast<MediaController*>(lv_event_get_user_data(event));
        if (controller == nullptr)
        {
            return;
        }

        const auto* data = static_cast<const ui_page_media_event_t*>(lv_event_get_param(event));
        if (data == nullptr)
        {
            return;
        }

        controller->HandleEvent(*data);
    }

    void MediaController::HandleEvent(const ui_page_media_event_t& event)
    {
        switch (event.signal)
        {
            case UI_PAGE_MEDIA_SIGNAL_PREVIOUS:
                if (!kTracks.empty())
                {
                    if (track_index_ == 0)
                    {
                        track_index_ = kTracks.size() - 1;
                    }
                    else
                    {
                        track_index_--;
                    }
                    APP_TRACEI(kTag, "Previous track requested: %s", kTracks[track_index_].title);
                }
                break;

            case UI_PAGE_MEDIA_SIGNAL_NEXT:
                if (!kTracks.empty())
                {
                    track_index_ = (track_index_ + 1) % kTracks.size();
                    APP_TRACEI(kTag, "Next track requested: %s", kTracks[track_index_].title);
                }
                break;

            case UI_PAGE_MEDIA_SIGNAL_PLAY_PAUSE:
                playing_ = !playing_;
                APP_TRACEI(kTag, playing_ ? "Play command" : "Pause command");
                break;

            case UI_PAGE_MEDIA_SIGNAL_VOLUME:
                volume_percent_ = std::min<uint8_t>(event.volume, 100U);
                APP_TRACEI(kTag, "Volume set to %u%%", static_cast<unsigned int>(volume_percent_));
                break;

            case UI_PAGE_MEDIA_SIGNAL_TRIGGER_SCENE:
                APP_TRACEI(kTag,
                           "Trigger quick scene: %s",
                           event.scene_id != nullptr ? event.scene_id : "(none)");
                break;
        }

        PushNowPlaying();
    }

    void MediaController::PushNowPlaying()
    {
        if (page_ == nullptr)
        {
            return;
        }

        const Track& track =
            kTracks.empty() ? Track{"media.none", "Idle", "", ""} : kTracks[track_index_];

        ui_page_media_now_playing_t now_playing = {
            .media_id = track.media_id,
            .title    = track.title,
            .artist   = track.artist,
            .source   = track.source,
            .playing  = playing_,
            .volume   = volume_percent_,
        };

        ui_page_media_set_now_playing(&now_playing);
    }

    void MediaController::PushScenes()
    {
        if (page_ == nullptr)
        {
            return;
        }

        ui_page_media_set_quick_scenes(kScenes.data(), kScenes.size());
    }

}  // namespace custom::integration
