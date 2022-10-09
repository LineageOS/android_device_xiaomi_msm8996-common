/*
 * Copyright (C) 2022 The LineageOS Project
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

#define LOG_TAG "tfa-calib"

#include <android-base/logging.h>
#include <endian.h>
#include <tinyalsa/asoundlib.h>

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT 0x20746d66
#define ID_DATA 0x61746164

struct riff_wave_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t wave_id;
};

struct chunk_header {
    uint32_t id;
    uint32_t sz;
};

struct chunk_fmt {
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
};

void set_mixer_value_by_name(mixer* mixer, const char* name, int value) {
    const auto ctl = mixer_get_ctl_by_name(mixer, name);

    if (!ctl) {
        LOG(ERROR) << "Failed to find mixer ctl for " << name;
        return;
    }

    if (mixer_ctl_set_value(ctl, 0, value) < 0) {
        LOG(ERROR) << "Failed to set ctl value " << value << " for " << name;
        return;
    }
}

bool check_param(struct pcm_params* params, pcm_param param, unsigned int value,
                 const char* param_name, const char* param_unit) {
    if (const auto min = pcm_params_get_min(params, param); value < min) {
        LOG(ERROR) << param_name << " is " << value << param_unit
                   << ", device only supports >= " << min << param_unit;
        return false;
    }

    if (const auto max = pcm_params_get_max(params, param); value > max) {
        LOG(ERROR) << param_name << " is " << value << param_unit
                   << ", device only supports <= " << max << param_unit;
        return false;
    }

    return true;
}

int sample_is_playable(unsigned int card, unsigned int device, unsigned int channels,
                       unsigned int rate, unsigned int bits, unsigned int period_size,
                       unsigned int period_count) {
    const auto params = pcm_params_get(card, device, PCM_OUT);
    if (!params) {
        LOG(ERROR) << "Unable to open PCM device " << device << ".";
        return 0;
    }

    auto can_play = check_param(params, PCM_PARAM_RATE, rate, "Sample rate", "Hz");
    can_play &= check_param(params, PCM_PARAM_CHANNELS, channels, "Sample", " channels");
    can_play &= check_param(params, PCM_PARAM_SAMPLE_BITS, bits, "Bitrate", " bits");
    can_play &= check_param(params, PCM_PARAM_PERIOD_SIZE, period_size, "Period size", " frames");
    can_play &= check_param(params, PCM_PARAM_PERIODS, period_count, "Period count", " periods");

    pcm_params_free(params);

    return can_play;
}

void play_sample(FILE* file, unsigned int card, unsigned int device, unsigned int channels,
                 unsigned int rate, unsigned int bits, unsigned int period_size,
                 unsigned int period_count, uint32_t data_sz) {
    pcm_config config{};
    config.channels = channels;
    config.rate = rate;
    config.period_size = period_size;
    config.period_count = period_count;
    if (bits == 32)
        config.format = PCM_FORMAT_S32_LE;
    else if (bits == 24)
        config.format = PCM_FORMAT_S24_3LE;
    else if (bits == 16)
        config.format = PCM_FORMAT_S16_LE;
    config.start_threshold = 0;
    config.stop_threshold = 0;
    config.silence_threshold = 0;

    if (!sample_is_playable(card, device, channels, rate, bits, period_size, period_count)) {
        return;
    }

    const auto pcm = pcm_open(card, device, PCM_OUT, &config);
    if (!pcm || !pcm_is_ready(pcm)) {
        LOG(ERROR) << "Unable to open PCM device " << device << " (" << pcm_get_error(pcm) << ")";
        return;
    }

    const auto size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));
    auto buffer = malloc(size);
    if (!buffer) {
        LOG(ERROR) << "Unable to allocate " << size << " bytes";
        free(buffer);
        pcm_close(pcm);
        return;
    }

    int num_read;

    do {
        const auto read_sz = size < data_sz ? size : data_sz;
        num_read = fread(buffer, 1, read_sz, file);
        if (num_read > 0) {
            if (pcm_write(pcm, buffer, num_read)) {
                LOG(ERROR) << "Error playing sample";
                break;
            }
            data_sz -= num_read;
        }
    } while (num_read > 0 && data_sz > 0);

    free(buffer);
    pcm_close(pcm);
}

bool play_sound(const char* filename) {
    const auto file = fopen(filename, "rb");
    if (!file) {
        LOG(ERROR) << "Unable to open file: " << filename;
        return false;
    }

    riff_wave_header riff_wave_header{};
    fread(&riff_wave_header, sizeof(riff_wave_header), 1, file);

    if ((riff_wave_header.riff_id != ID_RIFF) || (riff_wave_header.wave_id != ID_WAVE)) {
        LOG(ERROR) << "Error: '" << filename << "' is not a riff/wave file";
        fclose(file);
        return false;
    }

    chunk_header chunk_header{};
    chunk_fmt chunk_fmt{};
    bool more_chunks = true;

    do {
        fread(&chunk_header, sizeof(chunk_header), 1, file);

        switch (chunk_header.id) {
            case ID_FMT:
                fread(&chunk_fmt, sizeof(chunk_fmt), 1, file);
                /* If the format header is larger, skip the rest */
                if (chunk_header.sz > sizeof(chunk_fmt)) {
                    fseek(file, chunk_header.sz - sizeof(chunk_fmt), SEEK_CUR);
                }
                break;
            case ID_DATA:
                /* Stop looking for chunks */
                more_chunks = false;
                chunk_header.sz = le32toh(chunk_header.sz);
                break;
            default:
                /* Unknown chunk, skip bytes */
                fseek(file, chunk_header.sz, SEEK_CUR);
        }
    } while (more_chunks);

    play_sample(file, 0 /* card */, 0 /* device */, chunk_fmt.num_channels, chunk_fmt.sample_rate,
                chunk_fmt.bits_per_sample, 1024 /* period size */, 4 /* period count */,
                chunk_header.sz);

    fclose(file);

    return true;
}

int main() {
    if (const auto mixer = mixer_open(0)) {
        // Enable speaker
        set_mixer_value_by_name(mixer, "QUAT_MI2S_RX Audio Mixer MultiMedia1", 1);
        set_mixer_value_by_name(mixer, "left Profile", 0 /*music*/);

        // Play amplifier calibration sound
        play_sound("/vendor/etc/silence_short.wav");

        // Disable speaker
        set_mixer_value_by_name(mixer, "QUAT_MI2S_RX Audio Mixer MultiMedia1", 0);

        // Close mixer
        mixer_close(mixer);
    }

    return 0;
}
