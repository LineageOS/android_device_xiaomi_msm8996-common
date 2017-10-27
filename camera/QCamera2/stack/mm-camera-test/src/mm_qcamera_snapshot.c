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

// Camera dependencies
#include "mm_qcamera_app.h"
#include "mm_qcamera_dbg.h"

/* This callback is received once the complete JPEG encoding is done */
static void jpeg_encode_cb(jpeg_job_status_t status,
                           uint32_t client_hdl,
                           uint32_t jobId,
                           mm_jpeg_output_t *p_buf,
                           void *userData)
{
    uint32_t i = 0;
    mm_camera_test_obj_t *pme = NULL;
    LOGD(" BEGIN\n");

    pme = (mm_camera_test_obj_t *)userData;
    if (pme->jpeg_hdl != client_hdl ||
        jobId != pme->current_job_id ||
        !pme->current_job_frames) {
        LOGE(" NULL current job frames or not matching job ID (%d, %d)",
                    jobId, pme->current_job_id);
        return;
    }

    /* dump jpeg img */
    LOGE(" job %d, status=%d",  jobId, status);
    if (status == JPEG_JOB_STATUS_DONE && p_buf != NULL) {
        mm_app_dump_jpeg_frame(p_buf->buf_vaddr, p_buf->buf_filled_len, "jpeg", "jpg", jobId);
    }

    /* buf done current encoding frames */
    pme->current_job_id = 0;
    for (i = 0; i < pme->current_job_frames->num_bufs; i++) {
        if (MM_CAMERA_OK != pme->cam->ops->qbuf(pme->current_job_frames->camera_handle,
                                                pme->current_job_frames->ch_id,
                                                pme->current_job_frames->bufs[i])) {
            LOGE(" Failed in Qbuf\n");
        }
        mm_app_cache_ops((mm_camera_app_meminfo_t *) pme->current_job_frames->bufs[i]->mem_info,
                         ION_IOC_INV_CACHES);
    }

    free(pme->jpeg_buf.buf.buffer);
    free(pme->current_job_frames);
    pme->current_job_frames = NULL;

    /* signal snapshot is done */
    mm_camera_app_done();
}

int encodeData(mm_camera_test_obj_t *test_obj, mm_camera_super_buf_t* recvd_frame,
               mm_camera_stream_t *m_stream)
{

    int rc = -MM_CAMERA_E_GENERAL;
    mm_jpeg_job_t job;

    /* remember current frames being encoded */
    test_obj->current_job_frames =
        (mm_camera_super_buf_t *)malloc(sizeof(mm_camera_super_buf_t));
    if (!test_obj->current_job_frames) {
        LOGE(" No memory for current_job_frames");
        return rc;
    }
    *(test_obj->current_job_frames) = *recvd_frame;

    memset(&job, 0, sizeof(job));
    job.job_type = JPEG_JOB_TYPE_ENCODE;
    job.encode_job.session_id = test_obj->current_jpeg_sess_id;

    // TODO: Rotation should be set according to
    //       sensor&device orientation
    job.encode_job.rotation = 0;

    /* fill in main src img encode param */
    job.encode_job.main_dim.src_dim = m_stream->s_config.stream_info->dim;
    job.encode_job.main_dim.dst_dim = m_stream->s_config.stream_info->dim;
    job.encode_job.src_index = 0;

    job.encode_job.thumb_dim.src_dim = m_stream->s_config.stream_info->dim;
    job.encode_job.thumb_dim.dst_dim.width = DEFAULT_PREVIEW_WIDTH;
    job.encode_job.thumb_dim.dst_dim.height = DEFAULT_PREVIEW_HEIGHT;

    /* fill in sink img param */
    job.encode_job.dst_index = 0;

    if (test_obj->metadata != NULL) {
        job.encode_job.p_metadata = test_obj->metadata;
    } else {
        LOGE(" Metadata null, not set for jpeg encoding");
    }

    rc = test_obj->jpeg_ops.start_job(&job, &test_obj->current_job_id);
    if ( 0 != rc ) {
        free(test_obj->current_job_frames);
        test_obj->current_job_frames = NULL;
    }

    return rc;
}

