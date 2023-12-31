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

#include "Utils.h"

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <cstring>

#if _WINDOWS
#define localtime_r(_Time, _Tm) localtime_s(_Tm, _Time)
#endif

using namespace std;

const unsigned int Utils::REGISTER_DEFAULT_VALUE = 0xDEADDEAD;

// *************************************************************************************************
vector<string> Utils::Split(string str, char delimiter)
{
    vector<string> splitStr;
    size_t nextSpacePosition = str.find_first_of(delimiter);
    while (string::npos != nextSpacePosition)
    {
        splitStr.push_back(str.substr(0, nextSpacePosition));
        str = str.substr(nextSpacePosition + 1);
        nextSpacePosition = str.find_first_of(delimiter);
    }

    if (!str.empty())
    {
        splitStr.push_back(str);
    }
    return splitStr;
}

// *************************************************************************************************
string Utils::Concatenate(const vector<string>& vec, const char* delimiter)
{
    if (vec.empty())
    {
        return "";
    }
    if (vec.size() == 1U)
    {
        return vec.front();
    }

    stringstream ss;
    std::copy(vec.cbegin(), std::prev(vec.cend()), ostream_iterator<string>(ss, delimiter));
    ss << vec.back();
    return ss.str();
}

std::string Utils::GetTimeString(const TimeStamp &ts)
{
    int tm_hourTs = ts.m_localTime.tm_hour;
    const std::string ampm = tm_hourTs>=12 ? "PM" : "AM" ;
    if (tm_hourTs > 12)
    {
        tm_hourTs -= 12;
    }
    stringstream timeStampBuilder;
    timeStampBuilder << "[" << std::setfill('0') << std::setw(2) << ts.m_localTime.tm_mon + 1 << "/" <<
        std::setfill('0') << std::setw(2) << ts.m_localTime.tm_mday << "/" <<
        std::setfill('0') << std::setw(2) << ts.m_localTime.tm_year + 1900 << " " <<
        std::setfill('0') << std::setw(2) << tm_hourTs << ":" <<
        std::setfill('0') << std::setw(2) << ts.m_localTime.tm_min << ":" <<
        std::setfill('0') << std::setw(2) << ts.m_localTime.tm_sec << "." <<
        std::setfill('0') << std::setw(3) << ts.m_milliseconds << " " << ampm << "]";
    return timeStampBuilder.str();
}

// *************************************************************************************************
std::string Utils::GetTimeString(const std::chrono::system_clock::time_point& timePoint)
{
    // convert epoch time to struct with year, month, day, hour, minute, second fields
    time_t tt = chrono::system_clock::to_time_t(timePoint);
    TimeStamp ts {};
    localtime_r(&tt, &ts.m_localTime);

    // get milliseconds field
    const chrono::duration<double> tse = timePoint.time_since_epoch();
    ts.m_milliseconds = chrono::duration_cast<std::chrono::milliseconds>(tse).count() % 1000;
    return GetTimeString(ts);
}

// *************************************************************************************************
TimeStamp Utils::GetCurrentLocalTime()
{
    TimeStamp ts {};
    chrono::system_clock::time_point nowTimePoint = chrono::system_clock::now(); // get current time

    // convert epoch time to struct with year, month, day, hour, minute, second fields
    time_t now = chrono::system_clock::to_time_t(nowTimePoint);
    localtime_r(&now, &ts.m_localTime);

    // get milliseconds field
    const chrono::duration<double> tse = nowTimePoint.time_since_epoch();
    ts.m_milliseconds = chrono::duration_cast<std::chrono::milliseconds>(tse).count() % 1000;
    return ts;
}

