/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.lineage.touch-service.xiaomi_8996"

#include "KeySwapper.h"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/strings.h>

using ::android::base::ReadFileToString;
using ::android::base::Trim;
using ::android::base::WriteStringToFile;

namespace {

constexpr const char* kProcButtonsControlPath = "/proc/buttons/reversed_keys_enable";
constexpr const char* kProcTouchpanelControlPath = "/proc/touchpanel/reversed_keys_enable";

}  // anonymous namespace

namespace aidl {
namespace vendor {
namespace lineage {
namespace touch {

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

ndk::ScopedAStatus KeySwapper::getEnabled(bool* _aidl_return) {
    std::string buf;

    if (!has_key_swapper_) return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);

    if (!ReadFileToString(control_path_, &buf)) {
        LOG(ERROR) << "Failed to read current KeySwapper state";
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    *_aidl_return = Trim(buf) == "1";
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus KeySwapper::setEnabled(bool enabled) {
    if (!has_key_swapper_) return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);

    if (!WriteStringToFile(enabled ? "1" : "0", control_path_, true)) {
        LOG(ERROR) << "Failed to write KeySwapper state";
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    return ndk::ScopedAStatus::ok();
}

}  // namespace touch
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
