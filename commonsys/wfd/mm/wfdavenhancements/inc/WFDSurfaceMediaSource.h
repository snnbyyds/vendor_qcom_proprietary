/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _WFDSurfaceMediaSource_H
#define _WFDSurfaceMediaSource_H

#include <gui/IGraphicBufferProducer.h>
#include <gui/BufferQueue.h>
#include <utils/threads.h>
#include <utils/Vector.h>
#include <media/hardware/MetadataBufferType.h>
#include "media/stagefright/foundation/ABase.h"
#include <map>

class WFDMediaBufferObserver;

/**
 * Store key value pairs within mediaBuffer. No crit-sect here.
 * Caller will provide if needed. (Not needed for current video
 * capture usage).
 */
class WFDSmsKeyValueMap
{
public:
    bool findInt64(unsigned int key, int64_t& value) const;
    bool setInt64(unsigned int key, int64_t value);

private:
    typedef std::map<unsigned int, int64_t> Int64MapType;
    Int64MapType m_Int64Map;
};

class WFDMediaBuffer
{
public:
    WFDMediaBuffer(size_t size);
    ~WFDMediaBuffer();

    unsigned int refcount() const;
    void release();
    void add_ref();

    void* data();
    unsigned int range_offset() const;
    unsigned int range_length() const;
    unsigned int size() const;
    WFDSmsKeyValueMap& meta_data();

    void setObserver(WFDMediaBufferObserver *observer);

private:
    WFDMediaBuffer();
    WFDMediaBuffer(const WFDMediaBuffer&);
    WFDMediaBuffer& operator=(const WFDMediaBuffer&);

    const size_t m_Size;
    unsigned int mRefCount;
    const unsigned int m_RangeOffset;
    const unsigned int m_RangeLength;

    WFDSmsKeyValueMap WFDSmsKeyValueMap;
    void *m_Data;

    WFDMediaBufferObserver *m_pObserver;
};

class WFDMediaBufferObserver {
public:
    WFDMediaBufferObserver() {}
    virtual ~WFDMediaBufferObserver() {}

    virtual void signalBufferReturned(WFDMediaBuffer *buffer) = 0;

private:
    WFDMediaBufferObserver(const WFDMediaBufferObserver &);
    WFDMediaBufferObserver &operator=(const WFDMediaBufferObserver &);
};


