////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  camxbpsdemosaic36titan480.h
/// @brief BPS Demosaic36 register setting for Titan17x
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CAMXBPSDEMOSAIC36TITAN480_H
#define CAMXBPSDEMOSAIC36TITAN480_H

#include "camxisphwsetting.h"

CAMX_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief BPS Demosaic36 register setting for Titan17x
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class BPSDemosaic36Titan480 final : public ISPHWSetting
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create
    ///
    /// @brief  Create BPSDemosaic36Titan480 Object
    ///
    /// @param  ppHWSetting  Pointer to the pointer to the BPSDemosaic36Titan480 Object
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
    /// UpdateFirmwareData
    ///
    /// @brief  Update Firmware Data
    ///
    /// @param  pInputData       Pointer to the InputData
    /// @param  moduleEnable     Boolean TRUE if module is enabled
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult UpdateFirmwareData(
        VOID*  pInputData,
        BOOL   moduleEnable);

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
    /// @param  pInput       Pointer to the Input data to the Demosaic36 module for calculation
    /// @param  pOutput      Pointer to the Output data for DMI
    ///
    /// @return Return CamxResult.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult PackIQRegisterSetting(
        VOID*  pInput,
        VOID*  pOutput);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ~BPSDemosaic36Titan480
    ///
    /// @brief  Destructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~BPSDemosaic36Titan480();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// BPSDemosaic36Titan480
    ///
    /// @brief  Constructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    BPSDemosaic36Titan480();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DumpRegConfig
    ///
    /// @brief  Print register configuration of Crop module for debug
    ///
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

    VOID*  m_pRegCmd;                                                        ///< Register List of this Module
    VOID*  m_pModuleConfig;                                                  ///< Module configuration registers

    BPSDemosaic36Titan480(const BPSDemosaic36Titan480&)            = delete; ///< Disallow the copy constructor
    BPSDemosaic36Titan480& operator=(const BPSDemosaic36Titan480&) = delete; ///< Disallow assignment operator
};

CAMX_NAMESPACE_END

#endif // CAMXBPSDEMOSAIC36TITAN480_H
