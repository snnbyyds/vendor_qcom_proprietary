/******************************************************************************
#  Copyright (c) 2016 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    LteDirectDiscovery.pb.h
  @brief   lte direct discovery interface nano pb file

  DESCRIPTION
    lte direct discovery interface nano pb file generated
    from LteDirectDiscovery.proto
  ---------------------------------------------------------------------------
******************************************************************************/

/* Automatically generated nanopb header */
/* Generated by nanopb-0.2.8-dev at Thu Mar 17 13:00:49 2016. */

#ifndef _PB_LTEDIRECTDISCOVERY_PB_H_
#define _PB_LTEDIRECTDISCOVERY_PB_H_
#include <pb.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Enum definitions */
typedef enum _LteDirectDiscovery_MsgType {
    LteDirectDiscovery_MsgType_UNKNOWN = 0,
    LteDirectDiscovery_MsgType_REQUEST = 1,
    LteDirectDiscovery_MsgType_RESPONSE = 2,
    LteDirectDiscovery_MsgType_UNSOL_RESPONSE = 3
} LteDirectDiscovery_MsgType;

typedef enum _LteDirectDiscovery_Error {
    LteDirectDiscovery_Error_E_SUCCESS = 0,
    LteDirectDiscovery_Error_E_RADIO_NOT_AVAILABLE = 1,
    LteDirectDiscovery_Error_E_GENERIC_FAILURE = 2,
    LteDirectDiscovery_Error_E_NOT_SUPPORTED = 3,
    LteDirectDiscovery_Error_E_APP_AUTH_FAILURE = 4
} LteDirectDiscovery_Error;

typedef enum _LteDirectDiscovery_MsgId {
    LteDirectDiscovery_MsgId_REQUEST_BASE = 0,
    LteDirectDiscovery_MsgId_REQUEST_INITIALIZE = 1,
    LteDirectDiscovery_MsgId_REQUEST_GET_DEVICE_CAPABILITY = 2,
    LteDirectDiscovery_MsgId_REQUEST_TERMINATE = 3,
    LteDirectDiscovery_MsgId_REQUEST_GET_SERVICE_STATUS = 4,
    LteDirectDiscovery_MsgId_REQUEST_PUBLISH = 5,
    LteDirectDiscovery_MsgId_REQUEST_CANCEL_PUBLISH = 6,
    LteDirectDiscovery_MsgId_REQUEST_SUBSCRIBE = 7,
    LteDirectDiscovery_MsgId_REQUEST_CANCEL_SUBSCRIBE = 8,
    LteDirectDiscovery_MsgId_UNSOL_RSP_BASE = 1000,
    LteDirectDiscovery_MsgId_UNSOL_RESPONSE_DEVICE_CAPABILITY_CHANGED = 1001,
    LteDirectDiscovery_MsgId_UNSOL_RESPONSE_SERVICE_STATUS = 1002,
    LteDirectDiscovery_MsgId_UNSOL_RESPONSE_AUTHORIZATION_RESULT = 1003,
    LteDirectDiscovery_MsgId_UNSOL_RESPONSE_EXPRESSION_STATUS = 1004,
    LteDirectDiscovery_MsgId_UNSOL_RESPONSE_TRANSMISSION_STATUS = 1005,
    LteDirectDiscovery_MsgId_UNSOL_RESPONSE_RECEPTION_STATUS = 1006,
    LteDirectDiscovery_MsgId_UNSOL_RESPONSE_MATCH_EVENT = 1007
} LteDirectDiscovery_MsgId;

typedef enum _LteDirectDiscovery_DiscoveryType {
    LteDirectDiscovery_DiscoveryType_INVALID = 0,
    LteDirectDiscovery_DiscoveryType_OPEN = 1,
    LteDirectDiscovery_DiscoveryType_RESTRICTED = 2
} LteDirectDiscovery_DiscoveryType;

