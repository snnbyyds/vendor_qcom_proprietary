////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  camxncssscutils.h
/// @brief CamX Sensor QSEE interface utility definition
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CAMXNCSSSCUTILS_H
#define CAMXNCSSSCUTILS_H

// NOWHINE FILE CP006:  needed by STL library
// NOWHINE FILE CP008:  operator overload
// NOWHINE FILE CP011:  needed by QMI library
// NOWHINE FILE CP018:  needed by QMI library
// NOWHINE FILE CP019:  needed by QMI library
// NOWHINE FILE GR017:  needed by QMI library

#include <vector>

#include "camxdefs.h"
#include "camxncssscconnection.h"
#include "sns_client.pb.h"
#include "sns_suid.pb.h"
#include "sns_client_api_v01.h"


CAMX_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class represents sensor's unique ID (128-bit)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SensorUid
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// SensorUid
    ///
    /// @brief  SensorUid constructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    SensorUid();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// SensorUid
    ///
    /// @brief  suid lookup constructor
    ///
    /// @param  low    low 64 bits of suid
    /// @param  high   high 64 bits of suid
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    SensorUid(
        uint64_t low,
        uint64_t high);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// operator==
    ///
    /// @brief  comparision operator
    ///
    /// @param  rRHS    right hand variable
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool operator==(
        const SensorUid& rRHS) const
    {
        return (m_low == rRHS.m_low && m_high == rRHS.m_high);
    }

    SensorUid& operator= (const SensorUid& rSUID) = default;

    uint64_t m_low;                   ///< lower 64 bit suid
    uint64_t m_high;                  ///< high 64 bit suid

private:
    // Do not implement the copy constructor
    SensorUid(const SensorUid& rSUID)             = delete;
};

using suid_event_function = std::function<void(const std::string& rDatatype, const std::vector<SensorUid>& rSUIDs)>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class that implements for suid lookup
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class SuidLookup
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// SuidLookup
    ///
    /// @brief  suid lookup constructor
    ///
    /// @param  cb   client callback function
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    SuidLookup(
        suid_event_function cb);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// RequestSuid
    ///
    /// @brief  lookup the suid for a given datatype
    ///
    /// @param  datatype      data type to lookup
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void RequestSuid(
        std::string datatype);

private:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// HandleSSCEvent
    ///
    /// @brief  internal funciton for handling SSC callback event
    ///
    /// @param  pData      data buffer
    /// @param  size       data buffer size
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void HandleSSCEvent(
        const uint8_t*  pData,
        size_t          size);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// getSSCEventCb
    ///
    /// @brief  get the ssc event handle function
    ///
    /// @return event handle funciton
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ssc_event_cb getSSCEventCb()
    {
        return [this](const uint8_t* pData, size_t size)
        {
            HandleSSCEvent(pData, size);
        };
    }

    // Do not implement the copy constructor or assignment operator
    SuidLookup(const SuidLookup& rLookup)                  = delete;
    SuidLookup& operator= (const SuidLookup& rLookup) = delete;

    suid_event_function  m_callback;       ///< callback funciton to the client
    SSCConnection        m_sscConnection;  ///< ssc connection object

    UINT8                m_requestBuffer[SNS_CLIENT_REQ_LEN_MAX_V01]; ///< internal buffer for request message
};


CAMX_NAMESPACE_END
#endif // CAMXNCSSSCUTILS_H