////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  camxbpspipelinetitan480.h
/// @brief BPS Pipeline for Titan 480
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CAMXBPSPIPELINETITAN480_H
#define CAMXBPSPIPELINETITAN480_H

#include "camxisppipeline.h"

CAMX_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief BPS Pipeline 480 Class Implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class BPSPipelineTitan480 final: public ISPPipeline
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetModuleList
    ///
    /// @brief  Function to be implemented to get the Module list
    ///
    /// @param  pIQmoduleInfo structure into which capability info is filled
    ///
    /// @return CamxResult
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult GetModuleList(
                BPSIQModuleList* pIQmoduleInfo);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetModuleListForMode
    ///
    /// @brief  Function to be implemented to get the pipleine capability in mode requested
    ///
    /// @param  pIQmoduleInfo structure into which capability info is filled
    /// @param  mode          returns the capability in this mode
    ///
    /// @return CamxResult
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult GetModuleListForMode(
                BPSIQModuleList* pIQmoduleInfo,
                UINT32 mode);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetCapability
    ///
    /// @brief  virtual method to get the isp pipeline capability
    ///
    /// @param  pCapabilityInfo Pointer to the structure into which capability info is filled
    ///
    /// @return CamxResult
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult GetCapability(
        VOID* pCapabilityInfo);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// BPSPipelineTitan480
    ///
    /// @brief  Destructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ~BPSPipelineTitan480() = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// BPSPipelineTitan480
    ///
    /// @brief  Constructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    BPSPipelineTitan480() = default;

private:
    BPSPipelineTitan480(const BPSPipelineTitan480&)            = delete; ///< Disallow the copy constructor
    BPSPipelineTitan480& operator=(const BPSPipelineTitan480&) = delete; ///< Disallow assignment operator
};

CAMX_NAMESPACE_END

#endif // CAMXBPSPIPELINETITAN480_H