int createEncodingSession(mm_camera_test_obj_t *test_obj,
                          mm_camera_stream_t *m_stream,
                          mm_camera_buf_def_t *m_frame)
{
    mm_jpeg_encode_params_t encode_param;

    memset(&encode_param, 0, sizeof(mm_jpeg_encode_params_t));
    encode_param.jpeg_cb = jpeg_encode_cb;
    encode_param.userdata = (void*)test_obj;
    encode_param.encode_thumbnail = 0;
    encode_param.quality = 85;
    encode_param.color_format = MM_JPEG_COLOR_FORMAT_YCRCBLP_H2V2;
    encode_param.thumb_color_format = MM_JPEG_COLOR_FORMAT_YCRCBLP_H2V2;

    /* fill in main src img encode param */
    encode_param.num_src_bufs = 1;
    encode_param.src_main_buf[0].index = 0;
    encode_param.src_main_buf[0].buf_size = m_frame->frame_len;
    encode_param.src_main_buf[0].buf_vaddr = (uint8_t *)m_frame->buffer;
    encode_param.src_main_buf[0].fd = m_frame->fd;
    encode_param.src_main_buf[0].format = MM_JPEG_FMT_YUV;
    encode_param.src_main_buf[0].offset = m_stream->offset;

    /* fill in sink img param */
    encode_param.num_dst_bufs = 1;
    encode_param.dest_buf[0].index = 0;
    encode_param.dest_buf[0].buf_size = test_obj->jpeg_buf.buf.frame_len;
    encode_param.dest_buf[0].buf_vaddr = (uint8_t *)test_obj->jpeg_buf.buf.buffer;
    encode_param.dest_buf[0].fd = test_obj->jpeg_buf.buf.fd;
    encode_param.dest_buf[0].format = MM_JPEG_FMT_YUV;

    /* main dimension */
    encode_param.main_dim.src_dim = m_stream->s_config.stream_info->dim;
    encode_param.main_dim.dst_dim = m_stream->s_config.stream_info->dim;

    return test_obj->jpeg_ops.create_session(test_obj->jpeg_hdl,
                                             &encode_param,
                                             &test_obj->current_jpeg_sess_id);
}

/** mm_app_snapshot_metadata_notify_cb
 *  @bufs: Pointer to super buffer
 *  @user_data: Pointer to user data
 *
 *
 **/
__unused
static void mm_app_snapshot_metadata_notify_cb(mm_camera_super_buf_t *bufs,
  void *user_data)
{
  uint32_t i = 0;
  mm_camera_channel_t *channel = NULL;
  mm_camera_stream_t *p_stream = NULL;
  mm_camera_test_obj_t *pme = (mm_camera_test_obj_t *)user_data;
  mm_camera_buf_def_t *frame;
  metadata_buffer_t *pMetadata;

  if (NULL == bufs || NULL == user_data) {
    LOGE(" bufs or user_data are not valid ");
    return;
  }
  frame = bufs->bufs[0];

  /* find channel */
  for (i = 0; i < MM_CHANNEL_TYPE_MAX; i++) {
    if (pme->channels[i].ch_id == bufs->ch_id) {
      channel = &pme->channels[i];
      break;
    }
  }

  if (NULL == channel) {
    LOGE(" Channel object is null");
    return;
  }

  /* find meta stream */
  for (i = 0; i < channel->num_streams; i++) {
    if (channel->streams[i].s_config.stream_info->stream_type == CAM_STREAM_TYPE_METADATA) {
      p_stream = &channel->streams[i];
      break;
    }
  }

  if (NULL == p_stream) {
    LOGE(" cannot find metadata stream");
    return;
  }

  /* find meta frame */
  for (i = 0; i < bufs->num_bufs; i++) {
    if (bufs->bufs[i]->stream_id == p_stream->s_id) {
      frame = bufs->bufs[i];
      break;
    }
  }

  if (!pme->metadata) {
    /* The app will free the metadata, we don't need to bother here */
    pme->metadata = malloc(sizeof(metadata_buffer_t));
    if (NULL == pme->metadata) {
        LOGE(" malloc failed");
        return;
    }
  }

  memcpy(pme->metadata , frame->buffer, sizeof(metadata_buffer_t));

  pMetadata = (metadata_buffer_t *)frame->buffer;

  IF_META_AVAILABLE(cam_auto_focus_data_t, focus_data,
        CAM_INTF_META_AUTOFOCUS_DATA, pMetadata) {
    if (focus_data->focus_state == CAM_AF_STATE_FOCUSED_LOCKED) {
      LOGE(" AutoFocus Done Call Back Received\n");
      mm_camera_app_done();
    } else if (focus_data->focus_state == CAM_AF_STATE_NOT_FOCUSED_LOCKED) {
      LOGE(" AutoFocus failed\n");
      mm_camera_app_done();
    }
  }

  if (MM_CAMERA_OK != pme->cam->ops->qbuf(bufs->camera_handle,
                                          bufs->ch_id,
                                          frame)) {
    LOGE(" Failed in Preview Qbuf\n");
  }
  mm_app_cache_ops((mm_camera_app_meminfo_t *)frame->mem_info,
                   ION_IOC_INV_CACHES);
}

