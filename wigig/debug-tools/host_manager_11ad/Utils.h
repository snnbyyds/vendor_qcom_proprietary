/*
* Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*
* Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
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
*/

#pragma once

#include <vector>
#include <string>
#include <chrono>

struct TimeStamp
{
    tm m_localTime; // year, month, day, hour, minutes, seconds
    std::chrono::seconds::rep m_milliseconds;
};


class Utils
{
public:

    static std::vector<std::string> Split(std::string str, char delimiter);

    static std::string Concatenate(const std::vector<std::string>& vec, const char* delimiter);

    static std::string GetCurrentLocalTimeString();

    static std::string GetCurrentDotNetDateTimeString();

    static TimeStamp GetCurrentLocalTime();
    static std::string GetTimeString(const TimeStamp &ts);
    static std::string GetTimeString(const std::chrono::system_clock::time_point& timePoint);

    static std::string GetCurrentLocalTimeForFileName();

    static bool ConvertHexStringToDword(std::string str, uint32_t& word);

    static bool ConvertHexStringToDwordVector(std::string str, char delimiter, std::vector<uint32_t>& values);

    static bool ConvertDecimalStringToUnsignedInt(std::string str, unsigned int& ui);

    // Encode given binary array as Base64 string
    static std::string Base64Encode(const std::vector<unsigned char>& binaryData);
    static std::string Base64Encode(const unsigned char binaryData[], size_t length);

    // Decode the string given in Base64 format
    static bool Base64Decode(const std::string& base64Data, std::vector<unsigned char>& binaryData);

    const static unsigned int REGISTER_DEFAULT_VALUE;
};

// Timer is able to measure accumulated time periods. It is started upon creation and
// may be stopped then started again several times. Calling Start() twice eliminates the
// previous start point.
class Timer
{
public:

    Timer();

    void Start();
    void Stop();
    void Reset();

    double GetTotalMsec() const;

private:
    std::chrono::time_point<std::chrono::steady_clock> m_startPoint;
    std::chrono::duration<double, std::milli> m_duration;
};

std::ostream& operator<<(std::ostream& os, const Timer& timer);

class Immobile
{
public:
    Immobile() = default;
    Immobile(const Immobile&) = delete;
    Immobile& operator=(const Immobile&) = delete;
    Immobile(Immobile&&) = delete;
    Immobile& operator=(Immobile&&) = delete;
};


// This function replaces strlcpy() that is not available on all platforms.
// Parameters and return value are defined like in strlcpy().
// Destination is always null-terminated.
// Returns the source length. A caller should check that the real length is equal to this.
size_t SafeStringCopy(char *pDst, const char *pSrc, size_t dstCapacity);
