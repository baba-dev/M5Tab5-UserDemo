/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "hal/hal_esp32.h"
extern "C"
{
#include "utils/rx8130/rx8130.h"
}
#include <algorithm>
#include <bsp/m5stack_tab5.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <initializer_list>
#include <mooncake_log.h>
#include <sys/time.h>

extern esp_lcd_touch_handle_t _lcd_touch_handle;

static const std::string _tag = "hal";

static void lvgl_read_cb(lv_indev_t* indev, lv_indev_data_t* data)
{
    if (_lcd_touch_handle == NULL)
    {
        data->state = LV_INDEV_STATE_REL;
        return;
    }

    uint16_t touch_x[1];
    uint16_t touch_y[1];
    uint16_t touch_strength[1];
    uint8_t  touch_cnt = 0;

    esp_lcd_touch_read_data(_lcd_touch_handle);
    bool touchpad_pressed = esp_lcd_touch_get_coordinates(
        _lcd_touch_handle, touch_x, touch_y, touch_strength, &touch_cnt, 1);
    // mclog::tagInfo(_tag, "touchpad pressed: {}", touchpad_pressed);

    if (!touchpad_pressed)
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state   = LV_INDEV_STATE_PR;
        data->point.x = touch_x[0];
        data->point.y = touch_y[0];
    }
}

void HalEsp32::init()
{
    mclog::tagInfo(_tag, "init");

    mclog::tagInfo(_tag, "camera init");
    bsp_cam_osc_init();

    mclog::tagInfo(_tag, "i2c init");
    bsp_i2c_init();

    mclog::tagInfo(_tag, "io expander init");
    i2c_master_bus_handle_t i2c_bus_handle = bsp_i2c_get_handle();
    bsp_io_expander_pi4ioe_init(i2c_bus_handle);

    // Bring the downstream 5 V rails up before probing peripherals that rely on
    // them.  The IO expanders default to the rails being disabled at power-on,
    // so explicitly drive them high here to ensure the codec, RS485 transceiver
    // and Wi-Fi co-processor are powered for the remainder of the HAL
    // initialisation sequence.
    setUsb5vEnable(true);
    setExt5vEnable(true);
    delay(10);

    setChargeQcEnable(true);
    delay(50);
    setChargeEnable(true);
    // setChargeEnable(false);

    mclog::tagInfo(_tag, "i2c scan");
    bsp_i2c_scan();

    mclog::tagInfo(_tag, "codec init");
    delay(200);
    bsp_codec_init();

    mclog::tagInfo(_tag, "imu init");
    imu_init();

    mclog::tagInfo(_tag, "ina226 init");
    ina226.begin(i2c_bus_handle, 0x41);
    ina226.configure(INA226_AVERAGES_16,
                     INA226_BUS_CONV_TIME_1100US,
                     INA226_SHUNT_CONV_TIME_1100US,
                     INA226_MODE_SHUNT_BUS_CONT);
    ina226.calibrate(0.005, 8.192);
    mclog::tagInfo(_tag, "bus voltage: {}", ina226.readBusVoltage());

    mclog::tagInfo(_tag, "rx8130 init");
    rx8130.begin(i2c_bus_handle, 0x32);
    rx8130.initBat();
    clearRtcIrq();
    update_system_time();

    mclog::tagInfo(_tag, "display init");
    bsp_reset_tp();
    bsp_display_cfg_t cfg = {.lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
                             .buffer_size   = BSP_LCD_H_RES * BSP_LCD_V_RES,
                             .double_buffer = true,
                             .flags         = {
#if CONFIG_BSP_LCD_COLOR_FORMAT_RGB888
                                 .buff_dma = false,
#else
                                 .buff_dma = true,
#endif
                                 .buff_spiram = true,
                                 .sw_rotate   = true,
                             }};
    lvDisp = bsp_display_start_with_config(&cfg);
    lv_display_set_rotation(lvDisp, LV_DISPLAY_ROTATION_90);
    bsp_display_backlight_on();

    lvTouchpad = bsp_display_get_input_dev();
    if (lvTouchpad == nullptr)
    {
        mclog::tagWarn(_tag, "LVGL touch input not provided by BSP; creating fallback driver");

        lvTouchpad = lv_indev_create();
        if (lvTouchpad == nullptr)
        {
            mclog::tagError(_tag, "Failed to allocate LVGL touch input device");
        }
        else
        {
            lv_indev_set_type(lvTouchpad, LV_INDEV_TYPE_POINTER);
            lv_indev_set_read_cb(lvTouchpad, lvgl_read_cb);
            lv_indev_set_display(lvTouchpad, lvDisp);
        }
    }

    mclog::tagInfo(_tag, "usb host init");
    bsp_usb_host_start(BSP_USB_HOST_POWER_MODE_USB_DEV, true);

    mclog::tagInfo(_tag, "hid init");
    hid_init();

    mclog::tagInfo(_tag, "rs485 init");
    rs485_init();

    mclog::tagInfo(_tag, "set gpio output capability");
    set_gpio_output_capability();

    bsp_display_unlock();
}