static void mm_app_snapshot_notify_cb_raw(mm_camera_super_buf_t *bufs,
                                          void *user_data)
{

    int rc;
    uint32_t i = 0;
    mm_camera_test_obj_t *pme = (mm_camera_test_obj_t *)user_data;
    mm_camera_channel_t *channel = NULL;
    mm_camera_stream_t *m_stream = NULL;
    mm_camera_buf_def_t *m_frame = NULL;

    LOGD(" BEGIN\n");

    /* find channel */
    for (i = 0; i < MM_CHANNEL_TYPE_MAX; i++) {
        if (pme->channels[i].ch_id == bufs->ch_id) {
            channel = &pme->channels[i];
            break;
        }
    }
    if (NULL == channel) {
        LOGE(" Wrong channel id (%d)",  bufs->ch_id);
        rc = -1;
        goto EXIT;
    }

    /* find snapshot stream */
    for (i = 0; i < channel->num_streams; i++) {
        if (channel->streams[i].s_config.stream_info->stream_type == CAM_STREAM_TYPE_RAW) {
            m_stream = &channel->streams[i];
            break;
        }
    }
    if (NULL == m_stream) {
        LOGE(" cannot find snapshot stream");
        rc = -1;
        goto EXIT;
    }

    /* find snapshot frame */
    for (i = 0; i < bufs->num_bufs; i++) {
        if (bufs->bufs[i]->stream_id == m_stream->s_id) {
            m_frame = bufs->bufs[i];
            break;
        }
    }
    if (NULL == m_frame) {
        LOGE(" main frame is NULL");
        rc = -1;
        goto EXIT;
    }

    mm_app_dump_frame(m_frame, "main", "raw", m_frame->frame_idx);

EXIT:
    for (i=0; i<bufs->num_bufs; i++) {
        if (MM_CAMERA_OK != pme->cam->ops->qbuf(bufs->camera_handle,
                                                bufs->ch_id,
                                                bufs->bufs[i])) {
            LOGE(" Failed in Qbuf\n");
        }
    }

    mm_camera_app_done();

    LOGD(" END\n");
}

