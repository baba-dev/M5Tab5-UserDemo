/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include <gtest/gtest.h>

extern "C"
{
#include "integration/weather_formatter.h"
}

namespace
{

    TEST(WeatherFormatterTest, FormatsIndoorMetricsInFahrenheit)
    {
        weather_climate_payload_t payload = {
            .has_temperature_c    = true,
            .temperature_c        = 23.0f,
            .has_humidity_percent = true,
            .humidity_percent     = 48.4f,
            .hvac_mode            = "auto",
            .hvac_action          = "cooling",
        };

        weather_indoor_metrics_t metrics;
        weather_formatter_format_indoor(&payload, WEATHER_TEMPERATURE_UNIT_FAHRENHEIT, &metrics);

        EXPECT_STREQ("73 °F", metrics.temperature);
        EXPECT_STREQ("48% RH", metrics.humidity);
        EXPECT_STREQ("Auto (Cooling)", metrics.hvac_mode);
    }

    TEST(WeatherFormatterTest, FormatsIndoorMetricsInCelsius)
    {
        weather_climate_payload_t payload = {
            .has_temperature_c    = true,
            .temperature_c        = 22.4f,
            .has_humidity_percent = false,
            .humidity_percent     = 0.0f,
            .hvac_mode            = "heat",
            .hvac_action          = nullptr,
        };

        weather_indoor_metrics_t metrics;
        weather_formatter_format_indoor(&payload, WEATHER_TEMPERATURE_UNIT_CELSIUS, &metrics);

        EXPECT_STREQ("22 °C", metrics.temperature);
        EXPECT_STREQ("--", metrics.humidity);
        EXPECT_STREQ("Heat", metrics.hvac_mode);
    }

    TEST(WeatherFormatterTest, FormatsOutdoorSensorsWithConversion)
    {
        weather_sensor_payload_t sensors[] = {
            {.entity_id = "sensor.tab5_temperature",
             .has_value = true,
             .value     = 18.3f,
             .unit      = "°C"},
            {.entity_id = "sensor.tab5_humidity", .has_value = true, .value = 52.2f, .unit = "%"},
        };
        weather_outdoor_metric_t metrics[2];

        size_t count = weather_formatter_format_outdoor(sensors,
                                                        sizeof(sensors) / sizeof(sensors[0]),
                                                        WEATHER_TEMPERATURE_UNIT_FAHRENHEIT,
                                                        metrics,
                                                        sizeof(metrics) / sizeof(metrics[0]));

        ASSERT_EQ(2U, count);
        EXPECT_STREQ("sensor.tab5_temperature", metrics[0].label);
        EXPECT_STREQ("65 °F", metrics[0].value);
        EXPECT_STREQ("sensor.tab5_humidity", metrics[1].label);
        EXPECT_STREQ("52 %", metrics[1].value);
    }

    TEST(WeatherFormatterTest, FormatsForecastEntries)
    {
        weather_forecast_payload_t forecasts[] = {
            {.period_id  = "today",
             .has_high_c = true,
             .high_c     = 24.0f,
             .has_low_c  = true,
             .low_c      = 16.0f,
             .condition  = "rain"},
            {.period_id  = "Sun",
             .has_high_c = true,
             .high_c     = 26.0f,
             .has_low_c  = true,
             .low_c      = 18.0f,
             .condition  = "clear"},
        };
        weather_forecast_item_t items[3];

        size_t count = weather_formatter_format_forecast(forecasts,
                                                         sizeof(forecasts) / sizeof(forecasts[0]),
                                                         WEATHER_TEMPERATURE_UNIT_FAHRENHEIT,
                                                         items,
                                                         sizeof(items) / sizeof(items[0]));

        ASSERT_EQ(2U, count);
        EXPECT_STREQ("Today", items[0].day_label);
        EXPECT_STREQ("75° / 61°", items[0].temperature_range);
        EXPECT_EQ(WEATHER_FORECAST_ICON_RAIN, items[0].icon);

        EXPECT_STREQ("Sun", items[1].day_label);
        EXPECT_STREQ("79° / 64°", items[1].temperature_range);
        EXPECT_EQ(WEATHER_FORECAST_ICON_CLEAR, items[1].icon);
    }

}  // namespace
