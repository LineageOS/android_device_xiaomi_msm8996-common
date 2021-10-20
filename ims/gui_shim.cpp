#include <stdint.h>
#include <gui/IGraphicBufferProducer.h>

namespace android {
    extern "C" void _ZN7android7SurfaceC1ERKNS_2spINS_22IGraphicBufferProducerEEEbRKNS1_INS_7IBinderEEE(
        const sp<IGraphicBufferProducer>& bufferProducer, bool controlledByApp, const sp<IBinder>& surfaceControlHandle);

    extern "C" void _ZN7android7SurfaceC1ERKNS_2spINS_22IGraphicBufferProducerEEEb(
        const sp<IGraphicBufferProducer>& bufferProducer, bool controlledByApp) {
            _ZN7android7SurfaceC1ERKNS_2spINS_22IGraphicBufferProducerEEEbRKNS1_INS_7IBinderEEE(bufferProducer, controlledByApp, NULL);
    }
}
