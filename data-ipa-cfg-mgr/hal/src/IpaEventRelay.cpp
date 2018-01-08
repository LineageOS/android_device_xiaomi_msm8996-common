/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of The Linux Foundation nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#define LOG_TAG "IPAHALService/IpaEventRelay"
/* External Includes */
#include <cutils/log.h>

/* HIDL Includes */
#include <android/hardware/tetheroffload/control/1.0/ITetheringOffloadCallback.h>

/* Internal Includes */
#include "IpaEventRelay.h"

/* Namespace pollution avoidance */
using ::android::hardware::tetheroffload::control::V1_0::ITetheringOffloadCallback;
using ::android::hardware::tetheroffload::control::V1_0::OffloadCallbackEvent;


IpaEventRelay::IpaEventRelay(
        const ::android::sp<ITetheringOffloadCallback>& cb) : mFramework(cb) {
} /* IpaEventRelay */

void IpaEventRelay::onOffloadStarted() {
    ALOGI("onOffloadStarted()");
    mFramework->onEvent(OffloadCallbackEvent::OFFLOAD_STARTED);
} /* onOffloadStarted */

void IpaEventRelay::onOffloadStopped(StoppedReason reason) {
    ALOGI("onOffloadStopped(%d)", reason);
    switch (reason) {
        case REQUESTED:
            /*
             * No way to communicate this to Framework right now, they make an
             * assumption that offload is stopped when they remove the
             * configuration.
             */
             break;
        case ERROR:
            mFramework->onEvent(OffloadCallbackEvent::OFFLOAD_STOPPED_ERROR);
            break;
        case UNSUPPORTED:
            mFramework->onEvent(OffloadCallbackEvent::OFFLOAD_STOPPED_UNSUPPORTED);
            break;
        default:
            ALOGE("Unknown stopped reason(%d)", reason);
            break;
    }
} /* onOffloadStopped */

void IpaEventRelay::onOffloadSupportAvailable() {
    ALOGI("onOffloadSupportAvailable()");
    mFramework->onEvent(OffloadCallbackEvent::OFFLOAD_SUPPORT_AVAILABLE);
} /* onOffloadSupportAvailable */

void IpaEventRelay::onLimitReached() {
    ALOGI("onLimitReached()");
    mFramework->onEvent(OffloadCallbackEvent::OFFLOAD_STOPPED_LIMIT_REACHED);
} /* onLimitReached */
