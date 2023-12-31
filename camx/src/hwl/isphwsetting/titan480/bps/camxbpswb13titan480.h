////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  camxbpswb13titan480.h
/// @brief BPS WB13 register setting for Titan480
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CAMXBPSWB13TITAN480_H
#define CAMXBPSWB13TITAN480_H

#include "camxisphwsetting.h"

CAMX_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief BPS WB13 register setting for Titan480
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class BPSWB13Titan480 final : public ISPHWSetting
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create
    ///
    /// @brief  Create BPSWB13Titan480 Object
    ///
    /// @param  ppHWSetting  Pointer to the pointer to the BPSWB13Titan480 Object
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static CamxResult Create(
        ISPHWSetting** ppHWSetting);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// CreateCmdList
    ///
    /// @brief  Generate the Command List
    ///
    /// @param  pInputData       Pointer to the Inputdata
    /// @param  pDMIBufferOffset Pointer for DMI Buffer
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult CreateCmdList(
        VOID*   pInputData,
        UINT32* pDMIBufferOffset);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// UpdateTuningMetadata
    ///
    /// @brief  Update Tuning Metadata
    ///
    /// @param  pInputData      Pointer to the InputData
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult UpdateTuningMetadata(
        VOID*  pInputData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// PackIQRegisterSetting
    ///
    /// @brief  Calculate register settings based on common library result
    ///
    /// @param  pInput       Pointer to the Input data to the WB13 module for calculation
    /// @param  pOutput      Pointer to the Output data for DMI
    ///
    /// @return Return CamxResult.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult PackIQRegisterSetting(
        VOID*  pInput,
        VOID*  pOutput);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ~BPSWB13Titan480
    ///
    /// @brief  Destructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~BPSWB13Titan480();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// BPSWB13Titan480
    ///
    /// @brief  Constructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    BPSWB13Titan480();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DumpRegConfig
    ///
    /// @brief  Print register configuration of Crop module for debug
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID DumpRegConfig();

private:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Initialize
    ///
    /// @brief  Initialize parameters
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult Initialize();

    VOID*  m_pRegCmd;                                              ///< Register List of this Module

    BPSWB13Titan480(const BPSWB13Titan480&) = delete;              ///< Disallow the copy constructor
    BPSWB13Titan480& operator=(const BPSWB13Titan480&) = delete;   ///< Disallow assignment operator
};

CAMX_NAMESPACE_END

#endif // CAMXBPSWB13TITAN480_H