typedef enum _LteDirectDiscovery_Result {
    LteDirectDiscovery_Result_SUCCESS = 0,
    LteDirectDiscovery_Result_GENERIC_FAILURE = 1,
    LteDirectDiscovery_Result_IN_PROGRESS = 2,
    LteDirectDiscovery_Result_INVALID_EXPRESSION_SCOPE = 3,
    LteDirectDiscovery_Result_UNKNOWN_EXPRESSION = 4,
    LteDirectDiscovery_Result_INVALID_DISCOVERY_TYPE = 5,
    LteDirectDiscovery_Result_SERVICE_NOT_AVAILABLE = 6,
    LteDirectDiscovery_Result_APP_AUTH_FAILURE = 7,
    LteDirectDiscovery_Result_NOT_SUPPORTED = 8
} LteDirectDiscovery_Result;

/* Struct definitions */
typedef struct _LteDirectDiscovery_AuthorizationResult {
    pb_callback_t osAppId;
    bool has_result;
    LteDirectDiscovery_Result result;
} LteDirectDiscovery_AuthorizationResult;

typedef struct _LteDirectDiscovery_CancelPublish {
    pb_callback_t osAppId;
    pb_callback_t expression;
} LteDirectDiscovery_CancelPublish;

typedef struct _LteDirectDiscovery_CancelSubscribe {
    pb_callback_t osAppId;
    pb_callback_t expression;
} LteDirectDiscovery_CancelSubscribe;

typedef struct _LteDirectDiscovery_DeviceCapability {
    bool has_capability;
    uint32_t capability;
} LteDirectDiscovery_DeviceCapability;

typedef struct _LteDirectDiscovery_ExpressionStatus {
    pb_callback_t osAppId;
    pb_callback_t expression;
    bool has_result;
    LteDirectDiscovery_Result result;
} LteDirectDiscovery_ExpressionStatus;

typedef struct _LteDirectDiscovery_MatchEvent {
    pb_callback_t osAppId;
    pb_callback_t expression;
    pb_callback_t matchedExpression;
    bool has_state;
    uint32_t state;
    bool has_metaDataIndex;
    uint32_t metaDataIndex;
    pb_callback_t metaData;
} LteDirectDiscovery_MatchEvent;

typedef struct _LteDirectDiscovery_MsgHeader {
    uint32_t token;
    LteDirectDiscovery_MsgType type;
    LteDirectDiscovery_MsgId id;
    LteDirectDiscovery_Error error;
    pb_callback_t payload;
} LteDirectDiscovery_MsgHeader;

typedef struct _LteDirectDiscovery_Publish {
    pb_callback_t osAppId;
    pb_callback_t expression;
    bool has_expressionValidityTime;
    uint32_t expressionValidityTime;
    pb_callback_t metaData;
    bool has_discoveryType;
    LteDirectDiscovery_DiscoveryType discoveryType;
    bool has_duration;
    uint32_t duration;
} LteDirectDiscovery_Publish;

typedef struct _LteDirectDiscovery_ReceptionStatus {
    pb_callback_t osAppId;
    pb_callback_t expression;
    bool has_status;
    uint32_t status;
} LteDirectDiscovery_ReceptionStatus;

typedef struct _LteDirectDiscovery_ServiceStatus {
    bool has_publishAllowed;
    uint32_t publishAllowed;
    bool has_subscribeAllowed;
    uint32_t subscribeAllowed;
} LteDirectDiscovery_ServiceStatus;

typedef struct _LteDirectDiscovery_Subscribe {
    pb_callback_t osAppId;
    pb_callback_t expression;
    bool has_expressionValidityTime;
    uint32_t expressionValidityTime;
    bool has_discoveryType;
    LteDirectDiscovery_DiscoveryType discoveryType;
    bool has_duration;
    uint32_t duration;
} LteDirectDiscovery_Subscribe;

typedef struct _LteDirectDiscovery_Terminate {
    pb_callback_t osAppId;
} LteDirectDiscovery_Terminate;

typedef struct _LteDirectDiscovery_TransmissionStatus {
    pb_callback_t osAppId;
    pb_callback_t expression;
    bool has_status;
    uint32_t status;
} LteDirectDiscovery_TransmissionStatus;

/* Default values for struct fields */