void HalEsp32::set_gpio_output_capability()
{
    auto set_drive_cap = [](gpio_drive_cap_t cap, std::initializer_list<gpio_num_t> gpios)
    {
        for (gpio_num_t gpio : gpios)
        {
            esp_err_t ret = gpio_set_drive_capability(gpio, cap);
            if (ret == ESP_OK)
            {
                printf("GPIO %d drive capability set to %d\n", gpio, (int)cap);
            }
            else
            {
                printf("Failed to set GPIO %d drive capability: %s\n", gpio, esp_err_to_name(ret));
            }
        }
    };

    // EXT I2C lines are open-drain; keep the weakest drive strength to limit ringing.
    set_drive_cap(GPIO_DRIVE_CAP_0,
                  {
                      GPIO_NUM_0,
                      GPIO_NUM_1,
                  });

    // Maintain strong edges on the esp-hosted ESP32-C6 SPI link.
    set_drive_cap(GPIO_DRIVE_CAP_3,
                  {
                      GPIO_NUM_8,
                      GPIO_NUM_9,
                      GPIO_NUM_10,
                      GPIO_NUM_11,
                      GPIO_NUM_12,
                      GPIO_NUM_13,
                      GPIO_NUM_15,
                  });

    // Display RGB/8080 data lines toggle quickly; keep them at the maximum drive.
    set_drive_cap(GPIO_DRIVE_CAP_3,
                  {
                      GPIO_NUM_22,
                      GPIO_NUM_23,
                  });

    // I2S audio output requires high drive to avoid distortion on the codec bus.
    set_drive_cap(GPIO_DRIVE_CAP_3,
                  {
                      GPIO_NUM_26,
                      GPIO_NUM_27,
                      GPIO_NUM_28,
                      GPIO_NUM_29,
                      GPIO_NUM_30,
                  });

    // SYS I2C bus shares peripherals on the baseboard; keep the weaker drive.
    set_drive_cap(GPIO_DRIVE_CAP_0,
                  {
                      GPIO_NUM_31,
                      GPIO_NUM_32,
                  });

    // microSD runs SDMMC mode at high frequency; retain a strong push-pull drive.
    set_drive_cap(GPIO_DRIVE_CAP_3,
                  {
                      GPIO_NUM_39,
                      GPIO_NUM_40,
                      GPIO_NUM_41,
                      GPIO_NUM_42,
                      GPIO_NUM_43,
                      GPIO_NUM_44,
                  });
}

/* -------------------------------------------------------------------------- */
/*                                   System                                   */
/* -------------------------------------------------------------------------- */
#include <driver/temperature_sensor.h>
static temperature_sensor_handle_t _temp_sensor = nullptr;