static void mm_app_snapshot_notify_cb(mm_camera_super_buf_t *bufs,
                                      void *user_data)
{

    int rc = 0;
    uint32_t i = 0;
    mm_camera_test_obj_t *pme = (mm_camera_test_obj_t *)user_data;
    mm_camera_channel_t *channel = NULL;
    mm_camera_stream_t *p_stream = NULL;
    mm_camera_stream_t *m_stream = NULL;
    mm_camera_buf_def_t *p_frame = NULL;
    mm_camera_buf_def_t *m_frame = NULL;

    /* find channel */
    for (i = 0; i < MM_CHANNEL_TYPE_MAX; i++) {
        if (pme->channels[i].ch_id == bufs->ch_id) {
            channel = &pme->channels[i];
            break;
        }
    }
    if (NULL == channel) {
        LOGE(" Wrong channel id (%d)",  bufs->ch_id);
        rc = -1;
        goto error;
    }

    /* find snapshot stream */
    for (i = 0; i < channel->num_streams; i++) {
        if (channel->streams[i].s_config.stream_info->stream_type == CAM_STREAM_TYPE_SNAPSHOT) {
            m_stream = &channel->streams[i];
            break;
        }
    }
    if (NULL == m_stream) {
        LOGE(" cannot find snapshot stream");
        rc = -1;
        goto error;
    }

    /* find snapshot frame */
    for (i = 0; i < bufs->num_bufs; i++) {
        if (bufs->bufs[i]->stream_id == m_stream->s_id) {
            m_frame = bufs->bufs[i];
            break;
        }
    }
    if (NULL == m_frame) {
        LOGE(" main frame is NULL");
        rc = -1;
        goto error;
    }

    mm_app_dump_frame(m_frame, "main", "yuv", m_frame->frame_idx);

    /* find postview stream */
    for (i = 0; i < channel->num_streams; i++) {
        if (channel->streams[i].s_config.stream_info->stream_type == CAM_STREAM_TYPE_POSTVIEW) {
            p_stream = &channel->streams[i];
            break;
        }
    }
    if (NULL != p_stream) {
        /* find preview frame */
        for (i = 0; i < bufs->num_bufs; i++) {
            if (bufs->bufs[i]->stream_id == p_stream->s_id) {
                p_frame = bufs->bufs[i];
                break;
            }
        }
        if (NULL != p_frame) {
            mm_app_dump_frame(p_frame, "postview", "yuv", p_frame->frame_idx);
        }
    }

    mm_app_cache_ops((mm_camera_app_meminfo_t *)m_frame->mem_info,
                     ION_IOC_CLEAN_INV_CACHES);

    pme->jpeg_buf.buf.buffer = (uint8_t *)malloc(m_frame->frame_len);
    if ( NULL == pme->jpeg_buf.buf.buffer ) {
        LOGE(" error allocating jpeg output buffer");
        goto error;
    }

    pme->jpeg_buf.buf.frame_len = m_frame->frame_len;
    /* create a new jpeg encoding session */
    rc = createEncodingSession(pme, m_stream, m_frame);
    if (0 != rc) {
        LOGE(" error creating jpeg session");
        free(pme->jpeg_buf.buf.buffer);
        goto error;
    }

    /* start jpeg encoding job */
    rc = encodeData(pme, bufs, m_stream);
    if (0 != rc) {
        LOGE(" error creating jpeg session");
        free(pme->jpeg_buf.buf.buffer);
        goto error;
    }

error:
    /* buf done rcvd frames in error case */
    if ( 0 != rc ) {
        for (i=0; i<bufs->num_bufs; i++) {
            if (MM_CAMERA_OK != pme->cam->ops->qbuf(bufs->camera_handle,
                                                    bufs->ch_id,
                                                    bufs->bufs[i])) {
                LOGE(" Failed in Qbuf\n");
            }
            mm_app_cache_ops((mm_camera_app_meminfo_t *)bufs->bufs[i]->mem_info,
                             ION_IOC_INV_CACHES);
        }
    }

    LOGD(" END\n");
}

mm_camera_channel_t * mm_app_add_snapshot_channel(mm_camera_test_obj_t *test_obj)
{
    mm_camera_channel_t *channel = NULL;
    mm_camera_stream_t *stream = NULL;

    channel = mm_app_add_channel(test_obj,
                                 MM_CHANNEL_TYPE_SNAPSHOT,
                                 NULL,
                                 NULL,
                                 NULL);
    if (NULL == channel) {
        LOGE(" add channel failed");
        return NULL;
    }

    stream = mm_app_add_snapshot_stream(test_obj,
                                        channel,
                                        mm_app_snapshot_notify_cb,
                                        (void *)test_obj,
                                        1,
                                        1);
    if (NULL == stream) {
        LOGE(" add snapshot stream failed\n");
        mm_app_del_channel(test_obj, channel);
        return NULL;
    }

    return channel;
}

