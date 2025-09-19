# Changelog

## [1.6.0](https://github.com/baba-dev/M5Tab5-UserDemo/compare/v1.5.0...v1.6.0) (2025-09-19)


### Features

* **cctv:** add model and frigate glue ([81e208d](https://github.com/baba-dev/M5Tab5-UserDemo/commit/81e208d21be17d26ecb66e02ca6fc08a1bc5c750))
* **integration:** add weather formatter ([1ddf8c9](https://github.com/baba-dev/M5Tab5-UserDemo/commit/1ddf8c9adaebc8ec8c8f42074af35d17233e656c))
* **rooms:** add integration rooms provider ([3188438](https://github.com/baba-dev/M5Tab5-UserDemo/commit/31884380fb305a089da52b7badb28f1fe34f951b))
* **settings:** surface live action status ([ea36c9b](https://github.com/baba-dev/M5Tab5-UserDemo/commit/ea36c9b7d81e7c105fee39840172fdf5a0cde7c3))

## [1.5.0](https://github.com/baba-dev/M5Tab5-UserDemo/compare/v1.4.0...v1.5.0) (2025-09-19)


### Features

* **settings:** add threaded controller ([f13f0bf](https://github.com/baba-dev/M5Tab5-UserDemo/commit/f13f0bf33f502caa5129f290eb9e2c1749df2baa))
* **ui:** render wallpaper with launcher asset ([d9f9b37](https://github.com/baba-dev/M5Tab5-UserDemo/commit/d9f9b37a5c2874c83894f1195faa7e94e8434c8b))


### Bug Fixes

* **app:** include settings controller header ([273fb2a](https://github.com/baba-dev/M5Tab5-UserDemo/commit/273fb2ab731d5cdac69301434cdc9aeb073d0b71))
* **build:** normalize settings includes ([cf01dcc](https://github.com/baba-dev/M5Tab5-UserDemo/commit/cf01dcc2dee5dd4dfbb82f17c71eda48b34cfd41))
* **settings_ui:** refresh dim timer argument ([23c635d](https://github.com/baba-dev/M5Tab5-UserDemo/commit/23c635d93b1123968b9c8329c7ac5b4c45a6894a))
* **ui:** cache theme option count ([43c5228](https://github.com/baba-dev/M5Tab5-UserDemo/commit/43c52283d812e9dadd75da3478cbc8f4caa7a4a8))
* **ui:** include wallpaper assets from assets dir ([f205112](https://github.com/baba-dev/M5Tab5-UserDemo/commit/f205112f495ce015df8897033c9b99820c666a6f))

## [1.4.0](https://github.com/baba-dev/M5Tab5-UserDemo/compare/v1.3.0...v1.4.0) (2025-09-19)


### Features

* **settings:** add config services and tests ([7569b13](https://github.com/baba-dev/M5Tab5-UserDemo/commit/7569b13eda2c1e2d07fbfb48ca31b77ccb938343))
* **ui:** add media playback layout ([a6adfc6](https://github.com/baba-dev/M5Tab5-UserDemo/commit/a6adfc6fa2cc090c3b080d790c9288c2c6d1fa1d))
* **ui:** add settings management page ([b5ff353](https://github.com/baba-dev/M5Tab5-UserDemo/commit/b5ff353a104ae52f9aa706b5691994b5843365b6))
* **ui:** rebuild cctv page layout ([e2b470f](https://github.com/baba-dev/M5Tab5-UserDemo/commit/e2b470fd8c36b780bf199394431d0ee1797adc81))
* **ui:** refresh weather layout ([4bb1ae3](https://github.com/baba-dev/M5Tab5-UserDemo/commit/4bb1ae3fd406f6db520465844690b9d37332abc9))


### Bug Fixes

* **scripts:** handle findings outside root ([d2666e1](https://github.com/baba-dev/M5Tab5-UserDemo/commit/d2666e1c9082b39ebd7ada2dc42a0b82f76e00c5))
* **ui:** replace lv array size macro in loops ([9115d7f](https://github.com/baba-dev/M5Tab5-UserDemo/commit/9115d7ffbb1f564d465328165a41c9f07544c737))
* **ui:** store rooms content obj ([10a05ca](https://github.com/baba-dev/M5Tab5-UserDemo/commit/10a05ca7f7a819855670d58a5521eed858d2b406))
* **ui:** use valid cctv card icon ([a624c45](https://github.com/baba-dev/M5Tab5-UserDemo/commit/a624c45ef02790daaaf8c164708bd7d93648b782))

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
