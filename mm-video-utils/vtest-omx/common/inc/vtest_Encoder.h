/*-------------------------------------------------------------------
Copyright (c) 2013-2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
--------------------------------------------------------------------*/

#ifndef _VTEST_ENCODER_H
#define _VTEST_ENCODER_H

#include "OMX_Core.h"
#include "OMX_Component.h"
#include "OMX_QCOMExtns.h"
#include "vtest_ComDef.h"
#include "vtest_Thread.h"
#include "vtest_ISource.h"
#include "vtest_SignalQueue.h"
#include "vtest_BufferManager.h"
#ifdef USE_GBM
#include <gbm.h>
#include <gbm_priv.h>
#else
#include <qdMetaData.h>
#endif
#include <color_metadata.h>

namespace vtest {

#define MIN_ALLOWED_QP_H264   0     //for H264,HEVC min QP is 0 and max qp is 51
#define MAX_ALLOWED_QP_H264   51
#define MIN_ALLOWED_QP_MPEG4  1     //for H263,MPEG4 min QP is 1 and max qp is 31
#define MAX_ALLOWED_QP_MPEG4  31
#define MIN_ALLOWED_QP_VP8    0     //for VP8, min QP is 0 and max qp is 127
#define MAX_ALLOWED_QP_VP8    127

class Signal;

/**
 * @brief Class that wraps/simplifies OMX IL encoder component interactions.
 */
class Encoder : virtual public ISource {

public:
    Encoder(CodecConfigType *pConfig);
    ~Encoder();

