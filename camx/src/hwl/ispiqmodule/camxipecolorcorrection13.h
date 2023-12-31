////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  camxipecolorcorrection13.h
/// @brief IPEColorCorrection class declarations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CAMXIPECOLORCORRECTION13_H
#define CAMXIPECOLORCORRECTION13_H

#include "camxformats.h"
#include "camxispiqmodule.h"
#include "iqcommondefs.h"

CAMX_NAMESPACE_BEGIN

/// @brief CCM Manual Setting Data
struct ManualSetting
{
    BOOL                          manualCCMEnable;        ///< Enable Manual Control CCM
    BOOL                          manualAWBCCMEnable;     ///< Enable AWB CCM
    FLOAT*                        pManualCCM;             ///< Manual CCM Buffer
    UINT                          arraySize;              ///< Used to convert ManualCCM from 2D Array to 1D Array
    AWBCCMParams*                 pAWBCCM;                ///< AWB CCM Buffer
    cc_1_3_0::chromatix_cc13Type* pChromatix;             ///< Chromatix pointer to handle the q factor
};

static const UINT32 DefaultSaturation = 5;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Class for IPE ColorCorrection13 Module
///
///         This IQ blocks compensates global chroma difference due to R/G/B channel crosstalk by applying
///         a 3x3 matrix color correction matrix(CCM)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class IPEColorCorrection13 final : public ISPIQModule
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create
    ///
    /// @brief  Create Color Correction Object
    ///
    /// @param  pCreateData  Pointer to resource and intialization data for ColorCorrection13 Creation
    ///
    /// @return CamxResultSuccess if successful
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static CamxResult Create(
        IPEModuleCreateData* pCreateData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Initialize
    ///
    /// @brief  Initialize parameters
    ///
    /// @param  pCreateData  Pointer to resource and intialization data for ColorCorrection13 Creation
    ///
    /// @return CamxResultSuccess if successful
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult Initialize(
        IPEModuleCreateData* pCreateData);

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
    /// ~IPEColorCorrection13
    ///
    /// @brief  Destructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~IPEColorCorrection13();

protected:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// IPEColorCorrection13
    ///
    /// @brief  Constructor
    ///
    /// @param  pNodeIdentifier     String identifier for the Node that creating this IQ Module object
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    explicit IPEColorCorrection13(
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
    /// @return CamxResult success if the write operation is success
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult DeallocateCommonLibraryData();

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
    /// @return BOOL Indicates if the settings have changed compared to last setting
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    BOOL CheckDependenceChange(
        ISPInputData* pInputData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// UpdateIPEInternalData
    ///
    /// @brief  Update Pipeline input data, such as metadata, IQSettings.
    ///
    /// @param  pInputData Pointer to the ISP input data
    ///
    /// @return CamxResult success if the write operation is success
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult UpdateIPEInternalData(
        ISPInputData* pInputData
        ) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// RunCalculation
    ///
    /// @brief  Calculate the Register Value
    ///
    /// @param  pInputData Pointer to the ISP input data
    ///
    /// @return CamxResult Indicates if configuration calculation was success or failure
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult RunCalculation(
        const ISPInputData* pInputData);

    IPEColorCorrection13(const IPEColorCorrection13&)            = delete;  ///< Disallow the copy constructor
    IPEColorCorrection13& operator=(const IPEColorCorrection13&) = delete;  ///< Disallow assignment operator

    const CHAR*                   m_pNodeIdentifier;        ///< String identifier for the Node that created this object
    CC13InputData                 m_dependenceData;         ///< Dependence Data for this Module
    UINT                          m_numValidCCMs;           ///< Number of CCMs from 3A
    AWBCCMParams                  m_AWBCCM[MaxCCMs];        ///< CCM overide config from 3A
    FLOAT                         m_manualCCM[3][3];        ///< CCM matrix from app
    BOOL                          m_manualCCMOverride;      ///< Manual Control
    UINT8                         m_colorCorrectionMode;    ///< CC mode
    cc_1_3_0::chromatix_cc13Type* m_pChromatix;             ///< Pointer to tuning mode data
    BOOL                          m_validCCM;               ///< regsiter have valid CCM
};

CAMX_NAMESPACE_END

#endif // CAMXIPEColorCorrection_H
