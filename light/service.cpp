/*
 * Copyright 2018 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "android.hardware.light@2.0-service.xiaomi_8996"

#include <android-base/logging.h>
#include <hidl/HidlTransportSupport.h>
#include <utils/Errors.h>

#include "Light.h"

// libhwbinder:
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

// Generated HIDL files
using android::hardware::light::V2_0::ILight;
using android::hardware::light::V2_0::implementation::Light;

const static std::string kLcdBacklightPath = "/sys/class/leds/lcd-backlight/brightness";
const static std::string kLcdMaxBacklightPath = "/sys/class/leds/lcd-backlight/max_brightness";
const static std::string kButton1BacklightPath = "/sys/class/leds/button-backlight/brightness";
const static std::string kButton2BacklightPath = "/sys/class/leds/button-backlight1/brightness";
const static std::string kButton3BacklightPath = "/sys/class/leds/button-backlight2/brightness";
const static std::string kRedLedPath = "/sys/class/leds/red/brightness";
const static std::string kGreenLedPath = "/sys/class/leds/green/brightness";
const static std::string kBlueLedPath = "/sys/class/leds/blue/brightness";
const static std::string kRedBlinkPath = "/sys/class/leds/red/blink";
const static std::string kGreenBlinkPath = "/sys/class/leds/green/blink";
const static std::string kBlueBlinkPath = "/sys/class/leds/blue/blink";
const static std::string kRgbBlinkPath = "/sys/class/leds/rgb/rgb_blink";

int main() {
    uint32_t lcdMaxBrightness = 255;
    std::vector<std::ofstream> buttonBacklight;

    std::ofstream lcdBacklight(kLcdBacklightPath);
    if (!lcdBacklight) {
        int error = errno;
        LOG(ERROR) << "Failed to open " << kLcdBacklightPath << "(" << error << "): " << strerror(error);
        return -error;
    }

    std::ifstream lcdMaxBacklight(kLcdMaxBacklightPath);
    if (!lcdMaxBacklight) {
        int error = errno;
        LOG(ERROR) << "Failed to open " << kLcdMaxBacklightPath << "(" << error << "): " << strerror(error);
        return -error;
    } else {
        lcdMaxBacklight >> lcdMaxBrightness;
    }

    std::ofstream button1Backlight(kButton1BacklightPath);
    if (button1Backlight) {
        buttonBacklight.emplace_back(std::move(button1Backlight));
    } else {
        int error = errno;
        LOG(WARNING) << "Failed to open " << kButton1BacklightPath << "(" << error << "): " << strerror(error);
    }

    std::ofstream button2Backlight(kButton2BacklightPath);
    if (button2Backlight) {
        buttonBacklight.emplace_back(std::move(button2Backlight));
    } else {
        int error = errno;
        LOG(WARNING) << "Failed to open " << kButton2BacklightPath << "(" << error << "): " << strerror(error);
    }

    std::ofstream button3Backlight(kButton3BacklightPath);
    if (button3Backlight) {
        buttonBacklight.emplace_back(std::move(button3Backlight));
    } else {
        int error = errno;
        LOG(WARNING) << "Failed to open " << kButton3BacklightPath << "(" << error << "): " << strerror(error);
    }

    std::ofstream redLed(kRedLedPath);
    if (!redLed) {
        int error = errno;
        LOG(ERROR) << "Failed to open " << kRedLedPath << "(" << error << "): " << strerror(error);
        return -error;
    }

    std::ofstream greenLed(kGreenLedPath);
    if (!greenLed) {
        int error = errno;
        LOG(ERROR) << "Failed to open " << kGreenLedPath << "(" << error << "): " << strerror(error);
        return -error;
    }

    std::ofstream blueLed(kBlueLedPath);
    if (!blueLed) {
        int error = errno;
        LOG(ERROR) << "Failed to open " << kBlueLedPath << "(" << error << "): " << strerror(error);
        return -error;
    }

    std::ofstream redBlink(kRedBlinkPath);
    if (!redBlink) {
        int error = errno;
        LOG(ERROR) << "Failed to open " << kRedBlinkPath << "(" << error << "): " << strerror(error);
        return -error;
    }

    std::ofstream greenBlink(kGreenBlinkPath);
    if (!greenBlink) {
        int error = errno;
        LOG(ERROR) << "Failed to open " << kGreenBlinkPath << "(" << error << "): " << strerror(error);
        return -error;
    }

    std::ofstream blueBlink(kBlueBlinkPath);
    if (!blueBlink) {
        int error = errno;
        LOG(ERROR) << "Failed to open " << kBlueBlinkPath << "(" << error << "): " << strerror(error);
        return -error;
    }

    std::ofstream rgbBlink(kRgbBlinkPath);
    if (!rgbBlink) {
        int error = errno;
        LOG(ERROR) << "Failed to open " << kRgbBlinkPath << "(" << error << "): " << strerror(error);
        return -error;
    }

    android::sp<ILight> service = new Light({std::move(lcdBacklight), lcdMaxBrightness},
                                            std::move(buttonBacklight),
                                            std::move(redLed), std::move(greenLed), std::move(blueLed),
                                            std::move(redBlink), std::move(greenBlink), std::move(blueBlink),
                                            std::move(rgbBlink));

    configureRpcThreadpool(1, true);

    android::status_t status = service->registerAsService();

    if (status != android::OK) {
        LOG(ERROR) << "Cannot register Light HAL service";
        return 1;
    }

    LOG(INFO) << "Light HAL Ready.";
    joinRpcThreadpool();
    // Under normal cases, execution will not reach this line.
    LOG(ERROR) << "Light HAL failed to join thread pool.";
    return 1;
}