// *************************************************************************************************
string Utils::GetCurrentLocalTimeString()
{
    const TimeStamp ts = GetCurrentLocalTime();

    ostringstream currentTime;
    currentTime << (1900 + ts.m_localTime.tm_year) << '-'
                << std::setfill('0') << std::setw(2) << (ts.m_localTime.tm_mon + 1) << '-'
                << std::setfill('0') << std::setw(2) << ts.m_localTime.tm_mday << ' '
                << std::setfill('0') << std::setw(2) << ts.m_localTime.tm_hour << ':'
                << std::setfill('0') << std::setw(2) << ts.m_localTime.tm_min << ':'
                << std::setfill('0') << std::setw(2) << ts.m_localTime.tm_sec << '.'
                << std::setfill('0') << std::setw(3) << ts.m_milliseconds;

    return currentTime.str();
}
// *************************************************************************************************

string Utils::GetCurrentDotNetDateTimeString()
{
    const TimeStamp ts = GetCurrentLocalTime();
    time_t now = time(0); // UTC
    struct tm *ptmgm = gmtime(&now); // further convert to GMT presuming now in local
    if (!ptmgm)
    {   // shouldn't happen
        return "";
    }
    const time_t gmnow = mktime(ptmgm);
    time_t diff = gmnow - now;
    if (ptmgm->tm_isdst > 0)
    {
        diff = diff - 60 * 60;
    }
    diff /= 3600;
    const std::string timezoneSeperator = diff >= 0 ? "+" : "-";
    ostringstream timezoneStream;
    const auto absDiff = std::labs(static_cast<long>(diff));
    if(absDiff < 10)
    {
        timezoneStream << "0" << absDiff << ":00";
    }
    else
    {
        timezoneStream << absDiff << ":00";
    }

    ostringstream currentTime;
    currentTime << (1900 + ts.m_localTime.tm_year) << '-'
                << std::setfill('0') << std::setw(2) << (ts.m_localTime.tm_mon + 1) << '-'
                << std::setfill('0') << std::setw(2) << ts.m_localTime.tm_mday << 'T'
                << std::setfill('0') << std::setw(2) << ts.m_localTime.tm_hour << ':'
                << std::setfill('0') << std::setw(2) << ts.m_localTime.tm_min << ':'
                << std::setfill('0') << std::setw(2) << ts.m_localTime.tm_sec << '.'
                << std::setfill('0') << std::setw(3) << ts.m_milliseconds << timezoneSeperator
                << timezoneStream.str();

    return currentTime.str();
}

// *************************************************************************************************
string Utils::GetCurrentLocalTimeForFileName()
{
    const TimeStamp ts = GetCurrentLocalTime();

    ostringstream currentTime;
    currentTime << (1900 + ts.m_localTime.tm_year)
        << std::setfill('0') << std::setw(2) << (ts.m_localTime.tm_mon + 1)
        << std::setfill('0') << std::setw(2) << ts.m_localTime.tm_mday << '-'
        << std::setfill('0') << std::setw(2) << ts.m_localTime.tm_hour
        << std::setfill('0') << std::setw(2) << ts.m_localTime.tm_min
        << std::setfill('0') << std::setw(2) << ts.m_localTime.tm_sec << '-'
        << std::setfill('0') << std::setw(3) << ts.m_milliseconds;

    return currentTime.str();
}

// *************************************************************************************************
bool Utils::ConvertHexStringToDword(string str, uint32_t& word)
{
    if (str.find("0x") != 0) //The parameter is a hex string (assuming that a string starting with 0x must be hex)
    {
        return false;
    }

    char* end = nullptr;
    word = strtol(str.c_str(), &end, 16);
    return *end == '\0';
}

// *************************************************************************************************
bool Utils::ConvertHexStringToDwordVector(string str, char delimiter, vector<uint32_t>& values)
{
    vector<string> strValues = Utils::Split(str, delimiter);
    values.reserve(strValues.size());
    for (auto& strValue : strValues)
    {
        uint32_t word = 0;
        if (ConvertHexStringToDword(strValue, word))
        {
            values.push_back(word);
        }
        else
        {
            return false;
        }
    }

    return true;
}

// *************************************************************************************************
bool Utils::ConvertDecimalStringToUnsignedInt(string str, unsigned int& ui)
{
    unsigned long l;
    try
    {
        l = strtoul(str.c_str(), nullptr, 10); // 10 for decimal base
    }
    catch (...)
    {
        return false;
    }

    ui = l;
    return true;
}

