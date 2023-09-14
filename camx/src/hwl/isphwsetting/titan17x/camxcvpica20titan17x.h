////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  camxcvpica20titan17x.h
/// @brief CVP ICA20 register setting for Titan17x
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CAMXCVPICA20TITAN17X_H
#define CAMXCVPICA20TITAN17X_H

#include "camxtitan17xdefs.h"
#include "camxisphwsetting.h"
#include "camxispiqmodule.h"
#include "camxiqinterface.h"

CAMX_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief CVP ICA20 register setting for Titan17x
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CVPICA20Titan17x final : public ISPHWSetting
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// CVPICA20Titan17x
    ///
    /// @brief  Constructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CVPICA20Titan17x();


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
    /// @param  pInput       Pointer to the Input data to the ICA module for calculation
    ///
    /// @return Return CamxResult.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult SetupRegisterSetting(
        VOID*  pInput);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// CopyRegCmd
    ///
    /// @brief  Copy register settings to the input buffer
    ///
    /// @param  pData  Pointer to the Input data buffer
    ///
    /// @return Number of bytes copied
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual UINT32 CopyRegCmd(
        VOID* pData);


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
    /// ~CVPICA20Titan17x
    ///
    /// @brief  Destructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~CVPICA20Titan17x();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// WriteLUTtoDMI
    ///
    /// @brief  writes all LUTs from ICA into cmd buffer
    ///
    /// @param  pInputData Pointer to CVP module input data
    ///
    /// @return CamxResult Indicates if LUTs are successfully written into DMI cdm header
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult WriteLUTtoDMI(
        ISPInputData* pInputData);

private:
    CmdBuffer*            m_pLUTCmdBuffer;          ///< Shadow copy of module LUT command buffer
    VOID*                 m_pICAModuleData;         ///< ICA 20 module data pointer
    VOID*                 m_pCVPICAFrameCfgData;    ///< CVP ICA frame config data

    CVPICA20Titan17x(const CVPICA20Titan17x&)            = delete; ///< Disallow the copy constructor
    CVPICA20Titan17x& operator=(const CVPICA20Titan17x&) = delete; ///< Disallow assignment operator

};

CAMX_NAMESPACE_END

#endif // CAMXCVPICA20TITAN17x_H