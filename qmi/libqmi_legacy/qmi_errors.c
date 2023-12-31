// -*- Mode: C -*-
//=============================================================================
// FILE: qmi_errors.c
//
// SERVICES:
//
// DESCRIPTION: interpret error codes for logging
//
// File generated by generate_errstr.pl version 1.0
//
//  Copyright (c) 2013,2020 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.
//=============================================================================

//=============================================================================
// INCLUDES
//=============================================================================
#include "qmi_errors.h"
#include <stdio.h> // snprintf

//=============================================================================
// FUNCTION: qmi_errstr
//
// DESCRIPTION: returns the string representation of error codes
//  for debug logging
//
// RETURN: string
//=============================================================================
//
const char* qmi_errstr(int error_code) {

static char unknown[] = "(0xFFFFFFFF) un-mapped";

   switch (error_code) {

      case QMI_NO_ERR:
         return "(0) QMI_NO_ERR";
         break;
      case QMI_INTERNAL_ERR:
         return "(-1) QMI_INTERNAL_ERR";
         break;
      case QMI_SERVICE_ERR:
         return "(-2) QMI_SERVICE_ERR";
         break;
      case QMI_TIMEOUT_ERR:
         return "(-3) QMI_TIMEOUT_ERR";
         break;
      case QMI_EXTENDED_ERR:
         return "(-4) QMI_EXTENDED_ERR";
         break;
      case QMI_PORT_NOT_OPEN_ERR:
         return "(-5) QMI_PORT_NOT_OPEN_ERR";
         break;
      case QMI_IDL_BUFFER_TOO_SMALL:
         return "(-6) QMI_IDL_BUFFER_TOO_SMALL";
         break;
      case QMI_IDL_ARRAY_TOO_BIG:
         return "(-7) QMI_IDL_ARRAY_TOO_BIG";
         break;
      case QMI_IDL_MESSAGE_ID_NOT_FOUND:
         return "(-8) QMI_IDL_MESSAGE_ID_NOT_FOUND";
         break;
      case QMI_IDL_TLV_DUPLICATED:
         return "(-9) QMI_IDL_TLV_DUPLICATED";
         break;
      case QMI_IDL_LENGTH_INCONSISTENCY:
         return "(-10) QMI_IDL_LENGTH_INCONSISTENCY";
         break;
      case QMI_IDL_MISSING_TLV:
         return "(-11) QMI_IDL_MISSING_TLV";
         break;
      case QMI_IDL_PARAMETER_ERROR:
         return "(-12) QMI_IDL_PARAMETER_ERROR";
         break;
      case QMI_IDL_UNRECOGNIZED_SERVICE_VERSION:
         return "(-13) QMI_IDL_UNRECOGNIZED_SERVICE_VERSION";
         break;
      case QMI_MEMCOPY_ERROR:
         return "(-14) QMI_MEMCOPY_ERROR";
         break;
      case QMI_INVALID_TXN:
         return "(-15) QMI_INVALID_TXN";
         break;
      case QMI_CLIENT_ALLOC_FAILURE:
         return "(-16) QMI_CLIENT_ALLOC_FAILURE";
         break;
      case QMI_IDL_UNKNOWN_MANDATORY_TLV:
         return "(-17) QMI_IDL_UNKNOWN_MANDATORY_TLV";
         break;
      case QMI_SERVICE_NOT_PRESENT_ON_MODEM:
         return "(-18) QMI_SERVICE_NOT_PRESENT_ON_MODEM";
         break;
      case QMI_IDL_TLV_RESP_ERROR:
         return "(-19) QMI_IDL_TLV_RESP_ERROR";
         break;
      case QMI_IDL_LIB_EXTENDED_ERR:
         return "(-40) QMI_IDL_LIB_EXTENDED_ERR";
         break;
      case QMI_IDL_LIB_BUFFER_TOO_SMALL:
         return "(-41) QMI_IDL_LIB_BUFFER_TOO_SMALL";
         break;
      case QMI_IDL_LIB_ARRAY_TOO_BIG:
         return "(-42) QMI_IDL_LIB_ARRAY_TOO_BIG";
         break;
      case QMI_IDL_LIB_MESSAGE_ID_NOT_FOUND:
         return "(-43) QMI_IDL_LIB_MESSAGE_ID_NOT_FOUND";
         break;
      case QMI_IDL_LIB_TLV_DUPLICATED:
         return "(-44) QMI_IDL_LIB_TLV_DUPLICATED";
         break;
      case QMI_IDL_LIB_LENGTH_INCONSISTENCY:
         return "(-45) QMI_IDL_LIB_LENGTH_INCONSISTENCY";
         break;
      case QMI_IDL_LIB_MISSING_TLV:
         return "(-46) QMI_IDL_LIB_MISSING_TLV";
         break;
      case QMI_IDL_LIB_PARAMETER_ERROR:
         return "(-47) QMI_IDL_LIB_PARAMETER_ERROR";
         break;
      case QMI_IDL_LIB_UNRECOGNIZED_SERVICE_VERSION:
         return "(-48) QMI_IDL_LIB_UNRECOGNIZED_SERVICE_VERSION";
         break;
      case QMI_IDL_LIB_UNKNOWN_MANDATORY_TLV:
         return "(-49) QMI_IDL_LIB_UNKNOWN_MANDATORY_TLV";
         break;
      case QMI_IDL_LIB_INCOMPATIBLE_SERVICE_VERSION:
         return "(-50) QMI_IDL_LIB_INCOMPATIBLE_SERVICE_VERSION";
         break;
      case QMI_IDL_LIB_RANGE_FAILURE:
         return "(-51) QMI_IDL_LIB_INCOMPATIBLE_SERVICE_VERSION";
         break;
      default: {
         snprintf(unknown, sizeof(unknown), "(0x%04X) un-mapped", error_code);
         return unknown;
         break;
      }
   }
}

