.PHONY: help setup build test flash monitor fmt tidy assets qemu
IDF_PATH?=/opt/esp/idf
APP_DIR?=platforms/tab5
BUILD_DIR?=$(APP_DIR)/build

help:
	@grep -E '^[a-zA-Z_-]+:.*?## ' Makefile | awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-16s\033[0m %s\n", $$1, $$2}'

setup: ## Install git hooks / Python deps
	python -m pip install -U pip && pip install -r requirements.txt || true

build: ## Build firmware (esp-idf)
	cmake -S $(APP_DIR) -B $(BUILD_DIR) -DIDF_TARGET=esp32p4 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cmake --build $(BUILD_DIR) -j

test: ## Host/unit tests
	cmake -S tests -B tests/build
	cmake --build tests/build -j
	ctest --test-dir tests/build --output-on-failure

fmt: ## Format source
        ./tools/clang_tools.sh format

tidy: ## Static analysis
        ./tools/clang_tools.sh tidy

assets: ## Convert PNG/JPG → runtime-friendly assets + manifest
	python tools/gen_assets.py custom/assets out/assets

flash: build ## Flash device (adjust port)
	python $(IDF_PATH)/components/esptool_py/esptool/esptool.py --chip esp32p4 write_flash 0x0 $(BUILD_DIR)/M5Tab5.bin

monitor: ## Serial monitor
	miniterm.py /dev/ttyUSB0 115200

qemu: ## QEMU smoke tests placeholder
	@echo "QEMU smoke tests are not yet implemented for this project."
