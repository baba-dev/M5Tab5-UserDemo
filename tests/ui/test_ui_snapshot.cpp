/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <lodepng.h>
#include <lvgl.h>

namespace {

constexpr uint32_t kWidth  = 320;
constexpr uint32_t kHeight = 240;

class HeadlessDisplay {
  public:
    HeadlessDisplay() {
        lv_init();
        buffer_.resize(kWidth * kHeight, 0);
        display_ = lv_display_create(kWidth, kHeight);
        lv_display_set_color_format(display_, LV_COLOR_FORMAT_RGB565);
        lv_display_set_driver_data(display_, this);
        lv_display_set_flush_cb(display_, &HeadlessDisplay::FlushTrampoline);
        lv_display_set_buffers(display_, buffer_.data(), nullptr, buffer_.size() * sizeof(uint16_t),
                               LV_DISPLAY_RENDER_MODE_DIRECT);
        lv_display_set_default(display_);
    }

    ~HeadlessDisplay() {
        lv_deinit();
    }

    lv_display_t* Get() const { return display_; }

    std::vector<unsigned char> CaptureRgba() const {
        std::vector<unsigned char> rgba;
        rgba.resize(buffer_.size() * 4);
        for (std::size_t i = 0; i < buffer_.size(); ++i) {
            uint16_t pixel = buffer_[i];
            uint8_t r      = static_cast<uint8_t>(((pixel >> 11U) & 0x1FU) * 255U / 31U);
            uint8_t g      = static_cast<uint8_t>(((pixel >> 5U) & 0x3FU) * 255U / 63U);
            uint8_t b      = static_cast<uint8_t>((pixel & 0x1FU) * 255U / 31U);
            rgba[i * 4U + 0U] = r;
            rgba[i * 4U + 1U] = g;
            rgba[i * 4U + 2U] = b;
            rgba[i * 4U + 3U] = 255U;
        }
        return rgba;
    }

  private:
    static void FlushTrampoline(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
        auto* self = static_cast<HeadlessDisplay*>(lv_display_get_driver_data(disp));
        if (self != nullptr) {
            self->Flush(area, reinterpret_cast<const uint16_t*>(px_map));
        }
        lv_display_flush_ready(disp);
    }

    void Flush(const lv_area_t* area, const uint16_t* pixels) {
        if (area == nullptr || pixels == nullptr) {
            return;
        }
        uint32_t width  = static_cast<uint32_t>(area->x2 - area->x1 + 1);
        uint32_t height = static_cast<uint32_t>(area->y2 - area->y1 + 1);
        for (uint32_t row = 0; row < height; ++row) {
            uint32_t dest_y   = static_cast<uint32_t>(area->y1) + row;
            uint32_t dest_x   = static_cast<uint32_t>(area->x1);
            uint32_t dest_idx = dest_y * kWidth + dest_x;
            const uint16_t* src = pixels + row * width;
            std::copy(src, src + width, buffer_.begin() + static_cast<std::ptrdiff_t>(dest_idx));
        }
    }

    lv_display_t* display_ = nullptr;
    std::vector<uint16_t> buffer_;
};

std::vector<unsigned char> LoadPngRgba(const std::filesystem::path& path, unsigned& width, unsigned& height) {
    std::vector<unsigned char> image;
    unsigned error = lodepng::decode(image, width, height, path.string());
    if (error != 0) {
        throw std::runtime_error("Failed to load PNG: " + path.string() + " (" + std::to_string(error) + ")");
    }
    return image;
}

void WritePng(const std::filesystem::path& path, const std::vector<unsigned char>& rgba, unsigned width,
              unsigned height) {
    std::filesystem::create_directories(path.parent_path());
    unsigned error = lodepng::encode(path.string(), rgba, width, height);
    if (error != 0) {
        throw std::runtime_error("Failed to encode PNG: " + path.string() + " (" + std::to_string(error) + ")");
    }
}

std::filesystem::path GoldenPath() {
    auto file_dir = std::filesystem::path(__FILE__).parent_path();
    return (file_dir / "golden" / "home_page.png").lexically_normal();
}

std::vector<unsigned char> RenderHomeScreen() {
    HeadlessDisplay display;

    lv_obj_t* screen = lv_obj_create(nullptr);
    lv_obj_remove_style_all(screen);
    lv_obj_set_size(screen, kWidth, kHeight);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x1C2333), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(screen, lv_color_hex(0x4E6E9A), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, LV_PART_MAIN);

    lv_obj_t* title = lv_label_create(screen);
    lv_label_set_text(title, "Home");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 16, 16);

    lv_obj_t* card = lv_obj_create(screen);
    lv_obj_remove_style_all(card);
    lv_obj_set_size(card, 220, 120);
    lv_obj_set_style_bg_color(card, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_radius(card, 18, LV_PART_MAIN);
    lv_obj_set_style_pad_all(card, 16, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(card, 18, LV_PART_MAIN);
    lv_obj_set_style_shadow_spread(card, 2, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(card, lv_color_hex(0x1C2333), LV_PART_MAIN);
    lv_obj_align(card, LV_ALIGN_CENTER, 0, 10);

    lv_obj_t* card_label = lv_label_create(card);
    lv_label_set_text(card_label, "Lights: on");
    lv_obj_set_style_text_color(card_label, lv_color_hex(0x1C2333), LV_PART_MAIN);
    lv_obj_align(card_label, LV_ALIGN_LEFT_MID, 0, -10);

    lv_obj_t* value_label = lv_label_create(card);
    lv_label_set_text(value_label, "72°F");
    lv_obj_set_style_text_font(value_label, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_align(value_label, LV_ALIGN_LEFT_MID, 0, 24);

    lv_obj_t* arc = lv_arc_create(screen);
    lv_obj_set_size(arc, 120, 120);
    lv_obj_center(arc);
    lv_arc_set_value(arc, 60);

    lv_screen_load(screen);
    lv_timer_handler();
    lv_tick_inc(1);
    lv_refr_now(display.Get());
    lv_timer_handler();

    return display.CaptureRgba();
}

TEST(UiSnapshotTest, HomePageMatchesGolden) {
    auto snapshot = RenderHomeScreen();
    auto golden   = GoldenPath();
    bool update   = std::getenv("UPDATE_GOLDEN") != nullptr;

    unsigned golden_width  = 0;
    unsigned golden_height = 0;
    std::vector<unsigned char> golden_rgba;

    if (std::filesystem::exists(golden) && !update) {
        golden_rgba = LoadPngRgba(golden, golden_width, golden_height);
        ASSERT_EQ(golden_width, kWidth);
        ASSERT_EQ(golden_height, kHeight);
    } else {
        WritePng(golden, snapshot, kWidth, kHeight);
        golden_rgba = snapshot;
        golden_width = kWidth;
        golden_height = kHeight;
    }

    ASSERT_EQ(snapshot.size(), golden_rgba.size());
    EXPECT_EQ(snapshot, golden_rgba);
}

}  // namespace
