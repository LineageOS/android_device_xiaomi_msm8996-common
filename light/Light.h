/*
 * Copyright (C) 2018 The LineageOS Project
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

#pragma once

#include <android/hardware/light/2.0/ILight.h>
#include <hidl/Status.h>

#include <fstream>
#include <mutex>
#include <unordered_map>

namespace android {
namespace hardware {
namespace light {
namespace V2_0 {
namespace implementation {

class Led {
  private:
    Led(const Led&) = delete;
    Led& operator=(const Led&) = delete;

  public:
    Led(int index, const std::string& name);
    Led(Led&&) = default;
    virtual ~Led() {
    }

    explicit operator bool() const;
    bool operator!() const;

    void off() {
        setBrightness(0);
    }
    void setBrightness(int value);
    void setBlink(int brightness, int onMs, int offMs);

  private:
    int mIndex;
    std::string mName;
    std::ofstream mBrightness;
    std::ofstream mDutyPcts;
    std::ofstream mStartIdx;
    std::ofstream mPauseLo;
    std::ofstream mPauseHi;
    std::ofstream mRampStepMs;
    std::ofstream mBlink;
};

struct Light : public ILight {
    Light(std::pair<std::ofstream, uint32_t>&& lcd_backlight,
          std::vector<std::ofstream>&& button_backlight,
          Led&& red_led, Led&& green_led, Led&& blue_led,
          std::ofstream&& rgb_blink);

    // Methods from ::android::hardware::light::V2_0::ILight follow.
    Return<Status> setLight(Type type, const LightState& state) override;
    Return<void> getSupportedTypes(getSupportedTypes_cb _hidl_cb) override;

  private:
    void setAttentionLight(const LightState& state);
    void setBatteryLight(const LightState& state);
    void setButtonsBacklight(const LightState& state);
    void setLcdBacklight(const LightState& state);
    void setNotificationLight(const LightState& state);
    void setSpeakerBatteryLightLocked();
    void setSpeakerLightLocked(const LightState& state);

    std::pair<std::ofstream, uint32_t> mLcdBacklight;
    std::vector<std::ofstream> mButtonBacklight;
    Led mRedLed;
    Led mGreenLed;
    Led mBlueLed;
    std::ofstream mRgbBlink;

    LightState mAttentionState;
    LightState mBatteryState;
    LightState mNotificationState;

    std::unordered_map<Type, std::function<void(const LightState&)>> mLights;
    std::mutex mLock;
};

}  // namespace implementation
}  // namespace V2_0
}  // namespace light
}  // namespace hardware
}  // namespace android
