/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "integration/weather_formatter.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

namespace
{
    constexpr const char* kPlaceholderValue = "--";

    weather_temperature_unit_t g_preferred_unit = WEATHER_TEMPERATURE_UNIT_FAHRENHEIT;

    std::string trim(const std::string& text)
    {
        auto begin = std::find_if_not(
            text.begin(), text.end(), [](unsigned char ch) { return std::isspace(ch) != 0; });
        if (begin == text.end())
        {
            return {};
        }
        auto end = std::find_if_not(text.rbegin(),
                                    text.rend(),
                                    [](unsigned char ch) { return std::isspace(ch) != 0; })
                       .base();
        return std::string(begin, end);
    }

    std::string to_lower(const char* text)
    {
        if (text == nullptr)
        {
            return {};
        }
        std::string lowered(text);
        std::transform(lowered.begin(),
                       lowered.end(),
                       lowered.begin(),
                       [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
        return lowered;
    }

    std::string to_title_case(const char* text)
    {
        if (text == nullptr)
        {
            return {};
        }
        std::string input(text);
        std::string output;
        output.reserve(input.size());
        bool new_word = true;
        for (unsigned char ch : input)
        {
            if (ch == '_' || ch == '-' || ch == '/' || ch == '\\')
            {
                if (!output.empty() && output.back() != ' ')
                {
                    output.push_back(' ');
                }
                new_word = true;
                continue;
            }
            if (std::isspace(ch) != 0)
            {
                if (!output.empty() && output.back() != ' ')
                {
                    output.push_back(' ');
                }
                new_word = true;
                continue;
            }
            if (new_word)
            {
                output.push_back(static_cast<char>(std::toupper(ch)));
                new_word = false;
            }
            else
            {
                output.push_back(static_cast<char>(std::tolower(ch)));
            }
        }
        return trim(output);
    }

    int32_t round_temperature(float value)
    {
        return static_cast<int32_t>(std::lround(value));
    }

    float celsius_to_fahrenheit(float value)
    {
        return (value * 9.0f / 5.0f) + 32.0f;
    }

    bool is_valid_number(float value)
    {
        return !std::isnan(value) && !std::isinf(value);
    }

    std::string format_temperature_value(float temperature_c, weather_temperature_unit_t unit)
    {
        if (!is_valid_number(temperature_c))
        {
            return kPlaceholderValue;
        }

        float   converted = (unit == WEATHER_TEMPERATURE_UNIT_FAHRENHEIT)
                                ? celsius_to_fahrenheit(temperature_c)
                                : temperature_c;
        int32_t rounded   = round_temperature(converted);

        char        buffer[WEATHER_FORMATTER_MAX_VALUE_LEN];
        const char* suffix = (unit == WEATHER_TEMPERATURE_UNIT_FAHRENHEIT) ? "\xC2\xB0"
                                                                             "F"
                                                                           : "\xC2\xB0"
                                                                             "C";
        std::snprintf(buffer, sizeof(buffer), "%d %s", static_cast<int>(rounded), suffix);
        return std::string(buffer);
    }

    std::string format_humidity_value(float humidity_percent)
    {
        if (!is_valid_number(humidity_percent))
        {
            return kPlaceholderValue;
        }
        int32_t rounded = round_temperature(humidity_percent);
        char    buffer[WEATHER_FORMATTER_MAX_VALUE_LEN];
        std::snprintf(buffer, sizeof(buffer), "%d%% RH", static_cast<int>(rounded));
        return std::string(buffer);
    }

    std::string format_hvac_text(const weather_climate_payload_t* payload)
    {
        if (payload == nullptr)
        {
            return kPlaceholderValue;
        }
        std::string mode = to_title_case(payload->hvac_mode);
        if (mode.empty())
        {
            return kPlaceholderValue;
        }
        std::string action = to_title_case(payload->hvac_action);
        if (action.empty() || action == mode)
        {
            return mode;
        }
        return mode + " (" + action + ")";
    }

    bool unit_is_celsius(const std::string& unit)
    {
        return unit.find('c') != std::string::npos || unit.find('C') != std::string::npos;
    }

    bool unit_is_fahrenheit(const std::string& unit)
    {
        return unit.find('f') != std::string::npos || unit.find('F') != std::string::npos;
    }

    std::string normalize_unit_text(const char* unit)
    {
        if (unit == nullptr)
        {
            return {};
        }
        return trim(unit);
    }

    std::string format_sensor_value(float                      value,
                                    bool                       has_value,
                                    const std::string&         source_unit,
                                    weather_temperature_unit_t preferred_unit)
    {
        if (!has_value)
        {
            return kPlaceholderValue;
        }
        float       converted = value;
        std::string unit      = source_unit;
        if (unit.empty())
        {
            char buffer[WEATHER_FORMATTER_MAX_VALUE_LEN];
            std::snprintf(buffer, sizeof(buffer), "%d", round_temperature(converted));
            return std::string(buffer);
        }

        if (unit_is_celsius(unit) || unit == "\xC2\xB0")
        {
            if (preferred_unit == WEATHER_TEMPERATURE_UNIT_FAHRENHEIT)
            {
                converted = celsius_to_fahrenheit(value);
                unit      = "\xC2\xB0"
                            "F";
            }
            else
            {
                converted = value;
                unit      = "\xC2\xB0"
                            "C";
            }
        }
        else if (unit_is_fahrenheit(unit))
        {
            if (preferred_unit == WEATHER_TEMPERATURE_UNIT_CELSIUS)
            {
                converted = (value - 32.0f) * 5.0f / 9.0f;
                unit      = "\xC2\xB0"
                            "C";
            }
            else
            {
                unit = "\xC2\xB0"
                       "F";
            }
        }

        int32_t rounded = round_temperature(converted);
        char    buffer[WEATHER_FORMATTER_MAX_VALUE_LEN];
        if (!unit.empty())
        {
            if (unit == "%")
            {
                std::snprintf(buffer, sizeof(buffer), "%d %%", static_cast<int>(rounded));
            }
            else
            {
                std::snprintf(
                    buffer, sizeof(buffer), "%d %s", static_cast<int>(rounded), unit.c_str());
            }
        }
        else
        {
            std::snprintf(buffer, sizeof(buffer), "%d", static_cast<int>(rounded));
        }
        return std::string(buffer);
    }

    std::string format_period_label(const char* period_id)
    {
        std::string label = to_title_case(period_id);
        if (label.empty())
        {
            return {};
        }
        return label;
    }

    weather_forecast_icon_t map_condition_to_icon(const char* condition)
    {
        std::string normalized = to_lower(condition);
        if (normalized.empty())
        {
            return WEATHER_FORECAST_ICON_UNKNOWN;
        }
        if (normalized.find("thunder") != std::string::npos
            || normalized.find("storm") != std::string::npos)
        {
            return WEATHER_FORECAST_ICON_THUNDER;
        }
        if (normalized.find("snow") != std::string::npos)
        {
            return WEATHER_FORECAST_ICON_SNOW;
        }
        if (normalized.find("rain") != std::string::npos
            || normalized.find("shower") != std::string::npos)
        {
            return WEATHER_FORECAST_ICON_RAIN;
        }
        if (normalized.find("fog") != std::string::npos
            || normalized.find("mist") != std::string::npos
            || normalized.find("haze") != std::string::npos)
        {
            return WEATHER_FORECAST_ICON_FOG;
        }
        if (normalized.find("wind") != std::string::npos
            || normalized.find("breeze") != std::string::npos)
        {
            return WEATHER_FORECAST_ICON_WIND;
        }
        if (normalized.find("partly") != std::string::npos
            || normalized.find("mostly") != std::string::npos)
        {
            return WEATHER_FORECAST_ICON_PARTLY_CLOUDY;
        }
        if (normalized.find("cloud") != std::string::npos
            || normalized.find("overcast") != std::string::npos)
        {
            return WEATHER_FORECAST_ICON_CLOUDY;
        }
        if (normalized.find("clear") != std::string::npos
            || normalized.find("sun") != std::string::npos)
        {
            return WEATHER_FORECAST_ICON_CLEAR;
        }
        return WEATHER_FORECAST_ICON_UNKNOWN;
    }

    std::string format_forecast_range(const weather_forecast_payload_t* payload,
                                      weather_temperature_unit_t        unit)
    {
        if (payload == nullptr || !payload->has_high_c || !payload->has_low_c)
        {
            return kPlaceholderValue;
        }
        if (!is_valid_number(payload->high_c) || !is_valid_number(payload->low_c))
        {
            return kPlaceholderValue;
        }
        float high = payload->high_c;
        float low  = payload->low_c;
        if (unit == WEATHER_TEMPERATURE_UNIT_FAHRENHEIT)
        {
            high = celsius_to_fahrenheit(high);
            low  = celsius_to_fahrenheit(low);
        }
        int32_t rounded_high = round_temperature(high);
        int32_t rounded_low  = round_temperature(low);
        char    buffer[WEATHER_FORMATTER_MAX_RANGE_LEN];
        std::snprintf(buffer,
                      sizeof(buffer),
                      "%d\xC2\xB0 / %d\xC2\xB0",
                      static_cast<int>(rounded_high),
                      static_cast<int>(rounded_low));
        return std::string(buffer);
    }

    void copy_string(char* dest, size_t dest_len, const std::string& source)
    {
        if (dest == nullptr || dest_len == 0U)
        {
            return;
        }
        size_t to_copy = std::min(dest_len - 1U, source.size());
        std::memcpy(dest, source.c_str(), to_copy);
        dest[to_copy] = '\0';
    }

}  // namespace

extern "C"
{
    weather_temperature_unit_t weather_formatter_get_preferred_temperature_unit(void)
    {
        return g_preferred_unit;
    }

    void weather_formatter_set_preferred_temperature_unit(weather_temperature_unit_t unit)
    {
        if (unit != WEATHER_TEMPERATURE_UNIT_CELSIUS && unit != WEATHER_TEMPERATURE_UNIT_FAHRENHEIT)
        {
            return;
        }
        g_preferred_unit = unit;
    }

    void weather_formatter_format_indoor(const weather_climate_payload_t* payload,
                                         weather_temperature_unit_t       unit,
                                         weather_indoor_metrics_t*        out_metrics)
    {
        if (out_metrics == nullptr)
        {
            return;
        }
        if (payload == nullptr)
        {
            std::snprintf(out_metrics->temperature,
                          sizeof(out_metrics->temperature),
                          "%s",
                          kPlaceholderValue);
            std::snprintf(
                out_metrics->humidity, sizeof(out_metrics->humidity), "%s", kPlaceholderValue);
            std::snprintf(
                out_metrics->hvac_mode, sizeof(out_metrics->hvac_mode), "%s", kPlaceholderValue);
            return;
        }

        std::string temperature = payload->has_temperature_c
                                      ? format_temperature_value(payload->temperature_c, unit)
                                      : std::string(kPlaceholderValue);
        std::string humidity    = payload->has_humidity_percent
                                      ? format_humidity_value(payload->humidity_percent)
                                      : std::string(kPlaceholderValue);
        std::string hvac        = format_hvac_text(payload);

        copy_string(out_metrics->temperature, sizeof(out_metrics->temperature), temperature);
        copy_string(out_metrics->humidity, sizeof(out_metrics->humidity), humidity);
        copy_string(out_metrics->hvac_mode, sizeof(out_metrics->hvac_mode), hvac);
    }

    size_t weather_formatter_format_outdoor(const weather_sensor_payload_t* sensors,
                                            size_t                          sensor_count,
                                            weather_temperature_unit_t      unit,
                                            weather_outdoor_metric_t*       out_metrics,
                                            size_t                          out_count)
    {
        if (sensors == nullptr || out_metrics == nullptr || out_count == 0U)
        {
            return 0U;
        }

        size_t written = 0U;
        for (size_t i = 0; i < sensor_count && written < out_count; ++i)
        {
            const weather_sensor_payload_t& sensor = sensors[i];
            copy_string(out_metrics[written].label,
                        sizeof(out_metrics[written].label),
                        sensor.entity_id != nullptr ? sensor.entity_id : "");
            std::string unit_text = normalize_unit_text(sensor.unit);
            std::string value =
                format_sensor_value(sensor.value, sensor.has_value, unit_text, unit);
            copy_string(out_metrics[written].value, sizeof(out_metrics[written].value), value);
            ++written;
        }
        return written;
    }

    size_t weather_formatter_format_forecast(const weather_forecast_payload_t* forecasts,
                                             size_t                            forecast_count,
                                             weather_temperature_unit_t        unit,
                                             weather_forecast_item_t*          out_items,
                                             size_t                            out_count)
    {
        if (forecasts == nullptr || out_items == nullptr || out_count == 0U)
        {
            return 0U;
        }

        size_t written = 0U;
        for (size_t i = 0; i < forecast_count && written < out_count; ++i)
        {
            const weather_forecast_payload_t& payload = forecasts[i];
            std::string                       label   = format_period_label(payload.period_id);
            if (label.empty())
            {
                continue;
            }
            std::string range = format_forecast_range(&payload, unit);
            copy_string(out_items[written].day_label, sizeof(out_items[written].day_label), label);
            copy_string(out_items[written].temperature_range,
                        sizeof(out_items[written].temperature_range),
                        range.empty() ? kPlaceholderValue : range);
            out_items[written].icon = map_condition_to_icon(payload.condition);
            ++written;
        }
        return written;
    }

}  // extern "C"
