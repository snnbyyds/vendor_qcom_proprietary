////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  camxbpsbpcpdpc20titan17x.h
/// @brief BPS PDPC register setting for Titan17x
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CAMXBPSBPCPDPC20TITAN17X_H
#define CAMXBPSBPCPDPC20TITAN17X_H

#include "camxisphwsetting.h"

CAMX_NAMESPACE_BEGIN

static const UINT32 PDPCMaxLUT = 1;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief BPS BPCPDPC20 register setting for Titan17x
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class BPSBPCPDPC20Titan17x final : public ISPHWSetting
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create
    ///
    /// @brief  Create BPSBPCPDPC20Titan17x Object
    ///
    /// @param  ppHWSetting  Pointer to the pointer to the BPSBPCPDPC20Titan17x Object
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
    /// @param  pInput        Pointer to the Input data to the BPCPDPC20 module for calculation
    /// @param  pOutput       Pointer to the Output data for DMI
    ///
    /// @return Return CamxResult.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult PackIQRegisterSetting(
        VOID*  pInput,
        VOID*  pOutput);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ~BPSBPCPDPC20Titan17x
    ///
    /// @brief  Destructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~BPSBPCPDPC20Titan17x();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// BPSBPCPDPC20Titan17x
    ///
    /// @brief  Constructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    BPSBPCPDPC20Titan17x();

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

    VOID*  m_pRegCmd;                                                         ///< Register list if this Module
    VOID*  m_pModuleConfig;                                                   ///< Module configuration registers

    BPSBPCPDPC20Titan17x(const BPSBPCPDPC20Titan17x&) = delete;               ///< Disallow the copy constructor
    BPSBPCPDPC20Titan17x& operator=(const BPSBPCPDPC20Titan17x&) = delete;    ///< Disallow assignment operator
};

CAMX_NAMESPACE_END

#endif // CAMXBPSBPCPDPC20TITAN17X_H