    virtual PortBufferCapability GetBufferRequirements(OMX_U32 ePortIndex);
    virtual OMX_ERRORTYPE Start();
    virtual OMX_ERRORTYPE Stop();
    virtual OMX_ERRORTYPE Configure(CodecConfigType *pConfig,
            BufferManager *pBufManager, ISource *pSource, ISource *pSink);
    virtual OMX_ERRORTYPE SetBuffer(
            BufferInfo *pBufferInfo, ISource *pSource, OMX_U32 framenum);
    virtual OMX_ERRORTYPE AllocateBuffers(BufferInfo **pBuffers,
        OMX_BUFFERHEADERTYPE **pExtraDataBuffers, OMX_U32 nWidth,
        OMX_U32 nHeight, OMX_U32 nBufferCount, OMX_U32 nBufferSize,
        OMX_U32 nExtraDataBufferCount, OMX_U32 nExtraDataBufferSize,
        OMX_U32 ePortIndex, OMX_U32 nBufferUsage = 0);
    virtual OMX_ERRORTYPE UseBuffers(BufferInfo **pBuffers,
        OMX_BUFFERHEADERTYPE **ppExtraDataBuffers, OMX_U32 nWidth,
        OMX_U32 nHeight, OMX_U32 nBufferCount, OMX_U32 nBufferSize,
        OMX_U32 nExtraDataBufferCount, OMX_U32 nExtraDataBufferSize,
        OMX_U32 ePortIndex);
    virtual OMX_ERRORTYPE PortReconfig(OMX_U32 ePortIndex,
            OMX_U32 nWidth, OMX_U32 nHeight, const OMX_CONFIG_RECTTYPE& sRect);

private:
    virtual OMX_ERRORTYPE ThreadRun(OMX_PTR pThreadData);
    virtual OMX_ERRORTYPE FreeBuffer(
            BufferInfo *pBuffer, OMX_U32 ePortIndex);
    /**
     * @brief Set the encoder state
     *
     * This method can be asynchronous or synchronous. If asynchonous,
     * WaitState can be called to wait for the corresponding state
     * transition to complete.
     *
     * @param eState The state to enter
     * @param bSynchronous If OMX_TRUE, synchronously wait for the state transition to complete
     */
    OMX_ERRORTYPE SetState(OMX_STATETYPE eState, OMX_BOOL bSynchronous);
    /**
     * @brief Wait for the corresponding state transition to complete
     *
     * @param eState The state to wait for
     */
    OMX_ERRORTYPE WaitState(OMX_STATETYPE eState);
    /**
     * @brief Deliver an input (yuv) buffer to the encoder.
     *
     * @param pBuffer The populated input buffer.
     */
    OMX_ERRORTYPE DeliverInput(OMX_BUFFERHEADERTYPE *pBuffer);
    /**
     * @brief Deliver an output (bitstream) buffer to the encoder.
     *
     * @param pBuffer The un-populated output buffer.
     */
    OMX_ERRORTYPE DeliverOutput(OMX_BUFFERHEADERTYPE *pBuffer);
    /**
      * @brief Request for an iframe to be generated.
      */
    OMX_ERRORTYPE RequestIntraVOP();
    /**
     * @brief Set VIT Timing info
     */
    OMX_ERRORTYPE SetVUITimingInfo(OMX_S32 nVUITimingInfo);
    /**
     * @brief Set the intra period. It is valid to change this configuration at run-time
     *
     */
    OMX_ERRORTYPE SetIntraPeriod();
    /**
     * @brief Set the IDR period for AVC. It is valid to change
     *        this configuration at run-time
     *
     * @param nIRMode, nIntraMBs The intra period and IDR period and Mode
     */
    OMX_ERRORTYPE SetIntraRefresh(OMX_VIDEO_INTRAREFRESHTYPE nIRMode, OMX_S32 nIntraMBs);
    /**
    * @brief Set the Hier P Num Layers. It is valid to change
    *        this configuration at run-time
    */
    OMX_ERRORTYPE SetHierPNumLayers();
    /**
    * @brief Set the Max Hier P Layers.
    *
    * @param nMaxHPLayers the max of Hier P layers
    */
    OMX_ERRORTYPE SetMaxHierPLayers(OMX_U32 nMaxHPLayers);
    /**
    * @brief Set the Low latency mode to OMX component.
    *        This changes POC value in encoded stream.
    *
    * @param bLowLatency is the bool value to enable\disable
    */
    OMX_ERRORTYPE SetLowLatency(OMX_BOOL bLowLatency);
    /**
     * @brief Get the vendor extesion via given extension name
     *
     * @param sExtensionName is the extension name, ext is the extension
     */
     OMX_ERRORTYPE GetVendorExtension(string sExtensionName, OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE *ext);
    /**
    * @brief Set the Bitrate to OMX component.
    *
    * @param nBitrate is bitrate
    */
    OMX_ERRORTYPE SetBitrateOnly(OMX_S32 nBitrate);
    /**
    * @brief Set the rate control mode to OMX component via vendor extension
    *        for vendor extended modes: RC_OFF, VBR_VFR and CBR_VFR
    *
    * @param eControlRate is rate control mode
    */
    OMX_ERRORTYPE SetVendorRateControlMode(OMX_VIDEO_CONTROLRATETYPE eControlRate);
    /**
    * @brief Set the rate control mode and bit rate to OMX component
    *        for Google's defined modes: CQ, VBR_CFR and CBR_CFR
    *
    * @param eControlRate is rate control mode, nBitrate is bitrate
    */
    OMX_ERRORTYPE SetBitrate(OMX_VIDEO_CONTROLRATETYPE eControlRate, OMX_S32 nBitrate);
    /**
    * @brief Set Operating rate.
    *
    * @param nOperatingRate
    */
    OMX_ERRORTYPE SetVencOperatingRate(OMX_U32 nOperatingRate);
    /**
    * @brief Set Encoder SAR.
    *
    * @param nSarWidth nSarHeight
    */
    OMX_ERRORTYPE SetSarInfo(OMX_U32 nSarWidth, OMX_U32 nSarHeight);
    /**
    * @brief Set QP Range to OMX Component.
    *
    * @param minQP maxQP minQP_packed maxQP_packed
    */
    OMX_ERRORTYPE SetQPRange(OMX_U32 minQP_packed, OMX_U32 maxQP_packed);
    /**
     * @brief Set port frame size and get resulting buffer
     *        requirements
     *
     * @param ePortIndex input/output port
     * @param nWidth frame width
     * @param nHeight frame height
     * @param nBufferCountMin min requested buffers
     * @param nFrameRate frame rate (output port only)
     * @param nBufferSize [out] required buffer size
     * @param nBufferCount [out] required buffer count
     */
    OMX_ERRORTYPE SetPortParams(OMX_U32 ePortIndex,
            OMX_U32 nWidth, OMX_U32 nHeight, OMX_U32 nBufferCountMin,
            OMX_U32 nFrameRate, OMX_U32 *nBufferSize, OMX_U32 *nBufferCount);
    /**
     * @brief Change the encoding framerate
     *
     * @param nFramerate The updated frame rate
     */
    OMX_ERRORTYPE ChangeFrameRate(OMX_S32 nFramerate);

