/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 */

 /*
 * Copyright 2019 The Android Open Source Project
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

#include <hardware/audio.h>
#include <system/audio.h>

#include "device_port_proxy.h"

constexpr unsigned int kBluetoothDefaultSampleRate = 44100;
constexpr audio_format_t kBluetoothDefaultAudioFormatBitsPerSample =
    AUDIO_FORMAT_PCM_16_BIT;

constexpr unsigned int kBluetoothDefaultInputBufferMs = 20;

constexpr unsigned int kBluetoothDefaultOutputBufferMs = 10;
constexpr audio_channel_mask_t kBluetoothDefaultOutputChannelModeMask =
    AUDIO_CHANNEL_OUT_STEREO;

enum class BluetoothStreamState : uint8_t {
  DISABLED = 0,  // This stream is closing or set param "suspend=true"
  STANDBY,
  STARTING,
  STARTED,
  SUSPENDING,
  UNKNOWN,
};

std::ostream& operator<<(std::ostream& os, const BluetoothStreamState& state);

struct BluetoothStreamOut {
  // Must be the first member so it can be cast from audio_stream
  // or audio_stream_out pointer
  audio_stream_out stream_out_;
  ::vendor::qti::bluetooth_audio::V2_1::BluetoothAudioPortOut bluetooth_output_;
  int64_t last_write_time_us_;
  // Audio PCM Configs
  uint32_t sample_rate_;
  audio_channel_mask_t channel_mask_;
  audio_format_t format_;
  // frame is the number of samples per channel
  // frames count per tick
  size_t frames_count_;
  // total frames written, reset on standby
  uint64_t frames_rendered_;
  // total frames written after opened, never reset
  uint64_t frames_presented_;
  mutable std::mutex mutex_;
};

int open_output_stream_2_1(struct audio_hw_device* dev,
                            audio_io_handle_t handle, audio_devices_t devices,
                            audio_output_flags_t flags,
                            struct audio_config* config,
                            struct audio_stream_out** stream_out,
                            const char* address __unused);

void close_output_stream_2_1(struct audio_hw_device* dev,
                              struct audio_stream_out* stream);

size_t stream_input_buffer_size_2_1(const struct audio_hw_device* dev,
                                  const struct audio_config* config);

int open_input_stream_2_1(struct audio_hw_device* dev,
                           audio_io_handle_t handle, audio_devices_t devices,
                           struct audio_config* config,
                           struct audio_stream_in** stream_in,
                           audio_input_flags_t flags __unused,
                           const char* address __unused,
                           audio_source_t source __unused);

void close_input_stream_2_1(struct audio_hw_device* dev,
                             struct audio_stream_in* in);
