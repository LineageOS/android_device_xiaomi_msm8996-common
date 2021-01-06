/*
 * Copyright (C) 2021 The LineageOS Project
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

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/strings.h>

#include "KeySwapper.h"

namespace vendor {
namespace lineage {
namespace touch {
namespace V1_0 {
namespace implementation {

constexpr const char* kProcButtonsControlPath = "/proc/buttons/reversed_keys_enable";
constexpr const char* kProcTouchpanelControlPath = "/proc/touchpanel/reversed_keys_enable";

KeySwapper::KeySwapper() {
    if (!access(kProcButtonsControlPath, F_OK)) {
        control_path_ = kProcButtonsControlPath;
    } else if (!access(kProcTouchpanelControlPath, F_OK)) {
        control_path_ = kProcTouchpanelControlPath;
    } else {
        control_path_ = nullptr;
    }

    has_key_swapper_ = control_path_ != nullptr;
}

bool KeySwapper::isSupported() {
    return has_key_swapper_;
}

// Methods from ::vendor::lineage::touch::V1_0::IKeySwapper follow.
Return<bool> KeySwapper::isEnabled() {
    std::string buf;

    if (!has_key_swapper_) return false;

    if (!android::base::ReadFileToString(control_path_, &buf)) {
        LOG(ERROR) << "Failed to read " << control_path_;
        return false;
    }

    return android::base::Trim(buf) == "1";
}

Return<bool> KeySwapper::setEnabled(bool enabled) {
    if (!has_key_swapper_) return false;

    if (!android::base::WriteStringToFile(enabled ? "1" : "0", control_path_, true)) {
        LOG(ERROR) << "Failed to write " << control_path_;
        return false;
    }

    return true;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace touch
}  // namespace lineage
}  // namespace vendor
