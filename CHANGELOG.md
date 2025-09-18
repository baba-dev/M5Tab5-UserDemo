# Changelog

## [1.3.0](https://github.com/baba-dev/M5Tab5-UserDemo/compare/v1.2.0...v1.3.0) (2025-09-18)


### Features

* **ui:** add rooms page grid and tests ([bb91c7a](https://github.com/baba-dev/M5Tab5-UserDemo/commit/bb91c7aae869c5327eb88a9307e2b6931510a40c))


### Bug Fixes

* **desktop:** guard SDL helpers when disabled ([8128299](https://github.com/baba-dev/M5Tab5-UserDemo/commit/81282994a57cdaa837ea068129c7f4cdad1e7e9e))

## [1.2.0](https://github.com/baba-dev/M5Tab5-UserDemo/compare/v1.1.1...v1.2.0) (2025-09-18)


### Features

* **ui:** load wallpaper image ([606bb5c](https://github.com/baba-dev/M5Tab5-UserDemo/commit/606bb5c4e865f26c976e7604dc78f86740aca1fe))


### Bug Fixes

* **app:** restore screen background after startup anim ([3820251](https://github.com/baba-dev/M5Tab5-UserDemo/commit/3820251b4d5ea6f6238fc234d72309eb41446c76))
* **assets:** adjust asset mount include ([77c61ee](https://github.com/baba-dev/M5Tab5-UserDemo/commit/77c61ee8be13083e9cbaa2f7da535a695acd94e7))
* **assets:** package and resolve wallpaper files ([629bd24](https://github.com/baba-dev/M5Tab5-UserDemo/commit/629bd24671ae53d635b5045af212239219c7dd75))
* **config:** disable legacy i2c driver ([d411dbd](https://github.com/baba-dev/M5Tab5-UserDemo/commit/d411dbd027317ae1993071f7cff3117294cb0ab0))
* **i2c:** block legacy driver regressions ([ac8558e](https://github.com/baba-dev/M5Tab5-UserDemo/commit/ac8558e5d136fc75709d5181f105039d4f092c96))
* **i2c:** bypass legacy conflict guard ([2ddcfee](https://github.com/baba-dev/M5Tab5-UserDemo/commit/2ddcfeeeb8dc2a5f3469d0d27e2ac1af948c1bc9))
* **platform:** disable sd card features ([0003476](https://github.com/baba-dev/M5Tab5-UserDemo/commit/0003476553b13e73632441ee707711440bfcc846))
* **tab5:** stage custom assets layout ([5d2d6ea](https://github.com/baba-dev/M5Tab5-UserDemo/commit/5d2d6ead1893755142300b68a82340f75c122617))
* **ui:** improve rail swipe and wallpaper lookup ([d409bf3](https://github.com/baba-dev/M5Tab5-UserDemo/commit/d409bf3e912cb63a1e5d7605d280027174a466be))
* **ui:** include asset header via component path ([5b1a6e9](https://github.com/baba-dev/M5Tab5-UserDemo/commit/5b1a6e99209eda3647c1cd1060de98f9a1cf5e13))
* **ui:** restore wallpaper and swipe nav ([9099010](https://github.com/baba-dev/M5Tab5-UserDemo/commit/909901082bdc0948731eada1570bd0bb51675576))
* **ui:** simplify wallpaper and nav init ([4e6061b](https://github.com/baba-dev/M5Tab5-UserDemo/commit/4e6061bc9dd28503fe6f582de03e2d504c2a4539))
* **ui:** support lvgl9 wallpaper src ([0b3375d](https://github.com/baba-dev/M5Tab5-UserDemo/commit/0b3375db9b18c911bf60c1c0b62b1fcd628212de))
* **ui:** widen nav rail swipe zone ([77d05d4](https://github.com/baba-dev/M5Tab5-UserDemo/commit/77d05d493632fa2ccf82edb9eb853cef5796d4d6))
* **wifi:** guard ap init against repeated setup ([49b3af4](https://github.com/baba-dev/M5Tab5-UserDemo/commit/49b3af4a39dfbce14282c8481ea768c8420be896))

## [1.1.1](https://github.com/baba-dev/M5Tab5-UserDemo/compare/v1.1.0...v1.1.1) (2025-09-17)


### Bug Fixes

* **ui:** include wallpaper headers ([a184384](https://github.com/baba-dev/M5Tab5-UserDemo/commit/a184384188d9b0bbd7ff2f61bd32bd7706c31f32))

## [1.1.0](https://github.com/baba-dev/M5Tab5-UserDemo/compare/v1.0.0...v1.1.0) (2025-09-17)


### Features

* **assets:** load wallpapers from filesystem ([30ee611](https://github.com/baba-dev/M5Tab5-UserDemo/commit/30ee611d96c9f0a2737b291387c139cc4bb90933))
* **ui:** add left nav rail with 5 tabs and empty pages ([64990da](https://github.com/baba-dev/M5Tab5-UserDemo/commit/64990dab095eaf99dff149c71aa8d2493050a583))


### Bug Fixes

* **app:** include ui_root from component ([a5fde4e](https://github.com/baba-dev/M5Tab5-UserDemo/commit/a5fde4e0c675def9231c755ea2201ba928f615c9))

## 1.0.1 (2025-09-17)

### Fixes

* **i2c:** use NG touch panel IO to keep GT911 working after flashes ([fc3e861](https://github.com/baba-dev/M5Tab5-UserDemo/commit/fc3e861203b86a23a1e9f1055bb0af16f8eb7018))

### Build

* **devcontainer:** adopt the espressif/idf base image and auto-load tools with helper scripts ([d43c6cc](https://github.com/baba-dev/M5Tab5-UserDemo/commit/d43c6cc207187a275ada29055cca8e0c140935e2))

### Tests

* add coverage to detect legacy I²C driver usage in downstream components ([b470ce3](https://github.com/baba-dev/M5Tab5-UserDemo/commit/b470ce39975ec986e459a1e4695c3722e5dae25d))

## 1.0.0 (2025-09-17)


### Bug Fixes

* **wifi:** include cstring for std functions ([9b92b32](https://github.com/baba-dev/M5Tab5-UserDemo/commit/9b92b327165acf63bc462611d9bdd5f9654b651a))
