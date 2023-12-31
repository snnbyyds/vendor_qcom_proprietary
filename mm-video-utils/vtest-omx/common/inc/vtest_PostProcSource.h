/* ========================================================================= *
  Copyright (c) 2014-2017 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
* ========================================================================= */

#ifndef _VTEST_POSTPROC_SOURCE_H
#define _VTEST_POSTPROC_SOURCE_H

#include "ion/ion.h"
#include <linux/dma-buf.h>

#include "OMX_Core.h"
#include "vtest_ISource.h"
#include "vtest_IPostProc.h"

/*
 * This class wraps the android PostProcSource API and provides a simplified
 * interface to allocate and display buffers.
 */
namespace vtest {

class PostProcSource : virtual public ISource {

public:
    PostProcSource();
    ~PostProcSource();

    virtual PortBufferCapability GetBufferRequirements(OMX_U32 ePortIndex);
    virtual OMX_ERRORTYPE Stop();
    virtual OMX_ERRORTYPE Configure(CodecConfigType *pConfig,
            BufferManager *pBufManager, ISource *pSource, ISource *pSink);
    virtual OMX_ERRORTYPE SetBuffer(BufferInfo *pBuffer, ISource *pSource,
        OMX_U32 framenum = 0);
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

    class Stats {

    public:
        Stats();
        ~Stats();

    public:
        OMX_U32 m_nMax;
        OMX_U32 m_nMin;
        OMX_U32 m_nAvg;
        OMX_U32 m_nTotal;
        OMX_U32 m_nCount;

        class AutoStats {

        public:
            AutoStats(Stats *pStats);
            ~AutoStats();
        private:
            Stats *m_pStats;
            OMX_TICKS m_nStartTime;
            OMX_TICKS m_nEndTime;
            OMX_TICKS m_nRunTimeMillis;
        };
    };

private:
    OMX_U32 m_nFrames;
    OMX_U32 m_nFramerate;
    OMX_U32 m_nFrameWidth;
    OMX_U32 m_nFrameHeight;
    OMX_BOOL m_bSecureSession;
    SignalQueue *m_pBufferQueueOut;
    OMX_U32 m_nInputBufferSize;
    OMX_U32 m_nOutputBufferSize;
    SignalQueue *m_pFreeBufferQueue;
    IPostProc *m_pPostProcModule;
    Stats *m_pStats;
};

}

#endif //#ifndef _VTEST_POSTPROC_SOURCE_H

