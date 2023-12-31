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

#define LOG_TAG "BTAudioProviderA2dpOffload_2_1"

#include <android-base/logging.h>
#include <fmq/MessageQueue.h>
#include <hidl/MQDescriptor.h>

#include "A2dpOffloadAudioProvider.h"
#include "BluetoothAudioSessionReport.h"
#include "BluetoothAudioSupportedCodecsDB.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace bluetooth_audio {
namespace V2_1 {
namespace implementation {

using ::vendor::qti::bluetooth_audio::V2_1::BluetoothAudioSessionReport;
using ::android::hardware::kSynchronizedReadWrite;
using ::android::hardware::MessageQueue;
using ::android::hardware::Void;

using DataMQ = MessageQueue<uint8_t, kSynchronizedReadWrite>;

A2dpOffloadAudioProvider::A2dpOffloadAudioProvider()
    : BluetoothAudioProvider() {
  session_type_ = SessionType::A2DP_HARDWARE_OFFLOAD_DATAPATH;
}

bool A2dpOffloadAudioProvider::isValid(const SessionType& sessionType) {
  return (sessionType == session_type_);
}

Return<void> A2dpOffloadAudioProvider::startSession(
    const sp<::vendor::qti::hardware::bluetooth_audio::V2_0::IBluetoothAudioPort>& hostIf,
    const AudioConfiguration_2_0& audioConfig, startSession_cb _hidl_cb) {
  /**
   * Initialize the audio platform if audioConfiguration is supported.
   * Save the the IBluetoothAudioPort interface, so that it can be used
   * later to send stream control commands to the HAL client, based on
   * interaction with Audio framework.
   */
  if (!vendor::qti::bluetooth_audio::IsOffloadCodecConfigurationValid_2_0(
                 session_type_, audioConfig.codecConfig)) {
    _hidl_cb(BluetoothAudioStatus::UNSUPPORTED_CODEC_CONFIGURATION,
             DataMQ::Descriptor());
    return Void();
  }

  return BluetoothAudioProvider::startSession(hostIf, audioConfig, _hidl_cb);
}

Return<void> A2dpOffloadAudioProvider::startSession_2_1(
    const sp<IBluetoothAudioPort>& hostIf,
    const AudioConfiguration& audioConfig, startSession_cb _hidl_cb) {
  /**
   * Initialize the audio platform if audioConfiguration is supported.
   * Save the the IBluetoothAudioPort interface, so that it can be used
   * later to send stream control commands to the HAL client, based on
   * interaction with Audio framework.
   */
  if (!vendor::qti::bluetooth_audio::IsOffloadCodecConfigurationValid(
                 session_type_, audioConfig.codecConfig)) {
    _hidl_cb(BluetoothAudioStatus::UNSUPPORTED_CODEC_CONFIGURATION,
             DataMQ::Descriptor());
    return Void();
  }

  return BluetoothAudioProvider::startSession_2_1(hostIf, audioConfig, _hidl_cb);
}

Return<void> A2dpOffloadAudioProvider::onSessionReady(
    startSession_cb _hidl_cb) {

    BluetoothAudioSessionReport::OnSessionStarted(session_type_, stack_iface_,
                                  nullptr, audio_config_, audio_config_2_0_,
                                  stack_iface_2_0_, BluetoothAudioProvider::isClient_2_0());

  _hidl_cb(BluetoothAudioStatus::SUCCESS, DataMQ::Descriptor());
  return Void();
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace bluetooth_audio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
