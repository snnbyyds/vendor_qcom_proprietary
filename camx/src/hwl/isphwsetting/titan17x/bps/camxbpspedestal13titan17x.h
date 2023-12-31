////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  camxbpspedestal13titan17x.h
/// @brief BPS Pedestal13 register setting for Titan17x
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CAMXBPSPEDESTAL13TITAN17X_H
#define CAMXBPSPEDESTAL13TITAN17X_H

#include "camxisphwsetting.h"

CAMX_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief BPS Pedestal13 register setting for Titan17x
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class BPSPedestal13Titan17x final : public ISPHWSetting
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create
    ///
    /// @brief  Create BPSPedestal13Titan17x Object
    ///
    /// @param  ppHWSetting  Pointer to the poitner to the BPSPedestal13Titan17x Object
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static CamxResult Create(
        ISPHWSetting** ppHWSetting);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// WriteLUTtoDMI
    ///
    /// @brief  Write Look up table data pointer into DMI header
    ///
    /// @param  pInputData       Pointer to the ISP input data
    ///
    /// @return CamxResult Indicates if write to DMI header was success or failure
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult WriteLUTtoDMI(
        VOID*   pInputData);

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
    /// @param  pInput       Pointer to the Input data to the Pedestal13 module for calculation
    /// @param  pOutput      Pointer to the Output data for DMI
    ///
    /// @return Return CamxResult.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult PackIQRegisterSetting(
        VOID*  pInput,
        VOID*  pOutput);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ~BPSPedestal13Titan17x
    ///
    /// @brief  Destructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~BPSPedestal13Titan17x();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// BPSPedestal13Titan17x
    ///
    /// @brief  Constructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    BPSPedestal13Titan17x();

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

    BPSPedestal13Titan17x(const BPSPedestal13Titan17x&)            = delete; ///< Disallow the copy constructor
    BPSPedestal13Titan17x& operator=(const BPSPedestal13Titan17x&) = delete; ///< Disallow assignment operator
};

CAMX_NAMESPACE_END

#endif // CAMXBPSPEDESTAL13TITAN17X_H
