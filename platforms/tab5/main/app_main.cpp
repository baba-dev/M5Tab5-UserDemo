/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include <memory>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <sdkconfig.h>

#include "hal/hal_esp32.h"
#include <app.h>
#include <hal/hal.h>

#if defined(CONFIG_I2C_ENABLE_LEGACY_DRIVERS) && CONFIG_I2C_ENABLE_LEGACY_DRIVERS
#error "Legacy ESP-IDF I2C driver is unsupported; disable CONFIG_I2C_ENABLE_LEGACY_DRIVERS."
#endif

#if !defined(CONFIG_I2C_SKIP_LEGACY_CONFLICT_CHECK) || !CONFIG_I2C_SKIP_LEGACY_CONFLICT_CHECK
#error "Enable CONFIG_I2C_SKIP_LEGACY_CONFLICT_CHECK so the legacy driver stays dormant."
#endif

extern "C" void app_main(void)
{
    // 应用层初始化回调
    app::InitCallback_t callback;

    callback.onHalInjection = []() {
        // 注入桌面平台的硬件抽象
        hal::Inject(std::make_unique<HalEsp32>());
    };

    // 应用层启动
    app::Init(callback);
    while (!app::IsDone()) {
        app::Update();
        vTaskDelay(1);
    }
    app::Destroy();
}
