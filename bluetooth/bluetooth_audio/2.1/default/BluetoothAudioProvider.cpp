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

#define LOG_TAG "BTAudioProviderStub_2_1"

#include <android-base/logging.h>

#include "BluetoothAudioProvider.h"
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

void BluetoothAudioDeathRecipient::serviceDied(
    uint64_t cookie __unused,
    const wp<::android::hidl::base::V1_0::IBase>& who __unused) {
  LOG(ERROR) << "BluetoothAudioDeathRecipient::" << __func__
             << " - BluetoothAudio Service died";
  provider_->endSession();
}

BluetoothAudioProvider::BluetoothAudioProvider()
    : death_recipient_(new BluetoothAudioDeathRecipient(this)),
      session_type_(SessionType::UNKNOWN),
      audio_config_({}), audio_config_2_0_({}) {}

Return<void> BluetoothAudioProvider::startSession(
    const sp<::vendor::qti::hardware::bluetooth_audio::V2_0::IBluetoothAudioPort>& hostIf,
    const ::vendor::qti::hardware::bluetooth_audio::V2_0::AudioConfiguration& audioConfig,
    startSession_cb _hidl_cb) {
  if (hostIf == nullptr) {
    _hidl_cb(BluetoothAudioStatus::FAILURE, DataMQ::Descriptor());
    return Void();
  }
  /**
   * Initialize the audio platform if audioConfiguration is supported.
   * Save the the IBluetoothAudioPort interface, so that it can be used
   * later to send stream control commands to the HAL client, based on
   * interaction with Audio framework.
   */
  audio_config_2_0_ = audioConfig;
  stack_iface_2_0_ = hostIf;
  stack_iface_2_0_->linkToDeath(death_recipient_, 0);
  client_2_0 = true;
  LOG(INFO) << __func__ << " - SessionType=" << toString(session_type_)
            << ", AudioConfiguration=[" << toString(audio_config_) << "]";

  onSessionReady(_hidl_cb);
  return Void();
}
// Methods from ::vendor::qti::hardware::bluetooth_audio::V2_0::IBluetoothAudioProvider follow.
/*Return<void> BluetoothAudioProvider::startSession(const sp<::vendor::qti::hardware::bluetooth_audio::V2_0::IBluetoothAudioPort>& hostIf, const ::vendor::qti::hardware::bluetooth_audio::V2_0::AudioConfiguration& audioConfig, startSession_cb _hidl_cb) {
    // TODO implement
    return Void();
}*/

/*Return<void> BluetoothAudioProvider::streamStarted(::vendor::qti::hardware::bluetooth_audio::V2_0::Status status) {
    // TODO implement
    return Void();
}*/

Return<void> BluetoothAudioProvider::streamStarted(
    BluetoothAudioStatus status) {
  LOG(INFO) << __func__ << " - SessionType=" << toString(session_type_)
            << ", status=" << toString(status);

  /**
   * Streaming on control path has started,
   * HAL server should start the streaming on data path.
   */
  if (stack_iface_ || stack_iface_2_0_) {
    BluetoothAudioSessionReport::ReportControlStatus(session_type_, true,
                                                     status);
  } else {
    LOG(WARNING) << __func__ << " - SessionType=" << toString(session_type_)
                 << ", status=" << toString(status) << " has NO session";
  }

  return Void();
}
/*Return<void> BluetoothAudioProvider::streamSuspended(::vendor::qti::hardware::bluetooth_audio::V2_0::Status status) {
    // TODO implement
    return Void();
}*/
Return<void> BluetoothAudioProvider::streamSuspended(
    BluetoothAudioStatus status) {
  LOG(INFO) << __func__ << " - SessionType=" << toString(session_type_)
            << ", status=" << toString(status);

  /**
   * Streaming on control path has suspend,
   * HAL server should suspend the streaming on data path.
   */
  if (stack_iface_ || stack_iface_2_0_) {
    BluetoothAudioSessionReport::ReportControlStatus(session_type_, false,
                                                     status);
  } else {
    LOG(WARNING) << __func__ << " - SessionType=" << toString(session_type_)
                 << ", status=" << toString(status) << " has NO session";
  }

  return Void();
}
/*Return<void> BluetoothAudioProvider::updateSessionParams(const ::vendor::qti::hardware::bluetooth_audio::V2_0::SessionParams& sessionParams) {
    // TODO implement
    return Void();
}*/
Return<void> BluetoothAudioProvider::updateSessionParams(
    const SessionParams& params) {

  if (stack_iface_ || stack_iface_2_0_) {
    if(params.paramType != SessionParamType::UNKNOWN) {
      BluetoothAudioSessionReport::OnSessionParamUpdate(session_type_, params.paramType, params);
    }
  } else {
    LOG(WARNING) << __func__ << " - SessionType=" << toString(session_type_)
                 << " has NO session";
  }

  return Void();
}
/*Return<void> BluetoothAudioProvider::endSession() {
    // TODO implement
    return Void();
}*/
Return<void> BluetoothAudioProvider::endSession() {
  LOG(INFO) << __func__ << " - SessionType=" << toString(session_type_);

  std::unique_lock<std::mutex> guard(mutex_);
  if (stack_iface_ || stack_iface_2_0_) {
    BluetoothAudioSessionReport::OnSessionEnded(session_type_);
    if (stack_iface_ != nullptr)
      stack_iface_->unlinkToDeath(death_recipient_);
    else
      stack_iface_2_0_->unlinkToDeath(death_recipient_);
  } else {
    LOG(INFO) << __func__ << " - SessionType=" << toString(session_type_)
              << " has NO session";
  }

  /**
   * Clean up the audio platform as remote audio device is no
   * longer active
   */
  stack_iface_ = nullptr;
  stack_iface_2_0_ = nullptr;
  audio_config_ = {};
  audio_config_2_0_ = {};
  client_2_0 = false;
  return Void();
}

// Methods from ::vendor::qti::hardware::bluetooth_audio::V2_1::IBluetoothAudioProvider follow.
Return<void> BluetoothAudioProvider::startSession_2_1(const sp<::vendor::qti::hardware::bluetooth_audio::V2_1::IBluetoothAudioPort>& hostIf,
    const ::vendor::qti::hardware::bluetooth_audio::V2_1::AudioConfiguration& audioConfig, startSession_2_1_cb _hidl_cb) {
    if (hostIf == nullptr) {
        _hidl_cb(BluetoothAudioStatus::FAILURE, DataMQ::Descriptor());
        return Void();
    }
/**
 * Initialize the audio platform if audioConfiguration is supported.
 * Save the the IBluetoothAudioPort interface, so that it can be used
 * later to send stream control commands to the HAL client, based on
 * interaction with Audio framework.
 */
    audio_config_ = audioConfig;
    stack_iface_ = hostIf;
    stack_iface_->linkToDeath(death_recipient_, 0);
    client_2_0 = false;
    LOG(INFO) << __func__ << " - SessionType=" << toString(session_type_)
         << ", AudioConfiguration=[" << toString(audio_config_) << "]";

    onSessionReady(_hidl_cb);
    return Void();

}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//IBluetoothAudioProvider* HIDL_FETCH_IBluetoothAudioProvider(const char* /* name */) {
    //return new BluetoothAudioProvider();
//}
//
}  // namespace implementation
}  // namespace V2_1
}  // namespace bluetooth_audio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
