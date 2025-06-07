/*
 * SPDX-FileCopyrightText: 2019-2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.lineage.touch-service.xiaomi_8996"

#include "KeyDisabler.h"
#include "KeySwapper.h"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

using aidl::vendor::lineage::touch::KeyDisabler;
using aidl::vendor::lineage::touch::KeySwapper;

int main() {
    binder_status_t status = STATUS_OK;

    ABinderProcess_setThreadPoolMaxThreadCount(0);
    std::shared_ptr<KeyDisabler> keyDisabler = ndk::SharedRefBase::make<KeyDisabler>();
    std::shared_ptr<KeySwapper> keySwapper = ndk::SharedRefBase::make<KeySwapper>();

    const std::string instanceKeyDisabler = std::string(KeyDisabler::descriptor) + "/default";
    status = AServiceManager_addService(keyDisabler->asBinder().get(), instanceKeyDisabler.c_str());
    if (status != STATUS_OK) {
        LOG(WARNING) << "Can't register IKeyDisabler/default";
    }

    const std::string instanceKeySwapper = std::string(KeySwapper::descriptor) + "/default";
    status = AServiceManager_addService(keySwapper->asBinder().get(), instanceKeySwapper.c_str());
    if (status != STATUS_OK) {
        LOG(WARNING) << "Can't register IKeySwapper/default";
    }

    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;  // should not reach
}
