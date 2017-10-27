/* Copyright (c) 2012-2016, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
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
 *
 */

#ifndef __QCAMERA_STREAM_H__
#define __QCAMERA_STREAM_H__

// Camera dependencies
#include "camera.h"
#include "QCameraCmdThread.h"
#include "QCameraMem.h"
#include "QCameraAllocator.h"

extern "C" {
#include "mm_camera_interface.h"
}

namespace qcamera {

class QCameraStream;
typedef void (*stream_cb_routine)(mm_camera_super_buf_t *frame,
                                  QCameraStream *stream,
                                  void *userdata);

#define CAMERA_MAX_CONSUMER_BATCH_BUFFER_SIZE   16
#define CAMERA_MIN_VIDEO_BATCH_BUFFERS          3


class QCameraStream
{
public:
    QCameraStream(QCameraAllocator &allocator,
            uint32_t camHandle, uint32_t chId,
            mm_camera_ops_t *camOps, cam_padding_info_t *paddingInfo,
            bool deffered = false, cam_rotation_t online_rotation = ROTATE_0);
    virtual ~QCameraStream();
    virtual int32_t init(QCameraHeapMemory *streamInfoBuf,
            QCameraHeapMemory *miscBuf,
            uint8_t minStreamBufNum,
            stream_cb_routine stream_cb,
            void *userdata,
            bool bDynallocBuf);
    virtual int32_t processZoomDone(preview_stream_ops_t *previewWindow,
                                    cam_crop_data_t &crop_info);
    virtual int32_t bufDone(uint32_t index);
    virtual int32_t bufDone(const void *opaque, bool isMetaData);
    virtual int32_t processDataNotify(mm_camera_super_buf_t *bufs);
    virtual int32_t start();
    virtual int32_t stop();

    /* Used for deffered allocation of buffers */
    virtual int32_t allocateBuffers();
    virtual int32_t mapBuffers();
    virtual int32_t releaseBuffs();

    static void dataNotifyCB(mm_camera_super_buf_t *recvd_frame, void *userdata);
    static void dataNotifySYNCCB(mm_camera_super_buf_t *recvd_frame,
            void *userdata);
    static void *dataProcRoutine(void *data);
    static void *BufAllocRoutine(void *data);
    uint32_t getMyHandle() const {return mHandle;}
    bool isTypeOf(cam_stream_type_t type);
    bool isOrignalTypeOf(cam_stream_type_t type);
    int32_t getFrameOffset(cam_frame_len_offset_t &offset);
    int32_t getCropInfo(cam_rect_t &crop);
    int32_t setCropInfo(cam_rect_t crop);
    int32_t getFrameDimension(cam_dimension_t &dim);
    int32_t getFormat(cam_format_t &fmt);
    QCameraMemory *getStreamBufs() {return mStreamBufs;};
    QCameraHeapMemory *getStreamInfoBuf() {return mStreamInfoBuf;};
    QCameraHeapMemory *getMiscBuf() {return mMiscBuf;};
    uint32_t getMyServerID();
    cam_stream_type_t getMyType();
    cam_stream_type_t getMyOriginalType();
    int32_t acquireStreamBufs();

    int32_t mapBuf(uint8_t buf_type, uint32_t buf_idx,
            int32_t plane_idx, int fd, size_t size,
            mm_camera_map_unmap_ops_tbl_t *ops_tbl = NULL);
    int32_t mapBufs(cam_buf_map_type_list bufMapList,
            mm_camera_map_unmap_ops_tbl_t *ops_tbl = NULL);
    int32_t mapNewBuffer(uint32_t index);
    int32_t unmapBuf(uint8_t buf_type, uint32_t buf_idx, int32_t plane_idx,
            mm_camera_map_unmap_ops_tbl_t *ops_tbl = NULL);
    int32_t setParameter(cam_stream_parm_buffer_t &param);
    int32_t getParameter(cam_stream_parm_buffer_t &param);
    int32_t syncRuntimeParams();
    cam_stream_parm_buffer_t getOutputCrop() { return m_OutputCrop;};
    cam_stream_parm_buffer_t getImgProp() { return m_ImgProp;};

    static void releaseFrameData(void *data, void *user_data);
    int32_t configStream();
    bool isDeffered() const { return mDefferedAllocation; }
    bool isSyncCBEnabled() {return mSyncCBEnabled;};
    void deleteStream();

    uint8_t getBufferCount() { return mNumBufs; }
    uint32_t getChannelHandle() { return mChannelHandle; }
    int32_t getNumQueuedBuf();

    uint32_t mDumpFrame;
    uint32_t mDumpMetaFrame;
    uint32_t mDumpSkipCnt;

    void cond_wait();
    void cond_signal(bool forceExit = false);

    int32_t setSyncDataCB(stream_cb_routine data_cb);
    //Stream time stamp. We need this for preview stream to update display
    nsecs_t mStreamTimestamp;

    //Frame Buffer will be stored here in case framework batch mode.
    camera_memory_t *mCurMetaMemory; // Current metadata buffer ptr
    int8_t mCurBufIndex;             // Buffer count filled in current metadata
    int8_t mCurMetaIndex;            // Active metadata buffer index

    nsecs_t mFirstTimeStamp;         // Timestamp of first frame in Metadata.

