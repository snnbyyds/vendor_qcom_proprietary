/**
 * @file      swvenc_api.h
 * @brief     SwVenc framework API.
 * @copyright Copyright (c) 2014-2015,2017,2019 Qualcomm Technologies, Inc.
 *            All Rights Reserved.
 *            Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef _SWVENC_API_H_
#define _SWVENC_API_H_

#include "swvenc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Synchronous API to initialize SwVenc library.
 *
 * SwVenc initialized with default properties and callback function pointers.
 *
 * @param[out] p_swvenc:   Pointer to SwVenc handle.
 * @param[in]  codec:      Codec type.
 * @param[in]  p_callback: Pointer to callback functions structure.
 *
 * @retval SWVENC_S_SUCCESS
 * @retval SWVENC_S_NULL_POINTER
 * @retval SWVENC_S_NOT_IMPLEMENTED
 * @retval SWVENC_S_FAILURE
 * @retval SWVENC_S_INVALID_PARAMETERS
 */
SWVENC_STATUS swvenc_init(SWVENC_HANDLE   *p_swvenc,
                          SWVENC_CODEC     codec,
                          SWVENC_CALLBACK *p_callback);

/**
 * @brief Synchronous API to de-initialize SwVenc library.
 *
 * @param[in] swvenc: SwVenc handle.
 *
 * @retval SWVENC_S_SUCCESS
 * @retval SWVENC_S_NULL_POINTER
 */
SWVENC_STATUS swvenc_deinit(SWVENC_HANDLE swvenc);

/**
 * @brief Synchronous API to limit number of concurrent
          running instances to 3 with in a session.
 *
 * @param[in] swvenc: SwVenc handle.
 *
 * @retval SWVENC_S_SUCCESS
 * @retval SWVENC_S_NULL_POINTER
 * @SWVENC_S_INSUFFICIENT_RESOURCES
 */
SWVENC_STATUS swvenc_check_inst_load(SWVENC_HANDLE swvenc);

/**
 * @brief Synchronous API to set a specific encoder property.
 *
 * @param[in] swvenc:     SwVenc handle.
 * @param[in] p_property: Pointer to property structure.
 *
 * @retval SWVENC_S_SUCCESS
 * @retval SWVENC_S_NULL_POINTER
 * @retval SWVENC_S_INVALID_STATE
 * @retval SWVENC_S_INVALID_PARAMETERS
 * @retval SWVENC_S_UNSUPPORTED
 * @retval SWVENC_S_NOT_IMPLEMENTED
 */
SWVENC_STATUS swvenc_setproperty(SWVENC_HANDLE    swvenc,
                                 SWVENC_PROPERTY *p_property);

/**
 * @brief Synchronous API to get a specific encoder property.
 *
 * @param[in]     swvenc:     SwVenc handle.
 * @param[in,out] p_property: Pointer to property structure.
 *
 * @retval SWVENC_S_SUCCESS
 * @retval SWVENC_S_NULL_POINTER
 * @retval SWVENC_S_INVALID_STATE
 * @retval SWVENC_S_INVALID_PARAMETERS
 */
SWVENC_STATUS swvenc_getproperty(SWVENC_HANDLE    swvenc,
                                 SWVENC_PROPERTY *p_property);

/**
 * @brief Synchronous API to start encoding.
 *
 * Codec-specific resources are allocated.
 *
 * @param[in] swvenc: SwVenc handle.
 *
 * @retval SWVENC_S_SUCCESS
 * @retval SWVENC_S_NULL_POINTER
 * @retval SWVENC_S_INVALID_STATE
 * @retval SWVENC_S_FAILURE
 */
SWVENC_STATUS swvenc_start(SWVENC_HANDLE swvenc);

/**
 * @brief Synchronous API to stop encoding.
 *
 * Codec-specific resources are de-allocated.
 *
 * @param[in] swvenc: SwVenc handle.
 *
 * @retval SWVENC_S_SUCCESS
 * @retval SWVENC_S_NULL_POINTER
 * @retval SWVENC_S_INVALID_STATE
 * @retval SWVENC_S_FAILURE
 */
SWVENC_STATUS swvenc_stop(SWVENC_HANDLE swvenc);

