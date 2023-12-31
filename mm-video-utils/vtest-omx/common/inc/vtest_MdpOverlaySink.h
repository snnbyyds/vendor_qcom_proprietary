/* ========================================================================= *
  Copyright (c) 2014-2017, 2019 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
* ========================================================================= */

#ifndef _VTEST_MDP_OVERLAY_SINK_H
#define _VTEST_MDP_OVERLAY_SINK_H

#include <utils/RefBase.h>
// To resolve 'expected expression' error in Mat4.h, undef 'near' and 'far'
// before including SurfaceComposerClient.h, if they are already defined.
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif
#include <SurfaceComposerClient.h>
#include <Surface.h>
#include <ISurfaceComposer.h>
#include <DisplayInfo.h>
#include "OMX_Core.h"
#include "OMX_QCOMExtns.h"
#include "vtest_DecoderFileSink.h"
#include <linux/msm_mdp.h>
#include "ion/ion.h"
#include <linux/dma-buf.h>

namespace vtest {

class MdpOverlaySink : virtual public ISource {

public:
    MdpOverlaySink(OMX_BOOL bRotate);
    ~MdpOverlaySink();

    virtual PortBufferCapability GetBufferRequirements(OMX_U32 ePortIndex);
    virtual OMX_ERRORTYPE Configure(CodecConfigType *pConfig,
            BufferManager *pBufManager, ISource *pSource, ISource *pSink);
    virtual OMX_ERRORTYPE SetBuffer(BufferInfo *pBuffer, ISource *pSource,
        OMX_U32 framenum = 0);
    virtual OMX_ERRORTYPE PortReconfig(OMX_U32 ePortIndex,
            OMX_U32 nWidth, OMX_U32 nHeight, const OMX_CONFIG_RECTTYPE& sRect);

private:
    MdpOverlaySink();
    virtual OMX_ERRORTYPE ThreadRun(OMX_PTR pThreadData);
    void OverlayUnset();
    OMX_ERRORTYPE OverlaySet();
    OMX_ERRORTYPE OverlayEnqueueBuffer(struct OMX_BUFFERHEADERTYPE *pBufHdr);
    void FreeRotatorBuffer();
    OMX_ERRORTYPE AllocateRotatorBuffer();

private:
    OMX_U32 m_nFrames;
    OMX_U32 m_nFrameWidth;
    OMX_U32 m_nFrameHeight;
    OMX_BOOL m_bRotate;
    SignalQueue *m_pBufferQueue;
    OMX_S32 m_nFbFd;
    OMX_S32 m_nOverlayId;
    OMX_S32 m_nIonFd;
    OMX_S32 m_nDataFd;
    OMX_BOOL m_bSecureSession;
    struct mdp_overlay m_sOverlay;
    struct fb_var_screeninfo m_sVinfo;
    struct fb_fix_screeninfo m_sFinfo;
    struct ion_allocation_data m_sRotatorIonData;
};

}

#endif //#ifndef _VTEST_MDP_OVERLAY_SINK_H

