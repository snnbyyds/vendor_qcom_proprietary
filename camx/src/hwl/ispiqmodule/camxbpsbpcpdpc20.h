////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  camxbpsbpcpdpc20.h
/// @brief bpsbpcpdpc20 class declarations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CAMXBPSBPCPDPC20_H
#define CAMXBPSBPCPDPC20_H

#include "camxispiqmodule.h"
#include "iqcommondefs.h"
#include "pdpc_2_0_0.h"

CAMX_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief BPS BPC PDPC 20 Class Implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class BPSBPCPDPC20 final : public ISPIQModule
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create
    ///
    /// @brief  Create BPSBPCPDPC20 Object
    ///
    /// @param  pCreateData Pointer to the BPSBPCPDPC20 Creation
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static CamxResult Create(
        BPSModuleCreateData* pCreateData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FillCmdBufferManagerParams
    ///
    /// @brief  Fills the command buffer manager params needed by IQ Module
    ///
    /// @param  pInputData Pointer to the IQ Module Input data structure
    /// @param  pParam     Pointer to the structure containing the command buffer manager param to be filled by IQ Module
    ///
    /// @return CamxResultSuccess if successful
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult FillCmdBufferManagerParams(
       const ISPInputData*     pInputData,
       IQModuleCmdBufferParam* pParam);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Initialize
    ///
    /// @brief  Initialize parameters
    ///
    /// @param  pCreateData Pointer to the BPSBPCPDPC20 Creation
    ///
    /// @return CamxResultSuccess if successful
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult Initialize(
        BPSModuleCreateData* pCreateData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Execute
    ///
    /// @brief  Execute process capture request to configure module
    ///
    /// @param  pInputData Pointer to the ISP input data
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult Execute(
        ISPInputData* pInputData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ~BPSBPCPDPC20
    ///
    /// @brief  Destructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~BPSBPCPDPC20();

protected:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// BPSBPCPDPC20
    ///
    /// @brief  Constructor
    ///
    /// @param  pNodeIdentifier     String identifier for the Node that creating this IQ Module object
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    explicit BPSBPCPDPC20(
       const CHAR* pNodeIdentifier);

private:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// AllocateCommonLibraryData
    ///
    /// @brief  Allocate memory required for common library
    ///
    /// @return CamxResult success if the write operation is success
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult AllocateCommonLibraryData();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DeallocateCommonLibraryData
    ///
    /// @brief  Deallocate memory required for common library
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID DeallocateCommonLibraryData();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// CheckDependenceChange
    ///
    /// @brief  Check to see if the Dependence Trigger Data Changed
    ///
    /// @param  pInputData Pointer to the ISP input data
    ///
    /// @return TRUE if dependencies met
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    BOOL CheckDependenceChange(
        ISPInputData* pInputData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// RunCalculation
    ///
    /// @brief   Calculate the Register Value
    ///
    /// @param   pInputData Pointer to the ISP input data
    ///
    /// @return  CamxResult Indicates if configure DMI and Registers was success or failure
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult RunCalculation(
        ISPInputData* pInputData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FetchDMIBuffer
    ///
    /// @brief  Fetching PDPC DMI LUT
    ///
    /// @return CamxResult Indicates if Fetching DMI was success or failure
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult FetchDMIBuffer();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// UpdateBPSInternalData
    ///
    /// @brief  Update BPS internal data
    ///
    /// @param  pInputData Pointer to the ISP input data
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID UpdateBPSInternalData(
        ISPInputData* pInputData
        ) const;

    BPSBPCPDPC20(const BPSBPCPDPC20&)            = delete;     ///< Disallow the copy constructor
    BPSBPCPDPC20& operator=(const BPSBPCPDPC20&) = delete;     ///< Disallow assignment operator

    const CHAR*                       m_pNodeIdentifier;       ///< String identifier of this Node
    PDPC20IQInput                     m_dependenceData;        ///< Dependence Data for this Module
    CmdBufferManager*                 m_pLUTCmdBufferManager;  ///< Command buffer manager for all LUTs of BPC PDPC
    CmdBuffer*                        m_pLUTDMICmdBuffer;      ///< Command buffer for holding all LUTs
    PixelFormat                       m_pixelFormat;           ///< Pixel Format from the sensor
    UINT32*                           m_pPDPCMaskLUT;          ///< Pointer to PDCP mask
    pdpc_2_0_0::chromatix_pdpc20Type* m_pChromatix;            ///< Pointer to tuning mode data
};

CAMX_NAMESPACE_END

#endif // CAMXBPSBPCPDPC20_H
