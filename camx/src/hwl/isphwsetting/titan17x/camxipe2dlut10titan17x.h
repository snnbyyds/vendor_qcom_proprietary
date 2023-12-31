////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  camxipe2dlut10titan17x.h
/// @brief IPE 2DLUT10 register setting for Titan17x
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CAMXIPE2DLUT10TITAN17X_H
#define CAMXIPE2DLUT10TITAN17X_H

#include "camxtitan17xdefs.h"
#include "camxisphwsetting.h"
#include "camxcmdbuffer.h"
#include "camxispiqmodule.h"

CAMX_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief IPE 2DLUT 10 register setting for Titan17x
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class IPE2DLUT10Titan17x final : public ISPHWSetting
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// IPE2DLUT10Titan17x
    ///
    /// @brief  Constructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    IPE2DLUT10Titan17x();

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
    /// WriteLUTtoDMI
    ///
    /// @brief  writes all 14 LUTs from 2DLUT into cmd buffer
    ///
    /// @param  pInputData       Pointer to the Inputdata
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult WriteLUTtoDMI(
        ISPInputData* pInputData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// PackIQRegisterSetting
    ///
    /// @brief  Calculate register settings based on common library result
    ///
    /// @param  pInput       Pointer to the Input data to the IQ module for calculation
    /// @param  pOutput      Pointer to the Output data to the IQ module for DMI calculation
    ///
    /// @return Return CamxResult.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult PackIQRegisterSetting(
        VOID*  pInput,
        VOID*  pOutput);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// SetupRegisterSetting
    ///
    /// @brief  Setup register value based on CamX Input
    ///
    /// @param  pInput       Pointer to the Input data to the Demosaic37 module for calculation
    ///
    /// @return Return CamxResult.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult SetupRegisterSetting(
        VOID*  pInput);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DumpRegConfig
    ///
    /// @brief  Print register configuration of Crop module for debug
    ///
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual VOID DumpRegConfig();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetRegSize
    ///
    /// @brief  Returns register size
    ///
    ///
    /// @return Number of bytes of register structure.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual UINT GetRegSize();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ~IPE2DLUT10Titan17x
    ///
    /// @brief  Destructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~IPE2DLUT10Titan17x();

private:
    IPE2DLUT10Titan17x(const IPE2DLUT10Titan17x&)            = delete; ///< Disallow the copy constructor
    IPE2DLUT10Titan17x& operator=(const IPE2DLUT10Titan17x&) = delete; ///< Disallow assignment operator

    VOID*      m_pRegCmd;                                              ///< Register List of this Module
    CmdBuffer* m_pLUTDMICmdBuffer;                                     ///< Shadow copy of Command buffer for holding all 3 LUTs
};

CAMX_NAMESPACE_END

#endif // CAMXIPE2DLUT10TITAN17X_H