// Encode given binary array as Base64 string
string Utils::Base64Encode(const vector<unsigned char>& binaryData)
{
    return Base64Encode(binaryData.data(), binaryData.size());
}

string Utils::Base64Encode(const unsigned char binaryData[], size_t length)
{
    static const std::string base64Vocabulary = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string result;
    if (length == 0U)
    {
        return result;
    }

    // reserve room for the result (next higher multiple of 3, div by 3, mult by 4)
    unsigned mod3 = length % 3;
    result.reserve((length + (mod3 ? 3 - mod3 : 0)) * 4 / 3);

    int val1 = 0, val2 = -6;
    for (size_t i = 0U; i < length; ++i)
    {
        val1 = (val1 << 8) + static_cast<int>(binaryData[i]);
        val2 += 8;
        while (val2 >= 0)
        {
            result.push_back(base64Vocabulary[(val1 >> val2) & 0x3F]);
            val2 -= 6;
        }
    }

    if (val2 > -6)
    {
        result.push_back(base64Vocabulary[((val1 << 8) >> (val2 + 8)) & 0x3F]);
    }

    while (result.size() % 4)
    {
        result.push_back('=');
    }

    return result;
}

// Decode the string given in Base64 format
bool Utils::Base64Decode(const string& base64Data, vector<unsigned char>& binaryData)
{
    static const unsigned char lookup[] =
    {
        62,  255, 62,  255, 63,  52,  53, 54, 55, 56, 57, 58, 59, 60, 61, 255,
        255, 0,   255, 255, 255, 255, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
        10,  11,  12,  13,  14,  15,  16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
        255, 255, 255, 255, 63,  255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
        36,  37,  38,  39,  40,  41,  42, 43, 44, 45, 46, 47, 48, 49, 50, 51
    };

    binaryData.clear();
    if (base64Data.empty())
    {
        return false;
    }
    binaryData.reserve(base64Data.size() * 3 / 4);

    int val1 = 0, val2 = -8;
    for (const auto c : base64Data)
    {
        if (c < '+' || c > 'z')
        {
            break;
        }

        int mappedVal = static_cast<int>(lookup[c - '+']);
        if (mappedVal >= 64)
        {
            break;
        }

        val1 = (val1 << 6) + mappedVal;
        val2 += 6;

        if (val2 >= 0)
        {
            binaryData.push_back(char((val1 >> val2) & 0xFF));
            val2 -= 8;
        }
    }

    return true;
}

Timer::Timer()
    : m_startPoint(std::chrono::steady_clock::now())
    , m_duration(std::chrono::duration<double, std::milli>::zero())
{
}

void Timer::Start()
{
    m_startPoint = std::chrono::steady_clock::now();
}

void Timer::Stop()
{
    m_duration += (std::chrono::steady_clock::now() - m_startPoint);
}

void Timer::Reset()
{
    m_startPoint = std::chrono::steady_clock::now();
    m_duration = std::chrono::duration<double, std::milli>::zero();
}

double Timer::GetTotalMsec() const
{
    return m_duration.count();
}

std::ostream& operator<<(std::ostream& os, const Timer& timer)
{
    std::ios::fmtflags f(os.flags());
    os << fixed << showpoint << setprecision(1) << timer.GetTotalMsec() << " (msec)";
    os.flags(f);
    return os;
}

size_t SafeStringCopy(char *pDst, const char *pSrc, size_t dstCapacity)
{
    const size_t srcLength = (pSrc) ? strlen(pSrc) : 0;
    if (!pDst)
    {
        return srcLength;    // Not sure if 0 is a more appropriate value
    }

    if (!pSrc)
    {
        *pDst = '\0';
        return srcLength;
    }

    const size_t copiedChars = std::min(srcLength, dstCapacity - 1);    // Without \0
    memcpy(pDst, pSrc, copiedChars);                                    // Characters only, may be truncated
    pDst[copiedChars] = '\0';                                           // Make sure the string is terminated
    return srcLength;
}
