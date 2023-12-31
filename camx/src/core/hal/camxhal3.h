////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  camxhal3.h
/// @brief Landing methods for CamX implementation of HAL3
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// NOWHINE FILE GR017: Google types
// NOWHINE FILE NC011: Google calls it 'init'

#ifndef CAMXHAL3_H
#define CAMXHAL3_H

#include <hardware/camera3.h>

#include "camxdefs.h"
#include "chi.h"

CAMX_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Jump table for all HAL3 entry points
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct JumpTableHAL3
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // hw_module_methods_t entry points
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int (*open)(
        const struct hw_module_t*,
        const char*,
        struct hw_device_t**);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // camera_module_t entry points
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int (*get_number_of_cameras)(void);

    int (*get_camera_info)(
        int,
        struct camera_info*);

    int (*set_callbacks)(
        const camera_module_callbacks_t*);

    void (*get_vendor_tag_ops)(
        vendor_tag_ops_t*);

    int (*open_legacy)(
        const struct hw_module_t*,
        const char*,
        uint32_t,
        struct hw_device_t**);

    int (*set_torch_mode)(
        const char*,
        bool);

    int (*init)();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // vendor_tag_ops_t entry points
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int (*get_tag_count)(
        const vendor_tag_ops_t*);

    void (*get_all_tags)(
        const vendor_tag_ops_t*,
        uint32_t*);

    const char* (*get_section_name)(
        const vendor_tag_ops_t*,
        uint32_t);

    const char* (*get_tag_name)(
        const vendor_tag_ops_t*,
        uint32_t);

    int (*get_tag_type)(
        const vendor_tag_ops_t*,
        uint32_t);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // hw_device_t entry points
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int (*close)(
        struct hw_device_t*);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // camera3_device_ops_t entry points
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int (*initialize)(
        const struct camera3_device*,
        const camera3_callback_ops_t*);

    int (*configure_streams)(
        const struct camera3_device*,
        camera3_stream_configuration_t*);

    const camera_metadata_t* (*construct_default_request_settings)(
        const struct camera3_device*,
        int);

    int (*process_capture_request)(
        const struct camera3_device*,
        camera3_capture_request_t*);

    void (*dump)(
        const struct camera3_device*,
        int);

    int (*flush)(
        const struct camera3_device*);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // camera_module_callbacks_t exit points
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void (*camera_device_status_change)(
        const struct camera_module_callbacks*,
        int,
        int);

    void (*torch_mode_status_change)(
        const struct camera_module_callbacks*,
        const char*,
        int);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // camera3_callback_ops_t exit points
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void (*process_capture_result)(
        const struct camera3_callback_ops*,
        const camera3_capture_result_t*);

    void (*notify)(
        const struct camera3_callback_ops*,
        const camera3_notify_msg_t*);
};

extern JumpTableHAL3 g_jumpTableHAL3;       ///< Global driver HAL3 entry point jump table

/// @brief Sample code for using ExtendOpen and ModifySettings
/// NOWHINE DC002a : Sample code
void GenerateExtendOpenData(
    UINT32              numTokens,
    CHIEXTENDSETTINGS*  pToken);
/// NOWHINE DC002a : Sample code
void GenerateExtendCloseData(
    UINT32             numTokens,
    CHIEXTENDSETTINGS* pToken);
/// @brief Sample code for using ExtendOpen and ModifySettings
/// NOWHINE DC002a : Sample code
void GenerateModifySettingsData(
    ChiModifySettings* pToken);

CAMX_NAMESPACE_END

#endif // CAMXHAL3_H