    /**
     * @brief Change the encoding quality
     *
     * @param nFramerate The updated frame rate
     * @param nBitrate The updated bitrate
     */
    OMX_ERRORTYPE ChangeQuality(OMX_S32 nFramerate, OMX_S32 nBitrate);

    OMX_ERRORTYPE ProcessDynamicConfiguration(
            DynamicConfig *dynamic_config, int index);
    OMX_ERRORTYPE ProcessRoi(OMX_TICKS nTimeStamp);
    OMX_ERRORTYPE ReadRoiQp(OMX_QTI_VIDEO_CONFIG_ROIINFO &roiInfo);

    OMX_ERRORTYPE ProcessCvpMeta(OMX_BUFFERHEADERTYPE *pBuffer);
    OMX_ERRORTYPE SetDynamicConfig(OMX_INDEXTYPE config_param,
            DynamicConfigData* config_data);

    OMX_ERRORTYPE ProcessHDR10plusMetadata(OMX_BUFFERHEADERTYPE *pBuffer);
    OMX_ERRORTYPE ReadDynamicMetadata(ColorMetaData &colorData);
    OMX_ERRORTYPE SetVideoCSC();
    OMX_ERRORTYPE SetColorInfo(CodecConfigType *pConfig, OMX_BUFFERHEADERTYPE *pBuffer);

    static OMX_ERRORTYPE EventCallback(
            OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_PTR pAppData,
            OMX_IN OMX_EVENTTYPE eEvent, OMX_IN OMX_U32 nData1,
            OMX_IN OMX_U32 nData2, OMX_IN OMX_PTR pEventData);
    static OMX_ERRORTYPE EmptyDoneCallback(OMX_IN OMX_HANDLETYPE hComponent,
            OMX_IN OMX_PTR pAppData, OMX_IN OMX_BUFFERHEADERTYPE *pBuffer);
    static OMX_ERRORTYPE FillDoneCallback(OMX_IN OMX_HANDLETYPE hComponent,
            OMX_IN OMX_PTR pAppData, OMX_IN OMX_BUFFERHEADERTYPE *pBuffer);

    OMX_ERRORTYPE GetComponentRole(OMX_BOOL bSecureSession,
            OMX_VIDEO_CODINGTYPE eCodec, OMX_STRING *role);
    OMX_ERRORTYPE Flush(OMX_U32 nPortIndex);
    OMX_BOOL CheckColorFormatSupported(OMX_COLOR_FORMATTYPE nColorFormat,
            OMX_VIDEO_PARAM_PORTFORMATTYPE *pVideoPortFmt);

private:
    OMX_BOOL m_bInputEOSReached;
    SignalQueue *m_pSignalQueue;
    OMX_BOOL m_bSecureSession;
    CodecConfigType m_sConfig;
    OMX_VIDEO_CODINGTYPE m_eCodec;
    OMX_HANDLETYPE m_hEncoder;
    OMX_STATETYPE m_eState;
    OMX_STATETYPE m_eStatePending;
    OMX_U32 m_nInputBufferCount;
    OMX_U32 m_nOutputBufferCount;
    OMX_U32 m_nInputBufferSize;
    OMX_U32 m_nOutputBufferSize;
    OMX_U32 m_LTRId;
    OMX_S32 m_nDynamicIndexCount;
    DynamicConfigType m_sDynamicConfig;
    OMX_BOOL m_bPortReconfig;
    SignalQueue *m_pFreeBufferQueue;
    OMX_BOOL m_bMetaMode;
    MetaBufferType m_eMetaBufferType;
    OMX_U32 m_nFrameWidth;
    OMX_U32 m_nFrameHeight;
    OMX_U32 m_nColorFormat;
    FILE *mRoiQpFile;
    char *mRoiQpBuf;
    FILE *mMetadataFile;
    FILE *mCVPMetaFile;
    OMX_BOOL m_bHDRMetaEnabled;
};

} // namespace vtest


#endif // #ifndef  _VTEST_ENCODER_H