/**
 * @brief Synchronous API to request a sequence header from SwVenc.
 *
 * @param[in] swvenc:     SwVenc handle.
 * @param[in] p_opbuffer: Pointer to output bitstream buffer structure.
 *
 * @retval SWVENC_S_SUCCESS
 * @retval SWVENC_S_NULL_POINTER
 * @retval SWVENC_S_INVALID_STATE
 */
SWVENC_STATUS swvenc_getsequenceheader(SWVENC_HANDLE    swvenc,
                                       SWVENC_OPBUFFER *p_opbuffer);

/**
 * @brief Asynchronous API to send an input YUV buffer to SwVenc.
 *
 * The input YUV buffer is read and returned to client via callback function
 * pfn_empty_buffer_done().
 *
 * @param[in] swvenc:     SwVenc handle.
 * @param[in] p_ipbuffer: Pointer to input YUV buffer structure.
 *
 * @retval SWVENC_S_SUCCESS
 * @retval SWVENC_S_NULL_POINTER
 * @retval SWVENC_S_INVALID_STATE
 * @retval SWVENC_S_FAILURE
 */
SWVENC_STATUS swvenc_emptythisbuffer(SWVENC_HANDLE    swvenc,
                                     SWVENC_IPBUFFER *p_ipbuffer);

/**
 * @brief Asynchronous API to send an output bitstream buffer to SwVenc.
 *
 * The output bitstream buffer is filled and returned to client via callback
 * function pfn_fill_buffer_done().
 *
 * @param[in] swvenc:     SwVenc handle.
 * @param[in] p_opbuffer: Pointer to output bitstream buffer structure.
 *
 * @retval SWVENC_S_SUCCESS
 * @retval SWVENC_S_NULL_POINTER
 * @retval SWVENC_S_INVALID_STATE
 * @retval SWVENC_S_FAILURE
 */
SWVENC_STATUS swvenc_fillthisbuffer(SWVENC_HANDLE    swvenc,
                                    SWVENC_OPBUFFER *p_opbuffer);

/**
 * @brief Asynchronous API to flush input & output buffers.
 *
 * All input YUV buffers & output bitstream buffers with SwVenc are returned to
 * client via their respective callback functions. Completion of the flush
 * operation is notified to client via SWVENC_EVENT_FLUSH_DONE using callback
 * function pfn_swvenc_event_notification().
 *
 * @param[in] swvenc: SwVenc handle.
 *
 * @retval SWVENC_S_SUCCESS
 * @retval SWVENC_S_NULL_POINTER
 * @retval SWVENC_S_INVALID_STATE
 */
SWVENC_STATUS swvenc_flush(SWVENC_HANDLE swvenc);

/**
 * @brief Creates EGL session for rotation.
 *
 * @retval SWVENC_S_SUCCESS
 * @retval SWVENC_S_UNSUPPORTED
 */
SWVENC_STATUS swvenc_rotation_init();

/**
 * @brief Destroys EGL session for rotation.
 *
 * @retval SWVENC_S_SUCCESS
 */
SWVENC_STATUS swvenc_rotation_deinit();

/**
 * @brief Rotates a frame with a given angle.
 *
 *
 * @param[in] srcwidth : Width of buffer to rotate.
 * @param[in] srcheight : Height of buffer to rotate.
 * @param[in] dstheight : Height of rotated  buffer.
 * @param[in] dstwidth : Width of rotated  buffer
 * @param[in] rotation : Rotation angle with which we want to rotate.
 * @param[in] srcbuffer : Pointer to buffer to rotate.
 * @param[in] dstbuffer : Pointer to rotated buffer.
 *
 * @retval SWVENC_S_SUCCESS
 * @retval SWVENC_S_FAILURE
 */
SWVENC_STATUS swvenc_rotateFrame(uint32_t srcwidth,
                                 uint32_t srcheight,
                                 uint32_t dstheight,
                                 uint32_t dstwidth,
                                 int rotation,
                                 void* srcbuffer,
                                 void* dstbuffer);

#ifdef __cplusplus
} // closing brace for: extern "C" {
#endif

#endif // #ifndef _SWVENC_API_H_
