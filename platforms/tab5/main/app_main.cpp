/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
// clang-format off
#include <memory>

#include <app.h>
#include <esp_err.h>
#include <esp_heap_trace.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <hal/hal.h>
#include <sdkconfig.h>

#include "hal/hal_esp32.h"
// clang-format on

#if defined(CONFIG_I2C_ENABLE_LEGACY_DRIVERS) && CONFIG_I2C_ENABLE_LEGACY_DRIVERS
#    error "Legacy ESP-IDF I2C driver is unsupported; disable CONFIG_I2C_ENABLE_LEGACY_DRIVERS."
#endif

#if !defined(CONFIG_I2C_SKIP_LEGACY_CONFLICT_CHECK) || !CONFIG_I2C_SKIP_LEGACY_CONFLICT_CHECK
#    error "Enable CONFIG_I2C_SKIP_LEGACY_CONFLICT_CHECK so the legacy driver stays dormant."
#endif

#if CONFIG_HEAP_TRACING
namespace
{
    constexpr size_t    kHeapTraceDepth = 256;
    heap_trace_record_t s_heap_trace_records[kHeapTraceDepth];
}  // namespace
#endif

extern "C" void app_main(void)
{
    // 应用层初始化回调
    app::InitCallback_t callback;

#if CONFIG_HEAP_TRACING
    ESP_ERROR_CHECK(heap_trace_init_standalone(s_heap_trace_records, kHeapTraceDepth));
    ESP_ERROR_CHECK(heap_trace_start(HEAP_TRACE_ALL));
#endif

    callback.onHalInjection = []()
    {
        // 注入桌面平台的硬件抽象
        hal::Inject(std::make_unique<HalEsp32>());
    };

    // 应用层启动
    app::Init(callback);
    while (!app::IsDone())
    {
        app::Update();
        vTaskDelay(1);
    }
    app::Destroy();

#if CONFIG_HEAP_TRACING
    ESP_ERROR_CHECK(heap_trace_stop());
    heap_trace_dump();
#endif
}
