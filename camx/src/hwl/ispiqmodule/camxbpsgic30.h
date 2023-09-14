////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  camxbpsgic30.h
/// @brief bpsgic30 class declarations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CAMXBPSGIC30_H
#define CAMXBPSGIC30_H

#include "camxispiqmodule.h"
#include "gic_3_0_0.h"
#include "iqcommondefs.h"

CAMX_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief BPS GIC 34 Class Implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class BPSGIC30 final : public ISPIQModule
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create
    ///
    /// @brief  Create BPSGIC30 Object
    ///
    /// @param  pCreateData Pointer to the BPSGIC30 Creation
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static CamxResult Create(
        BPSModuleCreateData* pCreateData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Initialize
    ///
    /// @brief  Initialize parameters
    ///
    /// @param  pCreateData Pointer to the BPSGIC30 Creation
    ///
    /// @return CamxResultSuccess if successful
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult Initialize(
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
    /// ~BPSGIC30
    ///
    /// @brief  Destructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~BPSGIC30();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetRegCmd
    ///
    /// @brief  Retrieve the buffer of the register value
    ///
    /// @return Pointer of the register buffer
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual VOID* GetRegCmd();

protected:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// BPSGIC30
    ///
    /// @brief  Constructor
    ///
    /// @param  pNodeIdentifier     String identifier for the Node that creating this IQ Module object
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    explicit BPSGIC30(
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
    /// ValidateDependenceParams
    ///
    /// @brief  Validate the input configuration from app
    ///
    /// @param  pInputData Pointer to the ISP input data
    ///
    /// @return CamxResult Indicates if the input is valid or invalid
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult ValidateDependenceParams(
        const ISPInputData* pInputData);

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
    /// @param   pInputData Pointer to the GIC module input data
    ///
    /// @return  CamxResult
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult RunCalculation(
        ISPInputData* pInputData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FetchDMIBuffer
    ///
    /// @brief  Configure GIC DMI LUT
    ///
    /// @return CamxResult Indicates if fetch DMI was success or failure
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

    BPSGIC30(const BPSGIC30&)            = delete;           ///< Disallow the copy constructor
    BPSGIC30& operator=(const BPSGIC30&) = delete;           ///< Disallow assignment operator

    const CHAR*                     m_pNodeIdentifier;       ///< String identifier of this Node
    GIC30InputData                  m_dependenceData;        ///< Dependence/Input Data for this Module
    CmdBufferManager*               m_pLUTCmdBufferManager;  ///< Command buffer manager for all LUTs of GIC
    CmdBuffer*                      m_pLUTDMICmdBuffer;      ///< Command buffer for holding all LUTs
    UINT32*                         m_pGICNoiseLUT;          ///< Pointer for GIC LUT used for tuning data
    gic_3_0_0::chromatix_gic30Type* m_pChromatix;            ///< Pointer to tuning mode data
};

CAMX_NAMESPACE_END

#endif // CAMXBPSGIC30_H