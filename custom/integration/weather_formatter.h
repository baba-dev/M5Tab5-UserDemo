/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define WEATHER_FORMATTER_MAX_LABEL_LEN 48U
#define WEATHER_FORMATTER_MAX_VALUE_LEN 32U
#define WEATHER_FORMATTER_MAX_ICON_LEN  8U
#define WEATHER_FORMATTER_MAX_RANGE_LEN 32U
#define WEATHER_FORMATTER_MAX_HVAC_LEN  64U
#define WEATHER_FORMATTER_MAX_TEXT_LEN  64U

    typedef enum
    {
        WEATHER_TEMPERATURE_UNIT_CELSIUS    = 0,
        WEATHER_TEMPERATURE_UNIT_FAHRENHEIT = 1,
    } weather_temperature_unit_t;

    typedef enum
    {
        WEATHER_FORECAST_ICON_UNKNOWN = 0,
        WEATHER_FORECAST_ICON_CLEAR,
        WEATHER_FORECAST_ICON_PARTLY_CLOUDY,
        WEATHER_FORECAST_ICON_CLOUDY,
        WEATHER_FORECAST_ICON_RAIN,
        WEATHER_FORECAST_ICON_SNOW,
        WEATHER_FORECAST_ICON_WIND,
        WEATHER_FORECAST_ICON_FOG,
        WEATHER_FORECAST_ICON_THUNDER,
    } weather_forecast_icon_t;

    typedef struct
    {
        bool        has_temperature_c;
        float       temperature_c;
        bool        has_humidity_percent;
        float       humidity_percent;
        const char* hvac_mode;
        const char* hvac_action;
    } weather_climate_payload_t;

    typedef struct
    {
        const char* entity_id;
        bool        has_value;
        float       value;
        const char* unit;
    } weather_sensor_payload_t;

    typedef struct
    {
        const char* period_id;
        bool        has_high_c;
        float       high_c;
        bool        has_low_c;
        float       low_c;
        const char* condition;
    } weather_forecast_payload_t;

    typedef struct
    {
        char temperature[WEATHER_FORMATTER_MAX_VALUE_LEN];
        char humidity[WEATHER_FORMATTER_MAX_VALUE_LEN];
        char hvac_mode[WEATHER_FORMATTER_MAX_HVAC_LEN];
    } weather_indoor_metrics_t;

    typedef struct
    {
        char label[WEATHER_FORMATTER_MAX_LABEL_LEN];
        char value[WEATHER_FORMATTER_MAX_VALUE_LEN];
    } weather_outdoor_metric_t;

    typedef struct
    {
        char                    day_label[WEATHER_FORMATTER_MAX_LABEL_LEN];
        weather_forecast_icon_t icon;
        char                    temperature_range[WEATHER_FORMATTER_MAX_RANGE_LEN];
    } weather_forecast_item_t;

    weather_temperature_unit_t weather_formatter_get_preferred_temperature_unit(void);
    void weather_formatter_set_preferred_temperature_unit(weather_temperature_unit_t unit);

    void weather_formatter_format_indoor(const weather_climate_payload_t* payload,
                                         weather_temperature_unit_t       unit,
                                         weather_indoor_metrics_t*        out_metrics);

    size_t weather_formatter_format_outdoor(const weather_sensor_payload_t* sensors,
                                            size_t                          sensor_count,
                                            weather_temperature_unit_t      unit,
                                            weather_outdoor_metric_t*       out_metrics,
                                            size_t                          out_count);

    size_t weather_formatter_format_forecast(const weather_forecast_payload_t* forecasts,
                                             size_t                            forecast_count,
                                             weather_temperature_unit_t        unit,
                                             weather_forecast_item_t*          out_items,
                                             size_t                            out_count);

#ifdef __cplusplus
}
#endif