mm_camera_stream_t * mm_app_add_postview_stream(mm_camera_test_obj_t *test_obj,
                                                mm_camera_channel_t *channel,
                                                mm_camera_buf_notify_t stream_cb,
                                                void *userdata,
                                                uint8_t num_bufs,
                                                uint8_t num_burst)
{
    int rc = MM_CAMERA_OK;
    mm_camera_stream_t *stream = NULL;
    cam_capability_t *cam_cap = (cam_capability_t *)(test_obj->cap_buf.buf.buffer);

    stream = mm_app_add_stream(test_obj, channel);
    if (NULL == stream) {
        LOGE(" add stream failed\n");
        return NULL;
    }

    stream->s_config.mem_vtbl.get_bufs = mm_app_stream_initbuf;
    stream->s_config.mem_vtbl.put_bufs = mm_app_stream_deinitbuf;
    stream->s_config.mem_vtbl.clean_invalidate_buf =
      mm_app_stream_clean_invalidate_buf;
    stream->s_config.mem_vtbl.invalidate_buf = mm_app_stream_invalidate_buf;
    stream->s_config.mem_vtbl.user_data = (void *)stream;
    stream->s_config.stream_cb = stream_cb;
    stream->s_config.stream_cb_sync = NULL;
    stream->s_config.userdata = userdata;
    stream->num_of_bufs = num_bufs;

    stream->s_config.stream_info = (cam_stream_info_t *)stream->s_info_buf.buf.buffer;
    memset(stream->s_config.stream_info, 0, sizeof(cam_stream_info_t));
    stream->s_config.stream_info->stream_type = CAM_STREAM_TYPE_POSTVIEW;
    if (num_burst == 0) {
        stream->s_config.stream_info->streaming_mode = CAM_STREAMING_MODE_CONTINUOUS;
    } else {
        stream->s_config.stream_info->streaming_mode = CAM_STREAMING_MODE_BURST;
        stream->s_config.stream_info->num_of_burst = num_burst;
    }
    stream->s_config.stream_info->fmt = DEFAULT_PREVIEW_FORMAT;
    stream->s_config.stream_info->dim.width = DEFAULT_PREVIEW_WIDTH;
    stream->s_config.stream_info->dim.height = DEFAULT_PREVIEW_HEIGHT;
    stream->s_config.padding_info = cam_cap->padding_info;

    rc = mm_app_config_stream(test_obj, channel, stream, &stream->s_config);
    if (MM_CAMERA_OK != rc) {
        LOGE("config postview stream err=%d\n",  rc);
        return NULL;
    }

    return stream;
}

int mm_app_start_capture_raw(mm_camera_test_obj_t *test_obj, uint8_t num_snapshots)
{
    int32_t rc = MM_CAMERA_OK;
    mm_camera_channel_t *channel = NULL;
    mm_camera_stream_t *s_main = NULL;
    mm_camera_channel_attr_t attr;

    memset(&attr, 0, sizeof(mm_camera_channel_attr_t));
    attr.notify_mode = MM_CAMERA_SUPER_BUF_NOTIFY_BURST;
    attr.max_unmatched_frames = 3;
    channel = mm_app_add_channel(test_obj,
                                 MM_CHANNEL_TYPE_CAPTURE,
                                 &attr,
                                 mm_app_snapshot_notify_cb_raw,
                                 test_obj);
    if (NULL == channel) {
        LOGE(" add channel failed");
        return -MM_CAMERA_E_GENERAL;
    }

    test_obj->buffer_format = DEFAULT_RAW_FORMAT;
    s_main = mm_app_add_raw_stream(test_obj,
                                   channel,
                                   mm_app_snapshot_notify_cb_raw,
                                   test_obj,
                                   num_snapshots,
                                   num_snapshots);
    if (NULL == s_main) {
        LOGE(" add main snapshot stream failed\n");
        mm_app_del_channel(test_obj, channel);
        return rc;
    }

    rc = mm_app_start_channel(test_obj, channel);
    if (MM_CAMERA_OK != rc) {
        LOGE("start zsl failed rc=%d\n",  rc);
        mm_app_del_stream(test_obj, channel, s_main);
        mm_app_del_channel(test_obj, channel);
        return rc;
    }

    return rc;
}

int mm_app_stop_capture_raw(mm_camera_test_obj_t *test_obj)
{
    int rc = MM_CAMERA_OK;
    mm_camera_channel_t *ch = NULL;
    int i;
    cam_stream_size_info_t abc ;
    memset (&abc , 0, sizeof (cam_stream_size_info_t));

    ch = mm_app_get_channel_by_type(test_obj, MM_CHANNEL_TYPE_CAPTURE);

    rc = mm_app_stop_channel(test_obj, ch);
    if (MM_CAMERA_OK != rc) {
        LOGE("stop recording failed rc=%d\n",  rc);
    }

    for ( i = 0 ; i < ch->num_streams ; i++ ) {
        mm_app_del_stream(test_obj, ch, &ch->streams[i]);
    }
    rc = setmetainfoCommand(test_obj, &abc);
    if (rc != MM_CAMERA_OK) {
       LOGE(" meta info command failed\n");
    }
    mm_app_del_channel(test_obj, ch);

    return rc;
}

