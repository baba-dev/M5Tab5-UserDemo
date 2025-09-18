/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include <filesystem>
#include <gtest/gtest.h>

#include "settings.h"

namespace {

TEST(SettingsStoreTest, ProvidesDefaultsWhenMissing) {
    core::SettingsStore settings;
    EXPECT_EQ(settings.Get("volume", 5), 5);
    EXPECT_FALSE(settings.Contains("volume"));
}

TEST(SettingsStoreTest, PersistsValuesToDisk) {
    auto temp_dir = std::filesystem::temp_directory_path();
    auto path     = temp_dir / "settings_store_test.cfg";
    std::filesystem::remove(path);

    core::SettingsStore settings;
    settings.Set("volume", 12);
    settings.Set("name", std::string("tab5"));
    settings.Set("enabled", true);

    ASSERT_TRUE(settings.SaveToFile(path.string()));

    core::SettingsStore loaded;
    ASSERT_TRUE(loaded.LoadFromFile(path.string()));

    EXPECT_EQ(loaded.Get("volume", 0), 12);
    EXPECT_EQ(loaded.Get<std::string>("name", ""), "tab5");
    EXPECT_TRUE(loaded.Get("enabled", false));

    std::filesystem::remove(path);
}

TEST(SettingsStoreTest, RemoveClearsKeys) {
    core::SettingsStore settings;
    settings.Set("mode", std::string("auto"));
    EXPECT_TRUE(settings.Contains("mode"));
    EXPECT_TRUE(settings.Remove("mode"));
    EXPECT_FALSE(settings.Contains("mode"));
    EXPECT_EQ(settings.Size(), 0u);
}

}  // namespace
