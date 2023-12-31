// NOWHINE NC009 <- Shared file with system team so uses non-CamX file naming
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  lenr10interpolation.h
/// @brief LENR10 module setting calculation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef LENR10INTERPOLATION_H
#define LENR10INTERPOLATION_H

#include "lenr_1_0_0.h"
#include "iqcommondefs.h"
#include "iqsettingutil.h"

/// @brief Trigger information from 3A
// NOWHINE NC004c: Share code with system team
struct LENR10TriggerList
{
    lenr_1_0_0::chromatix_lenr10Type::control_methodStruct  controlType;            ///< chromatix data
    FLOAT                                                   triggerDRCgain;         ///< DRC Gain
    FLOAT                                                   triggerHDRAEC;          ///< HDRAEC trigger
    FLOAT                                                   triggerAEC;             ///< AEC trigger
    FLOAT                                                   triggerTotalScaleRatio; ///< triggerTotalScaleRatio trigger
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class that implements LENR10 tuning data interpolation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NOWHINE NC004c: Share code with system team
class LENR10Interpolation
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// CheckUpdateTrigger
    ///
    /// @brief  Check and update the trigger condition
    ///
    /// @param  pInput  Pointer to the Input data of the latest trigger data
    /// @param  pOutput Pointer to the dependence data of this IQ module
    ///
    /// @return TRUE for success
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static BOOL CheckUpdateTrigger(
        ISPIQTriggerData* pInput,
        LENR10InputData*   pOutput);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// RunInterpolation
    ///
    /// @brief  Perform the interpolation.
    ///
    /// @param  pInput Input data to the LENR10 Module
    /// @param  pData  The output of the interpolation algorithem
    ///
    /// @return TRUE for success
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static BOOL RunInterpolation(
        const LENR10InputData*           pInput,
        lenr_1_0_0::lenr10_rgn_dataType*  pData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DoInterpolation
    ///
    /// @brief  LENR10 Interpolation function
    ///
    /// @param  pData1   Pointer to the input data set 1
    /// @param  pData2   Pointer to the input data set 2
    /// @param  ratio    Interpolation Ratio
    /// @param  pOutput  Pointer to the result set
    ///
    /// @return Return TRUE on success
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static BOOL DoInterpolation(
        VOID* pData1,
        VOID* pData2,
        FLOAT ratio,
        VOID* pOutput);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// TotalScaleRatioSearchNode
    ///
    /// @brief  Search Function for TotalScale Nodes
    ///
    /// @param  pParentNode     Pointer to the Parent Node
    /// @param  pTriggerData    Pointer to the Trigger Value List
    /// @param  pChildNode      Pointer to the Child Node
    ///
    /// @return TRUE for success
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static UINT TotalScaleRatioSearchNode(
        TuningNode* pParentNode,
        VOID*       pTriggerData,
        TuningNode* pChildNode);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DRCGainSearchNode
    ///
    /// @brief  Search Function for DRCGain Nodes
    ///
    /// @param  pParentNode     Pointer to the Parent Node
    /// @param  pTriggerData    Pointer to the Trigger Value List
    /// @param  pChildNode      Pointer to the Child Node
    ///
    /// @return TRUE for success
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static UINT DRCGainSearchNode(
        TuningNode* pParentNode,
        VOID*       pTriggerData,
        TuningNode* pChildNode);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// HDRAECSearchNode
    ///
    /// @brief  Search Function for HDRAEC Nodes
    ///
    /// @param  pParentNode     Pointer to the Parent Node
    /// @param  pTriggerData    Pointer to the Trigger Value List
    /// @param  pChildNode      Pointer to the Child Node
    ///
    /// @return TRUE for success
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static UINT HDRAECSearchNode(
        TuningNode* pParentNode,
        VOID*       pTriggerData,
        TuningNode* pChildNode);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// AECSearchNode
    ///
    /// @brief  Search Function for AEC Nodes
    ///
    /// @param  pParentNode     Pointer to the Parent Node
    /// @param  pTriggerData    Pointer to the Trigger Value List
    /// @param  pChildNode      Pointer to the Child Node
    ///
    /// @return TRUE for success
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static UINT AECSearchNode(
        TuningNode* pParentNode,
        VOID*       pTriggerData,
        TuningNode* pChildNode);

private:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// InterpolationData
    ///
    /// @brief  Interpolation Function for LENR10 Module
    ///
    /// @param  pInput1   Input Region 1
    /// @param  pInput2   Input Region 2
    /// @param  ratio     Ratio for interpolation
    /// @param  pOutput   Output after interpolation
    ///
    /// @return return TRUE if succeed
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static BOOL InterpolationData(
        lenr_1_0_0::lenr10_rgn_dataType* pInput1,
        lenr_1_0_0::lenr10_rgn_dataType* pInput2,
        FLOAT                          ratio,
        lenr_1_0_0::lenr10_rgn_dataType* pOutput);
};
#endif // LENR10INTERPOLATION_H