/* Field tags (for use in manual encoding/decoding) */
#define LteDirectDiscovery_AuthorizationResult_osAppId_tag 1
#define LteDirectDiscovery_AuthorizationResult_result_tag 2
#define LteDirectDiscovery_CancelPublish_osAppId_tag 1
#define LteDirectDiscovery_CancelPublish_expression_tag 2
#define LteDirectDiscovery_CancelSubscribe_osAppId_tag 1
#define LteDirectDiscovery_CancelSubscribe_expression_tag 2
#define LteDirectDiscovery_DeviceCapability_capability_tag 2
#define LteDirectDiscovery_ExpressionStatus_osAppId_tag 1
#define LteDirectDiscovery_ExpressionStatus_expression_tag 2
#define LteDirectDiscovery_ExpressionStatus_result_tag 3
#define LteDirectDiscovery_MatchEvent_osAppId_tag 1
#define LteDirectDiscovery_MatchEvent_expression_tag 2
#define LteDirectDiscovery_MatchEvent_matchedExpression_tag 3
#define LteDirectDiscovery_MatchEvent_state_tag  4
#define LteDirectDiscovery_MatchEvent_metaDataIndex_tag 5
#define LteDirectDiscovery_MatchEvent_metaData_tag 6
#define LteDirectDiscovery_MsgHeader_token_tag   1
#define LteDirectDiscovery_MsgHeader_type_tag    2
#define LteDirectDiscovery_MsgHeader_id_tag      3
#define LteDirectDiscovery_MsgHeader_error_tag   4
#define LteDirectDiscovery_MsgHeader_payload_tag 5
#define LteDirectDiscovery_Publish_osAppId_tag   1
#define LteDirectDiscovery_Publish_expression_tag 2
#define LteDirectDiscovery_Publish_expressionValidityTime_tag 3
#define LteDirectDiscovery_Publish_metaData_tag  4
#define LteDirectDiscovery_Publish_discoveryType_tag 5
#define LteDirectDiscovery_Publish_duration_tag  6
#define LteDirectDiscovery_ReceptionStatus_osAppId_tag 1
#define LteDirectDiscovery_ReceptionStatus_expression_tag 2
#define LteDirectDiscovery_ReceptionStatus_status_tag 3
#define LteDirectDiscovery_ServiceStatus_publishAllowed_tag 2
#define LteDirectDiscovery_ServiceStatus_subscribeAllowed_tag 3
#define LteDirectDiscovery_Subscribe_osAppId_tag 1
#define LteDirectDiscovery_Subscribe_expression_tag 2
#define LteDirectDiscovery_Subscribe_expressionValidityTime_tag 3
#define LteDirectDiscovery_Subscribe_discoveryType_tag 4
#define LteDirectDiscovery_Subscribe_duration_tag 5
#define LteDirectDiscovery_Terminate_osAppId_tag 1
#define LteDirectDiscovery_TransmissionStatus_osAppId_tag 1
#define LteDirectDiscovery_TransmissionStatus_expression_tag 2
#define LteDirectDiscovery_TransmissionStatus_status_tag 3

/* Struct field encoding specification for nanopb */
extern const pb_field_t LteDirectDiscovery_MsgHeader_fields[6];
extern const pb_field_t LteDirectDiscovery_DeviceCapability_fields[2];
extern const pb_field_t LteDirectDiscovery_Terminate_fields[2];
extern const pb_field_t LteDirectDiscovery_ServiceStatus_fields[3];
extern const pb_field_t LteDirectDiscovery_Publish_fields[7];
extern const pb_field_t LteDirectDiscovery_CancelPublish_fields[3];
extern const pb_field_t LteDirectDiscovery_Subscribe_fields[6];
extern const pb_field_t LteDirectDiscovery_CancelSubscribe_fields[3];
extern const pb_field_t LteDirectDiscovery_AuthorizationResult_fields[3];
extern const pb_field_t LteDirectDiscovery_ExpressionStatus_fields[4];
extern const pb_field_t LteDirectDiscovery_TransmissionStatus_fields[4];
extern const pb_field_t LteDirectDiscovery_ReceptionStatus_fields[4];
extern const pb_field_t LteDirectDiscovery_MatchEvent_fields[7];

/* Maximum encoded size of messages (where known) */
#define LteDirectDiscovery_DeviceCapability_size 5
#define LteDirectDiscovery_ServiceStatus_size    10

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif