////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  chifeature2qcfa.h
/// @brief CHI feature QCFA derived class declarations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CHIFEATURE2QCFA_H
#define CHIFEATURE2QCFA_H


#include "chifeature2base.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Feature derived class
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CDK_VISIBILITY_PUBLIC ChiFeature2QCFA : public ChiFeature2Base
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create
    ///
    /// @brief  Static function to create feature instance
    ///
    /// @param  pCreateInputInfo   Pointer to create input info for feature
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static ChiFeature2QCFA* Create(
        ChiFeature2CreateInputInfo* pCreateInputInfo);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Destroy
    ///
    /// @brief  Destroy feature
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual VOID Destroy();

protected:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DoQueryCaps
    ///
    /// @brief  Query the capabilities of the derived feature
    ///
    /// @param  pQueryInfo Feature query information.
    ///
    /// @return CDKResultSuccess if successful or CDK error values in case of error.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CDKResult DoQueryCaps(
        ChiFeature2QueryInfo* pQueryInfo);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DoInitialize
    ///
    /// @brief  Initialize feature
    ///
    /// @param  pCreateInputInfo  Feature create input information.
    ///
    /// @return CDKResultSuccess if successful or CDK error values in case of error.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CDKResult DoInitialize(
        ChiFeature2CreateInputInfo* pCreateInputInfo);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DoPrepareRequest
    ///
    /// @brief  Method to prepare processing request.
    ///         The feature is expected to set the request object's context once during this function.
    ///
    /// @param  pRequestObject  Feature request object instance.
    ///
    /// @return CDKResultSuccess if successful or CDK error values in case of error.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CDKResult DoPrepareRequest(
        ChiFeature2RequestObject* pRequestObject) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// OnExecuteProcessRequest
    ///
    /// @brief  Method to execute process request.
    ///
    /// @param  pRequestObject  Feature request object instance.
    ///
    /// @return None.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CDKResult OnExecuteProcessRequest(
        ChiFeature2RequestObject* pRequestObject) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DoProcessResult
    ///
    /// @brief  Method to process result callback from CHI driver.
    ///
    /// @param  pResult                 Result from CHI driver.
    /// @param  pPrivateCallbackData    Private callback data.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual VOID DoProcessResult(
        CHICAPTURERESULT*   pResult,
        VOID*               pPrivateCallbackData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DoProcessMessage
    ///
    /// @brief  Method to process message callback from CHI driver.
    ///
    /// @param  pMessageDescriptor      Message from CHI driver.
    /// @param  pPrivateCallbackData    Private callback data.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual VOID DoProcessMessage(
        const CHIMESSAGEDESCRIPTOR* pMessageDescriptor,
        VOID*                       pPrivateCallbackData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DoProcessPartialResult
    ///
    /// @brief  Method to process partial capture result callback from CHI driver.
    ///
    /// @param  pCaptureResult          Partial capture result from CHI driver.
    /// @param  pPrivateCallbackData    Private callback data.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual VOID DoProcessPartialResult(
        CHIPARTIALCAPTURERESULT*   pCaptureResult,
        VOID*                      pPrivateCallbackData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DoCleanupRequest
    ///
    /// @brief  Method to cleanup processing request.
    ///
    /// @param  pRequestObject  Feature request object instance.
    ///
    /// @return CDKResultSuccess if successful or CDK error values in case of error.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CDKResult DoCleanupRequest(
        ChiFeature2RequestObject* pRequestObject) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DoFlush
    ///
    /// @brief  Method to flush.
    ///
    /// @return CDKResultSuccess if successful or CDK error values in case of error.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CDKResult DoFlush();

private:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ChiFeature2QCFA
    ///
    /// @brief  Default constructor.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ChiFeature2QCFA() = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ~ChiFeature2QCFA
    ///
    /// @brief  Virtual destructor.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~ChiFeature2QCFA();

    ChiFeature2QCFA(const ChiFeature2QCFA&)             = delete;   ///< Disallow the copy constructor
    ChiFeature2QCFA& operator= (const ChiFeature2QCFA&) = delete;   ///< Disallow assignment operator
};

#endif // CHIFEATURE2QCFA_H