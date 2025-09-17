# I2C Migration

## Summary of changes

The firmware now relies exclusively on the ESP-IDF NG I²C master driver that ships
with ESP-IDF 5.4 and newer. Mixing the legacy `driver/i2c.h` APIs with the NG
driver triggers a runtime abort, so every component and example has been updated
to include `driver/i2c_master.h` and allocate buses with
`i2c_new_master_bus()` instead of installing the legacy driver.

## Configuration expectations

`sdkconfig.defaults` disables the legacy I²C driver and keeps the conflict
checker active. If `idf.py menuconfig` reports that the legacy driver is
enabled, set `CONFIG_I2C_ENABLE_LEGACY_DRIVERS` to `n` and keep
`CONFIG_I2C_SKIP_LEGACY_CONFLICT_CHECK` disabled.

## Creating an I²C master bus

Use the NG driver helpers when wiring up peripherals. The snippet below shows
the pattern used by Tab5 components:

```c
#include "driver/i2c_master.h"

static i2c_master_bus_handle_t bus;

void init_bus(void)
{
    const i2c_master_bus_config_t bus_cfg = {
        .clk_source                   = I2C_CLK_SRC_DEFAULT,
        .i2c_port                     = I2C_NUM_0,
        .sda_io_num                   = 8,
        .scl_io_num                   = 18,
        .glitch_ignore_cnt            = 7,
        .flags.enable_internal_pullup = false,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus));
}
```

## Adding a new device to the bus

Most drivers (for example `esp_lcd_touch_tt21100`) accept the bus handle
directly and create their own device instance. If you need to manage the device
yourself, add it via `i2c_master_bus_add_device()` and keep the returned
`i2c_master_dev_handle_t` around for `i2c_master_transmit()` and
`i2c_master_transmit_receive()` calls. All transfers should use the NG helper
APIs—do not mix in `i2c_master_cmd_begin()` or the legacy link commands.

## Troubleshooting legacy conflicts

- If the firmware prints `CONFLICT! driver_ng is not allowed to be used with
  this old driver`, erase the flash and confirm no component still pulls in
  `driver/i2c.h`.
- The build now fails if either `CONFIG_I2C_ENABLE_LEGACY_DRIVERS` or
  `CONFIG_I2C_SKIP_LEGACY_CONFLICT_CHECK` is enabled; fix the configuration
  before flashing new firmware.
- Verify CI passed the "Enforce NG I2C usage" step; it runs `git grep` to block
  legacy includes or helper calls from landing in `main`.
- For stubborn build issues, re-run `python3 fetch_repos.py` to make sure every
  component is rebuilt against ESP-IDF 5.4.