int mm_app_start_capture(mm_camera_test_obj_t *test_obj,
                         uint8_t num_snapshots)
{
    int32_t rc = MM_CAMERA_OK;
    mm_camera_channel_t *channel = NULL;
    mm_camera_stream_t *s_main = NULL;
    mm_camera_stream_t *s_post = NULL;
    mm_camera_channel_attr_t attr;
    memset(&attr, 0, sizeof(mm_camera_channel_attr_t));
    attr.notify_mode = MM_CAMERA_SUPER_BUF_NOTIFY_CONTINUOUS;
    attr.max_unmatched_frames = 3;
    channel = mm_app_add_channel(test_obj,
                                 MM_CHANNEL_TYPE_CAPTURE,
                                 &attr,
                                 mm_app_snapshot_notify_cb,
                                 test_obj);
    if (NULL == channel) {
        LOGE(" add channel failed");
        return -MM_CAMERA_E_GENERAL;
    }

    s_main = mm_app_add_snapshot_stream(test_obj,
                                        channel,
                                        mm_app_snapshot_notify_cb,
                                        (void *)test_obj,
                                        CAPTURE_BUF_NUM,
                                        num_snapshots);
    if (NULL == s_main) {
        LOGE(" add main snapshot stream failed\n");
        mm_app_del_channel(test_obj, channel);
        return rc;
    }

    s_post = mm_app_add_postview_stream(test_obj,
                                        channel,
                                        NULL,
                                        NULL,
                                        CAPTURE_BUF_NUM,
                                        num_snapshots);
    if (NULL == s_main) {
        LOGE(" add main postview stream failed\n");
        mm_app_del_channel(test_obj, channel);
        return rc;
    }

    rc = mm_app_start_channel(test_obj, channel);
    if (MM_CAMERA_OK != rc) {
        LOGE("start zsl failed rc=%d\n",  rc);
        mm_app_del_stream(test_obj, channel, s_main);
        mm_app_del_channel(test_obj, channel);
        return rc;
    }

    return rc;
}

int mm_app_stop_capture(mm_camera_test_obj_t *test_obj)
{
    int rc = MM_CAMERA_OK;
    mm_camera_channel_t *ch = NULL;

    ch = mm_app_get_channel_by_type(test_obj, MM_CHANNEL_TYPE_CAPTURE);

    rc = mm_app_stop_and_del_channel(test_obj, ch);
    if (MM_CAMERA_OK != rc) {
        LOGE("stop capture channel failed rc=%d\n",  rc);
    }

    return rc;
}

int mm_app_take_picture(mm_camera_test_obj_t *test_obj, uint8_t is_burst_mode)
{
    LOGH("\nEnter %s!!\n");
    int rc = MM_CAMERA_OK;
    uint8_t num_snapshot = 1;
    int num_rcvd_snapshot = 0;

    if (is_burst_mode)
       num_snapshot = 6;

    //stop preview before starting capture.
    rc = mm_app_stop_preview(test_obj);
    if (rc != MM_CAMERA_OK) {
        LOGE(" stop preview failed before capture!!, err=%d\n", rc);
        return rc;
    }

    rc = mm_app_start_capture(test_obj, num_snapshot);
    if (rc != MM_CAMERA_OK) {
        LOGE(" mm_app_start_capture(), err=%d\n", rc);
        return rc;
    }
    while (num_rcvd_snapshot < num_snapshot) {
        LOGH("\nWaiting mm_camera_app_wait !!\n");
        mm_camera_app_wait();
        num_rcvd_snapshot++;
    }
    rc = mm_app_stop_capture(test_obj);
    if (rc != MM_CAMERA_OK) {
       LOGE(" mm_app_stop_capture(), err=%d\n", rc);
       return rc;
    }
    //start preview after capture.
    rc = mm_app_start_preview(test_obj);
    if (rc != MM_CAMERA_OK) {
        LOGE(" start preview failed after capture!!, err=%d\n",rc);
    }
    return rc;
}