namespace android {

class String8;
class GraphicBuffer;

// ASSUMPTIONS
// 1. WFDSurfaceMediaSource is initialized with width*height which
// can never change.  However, deqeueue buffer does not currently
// enforce this as in BufferQueue, dequeue can be used by Surface
// which can modify the default width and heght.  Also neither the width
// nor height can be 0.
// 2. setSynchronousMode is never used (basically no one should call
// setSynchronousMode(false)
// 3. setCrop, setTransform, setScalingMode should never be used
// 4. queueBuffer returns a filled buffer to the WFDSurfaceMediaSource. In addition, a
// timestamp must be provided for the buffer. The timestamp is in
// nanoseconds, and must be monotonically increasing. Its other semantics
// (zero point, etc) are client-dependent and should be documented by the
// client.
// 5. Once disconnected, WFDSurfaceMediaSource can be reused (can not
// connect again)
// 6. Stop is a hard stop, the last few frames held by the encoder
// may be dropped.  It is possible to wait for the buffers to be
// returned (but not implemented)

#define DEBUG_PENDING_BUFFERS   0

typedef enum WFDSMSConsumer
{
    WFD_SMS_CONSUMER_HW,
    WFD_SMS_CONSUMER_SW
}WFDSMSConsumerType;

class WFDSurfaceMediaSource : public virtual RefBase,
                              public WFDMediaBufferObserver,
                              protected ConsumerListener {
public:
    enum { MIN_UNDEQUEUED_BUFFERS = 4};

    struct FrameAvailableListener : public virtual RefBase {
        // onFrameAvailable() is called from queueBuffer() is the FIFO is
        // empty. You can use WFDSurfaceMediaSource::getQueuedCount() to
        // figure out if there are more frames waiting.
        // This is called without any lock held can be called concurrently by
        // multiple threads.
        virtual void onFrameAvailable() = 0;
    };

    WFDSurfaceMediaSource(uint32_t bufferWidth, uint32_t bufferHeight,
                          WFDSMSConsumerType eType, bool UBWCSupport = false);

    virtual ~WFDSurfaceMediaSource();

    // For the MediaSource interface for use by StageFrightRecorder:
    virtual status_t start(int32_t bufferCount);
    virtual status_t stop();
    bool read(WFDMediaBuffer **buffer);

    // Get / Set the frame rate used for encoding. Default fps = 30
    status_t setFrameRate(int32_t fps) ;
    int32_t getFrameRate( ) const;

    // The call for the StageFrightRecorder to tell us that
    // it is done using the MediaBuffer data so that its state
    // can be set to FREE for dequeuing
    virtual void signalBufferReturned(WFDMediaBuffer* buffer);
    // end of MediaSource interface

    // setFrameAvailableListener sets the listener object that will be notified
    // when a new frame becomes available.
    void setFrameAvailableListener(const sp<FrameAvailableListener>& listener);

    // dump our state in a String
    void dump(String8& result) const;
    void dump(String8& result, const char* prefix, char* buffer,
                                                    size_t SIZE) const;

    // metaDataStoredInVideoBuffers tells the encoder what kind of metadata
    // is passed through the buffers. Currently, it is set to ANWBuffer
    MetadataBufferType metaDataStoredInVideoBuffers() const;

    sp<IGraphicBufferProducer> getProducer() const { return mProducer; }

    // To be called before start()
    status_t setMaxAcquiredBufferCount(size_t count);

    // To be called before start()
    status_t setUseAbsoluteTimestamps();

    // get the buffer handle from MediaBuffer ptr
    static buffer_handle_t getMediaBufferHandle(WFDMediaBuffer *buffer);

    // Provide accessor to image pointer within the scope of ImageAccessor.
    // Image pointer is mmap'ped as part of "lock" api which is not elegant as
    //  locking a graphic buffer for too long may have other issues.
    // So, ImageAccessor is meant to be a short-lived object typically
    // allocated on function stack only (not on heap).
    class ImageAccessor
    {
    public:
        ImageAccessor(android::sp<android::WFDSurfaceMediaSource> pSMS,
                      buffer_handle_t bufferHandle);
        ~ImageAccessor();

        void *getImagePtr() const;

    private:
        ImageAccessor();
        ImageAccessor(const ImageAccessor&);
        ImageAccessor& operator=(const ImageAccessor&);

        sp<GraphicBuffer> m_pGraphicBuffer;
        void *m_pImg;
    };

protected:

    // Implementation of the BufferQueue::ConsumerListener interface.  These
    // calls are used to notify the Surface of asynchronous events in the
    // BufferQueue.
    virtual void onFrameAvailable(const BufferItem& item);

    // Used as a hook to BufferQueue::disconnect()
    // This is called by the client side when it is done
    // TODO: Currently, this also sets mStopped to true which
    // is needed for unblocking the encoder which might be
    // waiting to read more frames. So if on the client side,
    // the same thread supplies the frames and also calls stop
    // on the encoder, the client has to call disconnect before
    // it calls stop.
    // In the case of the camera,
    // that need not be required since the thread supplying the
    // frames is separate than the one calling stop.
    virtual void onBuffersReleased();

    // WFDSurfaceMediaSource can't handle sideband streams, so this is not expected
    // to ever be called. Does nothing.
    virtual void onSidebandStreamChanged();

    static bool isExternalFormat(uint32_t format);

private:
    // A BufferQueue, represented by these interfaces, is the exchange point
    // between the producer and this consumer
    sp<IGraphicBufferProducer> mProducer;
    sp<IGraphicBufferConsumer> mConsumer;

    struct SlotData {
        sp<GraphicBuffer> mGraphicBuffer;
        uint64_t mFrameNumber;
    };

    // mSlots caches GraphicBuffers and frameNumbers from the buffer queue
    SlotData mSlots[BufferQueue::NUM_BUFFER_SLOTS];

    // The permenent width and height of SMS buffers
    int mWidth;
    int mHeight;

    // mCurrentSlot is the buffer slot index of the buffer that is currently
    // being used by buffer consumer
    // (e.g. StageFrightRecorder in the case of WFDSurfaceMediaSource or GLTexture
    // in the case of Surface).
    // It is initialized to INVALID_BUFFER_SLOT,
    // indicating that no buffer slot is currently bound to the texture. Note,
    // however, that a value of INVALID_BUFFER_SLOT does not necessarily mean
    // that no buffer is bound to the texture. A call to setBufferCount will
    // reset mCurrentTexture to INVALID_BUFFER_SLOT.
    int mCurrentSlot;

    // mCurrentBuffers is a list of the graphic buffers that are being used by
    // buffer consumer (i.e. the video encoder). It's possible that these
    // buffers are not associated with any buffer slots, so we must track them
    // separately.  Buffers are added to this list in read, and removed from
    // this list in signalBufferReturned
    Vector<sp<GraphicBuffer> > mCurrentBuffers;

    size_t mNumPendingBuffers;

#if DEBUG_PENDING_BUFFERS
    Vector<MediaBuffer *> mPendingBuffers;
#endif

    // mFrameAvailableListener is the listener object that will be called when a
    // new frame becomes available. If it is not NULL it will be called from
    // queueBuffer.
    sp<FrameAvailableListener> mFrameAvailableListener;

    // mMutex is the mutex used to prevent concurrent access to the member
    // variables of WFDSurfaceMediaSource objects. It must be locked whenever the
    // member variables are accessed.
    mutable Mutex mMutex;

    ////////////////////////// For MediaSource
    // Set to a default of 30 fps if not specified by the client side
    int32_t mFrameRate;

    // mStarted is a flag to check if the recording is going on
    bool mStarted;

    // mNumFramesReceived indicates the number of frames recieved from
    // the client side
    int mNumFramesReceived;
    // mNumFramesEncoded indicates the number of frames passed on to the
    // encoder
    int mNumFramesEncoded;

    // mFirstFrameTimestamp is the timestamp of the first received frame.
    // It is used to offset the output timestamps so recording starts at time 0.
    int64_t mFirstFrameTimestamp;
    // mStartTimeNs is the start time passed into the source at start, used to
    // offset timestamps.
    int64_t mStartTimeNs;

    size_t mMaxAcquiredBufferCount;

    bool mUseAbsoluteTimestamps;

    // mFrameAvailableCondition condition used to indicate whether there
    // is a frame available for dequeuing
    Condition mFrameAvailableCondition;

    Condition mMediaBuffersAvailableCondition;

    uint64_t mConsumerUsageFlags;

    // Allocate and return a new MediaBuffer and pass the ANW buffer as metadata into it.
    void passMetadataBuffer_l(WFDMediaBuffer **buffer, ANativeWindowBuffer *bufferHandle) const;

    sp<GraphicBuffer> getGraphicBuffer(buffer_handle_t bufferHandle);

    // Avoid copying and equating and default constructor
    DISALLOW_EVIL_CONSTRUCTORS(WFDSurfaceMediaSource);
};

// ----------------------------------------------------------------------------
}; // namespace android

#endif // _WFDSurfaceMediaSource_H
