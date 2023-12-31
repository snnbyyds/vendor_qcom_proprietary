////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  camxhistalgorithmhandler.h
/// @brief This class handle creation  of HDR algorithm
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CAMXHISTALGORITHMHANDLER_H
#define CAMXHISTALGORITHMHANDLER_H

#include "chihistalgointerface.h"

#include "camxstatscommon.h"

CAMX_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief This class handles Hist algo creation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class HistAlgorithmHandler
{
public:

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// CreateHistAlgorithm
    ///
    /// @brief  This method creates an instance to the HDR algorithm.
    ///
    /// @param  pCreateParams    Pointer to create parameter list
    /// @param  ppHistAlgorithm  Pointer to the created HistAlgorithm instance
    /// @param  pfnCreate        Pointer to Algorithm entry function
    ///
    /// @return CamxResultSuccess if successful
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult CreateHistAlgorithm(
        const HistAlgoProcessCreateParamList*  pCreateParams,
        ChiHistAlgoProcess**                   ppHistAlgorithm,
        CREATEHISTOGRAMALGOPROCESS             pfnCreate);


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// HistAlgorithmHandler
    ///
    /// @brief  default constructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HistAlgorithmHandler();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ~HistAlgorithmHandler
    ///
    /// @brief  destructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~HistAlgorithmHandler();

private:

    HistAlgorithmHandler(const HistAlgorithmHandler&) = delete;               ///< Do not implement copy constructor
    HistAlgorithmHandler& operator=(const HistAlgorithmHandler&) = delete;    ///< Do not implement assignment operator

    OSLIBRARYHANDLE    m_hHandle;                                             ///< Handle to the loaded library
};

CAMX_NAMESPACE_END

#endif // CAMXHISTALGORITHMHANDLER_H