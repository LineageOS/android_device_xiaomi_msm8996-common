/*
 * Copyright (C) 2016 The CyanogenMod Project
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

#include <ui/GraphicBuffer.h>
#include <media/stagefright/MediaBuffer.h>

extern "C" {
extern void _ZN7android13GraphicBufferC1EPK13native_handleNS0_16HandleWrapMethodEjjijmj(
        android::MediaBuffer *thisptr, const native_handle_t *handle,
        android::GraphicBuffer::HandleWrapMethod method,
        uint32_t width, uint32_t height, android::PixelFormat format,
        uint32_t layerCount, uint64_t usage, uint32_t stride);

void _ZN7android13GraphicBufferC1EP19ANativeWindowBufferb(
         android::MediaBuffer *thisptr, ANativeWindowBuffer *buffer,
         bool keepOwnership) {
    _ZN7android13GraphicBufferC1EPK13native_handleNS0_16HandleWrapMethodEjjijmj(
            thisptr, buffer->handle, keepOwnership ? android::GraphicBuffer::TAKE_HANDLE :
            android::GraphicBuffer::WRAP_HANDLE, buffer->width, buffer->height, buffer->format,
            buffer->layerCount, buffer->usage, buffer->stride);
}

extern void _ZN7android11BufferQueue17createBufferQueueEPNS_2spINS_22IGraphicBufferProducerEEEPNS1_INS_22IGraphicBufferConsumerEEEb(
         void *outProducer, void *outConsumer, bool consumerIsSurfaceFlinger);

void _ZN7android11BufferQueue17createBufferQueueEPNS_2spINS_22IGraphicBufferProducerEEEPNS1_INS_22IGraphicBufferConsumerEEERKNS1_INS_19IGraphicBufferAllocEEE(
         void *outProducer, void *outConsumer, void *allocator __unused) {
    _ZN7android11BufferQueue17createBufferQueueEPNS_2spINS_22IGraphicBufferProducerEEEPNS1_INS_22IGraphicBufferConsumerEEEb(
            outProducer, outConsumer, false);
}

int _ZNK7android11MediaBuffer8refcountEv(android::MediaBuffer *thisptr) {
    return thisptr->refcount();
}
}
