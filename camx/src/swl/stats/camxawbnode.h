////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  camxawbnode.h
/// @brief Stats Auto White Balance node class declarations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CAMXAWBNODE_H
#define CAMXAWBNODE_H

#include "camxdefs.h"
#include "camxmem.h"
#include "camxnode.h"
#include "camxtypes.h"
#include "camxthreadmanager.h"
#include "camxstatscommon.h"
#include "camxstatsprocessormanager.h"
#include "camxmultistatsoperator.h"

CAMX_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class that implements the 3A node class
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AWBNode final : public Node
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create
    ///
    /// @brief  Static method to create AWBNode Object.
    ///
    /// @param  pCreateInputData  Node create input data
    /// @param  pCreateOutputData Node create output data
    ///
    /// @return Pointer to the concrete AWBNode object
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static AWBNode* Create(
        const NodeCreateInputData*  pCreateInputData,
        NodeCreateOutputData*       pCreateOutputData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Destroy
    ///
    /// @brief  This method destroys the derived instance of the interface
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual VOID Destroy();

protected:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ProcessingNodeInitialize
    ///
    /// @brief  Initialize the sw processing object
    ///
    /// @param  pCreateInputData  Node create input data
    /// @param  pCreateOutputData Node create output data
    ///
    /// @return CamxResultSuccess if successful
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult ProcessingNodeInitialize(
        const NodeCreateInputData*  pCreateInputData,
        NodeCreateOutputData*       pCreateOutputData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetPropertyDependencies
    ///
    /// @brief  Get the list of property dependencies from all stats processors and check if they are satisfied
    ///
    /// @param  pExecuteProcessRequestData          Pointer to Execute process request data
    /// @param  pStatsProcessRequestDataInfo        Pointer to Stats process request structure to be filled
    ///
    /// @return CamxResultSuccess if successful
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult GetPropertyDependencies(
        ExecuteProcessRequestData*  pExecuteProcessRequestData,
        StatsProcessRequestData*    pStatsProcessRequestDataInfo
    ) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetBufferDependencies
    ///
    /// @brief  Checks if the buffer dependency list and check if they are satisfied
    ///
    /// @param  pExecuteProcessRequestData      Pointer to Execute process request data
    ///
    /// @return CamxResultSuccess if successful
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult GetBufferDependencies(
        ExecuteProcessRequestData*  pExecuteProcessRequestData
    ) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetMultiStatsDependencies
    ///
    /// @brief  Checks if the multi stats dependency list and check if they are satisfied
    ///
    /// @param  pExecuteProcessRequestData      Pointer to Execute process request data
    /// @param  pStatsProcessRequestDataInfo    Pointer to Stats process request data
    ///
    /// @return CamxResultSuccess if successful
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult GetMultiStatsDependencies(
        ExecuteProcessRequestData*  pExecuteProcessRequestData,
        StatsProcessRequestData*    pStatsProcessRequestDataInfo
    ) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// PrepareStatsProcessRequestData
    ///
    /// @brief  Prepare data to pass to Stats ProcessRequest
    ///
    /// @param  pExecuteProcessRequestData  Pointer to Execute process request data
    /// @param  pStatsProcessRequestData    Pointer to Stats process request structure to be filled
    ///
    /// @return CamxResultSuccess if successful
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult PrepareStatsProcessRequestData(
        ExecuteProcessRequestData*  pExecuteProcessRequestData,
        StatsProcessRequestData*    pStatsProcessRequestData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ExecuteProcessRequest
    ///
    /// @brief  Pure virtual method to trigger process request for the stats processing node object.
    ///
    /// @param  pExecuteProcessRequestData Process request data
    ///
    /// @return CamxResultSuccess if successful and 0 dependencies, dependency information otherwise
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult ExecuteProcessRequest(
        ExecuteProcessRequestData* pExecuteProcessRequestData);

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
    /// PostPipelineCreate
    ///
    /// @brief  virtual method to be called at NotifyTopologyCreated time; node should take care of updates and initialize
    ///         blocks that has dependency on other nodes in the topology at this time.
    ///
    /// @return CamxResultSuccess if successful
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult PostPipelineCreate();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// AWBNode
    ///
    /// @brief  Constructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    AWBNode();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ~AWBNode
    ///
    /// @brief  Destructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~AWBNode();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief IStatsNotifier implementation
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// NotifyJobProcessRequestDone
    ///
    /// @brief  Notifies the statistic node that a job from the worker thread is completed
    ///
    /// @param  requestId   Request Id
    ///
    /// @return CamxResultSuccess if successful
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult NotifyJobProcessRequestDone(
        UINT64          requestId);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ProcessingNodeFinalizeInitialization
    ///
    /// @brief  Method to finalize the initialization of the node in the pipeline
    ///
    /// @param  pFinalizeInitializationData Finalize data
    ///
    /// @return CamxResultSuccess if successful
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult ProcessingNodeFinalizeInitialization(
        FinalizeInitializationData* pFinalizeInitializationData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// CanSkipAlgoProcessing
    ///
    /// @brief  Decides whether we can skip algo processing for the current frame based on skip factor
    ///
    /// @param  requestId current request id
    ///
    /// @return TRUE if we can skip otherwise FALSE
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    BOOL CanSkipAlgoProcessing(
        UINT64 requestId);

private:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// InitializePreviousSessionData
    ///
    /// @brief  This function is will get the previous session data from static node.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID InitializePreviousSessionData();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// InitializeMultiStats
    ///
    /// @brief  Initialize multi camera stats operator for multi camera usecase
    ///
    /// @return CamxResultSuccess if successful
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult InitializeMultiStats();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// IsForceSkipRequested
    ///
    /// @brief  Is force skip frame requested
    ///
    /// @return Boolean
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    BOOL IsForceSkipRequested();

    AWBNode(const AWBNode&)             = delete;       ///< Disallow the copy constructor.
    AWBNode& operator=(const AWBNode&)  = delete;       ///< Disallow assignment operator.

    IStatsProcessor*            m_pAWBStatsProcessor;   ///< Pointer to AWB Stats processor
    StatsNodeCreateData         m_statsCreateData;      ///< Create data from stats node
    StatsInitializeData         m_statsInitializeData;  ///< Save the stats settings pointer
    ChiContext*                 m_pChiContext;          ///< Chi context pointer
    UINT32                      m_bufferOffset;         ///< Offset from which to read buffers
    UINT32                      m_inputSkipFrameTag;    ///< Cached tagId for skip frame tag
    UINT                        m_skipPattern;          ///< Frames to skip
    StatsCameraInfo             m_cameraInfo;           ///< camera information
    MultiStatsOperator*         m_pMultiStatsOperator;  ///< Pointer to Multi stats operator
};

CAMX_NAMESPACE_END

#endif // CAMXAWBNODE_H
