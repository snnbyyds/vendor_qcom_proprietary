////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016-2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  camxhwcontext.h
/// @brief HwContext class declaration
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CAMXHWCONTEXT_H
#define CAMXHWCONTEXT_H

#include "camxcsl.h"
#include "camxcslsensordefs.h"
#include "camxdefs.h"
#include "camxstaticcaps.h"
#include "camxstatsparser.h"
#include "camxtypes.h"

CAMX_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct PlatformStaticCaps;

class  HwContext;
class  HwFactory;
class  ImageSensorModuleData;
class  Packet;
class  TuningDataManager;
class  HwEnvironment;
struct StaticSettings;

/// @brief Hardware context create data
struct HwContextCreateData
{
    HwContext*     pHwContext;      ///< Pointer to hw context object if create succeeds
    HwEnvironment* pHwEnvironment;  ///< Pointer to the hw environment
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class that implements the Hw context
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class HwContext
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Create
    ///
    /// @brief  Static function to create the concrete HW context object.
    ///
    /// @param  pCreateData Hw context create data
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static CamxResult Create(
        HwContextCreateData* pCreateData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// OpenSession
    ///
    /// @brief  Static function to Open a CSL Session.
    ///
    /// @param  phCSLSession Pointer to CSL Session
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CAMX_INLINE static CamxResult OpenSession(
        CSLHandle* phCSLSession)
    {
        CamxResult result = CSLOpen(reinterpret_cast<CSLHandle*>(phCSLSession));

        return result;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// CloseSession
    ///
    /// @brief  Static function to close CSL Session.
    ///
    /// @param  handle CSL Session handle
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CAMX_INLINE static CamxResult CloseSession(
        CSLHandle handle)
    {
        CSLHandle hCSLSession = static_cast<CSLHandle>(handle);

        CamxResult result = CSLClose(hCSLSession);

        return result;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// IsNodeTypeSinkNoBuffer
    ///
    /// @brief  Pure virtual method to query if it is a Sink node with no output buffer
    ///
    /// @param  nodeType Node Type
    ///
    /// @return TRUE if it is a Sink node with no output buffer, FALSE otherwise
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual BOOL IsNodeTypeSinkNoBuffer(
        UINT nodeType
        ) const = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Destroy
    ///
    /// @brief  Destroy the hw context object.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CAMX_INLINE VOID Destroy()
    {
        m_pHwEnvironment = NULL;

        CAMX_DELETE this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetStatsParser
    ///
    /// @brief  Pure virtual mode to get the stats parser object.
    ///
    /// @return The stats parser object.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual StatsParser* GetStatsParser() = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetDriverVersion
    ///
    /// @brief  Query version of a particular device driver from CSL.
    ///
    /// @param  deviceType  The device to query the version of.
    /// @param  pVersion    Pointer to version structure to be populated with driver version number. If a device is not
    ///                     supported the version will be reported as 0.0.0
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult GetDriverVersion(
        CSLDeviceType   deviceType,
        CSLVersion*     pVersion
        ) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetImageSensorModuleData
    ///
    /// @brief  Get the image sensor module data instance for this camera session.
    ///
    /// @param  cameraId    cameraId
    ///
    /// @return A const pointer to the ImageSensorModuleData class. NULL if not valid.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    const ImageSensorModuleData* GetImageSensorModuleData(
        UINT32 cameraId) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FlushLock
    ///
    /// @note   For a full description of the assumptions and implications of this method, see the description of CSLFLush.
    ///
    /// @brief  Obtain submission lock for the devices.
    ///
    /// @param  hCSLSession   CSL Session handle
    /// @param  rCSLFlushInfo Reference to CSL Flush Info
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult FlushLock(
        CSLHandle           hCSLSession,
        const CSLFlushInfo& rCSLFlushInfo);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FlushUnlock
    ///
    /// @note   For a full description of the assumptions and implications of this method, see the description of CSLFLush.
    ///
    /// @brief  release submission lock for the devices.
    ///
    /// @param  hCSLSession  CSL Session handle
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult FlushUnlock(
        CSLHandle hCSLSession);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FlushRequest
    ///
    /// @note   Passthrough to CSLCancelRequest.
    ///
    /// @brief  Cancel request for the current CSL session.
    ///
    /// @param  hCSLSession CSL Session handle
    /// @param  hLink       Pipeline link in CSL
    /// @param  cslSyncId   CSL Sync id that needed to be cancelled
    /// @param  requestId   Request to cancel
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult FlushRequest(
        CSLHandle     hCSLSession,
        CSLLinkHandle hLink,
        UINT64        cslSyncId,
        UINT64        requestId);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Link
    ///
    /// @note   For a full description of the assumptions and implications of this method, see the description of CSLLink.
    ///
    /// @brief  Link underlying devices.
    ///
    /// @param  hCSLSession CSL Session handle
    /// @param  phLink      Link handle
    /// @param  phDevices   Devices
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult Link(
        CSLHandle        hCSLSession,
        CSLLinkHandle*   phLink,
        CSLDeviceHandle* phDevices);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Unlink
    ///
    /// @note   For a full description of the assumptions and implications of this method, see the description of CSLUnlink.
    ///
    /// @brief  Link underlying devices.
    ///
    /// @param  hCSLSession CSL Session handle
    /// @param  phLink      Link handle
    /// @param  phDevices   Devices
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult Unlink(
        CSLHandle        hCSLSession,
        CSLLinkHandle*   phLink,
        CSLDeviceHandle* phDevices);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// StreamOn
    ///
    /// @note   For a full description of the assumptions and implications of this method, see the description of CSLStreamOn.
    ///
    /// @brief  Enable underlying devices to receive and process packets.
    ///
    /// @param  hCSLSession   CSL Session handle
    /// @param  phLink        Link handle
    /// @param  phDevices     Devices
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult StreamOn(
        CSLHandle        hCSLSession,
        CSLLinkHandle*   phLink,
        CSLDeviceHandle* phDevices);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// StreamOff
    ///
    /// @note   For a full description of the assumptions and implications of this method, see the description of CSLStreamOff.
    ///
    /// @brief  Disable underlying devices.
    ///
    /// @param  hCSLSession CSL Session handle
    /// @param  phLink      Link handle
    /// @param  phDevices   Devices
    /// @param  mode        Stream off mode
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult StreamOff(
        CSLHandle                   hCSLSession,
        CSLLinkHandle*              phLink,
        CSLDeviceHandle*            phDevices,
        CHIDEACTIVATEPIPELINEMODE   mode);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Submit
    ///
    /// @note   For a full description of the assumptions and implications of this method, see the description of CSLSubmit.
    ///
    /// @brief  Submit a packet to CSL.
    ///
    /// @param  hCSLSession  Handle for CSL Session.
    /// @param  hDevice      Handle to the CSL device to which the packet will be submitted
    /// @param  pPacket      Pointer to a packet object
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult Submit(
        CSLHandle       hCSLSession,
        CSLDeviceHandle hDevice,
        Packet*         pPacket);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetDeviceVersion
    ///
    /// @brief  Query version of a particular device from CSL.
    ///
    /// @param  deviceType  The device to query the version of.
    /// @param  pVersion    Pointer to version structure to be populated with device version number. If a device is not
    ///                     supported the version will be reported as 0.0.0
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult GetDeviceVersion(
        CSLDeviceType deviceType,
        CSLVersion*   pVersion
        ) const;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetStaticSettings
    ///
    /// @note   For a full description of the assumptions and implications of this method, see the description of CSLStreamOn.
    ///
    /// @brief  Enable underlying devices to receive and process packets.
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    const StaticSettings* GetStaticSettings();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetCameraSoc
    ///
    /// @brief  Query CSL and return an integer with SOC Id.
    ///
    /// @return An integer with SOC Id filled
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CAMX_INLINE UINT32 GetCameraSoc()
    {
        CSLCameraPlatform   CSLPlatform  = {};
        CamxResult          result       = CamxResultSuccess;
        UINT32              socId        = 0;

        result = CSLQueryCameraPlatform(&CSLPlatform);

        if (CamxResultSuccess == result)
        {
            socId = CSLPlatform.socId;
        }
        else
        {
            CAMX_LOG_ERROR(CamxLogGroupCore, "Titan17xContext returned invalid SOC Id");
        }

        return socId;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetNumberOfIPE
    ///
    /// @brief  Query CSL and return an integer with number of IPEs.
    ///
    /// @return An integer with number of IPEs filled
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual UINT32 GetNumberOfIPE()
    {
        return 1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// InitializeSOCDependentParams
    ///
    /// @brief  Initialize the SOC dependent params
    ///
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual CamxResult InitializeSOCDependentParams()
    {
        return CamxResultSuccess;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DumpRequest
    ///
    /// @brief  Dumps the information for the error request.
    ///
    /// @param  hCSLSession        CSL handle
    /// @param  pDumpRequestInfo   Pointer to dump request info.
    /// @param  pFilledLength      OUT size of buffer dumped.
    ///
    /// @return CamxResultSuccess if successful.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult DumpRequest(
        CSLHandle              hCSLSession,
        CSLDumpRequestInfo*    pDumpRequestInfo,
        SIZE_T*                pFilledLength);

protected:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Initialize
    ///
    /// @brief  Initialize the instantiated object
    ///
    /// @param  pCreateData Create data
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult Initialize(
        HwContextCreateData* pCreateData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ~HwContext
    ///
    /// @brief  Destructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    virtual ~HwContext();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// HwContext
    ///
    /// @brief  Constructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HwContext() = default;

private:
    // Do not implement the copy constructor or assignment operator
    HwContext(const HwContext&)            = delete;    ///< Disallow the copy constructor.
    HwContext& operator=(const HwContext&) = delete;    ///< Disallow assignment operator.

    HwEnvironment* m_pHwEnvironment;    ///< HW environment
};

CAMX_NAMESPACE_END

#endif // CAMXHWCONTEXT_H