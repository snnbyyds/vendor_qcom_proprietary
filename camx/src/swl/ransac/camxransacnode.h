////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  camxransacnode.h
/// @brief RANSACNode class declarations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CAMXRANSACNODE_H
#define CAMXRANSACNODE_H

#include "camxmem.h"
#include "camxnode.h"
#include "TransformEstimation.h"

CAMX_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class that implements the RANSAC node class
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class RANSACNode final : public Node
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create
    ///
    /// @brief     Static method to create RANSACNode Object.
    ///
    /// @param     pCreateInputData  Node create input data
    /// @param     pCreateOutputData Node create output data
    ///
    /// @return    Pointer to the concrete RANSACNode object
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static RANSACNode* Create(
        const NodeCreateInputData* pCreateInputData,
        NodeCreateOutputData*      pCreateOutputData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Destroy
    ///
    /// @brief     This method destroys the derived instance of the interface
    ///
    /// @return    None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual VOID Destroy();

protected:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// RANSACNode
    ///
    /// @brief     Constructor to initialize RANSAC node instance constants
    ///
    /// @return    None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    RANSACNode();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ~RANSACNode
    ///
    /// @brief     Destructor
    ///
    /// @return    None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~RANSACNode();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ProcessingNodeInitialize
    ///
    /// @brief     Initialize the hwl object
    ///
    /// @param     pCreateInputData     Node create input data
    /// @param     pCreateOutputData    Node create output data
    ///
    /// @return    CamxResultSuccess if successful
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult ProcessingNodeInitialize(
        const NodeCreateInputData* pCreateInputData,
        NodeCreateOutputData*      pCreateOutputData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ExecuteProcessRequest
    ///
    /// @brief  Pure virtual method to trigger process request for the hwl node object.
    ///
    /// @param  pExecuteProcessRequestData Process request data
    ///
    /// @return CamxResultSuccess if successful and 0 dependencies, dependency information otherwise
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult ExecuteProcessRequest(
        ExecuteProcessRequestData* pExecuteProcessRequestData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ProcessingNodeFinalizeInputRequirement
    ///
    /// @brief  Virtual method implemented by RANSAC node to determine its input buffer requirements based on all the output
    ///         buffer requirements
    ///
    /// @param  pBufferNegotiationData  Negotiation data for all output ports of a node
    ///
    /// @return Success if the negotiation was successful, Failure otherwise
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult ProcessingNodeFinalizeInputRequirement(
        BufferNegotiationData* pBufferNegotiationData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// QueryMetadataPublishList
    ///
    /// @brief  Method to query the publish list from the node
    ///
    /// @param  pPublistTagList List of tags published by the node
    ///
    /// @return CamxResultSuccess if successful
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult QueryMetadataPublishList(
        NodeMetadataList* pPublistTagList);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// IsNodeDisabledWithOverride
    ///
    /// @brief  virtual method that will be called during NewActiveStreamsSetup. Nodes may use
    ///         this hook to disable processing if theyre disabled through settings.
    ///
    /// @return CamxResultSuccess if successful
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual BOOL IsNodeDisabledWithOverride();

private:
    RANSACNode(const RANSACNode&) = delete;                 ///< Disallow the copy constructor.
    RANSACNode& operator=(const RANSACNode&) = delete;      ///< Disallow assignment operator.

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Cleanup
    ///
    /// @brief     This method cleans up any resources allocated by node
    ///
    /// @return    None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID Cleanup();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ConfigureLRMEConfidenceParameter
    ///
    /// @brief     This method configures the transform confidence and check if force Identity transform is enabled
    ///
    /// @param     pTransformConfidence      Transform confidence
    /// @param     pForceIdentityTransform   Force identity transform enabled or disabled
    ///
    /// @return    CamxResultSuccess on success else failure code
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult ConfigureLRMEConfidenceParameter(
        UINT32* pTransformConfidence,
        UINT8*  pForceIdentityTransform);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// PostICATransform
    ///
    /// @brief     This method publishes the ICA transform in metadata
    ///
    /// @param     pTransform       The transform from nclib
    /// @param     confidence       Transform confidence
    /// @param     pFrameSettings   LRME frame settings with which transform is calculated
    ///
    /// @return    CamxResultSuccess on success else failure code
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult PostICATransform(
        VOID* pTransform,
        UINT32 confidence,
        LRMEPropertyFrameSettings* pFrameSettings);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ConfigureICATransformSettings
    ///
    /// @brief     This method checks the ICA transform settings requirement and configure the publishing
    ///
    /// @param     pFrameSettings                LRME frame settings with which transform is calculated
    /// @param     numBatchedFrames              number of batched frames
    /// @param     requestId                     request Id
    /// @param     requestIdOffsetFromLastFlush  request Id offset from last flush trigger
    ///
    /// @return    CamxResultSuccess on success else failure code
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult ConfigureICATransformSettings(
        LRMEPropertyFrameSettings* pFrameSettings,
        UINT                       numBatchedFrames,
        UINT64                     requestId,
        UINT64                     requestIdOffsetFromLastFlush);

    UINT                     m_numInputPorts;                             ///< Number of input ports used by RANSAC
    UINT                     m_numOutputPorts;                            ///< Number of output ports used by RANSAC
    UINT8                    m_forceIdentityTransform;                    ///< Identity transform forced state
    CPerspectiveTransform    m_transform;                                 ///< Calculated transform
    INT32                    m_confidence = 0;                            ///< Calculated confidence
    BOOL                     m_alternateSkipProcessing;                   ///< Flag to skip the RANSAC Processing
};

CAMX_NAMESPACE_END

#endif // CAMXRANSACNODE_H