//=============================================================================
// FUNCTION: qmisvc_errstr
//
// DESCRIPTION: returns the string representation of error codes
//  for debug logging
//
// RETURN: string
//=============================================================================
//
const char* qmisvc_errstr(int error_code) {

static char unknown[] = "(0xFFFFFFFF) un-mapped";

   switch (error_code) {

      case QMI_ERR_NONE_V01:
         return "QMI_ERR_NONE";
         break;
      case QMI_ERR_MALFORMED_MSG_V01:
         return "QMI_ERR_MALFORMED_MSG";
         break;
      case QMI_ERR_NO_MEMORY_V01:
         return "QMI_ERR_NO_MEMORY";
         break;
      case QMI_ERR_INTERNAL_V01:
         return "QMI_ERR_INTERNAL";
         break;
      case QMI_ERR_ABORTED_V01:
         return "QMI_ERR_ABORTED";
         break;
      case QMI_ERR_CLIENT_IDS_EXHAUSTED_V01:
         return "QMI_ERR_CLIENT_IDS_EXHAUSTED";
         break;
      case QMI_ERR_UNABORTABLE_TRANSACTION_V01:
         return "QMI_ERR_UNABORTABLE_TRANSACTION";
         break;
      case QMI_ERR_INVALID_CLIENT_ID_V01:
         return "QMI_ERR_INVALID_CLIENT_ID";
         break;
      case QMI_ERR_NO_THRESHOLDS_V01:
         return "QMI_ERR_NO_THRESHOLDS";
         break;
      case QMI_ERR_INVALID_HANDLE_V01:
         return "QMI_ERR_INVALID_HANDLE";
         break;
      case QMI_ERR_INVALID_PROFILE_V01:
         return "QMI_ERR_INVALID_PROFILE";
         break;
      case QMI_ERR_INVALID_PINID_V01:
         return "QMI_ERR_INVALID_PINID";
         break;
      case QMI_ERR_INCORRECT_PIN_V01:
         return "QMI_ERR_INCORRECT_PIN";
         break;
      case QMI_ERR_NO_NETWORK_FOUND_V01:
         return "QMI_ERR_NO_NETWORK_FOUND";
         break;
      case QMI_ERR_CALL_FAILED_V01:
         return "QMI_ERR_CALL_FAILED";
         break;
      case QMI_ERR_OUT_OF_CALL_V01:
         return "QMI_ERR_OUT_OF_CALL";
         break;
      case QMI_ERR_NOT_PROVISIONED_V01:
         return "QMI_ERR_NOT_PROVISIONED";
         break;
      case QMI_ERR_MISSING_ARG_V01:
         return "QMI_ERR_MISSING_ARG";
         break;
      case QMI_ERR_ARG_TOO_LONG_V01:
         return "QMI_ERR_ARG_TOO_LONG";
         break;
      case QMI_ERR_INVALID_TX_ID_V01:
         return "QMI_ERR_INVALID_TX_ID";
         break;
      case QMI_ERR_DEVICE_IN_USE_V01:
         return "QMI_ERR_DEVICE_IN_USE";
         break;
      case QMI_ERR_OP_NETWORK_UNSUPPORTED_V01:
         return "QMI_ERR_OP_NETWORK_UNSUPPORTED";
         break;
      case QMI_ERR_OP_DEVICE_UNSUPPORTED_V01:
         return "QMI_ERR_OP_DEVICE_UNSUPPORTED";
         break;
      case QMI_ERR_NO_EFFECT_V01:
         return "QMI_ERR_NO_EFFECT";
         break;
      case QMI_ERR_NO_FREE_PROFILE_V01:
         return "QMI_ERR_NO_FREE_PROFILE";
         break;
      case QMI_ERR_INVALID_PDP_TYPE_V01:
         return "QMI_ERR_INVALID_PDP_TYPE";
         break;
      case QMI_ERR_INVALID_TECH_PREF_V01:
         return "QMI_ERR_INVALID_TECH_PREF";
         break;
      case QMI_ERR_INVALID_PROFILE_TYPE_V01:
         return "QMI_ERR_INVALID_PROFILE_TYPE";
         break;
      case QMI_ERR_INVALID_SERVICE_TYPE_V01:
         return "QMI_ERR_INVALID_SERVICE_TYPE";
         break;
      case QMI_ERR_INVALID_REGISTER_ACTION_V01:
         return "QMI_ERR_INVALID_REGISTER_ACTION";
         break;
      case QMI_ERR_INVALID_PS_ATTACH_ACTION_V01:
         return "QMI_ERR_INVALID_PS_ATTACH_ACTION";
         break;
      case QMI_ERR_AUTHENTICATION_FAILED_V01:
         return "QMI_ERR_AUTHENTICATION_FAILED";
         break;
      case QMI_ERR_PIN_BLOCKED_V01:
         return "QMI_ERR_PIN_BLOCKED";
         break;
      case QMI_ERR_PIN_PERM_BLOCKED_V01:
         return "QMI_ERR_PIN_PERM_BLOCKED";
         break;
      case QMI_ERR_SIM_NOT_INITIALIZED_V01:
         return "QMI_ERR_SIM_NOT_INITIALIZED";
         break;
      case QMI_ERR_MAX_QOS_REQUESTS_IN_USE_V01:
         return "QMI_ERR_MAX_QOS_REQUESTS_IN_USE";
         break;
      case QMI_ERR_INCORRECT_FLOW_FILTER_V01:
         return "QMI_ERR_INCORRECT_FLOW_FILTER";
         break;
      case QMI_ERR_NETWORK_QOS_UNAWARE_V01:
         return "QMI_ERR_NETWORK_QOS_UNAWARE";
         break;
      case QMI_ERR_INVALID_ID_V01:
         return "QMI_ERR_INVALID_ID/QOS_ID";
         break;
      case QMI_ERR_REQUESTED_NUM_UNSUPPORTED_V01:
         return "QMI_ERR_REQUESTED_NUM_UNSUPPORTED";
         break;
      case QMI_ERR_INTERFACE_NOT_FOUND_V01:
         return "QMI_ERR_INTERFACE_NOT_FOUND";
         break;
      case QMI_ERR_FLOW_SUSPENDED_V01:
         return "QMI_ERR_FLOW_SUSPENDED";
         break;
      case QMI_ERR_INVALID_DATA_FORMAT_V01:
         return "QMI_ERR_INVALID_DATA_FORMAT";
         break;
      case QMI_ERR_GENERAL_V01:
         return "QMI_ERR_GENERAL";
         break;
      case QMI_ERR_UNKNOWN_V01:
         return "QMI_ERR_UNKNOWN";
         break;
      case QMI_ERR_INVALID_ARG_V01:
         return "QMI_ERR_INVALID_ARG";
         break;
      case QMI_ERR_INVALID_INDEX_V01:
         return "QMI_ERR_INVALID_INDEX";
         break;
      case QMI_ERR_NO_ENTRY_V01:
         return "QMI_ERR_NO_ENTRY";
         break;
      case QMI_ERR_DEVICE_STORAGE_FULL_V01:
         return "QMI_ERR_DEVICE_STORAGE_FULL";
         break;
      case QMI_ERR_DEVICE_NOT_READY_V01:
         return "QMI_ERR_DEVICE_NOT_READY";
         break;
      case QMI_ERR_NETWORK_NOT_READY_V01:
         return "QMI_ERR_NETWORK_NOT_READY";
         break;
      case QMI_ERR_CAUSE_CODE_V01:
         return "QMI_ERR_CAUSE_CODE";
         break;
      case QMI_ERR_MESSAGE_NOT_SENT_V01:
         return "QMI_ERR_MESSAGE_NOT_SENT";
         break;
      case QMI_ERR_MESSAGE_DELIVERY_FAILURE_V01:
         return "QMI_ERR_MESSAGE_DELIVERY_FAILURE";
         break;
      case QMI_ERR_INVALID_MESSAGE_ID_V01:
         return "QMI_ERR_INVALID_MESSAGE_ID";
         break;
      case QMI_ERR_ENCODING_V01:
         return "QMI_ERR_ENCODING";
         break;
      case QMI_ERR_AUTHENTICATION_LOCK_V01:
         return "QMI_ERR_AUTHENTICATION_LOCK";
         break;
      case QMI_ERR_INVALID_TRANSITION_V01:
         return "QMI_ERR_INVALID_TRANSITION";
         break;
      case QMI_ERR_NOT_A_MCAST_IFACE_V01:
         return "QMI_ERR_NOT_A_MCAST_IFACE";
         break;
      case QMI_ERR_MAX_MCAST_REQUESTS_IN_USE_V01:
         return "QMI_ERR_MAX_MCAST_REQUESTS_IN_USE";
         break;
      case QMI_ERR_INVALID_MCAST_HANDLE_V01:
         return "QMI_ERR_INVALID_MCAST_HANDLE";
         break;
      case QMI_ERR_INVALID_IP_FAMILY_PREF_V01:
         return "QMI_ERR_INVALID_IP_FAMILY_PREF";
         break;
      case QMI_ERR_SESSION_INACTIVE_V01:
         return "QMI_ERR_SESSION_INACTIVE";
         break;
      case QMI_ERR_SESSION_INVALID_V01:
         return "QMI_ERR_SESSION_INVALID";
         break;
      case QMI_ERR_SESSION_OWNERSHIP_V01:
         return "QMI_ERR_SESSION_OWNERSHIP";
         break;
      case QMI_ERR_INSUFFICIENT_RESOURCES_V01:
         return "QMI_ERR_INSUFFICIENT_RESOURCES";
         break;
      case QMI_ERR_DISABLED_V01:
         return "QMI_ERR_DISABLED";
         break;
      case QMI_ERR_INVALID_OPERATION_V01:
         return "QMI_ERR_INVALID_OPERATION";
         break;
      case QMI_ERR_INVALID_QMI_CMD_V01:
         return "QMI_ERR_INVALID_QMI_CMD";
         break;
      case QMI_ERR_TPDU_TYPE_V01:
         return "QMI_ERR_TPDU_TYPE";
         break;
      case QMI_ERR_SMSC_ADDR_V01:
         return "QMI_ERR_SMSC_ADDR";
         break;
      case QMI_ERR_INFO_UNAVAILABLE_V01:
         return "QMI_ERR_INFO_UNAVAILABLE";
         break;
      case QMI_ERR_SEGMENT_TOO_LONG_V01:
         return "QMI_ERR_SEGMENT_TOO_LONG";
         break;
      case QMI_ERR_SEGMENT_ORDER_V01:
         return "QMI_ERR_SEGMENT_ORDER";
         break;
      case QMI_ERR_BUNDLING_NOT_SUPPORTED_V01:
         return "QMI_ERR_BUNDLING_NOT_SUPPORTED";
         break;
      case QMI_ERR_OP_PARTIAL_FAILURE_V01:
         return "QMI_ERR_OP_PARTIAL_FAILURE";
         break;
      case QMI_ERR_POLICY_MISMATCH_V01:
         return "QMI_ERR_POLICY_MISMATCH";
         break;
      case QMI_ERR_SIM_FILE_NOT_FOUND_V01:
         return "QMI_ERR_SIM_FILE_NOT_FOUND";
         break;
      case QMI_ERR_EXTENDED_INTERNAL_V01:
         return "QMI_ERR_EXTENDED_INTERNAL";
         break;
      case QMI_ERR_ACCESS_DENIED_V01:
         return "QMI_ERR_ACCESS_DENIED";
         break;
      case QMI_ERR_HARDWARE_RESTRICTED_V01:
         return "QMI_ERR_HARDWARE_RESTRICTED";
         break;
      case QMI_ERR_ACK_NOT_SENT_V01:
         return "QMI_ERR_ACK_NOT_SENT";
         break;
      case QMI_ERR_INJECT_TIMEOUT_V01:
         return "QMI_ERR_INJECT_TIMEOUT";
         break;
      case QMI_ERR_INCOMPATIBLE_STATE_V01:
         return "QMI_ERR_INCOMPATIBLE_STATE";
         break;
      case QMI_ERR_FDN_RESTRICT_V01:
         return "QMI_ERR_FDN_RESTRICT";
         break;
      case QMI_ERR_SUPS_FAILURE_CAUSE_V01:
         return "QMI_ERR_SUPS_FAILURE_CAUSE";
         break;
      case QMI_ERR_NO_RADIO_V01:
         return "QMI_ERR_NO_RADIO";
         break;
      case QMI_ERR_NOT_SUPPORTED_V01:
         return "QMI_ERR_NOT_SUPPORTED";
         break;
      case QMI_ERR_NO_SUBSCRIPTION_V01:
         return "QMI_ERR_NO_SUBSCRIPTION";
         break;
      case QMI_ERR_CARD_CALL_CONTROL_FAILED_V01:
         return "QMI_ERR_CARD_CALL_CONTROL_FAILED";
         break;
      case QMI_ERR_NETWORK_ABORTED_V01:
         return "QMI_ERR_NETWORK_ABORTED";
         break;
      case QMI_ERR_MSG_BLOCKED_V01:
         return "QMI_ERR_MSG_BLOCKED";
         break;
      case QMI_ERR_INVALID_SESSION_TYPE_V01:
         return "QMI_ERR_INVALID_SESSION_TYPE";
         break;
      case QMI_ERR_INVALID_PB_TYPE_V01:
         return "QMI_ERR_INVALID_PB_TYPE";
         break;
      case QMI_ERR_NO_SIM_V01:
         return "QMI_ERR_NO_SIM";
         break;
      case QMI_ERR_PB_NOT_READY_V01:
         return "QMI_ERR_PB_NOT_READY";
         break;
      case QMI_ERR_PIN_RESTRICTION_V01:
         return "QMI_ERR_PIN_RESTRICTION";
         break;
      case QMI_ERR_PIN2_RESTRICTION_V01:
         return "QMI_ERR_PIN2_RESTRICTION";
         break;
      case QMI_ERR_PUK_RESTRICTION_V01:
         return "QMI_ERR_PUK_RESTRICTION";
         break;
      case QMI_ERR_PUK2_RESTRICTION_V01:
         return "QMI_ERR_PUK2_RESTRICTION";
         break;
      case QMI_ERR_PB_ACCESS_RESTRICTED_V01:
         return "QMI_ERR_PB_ACCESS_RESTRICTED";
         break;
      case QMI_ERR_PB_DELETE_IN_PROG_V01:
         return "QMI_ERR_PB_DELETE_IN_PROG";
         break;
      case QMI_ERR_PB_TEXT_TOO_LONG_V01:
         return "QMI_ERR_PB_TEXT_TOO_LONG";
         break;
      case QMI_ERR_PB_NUMBER_TOO_LONG_V01:
         return "QMI_ERR_PB_NUMBER_TOO_LONG";
         break;
      case QMI_ERR_PB_HIDDEN_KEY_RESTRICTION_V01:
         return "QMI_ERR_PB_HIDDEN_KEY_RESTRICTION";
         break;
      default: {
         snprintf(unknown, sizeof(unknown), "(0x%04X) un-mapped", error_code);
         return unknown;
         break;
      }
   }
}