void HalEsp32::delay(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

uint32_t HalEsp32::millis()
{
    return esp_timer_get_time() / 1000;
}

int HalEsp32::getCpuTemp()
{
    if (_temp_sensor == nullptr)
    {
        temperature_sensor_config_t temp_sensor_config = {
            .range_min = 20,
            .range_max = 100,
        };
        temperature_sensor_install(&temp_sensor_config, &_temp_sensor);
        temperature_sensor_enable(_temp_sensor);
    }

    float temp = 0;
    temperature_sensor_get_celsius(_temp_sensor, &temp);

    return temp;
}

/* -------------------------------------------------------------------------- */
/*                                   Display                                  */
/* -------------------------------------------------------------------------- */
void HalEsp32::setDisplayBrightness(uint8_t brightness)
{
    _current_lcd_brightness = std::clamp((int)brightness, 0, 100);
    mclog::tagInfo("hal", "set display brightness: {}%", _current_lcd_brightness);
    bsp_display_brightness_set(_current_lcd_brightness);
}

uint8_t HalEsp32::getDisplayBrightness()
{
    return _current_lcd_brightness;
}

void HalEsp32::lvglLock()
{
    lvgl_port_lock(0);
}

void HalEsp32::lvglUnlock()
{
    lvgl_port_unlock();
}

/* -------------------------------------------------------------------------- */
/*                                     RTC                                    */
/* -------------------------------------------------------------------------- */
void HalEsp32::clearRtcIrq()
{
    mclog::tagInfo(_tag, "clear rtc irq");
    rx8130.clearIrqFlags();
    rx8130.disableIrq();
}

void HalEsp32::setRtcTime(tm time)
{
    mclog::tagInfo(_tag,
                   "set rtc time to {}/{}/{} {:02d}:{:02d}:{:02d}",
                   time.tm_year + 1900,
                   time.tm_mon + 1,
                   time.tm_mday,
                   time.tm_hour,
                   time.tm_min,
                   time.tm_sec);
    rx8130.setTime(&time);
    delay(50);

    update_system_time();
}

void HalEsp32::update_system_time()
{
    mclog::tagInfo(_tag, "update system time");
    struct tm time;
    rx8130.getTime(&time);
    mclog::tagInfo(_tag,
                   "sync to rtc time: {}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}",
                   time.tm_year + 1900,
                   time.tm_mon + 1,
                   time.tm_mday,
                   time.tm_hour,
                   time.tm_min,
                   time.tm_sec);
    struct timeval now;
    now.tv_sec  = mktime(&time);
    now.tv_usec = 0;
    settimeofday(&now, NULL);
}

/* -------------------------------------------------------------------------- */
/*                                   SD Card                                  */
/* -------------------------------------------------------------------------- */
bool HalEsp32::isSdCardMounted()
{
    return _sd_card_mounted;
}

std::vector<hal::HalBase::FileEntry_t> HalEsp32::scanSdCard(const std::string& dirPath)
{
    (void)dirPath;
    _sd_card_mounted = false;

    mclog::tagWarn(_tag, "SD card access disabled; skipping scan request");

    return {};
}

/* -------------------------------------------------------------------------- */
/*                                  Interface                                 */
/* -------------------------------------------------------------------------- */
bool HalEsp32::usbCDetect()
{
    return bsp_usb_c_detect();
    // return false;
}

bool HalEsp32::headPhoneDetect()
{
    return bsp_headphone_detect();
}

std::vector<uint8_t> HalEsp32::i2cScan(bool isInternal)
{
    i2c_master_bus_handle_t i2c_bus_handle;
    std::vector<uint8_t>    addrs;

    if (isInternal)
    {
        i2c_bus_handle = bsp_i2c_get_handle();
    }
    else
    {
        i2c_bus_handle = bsp_ext_i2c_get_handle();
    }

    esp_err_t ret;
    uint8_t   address;

    for (int i = 16; i < 128; i += 16)
    {
        for (int j = 0; j < 16; j++)
        {
            fflush(stdout);
            address = i + j;
            ret     = i2c_master_probe(i2c_bus_handle, address, 50);
            if (ret == ESP_OK)
            {
                addrs.push_back(address);
            }
        }
    }

    return addrs;
}

void HalEsp32::initPortAI2c()
{
    mclog::tagInfo(_tag, "init port a i2c");
    bsp_ext_i2c_init();
}

void HalEsp32::deinitPortAI2c()
{
    mclog::tagInfo(_tag, "deinit port a i2c");
    bsp_ext_i2c_deinit();
}

void HalEsp32::gpioInitOutput(uint8_t pin)
{
    gpio_set_pull_mode((gpio_num_t)pin, GPIO_PULLUP_ONLY);
    gpio_set_direction((gpio_num_t)pin, GPIO_MODE_OUTPUT);
}

void HalEsp32::gpioSetLevel(uint8_t pin, bool level)
{
    gpio_set_level((gpio_num_t)pin, level);
}

void HalEsp32::gpioReset(uint8_t pin)
{
    gpio_set_level((gpio_num_t)pin, false);
}
