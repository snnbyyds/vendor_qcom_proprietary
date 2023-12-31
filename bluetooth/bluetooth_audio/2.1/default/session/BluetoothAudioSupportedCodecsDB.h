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

#pragma once

#include <vendor/qti/hardware/bluetooth_audio/2.1/types.h>

namespace vendor {
namespace qti {
namespace bluetooth_audio {

using ::vendor::qti::hardware::bluetooth_audio::V2_1::CodecCapabilities;
using ::vendor::qti::hardware::bluetooth_audio::V2_1::CodecConfiguration;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::PcmParameters;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::SessionType;
using CodecCapabilities_2_0 =
     ::vendor::qti::hardware::bluetooth_audio::V2_0::CodecCapabilities;
using CodecConfiguration_2_0 =
     ::vendor::qti::hardware::bluetooth_audio::V2_0::CodecConfiguration;
std::vector<PcmParameters> GetSoftwarePcmCapabilities();
std::vector<CodecCapabilities> GetOffloadCodecCapabilities(
    const SessionType& session_type);
std::vector<CodecCapabilities_2_0> GetOffloadCodecCapabilities_2_0(
    const SessionType& session_type);

bool IsSoftwarePcmConfigurationValid(const PcmParameters& pcm_config);
bool IsOffloadCodecConfigurationValid(const SessionType& session_type,
                                      const CodecConfiguration& codec_config);
bool IsOffloadCodecConfigurationValid_2_0(const SessionType& session_type,
                                      const CodecConfiguration_2_0& codec_config);

}  // namespace bluetooth_audio
}  // namespace qti
}  // namespace vendor