    // Buffer storage structure.
    typedef struct {
        bool consumerOwned; // Metadata is with Consumer if TRUE
        uint8_t numBuffers; // Num of buffer need to released
        uint8_t buf_index[CAMERA_MAX_CONSUMER_BATCH_BUFFER_SIZE];
    } MetaMemory;
    MetaMemory mStreamMetaMemory[CAMERA_MIN_VIDEO_BATCH_BUFFERS];

private:
    uint32_t mCamHandle;
    uint32_t mChannelHandle;
    uint32_t mHandle; // stream handle from mm-camera-interface
    mm_camera_ops_t *mCamOps;
    cam_stream_info_t *mStreamInfo; // ptr to stream info buf
    mm_camera_stream_mem_vtbl_t mMemVtbl;
    uint8_t mNumBufs;
    uint8_t mNumPlaneBufs;
    uint8_t mNumBufsNeedAlloc;
    uint8_t *mRegFlags;
    stream_cb_routine mDataCB;
    stream_cb_routine mSYNCDataCB;
    void *mUserData;

    QCameraQueue     mDataQ;
    QCameraCmdThread mProcTh; // thread for dataCB

    QCameraHeapMemory *mStreamInfoBuf;
    QCameraHeapMemory *mMiscBuf;
    QCameraMemory *mStreamBufs;
    QCameraMemory *mStreamBatchBufs;
    QCameraAllocator &mAllocator;
    mm_camera_buf_def_t *mBufDefs;
    mm_camera_buf_def_t *mPlaneBufDefs;
    cam_frame_len_offset_t mFrameLenOffset;
    cam_padding_info_t mPaddingInfo;
    cam_rect_t mCropInfo;
    cam_rotation_t mOnlineRotation;
    pthread_mutex_t mCropLock; // lock to protect crop info
    pthread_mutex_t mParameterLock; // lock to sync access to parameters
    bool mStreamBufsAcquired;
    bool m_bActive; // if stream mProcTh is active
    bool mDynBufAlloc; // allow buf allocation in 2 steps
    pthread_t mBufAllocPid;
    mm_camera_map_unmap_ops_tbl_t m_MemOpsTbl;
    cam_stream_parm_buffer_t m_OutputCrop;
    cam_stream_parm_buffer_t m_ImgProp;

    static int32_t get_bufs(
                     cam_frame_len_offset_t *offset,
                     uint8_t *num_bufs,
                     uint8_t **initial_reg_flag,
                     mm_camera_buf_def_t **bufs,
                     mm_camera_map_unmap_ops_tbl_t *ops_tbl,
                     void *user_data);

    static int32_t get_bufs_deffered(
            cam_frame_len_offset_t *offset,
            uint8_t *num_bufs,
            uint8_t **initial_reg_flag,
            mm_camera_buf_def_t **bufs,
            mm_camera_map_unmap_ops_tbl_t *ops_tbl,
            void *user_data);

    static int32_t put_bufs(
                     mm_camera_map_unmap_ops_tbl_t *ops_tbl,
                     void *user_data);

    static int32_t put_bufs_deffered(
            mm_camera_map_unmap_ops_tbl_t *ops_tbl,
            void *user_data);

    static int32_t set_config_ops(
            mm_camera_map_unmap_ops_tbl_t *ops_tbl,
            void *user_data);

    static int32_t invalidate_buf(uint32_t index, void *user_data);
    static int32_t clean_invalidate_buf(uint32_t index, void *user_data);

    static int32_t backgroundAllocate(void* data);
    static int32_t backgroundMap(void* data);

    int32_t getBufs(cam_frame_len_offset_t *offset,
                     uint8_t *num_bufs,
                     uint8_t **initial_reg_flag,
                     mm_camera_buf_def_t **bufs,
                     mm_camera_map_unmap_ops_tbl_t *ops_tbl);
    int32_t getBufsDeferred(cam_frame_len_offset_t *offset,
            uint8_t *num_bufs,
            uint8_t **initial_reg_flag,
            mm_camera_buf_def_t **bufs,
            mm_camera_map_unmap_ops_tbl_t *ops_tbl);
    int32_t putBufs(mm_camera_map_unmap_ops_tbl_t *ops_tbl);
    int32_t putBufsDeffered();

    /* Used for deffered allocation of buffers */
    int32_t allocateBatchBufs(cam_frame_len_offset_t *offset,
            uint8_t *num_bufs, uint8_t **initial_reg_flag,
            mm_camera_buf_def_t **bufs, mm_camera_map_unmap_ops_tbl_t *ops_tbl);

    int32_t releaseBatchBufs(mm_camera_map_unmap_ops_tbl_t *ops_tbl);

    int32_t invalidateBuf(uint32_t index);
    int32_t cleanInvalidateBuf(uint32_t index);
    int32_t calcOffset(cam_stream_info_t *streamInfo);
    int32_t unmapStreamInfoBuf();
    int32_t releaseStreamInfoBuf();
    int32_t releaseMiscBuf();
    int32_t mapBufs(QCameraMemory *heapBuf, cam_mapping_buf_type bufType,
            mm_camera_map_unmap_ops_tbl_t *ops_tbl = NULL);
    int32_t unMapBuf(QCameraMemory *heapBuf, cam_mapping_buf_type bufType,
            mm_camera_map_unmap_ops_tbl_t *ops_tbl = NULL);

    bool mDefferedAllocation;

    bool wait_for_cond;
    pthread_mutex_t m_lock;
    pthread_cond_t m_cond;

    BackgroundTask mAllocTask;
    uint32_t mAllocTaskId;
    BackgroundTask mMapTask;
    uint32_t mMapTaskId;

    bool mSyncCBEnabled;
};

}; // namespace qcamera

#endif /* __QCAMERA_STREAM_H__ */
