#ifndef __HDCP_MANAGER_H__
#define __HDCP_MANAGER_H__
/*==============================================================================
*       HDCPManager.h
*
*  DESCRIPTION:
*       Manager class for HDCP
*
*
*  Copyright (c) 2012 - 2014, 2017, 2020 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*===============================================================================
*/
/*==============================================================================
                             Edit History
================================================================================
*/

/*==============================================================================
**               Includes and Public Data Declarations
**==============================================================================
*/

/* =============================================================================

                     INCLUDE FILES FOR MODULE

================================================================================
*/
#include "stdio.h"
#include "MMThread.h"
#include <utils/String8.h>

/*==============================================================================

                        DATA DECLARATIONS

================================================================================
*/
/*------------------------------------------------------------------------------
** Constant / Define Declarations
**------------------------------------------------------------------------------
*/
#define HDCP_AUDIO_TRACK_ID 1
#define HDCP_VIDEO_TRACK_ID 0
/*------------------------------------------------------------------------------
** Forward Declarations
**------------------------------------------------------------------------------
***/

/*------------------------------------------------------------------------------
** Type Declarations
**------------------------------------------------------------------------------
***/
typedef enum HDCPMode
{
    HDCP_MODE_TX,
    HDCP_MODE_RX,
    HDCP_MODE_UNKNOWN = 0xFFFFFFFF
}HDCPModeType;

typedef enum HDCPStatus
{
    HDCP_FAIL,
    HDCP_BAD_PARAMS,
    HDCP_SUCCESS,
    HDCP_IN_PROGRESS,
    HDCP_INIT_COMPLETE,
    HDCP_UPSTREAM_CLOSE,
    HDCP_DOWNSTREAM_CLOSE,
    HDCP_UNAUTHENTICATED_CONNECTION,
    HDCP_UNAUTHORIZED_CONNECTION,
    HDCP_REVOKED_CONNECTION,
    HDCP_INVALID_STATE,
    HDCP_INVALID_STATUS = 0xFFFFFFFF
}HDCPStatusType;

typedef enum HDCPState
{
    HDCP_STATE_DEINIT,
    HDCP_STATE_INIT,
    HDCP_STATE_CONNECTING,
    HDCP_STATE_LISTENING,
    HDCP_STATE_DISCONNECTING,
    HDCP_STATE_CONNECTED,
    HDCP_STATE_PROCESSING,
    HDCP_STATE_ERROR = 0xFFFFFFFF
}HDCPStateType;

typedef enum HDCPCocec
{
     HDCP_CODEC_AAC = 1,
     HDCP_CODEC_AC3,
     HDCP_CODEC_LPCM,
     HDCP_CODEC_AVC,
     HDCP_CODEC_HEVC
}HDCPCodecType;

typedef struct HDCPIOBuffer
{
    bool isFd;
    // if isFd is true, handle is a share memory handle,
    // if isFd is false, handle is a memory pointer
    uint64_t handle;
} HDCPIOBufferType;

typedef void (*HDCPEventCbType)(HDCPStateType eState,
                                  HDCPStatusType eStatus,
                                  void *pUserData);


/*------------------------------------------------------------------------------
** Global Constant Data Declarations
**------------------------------------------------------------------------------
***/

/*------------------------------------------------------------------------------
** Global Data Declarations
**------------------------------------------------------------------------------
***/

/*==============================================================================
**                          Macro Definitions
**==============================================================================
***/


/*==============================================================================
**                        Class Declarations
**==============================================================================
***/
class HDCPManager
{
public:

    static HDCPManager* CreateHDCPManager();

    static void DestroyHDCPManager(HDCPManager*);

    static bool isHDCPAvailable();

    ~HDCPManager();

    HDCPStatusType registerCallback
    (
        HDCPEventCbType pCb,
        void *pUsrData
    );

    HDCPStateType  getHDCPManagerState();

    HDCPStatusType initializeHDCPManager();

    HDCPStatusType deinitializeHDCPManager();

    HDCPStatusType setupHDCPSession
    (
        HDCPModeType eMode,
        unsigned int nIpAddr,
        unsigned short portNum
    );

    HDCPStatusType teardownHDCPSession();

    void  HDCPManThread( void );

    HDCPStatusType decrypt
    (
        const unsigned char *pesPrivateData,
        const HDCPIOBufferType *pIn,
        HDCPIOBufferType *pOut,
        unsigned long nLen,
        unsigned long nPesPvtDataLen,
        unsigned long nOutputOffset,
        int nStreamId
    );

    HDCPStatusType decrypt
    (
        const unsigned char *pesPrivateData,
        const unsigned char *pIn,
        unsigned char *pOut,
        unsigned long nLen,
        unsigned long nPesPvtDataLen,
        unsigned long nOutputOffset,
        int nStreamId
    );

    HDCPStatusType setParameter
    (
        int paramID,
        void *paramData
    );

    void setStreamType(int nStreamType); // Set the stream Type
    bool isCodecInfoSet(int iMedia); // if codec info is set or not
    void constructCodecAndStreamType(int nStream, HDCPCodecType eCodec); // constructs the codec and stream combination

    void HDCPEventCallBack
    (
        int status,
        void *deviceHandle
    );

private:
    HDCPManager();
    HDCPManager(const HDCPManager&) = delete;
    HDCPManager& operator=(const HDCPManager&) = delete;


    static int threadEntry
    (
        void* ptr
    );



    void initData();


    HDCPStateType m_eState;
    FILE *fp;
    MM_HANDLE m_pHDCPManThread;
    MM_HANDLE m_pHDCPManSignalQ;
    MM_HANDLE m_pHDCPManCloseSignal;
    MM_HANDLE m_pListenSignal;
    MM_HANDLE m_pConnectSignal;
    MM_HANDLE m_pDisconnectSignal;

    static const int m_nHDCPManCloseEvent;
    static const int m_nHDCPManListenEvent;
    static const int m_nHDCPManConnectEvent;
    static const int m_nHDCPManDisconnectEvent;
    unsigned short m_nPortNum;
    unsigned int m_ipAddr;
    unsigned char  m_ipAddrArray[4];
    HDCPEventCbType m_pEventCb;
    void *m_pUsrData;
    HDCPModeType m_eMode;
    static void *m_hHDCPRptHandle; //Until HDCP1x and HDCP HLOS support clientdata
    static void * m_hHDCPTransportHandle;
    static HDCPStatus m_eTopologyStatus; // keeps track of topology update status
    static bool m_bPropagationStatus; // keeps track of propagation status from HDCP HLOS Lib
    static MM_HANDLE m_hCriticalSect;

    bool m_bAudioCodecInfoAvailable;
    bool m_bVideoCodecInfoAvailable;

#ifdef KERNEL_5_4
    int m_nQceFd;
#endif

    unsigned int m_nRetryCount; //tetry count for Event CIPHER ENABLE/DISABLE
    int m_nStreamType; //Audio/Video steam information will be stored
    int m_nAudioStreamType; // If Audio codec information is updated or not
    int m_nVideoStreamType; // Video codec information is updated or not
    // CB from Display HDCP manager to get topology update
    static int DISPCallBack(int eventType, void *arg1);
    void hdcp_update_topology(void *msg);

};
#endif/*__HDCP_SINK_MANAGER_H__ */
