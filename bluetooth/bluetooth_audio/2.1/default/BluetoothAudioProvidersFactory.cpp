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

#define LOG_TAG "BTAudioProvidersFactory_2_1"

#include <android-base/logging.h>

#include "BluetoothAudioProvidersFactory.h"
#include "BluetoothAudioSupportedCodecsDB.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace bluetooth_audio {
namespace V2_1 {
namespace implementation {

using ::android::hardware::hidl_vec;
using ::android::hardware::Void;
A2dpSoftwareAudioProvider
    BluetoothAudioProvidersFactory::a2dp_software_provider_instance_;
A2dpOffloadAudioProvider
    BluetoothAudioProvidersFactory::a2dp_offload_provider_instance_;
HearingAidAudioProvider
    BluetoothAudioProvidersFactory::hearing_aid_provider_instance_;

// Methods from ::vendor::qti::hardware::bluetooth_audio::V2_0::IBluetoothAudioProvidersFactory follow.
Return<void> BluetoothAudioProvidersFactory::openProvider(::vendor::qti::hardware::bluetooth_audio::V2_0::SessionType sessionType, openProvider_cb _hidl_cb) {
  LOG(INFO) << __func__ << " - SessionType=" << toString(sessionType);
  BluetoothAudioStatus status = BluetoothAudioStatus::SUCCESS;
  BluetoothAudioProvider* provider = nullptr;
  switch (sessionType) {
    case SessionType::A2DP_SOFTWARE_ENCODING_DATAPATH:
      provider = &a2dp_software_provider_instance_;
      break;
    case SessionType::A2DP_HARDWARE_OFFLOAD_DATAPATH:
      provider = &a2dp_offload_provider_instance_;
      break;
    case SessionType::HEARING_AID_SOFTWARE_ENCODING_DATAPATH:
      provider = &hearing_aid_provider_instance_;
      break;
    default:
      status = BluetoothAudioStatus::FAILURE;
  }
  if (provider == nullptr || !provider->isValid(sessionType)) {
    provider = nullptr;
    status = BluetoothAudioStatus::FAILURE;
    LOG(ERROR) << __func__ << " - SessionType=" << toString(sessionType)
               << ", status=" << toString(status);
  }
  _hidl_cb(status, provider);
  return Void();
}

Return<void> BluetoothAudioProvidersFactory::getProviderCapabilities(::vendor::qti::hardware::bluetooth_audio::V2_0::SessionType sessionType, getProviderCapabilities_cb _hidl_cb) {
  hidl_vec<AudioCapabilities_2_0> audio_capabilities =
      hidl_vec<AudioCapabilities_2_0>(0);
  if (sessionType == SessionType::A2DP_HARDWARE_OFFLOAD_DATAPATH) {
    std::vector<CodecCapabilities_2_0> db_codec_capabilities =
        vendor::qti::bluetooth_audio::GetOffloadCodecCapabilities_2_0(sessionType);
    if (db_codec_capabilities.size()) {
      audio_capabilities.resize(db_codec_capabilities.size());
      for (int i = 0; i < db_codec_capabilities.size(); ++i) {
        audio_capabilities[i].codecCapabilities = db_codec_capabilities[i];
      }
    }
  } else if (sessionType != SessionType::UNKNOWN) {
    std::vector<PcmParameters> db_pcm_capabilities =
        vendor::qti::bluetooth_audio::GetSoftwarePcmCapabilities();
    if (db_pcm_capabilities.size() == 1) {
      audio_capabilities.resize(1);
      audio_capabilities[0].pcmCapabilities = db_pcm_capabilities[0];
    }
  }
  LOG(INFO) << __func__ << " - SessionType=" << toString(sessionType)
            << " supports " << audio_capabilities.size() << " codecs";
  _hidl_cb(audio_capabilities);
  return Void();
}

// Methods from ::vendor::qti::hardware::bluetooth_audio::V2_1::IBluetoothAudioProvidersFactory follow.
Return<void> BluetoothAudioProvidersFactory::openProvider_2_1(::vendor::qti::hardware::bluetooth_audio::V2_0::SessionType sessionType, openProvider_2_1_cb _hidl_cb) {
    LOG(INFO) << __func__ << " - SessionType=" << toString(sessionType);
    BluetoothAudioStatus status = BluetoothAudioStatus::SUCCESS;
    BluetoothAudioProvider* provider = nullptr;
    switch (sessionType) {
        case SessionType::A2DP_SOFTWARE_ENCODING_DATAPATH:
            provider = &a2dp_software_provider_instance_;
        break;
        case SessionType::A2DP_HARDWARE_OFFLOAD_DATAPATH:
            provider = &a2dp_offload_provider_instance_;
        break;
        case SessionType::HEARING_AID_SOFTWARE_ENCODING_DATAPATH:
            provider = &hearing_aid_provider_instance_;
        break;
        default:
            status = BluetoothAudioStatus::FAILURE;
    }
    if (provider == nullptr || !provider->isValid(sessionType)) {
        provider = nullptr;
        status = BluetoothAudioStatus::FAILURE;
        LOG(ERROR) << __func__ << " - SessionType=" << toString(sessionType)
           << ", status=" << toString(status);
    }
    _hidl_cb(status, provider);
    return Void();
}

Return<void> BluetoothAudioProvidersFactory::getProviderCapabilities_2_1(::vendor::qti::hardware::bluetooth_audio::V2_0::SessionType sessionType, getProviderCapabilities_2_1_cb _hidl_cb) {
    hidl_vec<AudioCapabilities> audio_capabilities =
    hidl_vec<AudioCapabilities>(0);
    if (sessionType == SessionType::A2DP_HARDWARE_OFFLOAD_DATAPATH) {
        std::vector<CodecCapabilities> db_codec_capabilities =
            vendor::qti::bluetooth_audio::GetOffloadCodecCapabilities(sessionType);
        if (db_codec_capabilities.size()) {
            audio_capabilities.resize(db_codec_capabilities.size());
            for (int i = 0; i < db_codec_capabilities.size(); ++i) {
                audio_capabilities[i].codecCapabilities = db_codec_capabilities[i];
            }
        }
    } else if (sessionType != SessionType::UNKNOWN) {
        std::vector<PcmParameters> db_pcm_capabilities =
            vendor::qti::bluetooth_audio::GetSoftwarePcmCapabilities();
        if (db_pcm_capabilities.size() == 1) {
            audio_capabilities.resize(1);
            audio_capabilities[0].pcmCapabilities = db_pcm_capabilities[0];
        }
    }
    LOG(INFO) << __func__ << " - SessionType=" << toString(sessionType)
        << " supports " << audio_capabilities.size() << " codecs";
    _hidl_cb(audio_capabilities);
    return Void();
}

IBluetoothAudioProvidersFactory* HIDL_FETCH_IBluetoothAudioProvidersFactory(
    const char* /* name */) {
  return new BluetoothAudioProvidersFactory();
}

}  // namespace implementation
}  // namespace V2_1
}  // namespace bluetooth_audio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
