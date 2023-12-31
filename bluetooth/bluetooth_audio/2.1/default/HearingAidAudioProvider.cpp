/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 */

 /*
 * Copyright 2018 The Android Open Source Project
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

#define LOG_TAG "BTAudioProviderHearingAid_2_1"

#include <android-base/logging.h>

#include "BluetoothAudioSessionReport.h"
#include "BluetoothAudioSupportedCodecsDB.h"
#include "HearingAidAudioProvider.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace bluetooth_audio {
namespace V2_1 {
namespace implementation {

using ::vendor::qti::bluetooth_audio::V2_1::BluetoothAudioSessionReport;
using ::android::hardware::Void;

static constexpr uint32_t kPcmFrameSize = 4;  // 16 bits per sample / stereo
static constexpr uint32_t kPcmFrameCount = 128;
static constexpr uint32_t kRtpFrameSize = kPcmFrameSize * kPcmFrameCount;
static constexpr uint32_t kRtpFrameCount = 7;  // max counts by 1 tick (20ms)
static constexpr uint32_t kBufferSize = kRtpFrameSize * kRtpFrameCount;
static constexpr uint32_t kBufferCount = 1;  // single buffer
static constexpr uint32_t kDataMqSize = kBufferSize * kBufferCount;

HearingAidAudioProvider::HearingAidAudioProvider()
    : BluetoothAudioProvider(), mDataMQ(nullptr) {
  LOG(INFO) << __func__ << " - size of audio buffer " << kDataMqSize
            << " byte(s)";
  std::unique_ptr<DataMQ> tempDataMQ(
      new DataMQ(kDataMqSize, /* EventFlag */ true));
  if (tempDataMQ && tempDataMQ->isValid()) {
    mDataMQ = std::move(tempDataMQ);
    session_type_ = SessionType::HEARING_AID_SOFTWARE_ENCODING_DATAPATH;
  } else {
    ALOGE_IF(!tempDataMQ, "failed to allocate data MQ");
    ALOGE_IF(tempDataMQ && !tempDataMQ->isValid(), "data MQ is invalid");
  }
}

bool HearingAidAudioProvider::isValid(const SessionType& sessionType) {
  return (sessionType == session_type_ && mDataMQ && mDataMQ->isValid());
}

Return<void> HearingAidAudioProvider::startSession(
    const sp<::vendor::qti::hardware::bluetooth_audio::V2_0::IBluetoothAudioPort>& hostIf,
    const AudioConfiguration_2_0& audioConfig, startSession_cb _hidl_cb) {
  /**
   * Initialize the audio platform if audioConfiguration is supported.
   * Save the the IBluetoothAudioPort interface, so that it can be used
   * later to send stream control commands to the HAL client, based on
   * interaction with Audio framework.
   */
  if (!vendor::qti::bluetooth_audio::IsSoftwarePcmConfigurationValid(
                 audioConfig.pcmConfig)) {
    LOG(WARNING) << __func__ << " - Unsupported PCM Configuration="
                 << toString(audioConfig.pcmConfig);
    _hidl_cb(BluetoothAudioStatus::UNSUPPORTED_CODEC_CONFIGURATION,
             DataMQ::Descriptor());
    return Void();
  }

  return BluetoothAudioProvider::startSession(hostIf, audioConfig, _hidl_cb);
}

Return<void> HearingAidAudioProvider::startSession_2_1(
    const sp<IBluetoothAudioPort>& hostIf,
    const AudioConfiguration& audioConfig, startSession_2_1_cb _hidl_cb) {
  /**
   * Initialize the audio platform if audioConfiguration is supported.
   * Save the the IBluetoothAudioPort interface, so that it can be used
   * later to send stream control commands to the HAL client, based on
   * interaction with Audio framework.
   */
  if (!vendor::qti::bluetooth_audio::IsSoftwarePcmConfigurationValid(
                 audioConfig.pcmConfig)) {
    LOG(WARNING) << __func__ << " - Unsupported PCM Configuration="
                 << toString(audioConfig.pcmConfig);
    _hidl_cb(BluetoothAudioStatus::UNSUPPORTED_CODEC_CONFIGURATION,
             DataMQ::Descriptor());
    return Void();
  }

  return BluetoothAudioProvider::startSession_2_1(hostIf, audioConfig, _hidl_cb);
}

Return<void> HearingAidAudioProvider::onSessionReady(startSession_cb _hidl_cb) {
  if (mDataMQ && mDataMQ->isValid()) {
    BluetoothAudioSessionReport::OnSessionStarted(session_type_, stack_iface_,
                                  mDataMQ->getDesc(), audio_config_, audio_config_2_0_,
                                  stack_iface_2_0_,BluetoothAudioProvider::isClient_2_0());

    _hidl_cb(BluetoothAudioStatus::SUCCESS, *mDataMQ->getDesc());
  } else {
    _hidl_cb(BluetoothAudioStatus::FAILURE, DataMQ::Descriptor());
  }
  return Void();
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace bluetooth_audio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
