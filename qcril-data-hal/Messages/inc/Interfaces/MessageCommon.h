/*===========================================================================

  Copyright (c) 2018-2020, 2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#include <string>
#include <vector>
#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"
#include "network_access_service_v01.h"

#define KEEPALIVE_REFACTOR
#define HAS_QCRIL_DATA_1_5_RESPONSE_TYPES

#ifndef MESSAGECOMMON
#define MESSAGECOMMON


namespace rildata {

enum KeepaliveStatusCode : int {
  ACTIVE,
  INACTIVE,
  PENDING,
};

enum class RequestSource_t {
  RADIO = 0,
  IWLAN = 1,
};

enum class ResponseError_t : int32_t {
    NO_ERROR = 0,
    NOT_SUPPORTED,
    NOT_AVAILABLE,
    CALL_NOT_AVAILABLE,
    INTERNAL_ERROR,
    INVALID_ARGUMENT,
};

enum class RatType_t : int32_t {
    RAT_UNSPECIFIED = 0,
    RAT_4G,
    RAT_5G,
    RAT_SPLITED,
};
inline std::ostream& operator<<(std::ostream& os, RatType_t r);

enum class NrIconEnum_t : int {
    NONE = 0,
    BASIC,
    UWB
};

enum class ApAssistHandoverFailCause_t : int32_t {
  NONE = 0,
  TIMEOUT,
  APN_PREF_SYSTEM_CHANGE_REQUEST_FAILURE,
  HANDOFF_FAILURE,
};

enum class DataCallFailCause_t  : int32_t {
  NONE = 0,

  /* 3GPP specification defined call end reasons (Type = 6) */
  OPERATOR_BARRED = 0x08, // OPERATOR_DETERMINED_BARRING (8)
  NAS_SIGNALLING = 0x0E, // NAS_SIGNALLING_ERROR (14)
  LLC_SNDCP = 0x19, // LLC_SNDCP_FAILURE (25)
  INSUFFICIENT_RESOURCES = 0x1A, // INSUFFICIENT_RESOURCES (26)
  MISSING_UKNOWN_APN = 0x1B, // UNKNOWN_APN (27)
  UNKNOWN_PDP_ADDRESS_TYPE = 0x1C, // UNKNOWN_PDP (28)
  USER_AUTHENTICATION = 0x1D, // AUTH_FAILED (29)
  ACTIVATION_REJECT_GGSN = 0x1E, // GGSN_REJECT (30)
  ACTIVATION_REJECT_UNSPECIFIED = 0x1F, // ACTIVATION_REJECT (31)
  SERVICE_OPTION_NOT_SUPPORTED = 0x20, // OPTION_NOT_SUPPORTED (32)
  SERVICE_OPTION_NOT_SUBSCRIBED = 0x21, // OPTION_UNSUBSCRIBED (33)
  SERVICE_OPTION_OUT_OF_ORDER = 0x22, // OPTION_TEMP_OOO (34)
  NSAPI_IN_USE = 0x23, // NSAPI_ALREADY_USED (35)
  REGULAR_DEACTIVATION = 0x24, // REGULAR_DEACTIVATION (36)
  QOS_NOT_ACCEPTED = 0x25, // QOS_NOT_ACCEPTED (37)
  NETWORK_FAILURE = 0x26, // NETWORK_FAILURE (38)
  UMTS_REACTIVATION_REQ = 0x27, // UMTS_REACTIVATION_REQ (39)
  FEATURE_NOT_SUPP = 0x28, // FEATURE_NOT_SUPPORTED (40)
  TFT_SEMANTIC_ERROR = 0x29, // TFT_SEMANTIC_ERROR (41)
  TFT_SYTAX_ERROR = 0x2A, // TFT_SYNTAX_ERROR (42)
  UNKNOWN_PDP_CONTEXT = 0x2B, // UNKNOWN_PDP_CONTEXT (43)
  FILTER_SEMANTIC_ERROR = 0x2C, // FILTER_SEMANTIC_ERROR (44)
  FILTER_SYTAX_ERROR = 0x2D, // FILTER_SYNTAX_ERROR (45)
  PDP_WITHOUT_ACTIVE_TFT = 0x2E, // PDP_WITHOUT_ACTIVE_TFT (46)
  ACTIVATION_REJECTED_BCM_VIOLATION = 0x30, // ACTIVATION_REJECTED_BCM_VIOLATION (48)
  ONLY_IPV4_ALLOWED = 0x32, // IP_V4_ONLY_ALLOWED (50)
  ONLY_IPV6_ALLOWED = 0x33, // IP_V6_ONLY_ALLOWED (51)
  ONLY_SINGLE_BEARER_ALLOWED = 0x34, // SINGLE_ADDR_BEARER_ONLY (52)
  ESM_INFO_NOT_RECEIVED = 0x35, // ESM_INFO_NOT_RECEIVED (53)
  PDN_CONN_DOES_NOT_EXIST = 0x36, // PDN_CONN_DOES_NOT_EXIST (54)
  MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED = 0x37, // MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED (55)
  COLLISION_WITH_NETWORK_INITIATED_REQUEST = 0x38, // COLLISION_WITH_NW_INIT_REQ (56)
  ONLY_IPV4V6_ALLOWED = 0x39, // IP_V4V6_ONLY_ALLOWED (57)
  ONLY_NON_IP_ALLOWED = 0x3A, // NON_IP_ONLY_ALLOWED (58)
  UNSUPPORTED_QCI_VALUE = 0x3B, // UNSUPPORTED_QCI_VALUE (59)
  BEARER_HANDLING_NOT_SUPPORTED = 0x3C, // BEARER_HANDLING_NOT_SUPPORTED (60)
  MAX_ACTIVE_PDP_CONTEXT_REACHED = 0x41, // MAX_ACTIVE_PDP_CONTEXT_REACHED (65)
  UNSUPPORTED_APN_IN_CURRENT_PLMN = 0x42, // UNSUPPORTED_APN_IN_CURRENT_PLMN (66)
  INVALID_TRANSACTION_ID = 0x51, // INVALID_TRANSACTION_ID (81)
  MESSAGE_INCORRECT_SEMANTIC = 0x5F, // MESSAGE_INCORRECT_SEMANTIC (95)
  INVALID_MANDATORY_INFO = 0x60, // INVALID_MANDATORY_INFO (96)
  MESSAGE_TYPE_UNSUPPORTED = 0x61, // MESSAGE_TYPE_UNSUPPORTED (97)
  MSG_TYPE_NONCOMPATIBLE_STATE = 0x62, // MSG_TYPE_NONCOMPATIBLE_STATE (98)
  UNKNOWN_INFO_ELEMENT = 0x63, // UNKNOWN_INFO_ELEMENT (99)
  CONDITIONAL_IE_ERROR = 0x64, // CONDITIONAL_IE_ERROR (100)
  MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE = 0x65, // MSG_AND_PROTOCOL_STATE_INCOMPATIBLE (101)
  PROTOCOL_ERRORS = 0x6F, // PROTOCOL_ERROR (111)
  APN_TYPE_CONFLICT = 0x70, // APN_TYPE_CONFLICT (112)
  INVALID_PCSCF_ADDR = 0x71, // INVALID_PCSCF_ADDRESS (113)
  INTERNAL_CALL_PREEMPT_BY_HIGH_PRIO_APN = 0x72, // INTERNAL_CALL_PREEMPT_BY_HIGH_PRIO_APN (114)
  EMM_ACCESS_BARRED = 0x73, // EMM_ACCESS_BARRED (115)
  EMERGENCY_IFACE_ONLY = 0x74, // EMERGENCY_IFACE_ONLY (116)
  IFACE_MISMATCH = 0x75, // IFACE_MISMATCH (117)
  COMPANION_IFACE_IN_USE = 0x76, // COMPANION_IFACE_IN_USE (118)
  IP_ADDRESS_MISMATCH = 0x77, // IP_ADDRESS_MISMATCH (119)
  IFACE_AND_POL_FAMILY_MISMATCH = 0x78, // IFACE_AND_POL_FAMILY_MISMATCH (120)
  EMM_ACCESS_BARRED_INFINITE_RETRY = 0x79, // EMM_ACCESS_BARRED_INFINITE_RETRY (121)
  AUTH_FAILURE_ON_EMERGENCY_CALL = 0x7A, // AUTH_FAILURE_ON_EMERGENCY_CALL (122)
  INVALID_DNS_ADDR = 0x7B, // INVALID_DNS_ADDR (123)

  /* Mobile IP call end reasons (Type = 1) */
  MIP_FA_REASON_UNSPECIFIED = 0x7D0, // MIP_FA_ERR_REASON_UNSPECIFIED (64)
  MIP_FA_ADMIN_PROHIBITED = 0x7D1, // MIP_FA_ERR_ADMINISTRATIVELY_PROHIBITED (65)
  MIP_FA_INSUFFICIENT_RESOURCES = 0x7D2, // MIP_FA_ERR_INSUFFICIENT_RESOURCES (66)
  MIP_FA_MOBILE_NODE_AUTHENTICATION_FAILURE = 0x7D3, // MIP_FA_ERR_MOBILE_NODE_AUTHENTICATION_FAILURE (67)
  MIP_FA_HOME_AGENT_AUTHENTICATION_FAILURE = 0x7D4, // MIP_FA_ERR_HA_AUTHENTICATION_FAILURE (68)
  MIP_FA_REQUESTED_LIFETIME_TOO_LONG = 0x7D5, // MIP_FA_ERR_REQUESTED_LIFETIME_TOO_LONG (69)
  MIP_FA_MALFORMED_REQUEST = 0x7D6, // MIP_FA_ERR_MALFORMED_REQUEST (70)
  MIP_FA_MALFORMED_REPLY = 0x7D7, // MIP_FA_ERR_MALFORMED_REPLY (71)
  MIP_FA_ENCAPSULATION_UNAVAILABLE = 0x7D8, // MIP_FA_ERR_ENCAPSULATION_UNAVAILABLE (72)
  MIP_FA_VJ_HEADER_COMPRESSION_UNAVAILABLE = 0x7D9, // MIP_FA_ERR_VJHC_UNAVAILABLE (73)
  MIP_FA_REVERSE_TUNNEL_UNAVAILABLE = 0x7DA, // MIP_FA_ERR_REVERSE_TUNNEL_UNAVAILABLE (74)
  MIP_FA_REVERSE_TUNNEL_IS_MANDATORY = 0x7DB, // MIP_FA_ERR_REVERSE_TUNNEL_IS_MANDATORY_AND_T_BIT_NOT_SET (75)
  MIP_FA_DELIVERY_STYLE_NOT_SUPPORTED = 0x7DC, // MIP_FA_ERR_DELIVERY_STYLE_NOT_SUPPORTED (79)
  MIP_FA_MISSING_NAI = 0x7DD, // MIP_FA_ERR_MISSING_NAI (97)
  MIP_FA_MISSING_HOME_AGENT = 0x7DE, // MIP_FA_ERR_MISSING_HA (98)
  MIP_FA_MISSING_HOME_ADDRESS = 0x7DF, // MIP_FA_ERR_MISSING_HA (99)
  MIP_FA_UNKNOWN_CHALLENGE = 0x7E0, // MIP_FA_ERR_UNKNOWN_CHALLENGE (104)
  MIP_FA_MISSING_CHALLENGE = 0x7E1, // MIP_FA_ERR_MISSING_CHALLENGE (105)
  MIP_FA_STALE_CHALLENGE = 0x7E2, // MIP_FA_ERR_STALE_CHALLENGE (106)
  MIP_HA_REASON_UNSPECIFIED = 0x7E3, // MIP_HA_ERR_REASON_UNSPECIFIED (128)
  MIP_HA_ADMIN_PROHIBITED = 0x7E4, // MIP_HA_ERR_ADMINISTRATIVELY_PROHIBITED (129)
  MIP_HA_INSUFFICIENT_RESOURCES = 0x7E5, // MIP_HA_ERR_INSUFFICIENT_RESOURCES (130)
  MIP_HA_MOBILE_NODE_AUTHENTICATION_FAILURE = 0x7E6, // MIP_HA_ERR_MOBILE_NODE_AUTHENTICATION_FAILURE (131)
  MIP_HA_FOREIGN_AGENT_AUTHENTICATION_FAILURE = 0x7E7, // MIP_HA_ERR_FA_AUTHENTICATION_FAILURE (132)
  MIP_HA_REGISTRATION_ID_MISMATCH = 0x7E8, // MIP_HA_ERR_REGISTRATION_ID_MISMATCH (133)
  MIP_HA_MALFORMED_REQUEST = 0x7E9, // MIP_HA_ERR_MALFORMED_REQUEST (134)
  MIP_HA_UNKNOWN_HOME_AGENT_ADDRESS = 0x7EA, // IP_HA_ERR_UNKNOWN_HA_ADDR (135)
  MIP_HA_REVERSE_TUNNEL_UNAVAILABLE = 0x7EB, // MIP_HA_ERR_REVERSE_TUNNEL_UNAVAILABLE (136)
  MIP_HA_REVERSE_TUNNEL_IS_MANDATORY = 0x7EC, // MIP_HA_ERR_REVERSE_TUNNEL_IS_MANDATORY_AND_T_BIT_NOT_SET (137)
  MIP_HA_ENCAPSULATION_UNAVAILABLE = 0x7ED, // MIP_HA_ERR_ENCAPSULATION_UNAVAILABLE (139)

  /* Internal call end reasons (Type = 2) */
  CLOSE_IN_PROGRESS = 0x7EE, // CLOSE_IN_PROGRESS (205)
  NETWORK_INITIATED_TERMINATION = 0x7EF, // NW_INITIATED_TERMINATION (206)
  MODEM_APP_PREEMPTED = 0x7F0, // APP_PREEMPTED (207)
  PDN_IPV4_CALL_DISALLOWED = 0x7F1, // ERR_PDN_IPV4_CALL_DISALLOWED (208)
  PDN_IPV4_CALL_THROTTLED = 0x7F2, // ERR_PDN_IPV4_CALL_THROTTLED (209)
  PDN_IPV6_CALL_DISALLOWED = 0x7F3, // RR_PDN_IPV6_CALL_DISALLOWED (210)
  PDN_IPV6_CALL_THROTTLED = 0x7F4, // ERR_PDN_IPV6_CALL_THROTTLED (211)
  MODEM_RESTART = 0x7F5, // MODEM_RESTART (212)
  PDP_PPP_NOT_SUPPORTED = 0x7F6, // PDP_PPP_NOT_SUPPORTED (213)
  UNPREFERRED_RAT = 0x7F7, // UNPREFERRED_RAT (214)
  PHYSICAL_LINK_CLOSE_IN_PROGRESS = 0x7F8, // PHYS_LINK_CLOSE_IN_PROGRESS (215)
  APN_PENDING_HANDOVER = 0x7F9, // APN_PENDING_HANDOVER (216)
  PROFILE_BEARER_INCOMPATIBLE = 0x7FA, // PROFILE_BEARER_INCOMPATIBLE (217)
  SIM_CARD_CHANGED = 0x7FB, // MMGSDI_CARD_EVT (218)
  LOW_POWER_MODE_OR_POWERING_DOWN = 0x7FC, // LPM_OR_PWR_DOWN (219)
  APN_DISABLED = 0x7FD, // MANDATORY_APN_DISABLED (225)
  MAX_PPP_INACTIVITY_TIMER_EXPIRED = 0x7FE, // MPIT_EXPIRED (221)
  IPV6_ADDRESS_TRANSFER_FAILED = 0x7FF, // IPV6_ADDR_TRANSFER_FAILED (222)
  TRAT_SWAP_FAILED = 0x800, // TRAT_SWAP_FAILED (223)
  EHRPD_TO_HRPD_FALLBACK = 0x801, // EHRPD_TO_HRPD_FALLBACK (224)
  MIP_CONFIG_FAILURE = 0x802, // MIP_CONFIG_FAILURE (226)
  PDN_INACTIVITY_TIMER_EXPIRED = 0x803, // INTERNAL_PDN_INACTIVITY_TIMER_EXPIRED // (227)
  MAX_IPV4_CONNECTIONS = 0x804, // MAX_V4_CONNECTIONS (228)
  MAX_IPV6_CONNECTIONS = 0x805, // MAX_V6_CONNECTIONS (229)
  APN_MISMATCH = 0x806, // APN_MISMATCH (230)
  IP_VERSION_MISMATCH = 0x807, // IP_VERSION_MISMATCH (231)
  DUN_CALL_DISALLOWED = 0x808, // DUN_CALL_DISALLOWED (232)
  INTERNAL_EPC_NONEPC_TRANSITION = 0x809, // INTERNAL_EPC_NONEPC_TRANSITION (234)
  INTERFACE_IN_USE = 0x80A, // IFACE_IN_USE (237)
  APN_DISALLOWED_ON_ROAMING = 0x80B, // APN_DISALLOWED_ON_ROAMING (239)
  APN_PARAMETERS_CHANGED = 0x80C, // APN_PARAM_CHANGED (240)
  NULL_APN_DISALLOWED = 0x80D, // NULL_APN_DISALLOWED (242)
  THERMAL_MITIGATION = 0x80E, // THERMAL_MITIGATION (243)
  DATA_SETTINGS_DISABLED = 0x80F, // DATA_SETTINGS_DISABLED (245)
  DATA_ROAMING_SETTINGS_DISABLED = 0x810, // DATA_ROAMING_SETTINGS_DISABLED (246)
  DDS_SWITCHED = 0x811, // DDS_CALL_ABORT (248)
  FORBIDDEN_APN_NAME = 0x812, // INVALID_APN_NAME (253)
  DDS_SWITCH_IN_PROGRESS = 0x813, // DDS_SWITCH_IN_PROGRESS (254)
  CALL_DISALLOWED_IN_ROAMING = 0x814, // CALL_DISALLOWED_IN_ROAMING (255)
  NON_IP_NOT_SUPPORTED = 0x815, // NON_IP_NOT_SUPPORTED (257)
  PDN_NON_IP_CALL_THROTTLED = 0x816, // ERR_PDN_NON_IP_CALL_THROTTLED (258)
  PDN_NON_IP_CALL_DISALLOWED = 0x817, // ERR_PDN_NON_IP_CALL_DISALLOWED (259)

  /* Call Manager defined call end reasons (Type = 3) */
  CDMA_LOCK = 0x818, // CDMA_LOCK (500)
  CDMA_INTERCEPT = 0x819, // INTERCEPT (501)
  CDMA_REORDER = 0x81A, // REORDER (502)
  CDMA_RELEASE_DUE_TO_SO_REJECTION = 0x81B, // REL_SO_REJ (503)
  CDMA_INCOMING_CALL = 0x81C, // INCOM_CALL (504)
  CDMA_ALERT_STOP = 0x81D, // ALERT_STOP (505)
  CHANNEL_ACQUISITION_FAILURE = 0x81E, // ACTIVATION (506)
  MAX_ACCESS_PROBE = 0x81F, // MAX_ACCESS_PROBE (507)
  CONCURRENT_SERVICE_NOT_SUPPORTED_BY_BASE_STATION = 0x820, // CCS_NOT_SUPPORTED_BY_BS (508)
  NO_RESPONSE_FROM_BASE_STATION = 0x821, // NO_RESPONSE_FROM_BS (509)
  REJECTED_BY_BASE_STATION = 0x822, // REJECTED_BY_BS (510)
  CONCURRENT_SERVICES_INCOMPATIBLE = 0x823, // INCOMPATIBLE (511)
  NO_CDMA_SERVICE = 0x824, // NO_CDMA_SRV (515)
  RUIM_NOT_PRESENT = 0x825, // UIM_NOT_PRESENT (518)
  CDMA_RETRY_ORDER = 0x826, // RETRY_ORDER (519)
  ACCESS_BLOCK = 0x827, // ACCESS_BLOCK (520)
  ACCESS_BLOCK_ALL = 0x828, // ACCESS_BLOCK_ALL (521)
  IS707B_MAX_ACCESS_PROBES = 0x829, // IS707B_MAX_ACC (522)
  THERMAL_EMERGENCY = 0x82A, // THERMAL_EMERGENCY(523)
  CONCURRENT_SERVICES_NOT_ALLOWED = 0x82B, // USER_CALL_ORIG_DURING_VOICE_CALL (523)
  INCOMING_CALL_REJECTED = 0x82C, // INCOM_REJ (1001)
  NO_SERVICE_ON_GATEWAY = 0x82D, // NEW_NO_GW_SRV (1002)
  NO_GPRS_CONTEXT = 0x82E, // NEW_NO_GPRS_CONTEXT (1003)
  ILLEGAL_MS = 0x82F, // NEW_ILLEGAL_MS (1004)
  ILLEGAL_ME = 0x830, // NEW_ILLEGAL_ME (1005)
  GPRS_SERVICES_AND_NON_GPRS_SERVICES_NOT_ALLOWED = 0x831, // NEW_GPRS_SERVICES_AND_NON_GPRS_SERVICES_NOT_ALLOWED (1006)
  GPRS_SERVICES_NOT_ALLOWED = 0x832, // NEW_GPRS_SERVICES_NOT_ALLOWED (1007)
  MS_IDENTITY_CANNOT_BE_DERIVED_BY_THE_NETWORK = 0x833, // NEW_MS_IDENTITY_CANNOT_BE_DERIVED_BY_THE_NETWORK (1008)
  IMPLICITLY_DETACHED = 0x834, // NEW_IMPLICITLY_DETACHED (1009)
  PLMN_NOT_ALLOWED = 0x835, // NEW_PLMN_NOT_ALLOWED (1010)
  LOCATION_AREA_NOT_ALLOWED = 0x836, // NEW_LA_NOT_ALLOWED (1011)
  GPRS_SERVICES_NOT_ALLOWED_IN_THIS_PLMN = 0x837, // NEW_GPRS_SERVICES_NOT_ALLOWED_IN_THIS_PLMN (1012)
  PDP_DUPLICATE = 0x838, // NEW_PDP_DUPLICATE (1013)
  UE_RAT_CHANGE = 0x839, // NEW_UE_RAT_CHANGE (1014)
  CONGESTION = 0x83A, // NEW_CONGESTION (1015)
  NO_PDP_CONTEXT_ACTIVATED = 0x83B, // NEW_NO_PDP_CONTEXT_ACTIVATED (1016)
  ACCESS_CLASS_DSAC_REJECTION = 0x83C, // NEW_ACCESS_CLASS_DSAC_REJECTION (1017)
  PDP_ACTIVATE_MAX_RETRY_FAILED = 0x83D, // PDP_ACTIVATE_MAX_RETRY_FAILED (1018)
  RADIO_ACCESS_BEARER_FAILURE = 0x83E, // RAB_FAILURE (1019)
  ESM_UNKNOWN_EPS_BEARER_CONTEXT = 0x83F, // ESM_UNKNOWN_EPS_BEARER_CONTEXT (1025)
  DRB_RELEASED_BY_RRC = 0x840, // DRB_RELEASED_AT_RRC (1026)
  CONNECTION_RELEASED = 0x841, // NAS_SIG_CONN_RELEASED (1027)
  EMM_DETACHED = 0x842, // REASON_EMM_DETACHED (1028)
  EMM_ATTACH_FAILED = 0x843, // EMM_ATTACH_FAILED (1029)
  EMM_ATTACH_STARTED = 0x844, // EMM_ATTACH_STARTED (1030)
  LTE_NAS_SERVICE_REQUEST_FAILED = 0x845, // LTE_NAS_SERVICE_REQ_FAILED (1031)
  DUPLICATE_BEARER_ID = 0x846, // ESM_ACTIVE_DEDICATED_BEARER_REACTIVATED_BY_NW (1032)
  ESM_COLLISION_SCENARIOS = 0x847, // ESM_LOWER_LAYER_FAILURE (1033)
  ESM_BEARER_DEACTIVATED_TO_SYNC_WITH_NETWORK = 0x848, // ESM_SYNC_UP_WITH_NW (1034)
  ESM_NW_ACTIVATED_DED_BEARER_WITH_ID_OF_DEF_BEARER = 0x849, // ESM_NW_ACTIVATED_DED_BEARER_WITH_ID_OF_DEF_BEARER (1035)
  ESM_BAD_OTA_MESSAGE = 0x84A, // ESM_BAD_OTA_MESSAGE (1036)
  ESM_DOWNLOAD_SERVER_REJECTED_THE_CALL = 0x84B, // ESM_DS_REJECTED_THE_CALL (1037)
  ESM_CONTEXT_TRANSFERRED_DUE_TO_IRAT = 0x84C, // ESM_CONTEXT_TRANSFERED_DUE_TO_IRAT (1038)
  DS_EXPLICIT_DEACTIVATION = 0x84D, // DS_EXPLICIT_DEACT (1039)
  ESM_LOCAL_CAUSE_NONE = 0x84E, // ESM_LOCAL_CAUSE_NONE (1040)
  LTE_THROTTLING_NOT_REQUIRED = 0x84F, // LTE_NAS_SERVICE_REQ_FAILED_NO_THROTTLE (1041)
  ACCESS_CONTROL_LIST_CHECK_FAILURE = 0x850, // ACL_FAILURE (1042)
  SERVICE_NOT_ALLOWED_ON_PLMN = 0x851, // LTE_NAS_SERVICE_REQ_FAILED_DS_DISALLOW (1043)
  EMM_T3417_EXPIRED = 0x852, // EMM_T3417_EXPIRED (1044)
  EMM_T3417_EXT_EXPIRED = 0x853, // EMM_T3417_EXT_EXPIRED (1045)
  RRC_UPLINK_DATA_TRANSMISSION_FAILURE = 0x854, // LRRC_UL_DATA_CNF_FAILURE_TXN (1046)
  RRC_UPLINK_DELIVERY_FAILED_DUE_TO_HANDOVER = 0x855, // LRRC_UL_DATA_CNF_FAILURE_HO (1047)
  RRC_UPLINK_CONNECTION_RELEASE = 0x856, // LRRC_UL_DATA_CNF_FAILURE_CONN_REL (1048)
  RRC_UPLINK_RADIO_LINK_FAILURE = 0x857, // LRRC_UL DATA_CNF_FAILURE_RLF (1049)
  RRC_UPLINK_ERROR_REQUEST_FROM_NAS = 0x858, // LRRC_UL_DATA_CNF_FAILURE_CTRL_NOT_CONN (1050)
  RRC_CONNECTION_ACCESS_STRATUM_FAILURE = 0x859, // LRRC_CONN_EST_FAILURE (1051)
  RRC_CONNECTION_ANOTHER_PROCEDURE_IN_PROGRESS = 0x85A, // LRRC_CONN_EST_FAILURE_ABORTED (1052)
  RRC_CONNECTION_ACCESS_BARRED = 0x85B, // LRRC_CONN_EST_FAILURE_ACCESS_BARRED (1053)
  RRC_CONNECTION_CELL_RESELECTION = 0x85C, // LRRC_CONN_EST_FAILURE_CELL_RESEL (1054)
  RRC_CONNECTION_CONFIG_FAILURE = 0x85D, // LRRC_CONN_EST_FAILURE_CONFIG_FAILURE (1055)
  RRC_CONNECTION_TIMER_EXPIRED = 0x85E, // LRRC_CONN_EST_FAILURE_TIMER_EXPIRED (1056)
  RRC_CONNECTION_LINK_FAILURE = 0x85F, // LRRC_CONN_EST_FAILURE_LINK_FAILURE (1057)
  RRC_CONNECTION_CELL_NOT_CAMPED = 0x860, // LRRC_CONN_EST_FAILURE_NOT_CAMPED (1058)
  RRC_CONNECTION_SYSTEM_INTERVAL_FAILURE = 0x861, // LRRC_CONN_EST_FAILURE_SI_FAILURE (1059)
  RRC_CONNECTION_REJECT_BY_NETWORK = 0x862, // LRRC_CONN_EST_FAILURE_CONN_REJECT (1060)
  RRC_CONNECTION_NORMAL_RELEASE = 0x863, // LRRC_CONN_REL_NORMAL (1061)
  RRC_CONNECTION_RADIO_LINK_FAILURE = 0x864, // LRRC_CONN_REL_RLF (1062)
  RRC_CONNECTION_REESTABLISHMENT_FAILURE = 0x865, // LRRC_CONN_REL_CRE_ FAILURE (1063)
  RRC_CONNECTION_OUT_OF_SERVICE_DURING_CELL_REGISTER = 0x866, // LRRC_CONN_REL_OOS_DURING_CRE (1064)
  RRC_CONNECTION_ABORT_REQUEST = 0x867, // LRRC_CONN_REL_ABORTED (1065)
  RRC_CONNECTION_SYSTEM_INFORMATION_BLOCK_READ_ERROR = 0x868, // LRRC_CONN_REL_SIB_READ_ERROR (1066)
  NETWORK_INITIATED_DETACH_WITH_AUTO_REATTACH = 0x869, // DETACH_WITH_REATTACH_LTE_NW_DETACH (1067)
  NETWORK_INITIATED_DETACH_NO_AUTO_REATTACH = 0x86A, // DETACH_WITH_OUT_REATTACH_LTE_NW_ DETACH (1068)
  ESM_PROCEDURE_TIME_OUT = 0x86B, // ESM_PROC_TIME_OUT (1069)
  INVALID_CONNECTION_ID = 0x86C, // INVALID_CONNECTION_ID (1070)
  MAXIMIUM_NSAPIS_EXCEEDED = 0x86D, // INVALID_NSAPI (1071)
  INVALID_PRIMARY_NSAPI = 0x86E, // INVALID_PRI_NSAPI (1072)
  CANNOT_ENCODE_OTA_MESSAGE = 0x86F, // INVALID_FIELD (1073)
  RADIO_ACCESS_BEARER_SETUP_FAILURE = 0x870, // RAB_SETUP_FAILURE (1074)
  PDP_ESTABLISH_TIMEOUT_EXPIRED = 0x871, // PDP_ESTABLISH_MAX_TIMEOUT (1075)
  PDP_MODIFY_TIMEOUT_EXPIRED = 0x872, // PDP_MODIFY_MAX_TIMEOUT (1076)
  PDP_INACTIVE_TIMEOUT_EXPIRED = 0x873, // PDP_INACTIVE_MAX_TIMEOUT (1077)
  PDP_LOWERLAYER_ERROR = 0x874, // PDP_LOWERLAYER_ERROR (1078)
  PDP_MODIFY_COLLISION = 0x875, // PDP_MODIFY_COLLISION (1080)
  MAXINUM_SIZE_OF_L2_MESSAGE_EXCEEDED = 0x876, // MESSAGE_EXCEED_MAX_L2_LIMIT (1086)
  NAS_REQUEST_REJECTED_BY_NETWORK = 0x877, // SM_NAS_SRV_REQ_FAILURE (1087)
  RRC_CONNECTION_INVALID_REQUEST = 0x878, // RRC_CONN_EST_FAILURE_REQ_ERROR (1088)
  RRC_CONNECTION_TRACKING_AREA_ID_CHANGED = 0x879, // RRC_CONN_EST_FAILURE_TAI_CHANGE (1089)
  RRC_CONNECTION_RF_UNAVAILABLE = 0x87A, // RRC_CONN_EST_FAILURE_RF_UNAVAILABLE (1090)
  RRC_CONNECTION_ABORTED_DUE_TO_IRAT_CHANGE = 0x87B, // RRC_CONN_REL_ABORTED_IRAT_SUCCESS (1091)
  RRC_CONNECTION_RELEASED_SECURITY_NOT_ACTIVE = 0x87C, // RRC_CONN_REL_RLF_SEC_NOT_ACTIVE (1092)
  RRC_CONNECTION_ABORTED_AFTER_HANDOVER = 0x87D, // RRC_CONN_REL_IRAT_TO_LTE_ABORTED (1093)
  RRC_CONNECTION_ABORTED_AFTER_IRAT_CELL_CHANGE = 0x87E, // RRC_CONN_REL_IRAT_FROM_LTE_TO_G_CCO_SUCCESS (1094)
  RRC_CONNECTION_ABORTED_DURING_IRAT_CELL_CHANGE = 0x87F, // RRC_CONN_REL_IRAT_FROM_LTE_TO_G_CCO_ABORTED (1095)
  IMSI_UNKNOWN_IN_HOME_SUBSCRIBER_SERVER = 0x880, // IMSI_UNKNOWN_IN_HSS (1096)
  IMEI_NOT_ACCEPTED = 0x881, // IMEI_NOT_ACCEPTED (1097)
  EPS_SERVICES_AND_NON_EPS_SERVICES_NOT_ALLOWED = 0x882, // EPS_SERVICES_AND_NON_EPS_SERVICES_NOT_ALLOWED (1098)
  EPS_SERVICES_NOT_ALLOWED_IN_PLMN = 0x883, // EPS_SERVICES_NOT_ALLOWED_IN_PLMN (1099)
  MSC_TEMPORARILY_NOT_REACHABLE = 0x884, // MSC_TEMPORARILY_NOT_REACHABLE (1100)
  CS_DOMAIN_NOT_AVAILABLE = 0x885, // CS_DOMAIN_NOT_AVAILABLE (1101)
  ESM_FAILURE = 0x886, // ESM_FAILURE (1102)
  MAC_FAILURE = 0x887, // MAC_FAILURE (1103)
  SYNCHRONIZATION_FAILURE = 0x888, // SYNCH_FAILURE (1104)
  UE_SECURITY_CAPABILITIES_MISMATCH = 0x889, // UE_SECURITY_CAPABILITIES_MISMATCH (1105)
  SECURITY_MODE_REJECTED = 0x88A, // SECURITY_MODE_REJ_UNSPECIFIED (1106)
  UNACCEPTABLE_NON_EPS_AUTHENTICATION = 0x88B, // NON_EPS_AUTH_UNACCEPTABLE (1107)
  CS_FALLBACK_CALL_ESTABLISHMENT_NOT_ALLOWED = 0x88C, // CS_FALLBACK_CALL_EST_NOT_ALLOWED (1108)
  NO_EPS_BEARER_CONTEXT_ACTIVATED = 0x88D, // NO_EPS_BEARER_CONTEXT_ACTIVATED (1109)
  INVALID_EMM_STATE = 0x88E, // EMM_INVALID_STATE (1110)
  NAS_LAYER_FAILURE = 0x88F, // NAS_LAYER_FAILURE (1111)
  MULTIPLE_PDP_CALL_NOT_ALLOWED = 0x890, // MULTI_PDN_NOT_ALLOWED (1112)
  EMBMS_NOT_ENABLED = 0x891, // EMBMS_NOT_ENABLED (1113)
  IRAT_HANDOVER_FAILED = 0x892, // PENDING_REDIAL_CALL_CLEANUP (1114)
  EMBMS_REGULAR_DEACTIVATION = 0x893, // EMBMS_REGULAR_DEACTIVATION (1115)
  TEST_LOOPBACK_REGULAR_DEACTIVATION = 0x894, // TLB_REGULAR_DEACTIVATION (1116)
  LOWER_LAYER_REGISTRATION_FAILURE = 0x895, // LOWER_LAYER_REGISTRATION_FAILURE (1117)
  DATA_PLAN_EXPIRED = 0x896, // DETACH_EPS_SERVICES_NOT_ALLOWED (1118)
  UMTS_HANDOVER_TO_IWLAN = 0x897, // SM_INTERNAL_PDP_DEACTIVATION (1119)
  EVDO_CONNECTION_DENY_BY_GENERAL_OR_NETWORK_BUSY = 0x898, // CD_GEN_OR_BUSY (1500)
  EVDO_CONNECTION_DENY_BY_BILLING_OR_AUTHENTICATION_FAILURE = 0x899, // CD_BILL_OR_AUTH (1501)
  EVDO_HDR_CHANGED = 0x89A, // CHG_HDR (1502)
  EVDO_HDR_EXITED = 0x89B, // EXIT_HDR (1503)
  EVDO_HDR_NO_SESSION = 0x89C, // HDR_NO_SESSION (1504)
  EVDO_USING_GPS_FIX_INSTEAD_OF_HDR_CALL = 0x89D, // HDR_ORIG_DURING_GPS_FIX (1505)
  EVDO_HDR_CONNECTION_SETUP_TIMEOUT = 0x89E, // HDR_CS_TIMEOUT (1506)
  FAILED_TO_ACQUIRE_COLOCATED_HDR = 0x89F, // COLLOC_ACQ_FAIL (1508)
  OTASP_COMMIT_IN_PROGRESS = 0x8A0, // OTASP_COMMIT_IN_PROG (1509)
  NO_HYBRID_HDR_SERVICE = 0x8A1, // NO_HYBR_HDR_SRV (1510)
  HDR_NO_LOCK_GRANTED = 0x8A2, // HDR_NO_LOCK_GRANTED (1511)
  DBM_OR_SMS_IN_PROGRESS = 0x8A3, // HOLD_OTHER_IN_PROG (1512)
  HDR_FADE = 0x8A4, // HDR_FADE (1513)
  HDR_ACCESS_FAILURE = 0x8A5, // HDR_ACC_FAIL (1514)
  UNSUPPORTED_1X_PREV = 0x8A6, // UNSUPPORTED_1X_PREV (1515)
  LOCAL_END = 0x8A7, // CLIENT_END (2000)
  NO_SERVICE = 0x8A8, // NO_SRV (2001)
  FADE = 0x8A9, // FADE (2002)
  NORMAL_RELEASE = 0x8AA, // REL_NORMAL (2003)
  ACCESS_ATTEMPT_ALREADY_IN_PROGRESS = 0x8AB, // ACC_IN_PROG (2004)
  REDIRECTION_OR_HANDOFF_IN_PROGRESS = 0x8AC, // REDIR_OR_HANDOFF (2006)
  EMERGENCY_MODE = 0x8AD, // EMERGENCY_MODE (2501)
  PHONE_IN_USE = 0x8AE, // PHONE_IN_USE (2502)
  INVALID_MODE = 0x8AF, // INVALID_MODE (2503)
  INVALID_SIM_STATE = 0x8B0, // INVALID_SIM_STATE (2504)
  NO_COLLOCATED_HDR = 0x8B1, // NO_COLLOC_HDR (2505)
  UE_IS_ENTERING_POWERSAVE_MODE = 0x8B2, // EMM_DETACHED_PSM (2507)
  DUAL_SWITCH = 0x8B3, // DUAL_SWITCH (2508)

  /* PPP call end reasons (Type = 7) */
  PPP_TIMEOUT = 0x8B4, // TIMEOUT (1)
  PPP_AUTH_FAILURE = 0x8B5, // AUTH_FAILURE (2)
  PPP_OPTION_MISMATCH = 0x8B6, // OPTION_MISMATCH (3)
  PPP_PAP_FAILURE = 0x8B7, // PAP_FAILURE (31)
  PPP_CHAP_FAILURE = 0x8B8, // CHAP_FAILURE (32)
  PPP_CLOSE_IN_PROGRESS = 0x8B9, // CLOSE_IN_PROGRESS (33)

  /* 3GPP specification defined call end reasons (Type = 8) */
  LIMITED_TO_IPV4 = 0x8BA, // SUBS_LIMITED_TO_V4 (1)
  LIMITED_TO_IPV6 = 0x8BB, // SUBS_LIMITED_TO_V6 (2)
  VSNCP_TIMEOUT = 0x8BC, // VSNCP_TIMEOUT (4)
  VSNCP_GEN_ERROR = 0x8BD, // VSNCP_3GPP2I_GEN_ERROR (6)
  VSNCP_APN_UNATHORIZED = 0x8BE, // VSNCP_3GPP2I_UNAUTH_APN (7)
  VSNCP_PDN_LIMIT_EXCEEDED = 0x8BF, // VSNCP_3GPP2I_PDN_LIMIT_EXCEED (8)
  VSNCP_NO_PDN_GATEWAY_ADDRESS = 0x8C0, // VSNCP_3GPP2I_NO_PDN_GW (9)
  VSNCP_PDN_GATEWAY_UNREACHABLE = 0x8C1, // VSNCP_3GPP2I_PDN_GW_UNREACH (10)
  VSNCP_PDN_GATEWAY_REJECT = 0x8C2, // VSNCP_3GPP2I_PDN_GW_REJ (11)
  VSNCP_INSUFFICIENT_PARAMETERS = 0x8C3, // VSNCP_3GPP2I_INSUFF_PARAM (12)
  VSNCP_RESOURCE_UNAVAILABLE = 0x8C4, // VSNCP_3GPP2I_RESOURCE_UNAVAIL (13)
  VSNCP_ADMINISTRATIVELY_PROHIBITED = 0x8C5, // VSNCP_3GPP2I_ADMIN_PROHIBIT (14)
  VSNCP_PDN_ID_IN_USE = 0x8C6, // VSNCP_3GPP2I_PDN_ID_IN_USE (15)
  VSNCP_SUBSCRIBER_LIMITATION = 0x8C7, // VSNCP_3GPP2I_SUBSCR_LIMITATION (16)
  VSNCP_PDN_EXISTS_FOR_THIS_APN = 0x8C8, // VSNCP_3GPP2I_PDN_EXISTS_FOR_THIS_APN (17)
  VSNCP_RECONNECT_NOT_ALLOWED = 0x8C9, // VSNCP_3GPP2I_RECONNECT_NOT_ALLOWED (19)

  /* IPv6 call end reasons (Type = 9) */
  IPV6_PREFIX_UNAVAILABLE = 0x8CA, // IPV6_ERR_PREFIX_UNAVAILABLE (1)

  /* Handoff failure reasons (Type = 12) */
  HANDOFF_PREFERENCE_CHANGED = 0x8CB, // WDS_VCER_ HANDOFF_PREF_SYS_BACK_TO_ SRAT (1)

  /* OEM error causes */
  OEM_DCFAILCAUSE_1 = 0x1001,  // Connection exist in different transport
  OEM_DCFAILCAUSE_2 = 0x1002,
  OEM_DCFAILCAUSE_3 = 0x1003,
  OEM_DCFAILCAUSE_4 = 0x1004,
  OEM_DCFAILCAUSE_5 = 0x1005,
  OEM_DCFAILCAUSE_6 = 0x1006,
  OEM_DCFAILCAUSE_7 = 0x1007,
  OEM_DCFAILCAUSE_8 = 0x1008,
  OEM_DCFAILCAUSE_9 = 0x1009,
  OEM_DCFAILCAUSE_10 = 0x100A,
  OEM_DCFAILCAUSE_11 = 0x100B,
  OEM_DCFAILCAUSE_12 = 0x100C,
  OEM_DCFAILCAUSE_13 = 0x100D,
  OEM_DCFAILCAUSE_14 = 0x100E,
  OEM_DCFAILCAUSE_15 = 0x100F,

  /* Legacy error causes */
  VOICE_REGISTRATION_FAIL = -1,
  DATA_REGISTRATION_FAIL = -2,
  SIGNAL_LOST = -3,
  PREF_RADIO_TECH_CHANGED = -4,
  RADIO_POWER_OFF = -5,
  TETHERED_CALL_ACTIVE = -6,
  ERROR_UNSPECIFIED = 0xffff,
};

enum class EmbmsDataCallFailCause_t : int32_t {
  ERROR_NONE = 0,
  ERROR_UNKNOWN = 1,
  ERROR_ALREADY_DONE = 2,
  ERROR_NOT_ALLOWED = 3,
  ERROR_MISSING_CONTROL_INFO = 4,
  ERROR_MISSING_TMGI = 5,
  ERROR_MCAST_OOC = 6,
  ERROR_UCAST_OOS = 7,
  ERROR_FREQUENCY_CONFLICT = 8,
  ERROR_MAX_TMGI_ALREADY_ACTIVE = 9,
  SUCCESS_RADIO_TUNE_IN_PROGRESS_UCAST_IDLE = 100,
  SUCCESS_RADIO_TUNE_IN_PROGRESS_UCAST_CONNECTED = 101
};

enum class AccessNetwork_t : int32_t {
  UNKNOWN   = 0,
  GERAN     = 1,
  UTRAN     = 2,
  EUTRAN    = 3,
#ifndef USING_RIL_ACCESS_NETWORKS
  CDMA2000  = 4,
#endif
  CDMA      = 4,
  IWLAN     = 5,
  NGRAN     = 6,
};
inline std::ostream& operator<<(std::ostream& os, AccessNetwork_t an);

enum class DataRequestReason_t : int32_t {
  NORMAL    = 1,
  SHUTDOWN  = 2,
  HANDOVER  = 3,
};

enum class ApnAuthType_t : int32_t {
  NO_PAP_NO_CHAP = 0,
  PAP_NO_CHAP    = 1,
  NO_PAP_CHAP    = 2,
  PAP_CHAP       = 3,
};

enum class DataProfileId_t : int32_t {
  DEFAULT    = 0,
  TETHERED   = 1,
  IMS        = 2,
  FOTA       = 3,
  CBS        = 4,
  OEM_BASE   = 1000,
  INVALID    = -1,
};

enum class DataProfileInfoType_t : int32_t {
  COMMON     = 0,
  THREE_GPP  = 1,
  THREE_GPP2 = 2,
};

enum class ApnTypes_t : int32_t {
  NONE      = 0,
  DEFAULT   = 1 << 0,
  MMS       = 1 << 1,
  SUPL      = 1 << 2,
  DUN       = 1 << 3,
  HIPRI     = 1 << 4,
  FOTA      = 1 << 5,
  IMS       = 1 << 6,
  CBS       = 1 << 7,
  IA        = 1 << 8,
  EMERGENCY = 1 << 9,
  MCX       = 1 << 10,
  XCAP      = 1 << 11,
};
inline std::ostream& operator<<(std::ostream& os, ApnTypes_t type);

enum class RadioAccessFamily_t : int32_t {
  UNKNOWN   = 1 << 0,
  GPRS      = 1 << 1,
  EDGE      = 1 << 2,
  UMTS      = 1 << 3,
  IS95A     = 1 << 4,
  IS95B     = 1 << 5,
  ONE_X_RTT = 1 << 6,
  EVDO_0    = 1 << 7,
  EVDO_A    = 1 << 8,
  HSDPA     = 1 << 9,
  HSUPA     = 1 << 10,
  HSPA      = 1 << 11,
  EVDO_B    = 1 << 12,
  EHRPD     = 1 << 13,
  LTE       = 1 << 14,
  HSPAP     = 1 << 15,
  GSM       = 1 << 16,
  TD_SCDMA  = 1 << 17,
  LTE_CA    = 1 << 19,
  NR        = 1 << 20,
  WLAN      = 1 << 21,
};
inline std::ostream& operator<<(std::ostream& os, RadioAccessFamily_t r);

enum class DataRegState_t : int32_t {
  NOT_REG_AND_NOT_SEARCHING = 0,
  REG_HOME = 1,
  NOT_REG_AND_SEARCHING = 2,
  REG_DENIED = 3,
  UNKNOWN = 4,
  REG_ROAMING = 5,
  /* emergency calls are enabled */
  NOT_REG_AND_NOT_SEARCHING_EMERGENCY = 10,
  NOT_REG_AND_SEARCHING_EMERGENCY = 12,
  REG_DENIED_EMERGENCY = 13,
  UNKNOWN_EMERGENCY = 14,
};

enum class SetPreferredDataModemResult_t : int {
  NO_ERROR = 0,
  DDS_SWITCH_NOT_ALLOWED,
  DDS_SWITCH_FAILED,
  DDS_SWITCH_TIMEOUT,
  REQUEST_ALREADY_PENDING,
  INVALID_ARG,
  INVALID_OPERATION,
  QMI_ERROR,
  MODEM_ENDPOINT_NOT_OPERATIONAL
};

enum class CellConnectionStatus_t : int32_t {
  NONE = 0,
  PRIMARY_SERVING = 1,
  SECONDARY_SERVING = 2,
};

enum class FrequencyRange_t : int32_t {
  UNKNOWN = 0,
  LOW = 1,
  MID = 2,
  HIGH = 3,
  MMWAVE = 4,
};
inline std::ostream& operator<<(std::ostream& os, FrequencyRange_t fr);

enum class HandoffState_t : uint16_t {
  PrefSysChangedSuccess = 0,
  PrefSysChangedFailure = 1,
  Init                  = 2,
  Success               = 3,
  Failure               = 4
};

enum class IpFamilyType_t : int {
  NotAvailable =  0,
  IPv4 = 1 << 0,
  IPv6 = 1 << 1,
  IPv4v6 = 1 << 2,
};

enum class AddressProperty_t : int32_t {
  NONE = 0,
  DEPRECATED = 0x20,
};

enum class DsiInitStatus_t :  uint8_t {
  PENDING_RELEASE = 0,
  RELEASED = 1,
  STARTED = 2,
  COMPLETED = 3,
};

enum class IARequestStatus_t :  uint8_t {
  IA_FAILED = 0,
  IA_SUCCESS = 1,
  IA_PROGRESS = 2,
};

struct RadioFrequencyInfo_t {
  FrequencyRange_t range;
  int32_t channelNumber;
  void dump(std::string padding, std::ostream& os) const {
    os << padding << "{" << range << "," << channelNumber << "}";
  }
};

struct BearerInfo_t {
  int32_t bearerId;
  RatType_t uplink;
  RatType_t downlink;
  void dump(std::string padding, std::ostream& os) const {
    os << padding << "{" << bearerId << ", uplink=" << uplink <<
          ", downlink=" << downlink << "}";
  }
};

struct AllocatedBearer_t {
  int32_t cid;
  string apn;
  string ifaceName;
  vector<BearerInfo_t> bearers;
};

struct AllocatedBearerResult_t {
  ResponseError_t error;
  vector<AllocatedBearer_t> connections;
};

struct DataProfileInfo_t {
  DataProfileId_t profileId;
  string apn;
  string protocol;
  string roamingProtocol;
  ApnAuthType_t authType;
  string username;
  string password;
  DataProfileInfoType_t dataProfileInfoType;
  int32_t maxConnsTime;
  int32_t maxConns;
  int32_t waitTime;
  bool enableProfile;
  ApnTypes_t supportedApnTypesBitmap;
  RadioAccessFamily_t bearerBitmap;
  int32_t mtu;
  bool preferred;
  bool persistent;
  void dump(std::string padding, std::ostream& os) const {
    os << std::boolalpha << padding << "{"
        << (int)profileId << ":"
        << apn << ","
        << protocol << ","
        << roamingProtocol << ","
        << (int)authType << ","
        << username << ","
        << password << ","
        << (int)dataProfileInfoType << ","
        << maxConnsTime << ","
        << maxConns << ","
        << waitTime << ","
        << enableProfile << ","
        << (int)supportedApnTypesBitmap << ","
        << bearerBitmap << ","
        << mtu << ","
        << preferred << ","
        << persistent << "}";
  }
};

struct LinkAddress_t {
  std::string address;
  AddressProperty_t properties;
  uint64_t deprecationTime;
  uint64_t expirationTime;
  LinkAddress_t(): address(""), properties(AddressProperty_t::NONE), deprecationTime(-1), expirationTime(-1) {}
  void dump(std::string padding, std::ostream& os) const {
    os << padding << "addr=" << address << " props=" << (int)properties
        << " dep=" << deprecationTime << " exp=" << expirationTime;
  }
};


enum class QosProtocol_t : int32_t {
  UNSPECIFIED = -1,
  TCP         = 6,
  UDP         = 17,
  ESP         = 50,
  AH          = 51,
};

enum class QosFilterDirection_t : int32_t {
  DOWNLINK      = 0,
  UPLINK        = 1,
  BIDIRECTIONAL = 2,
};

struct QosBandwidth_t {
  int32_t maxBitrateKbps;
  int32_t guaranteedBitrateKbps;
  void dump(std::string padding, std::ostream& os) const {
    os << padding << "{"
       << maxBitrateKbps << ","
       << guaranteedBitrateKbps << "}";
  }
};

struct EpsQos_t {
  uint16_t qci;
  QosBandwidth_t downlink;
  QosBandwidth_t uplink;
  void dump(std::string padding, std::ostream& os) const {
    os << padding << "{";
    os << qci << ",";
    downlink.dump("epsdl=", os);
    os << ",";
    uplink.dump("epsul=", os);
    os << "}";
  }
};

struct NrQos_t {
  uint16_t fiveQi;
  QosBandwidth_t downlink;
  QosBandwidth_t uplink;
  uint8_t qfi;
  uint16_t averagingWindowMs;
  void dump(std::string padding, std::ostream& os) const {
    os << padding << "{";
    os << fiveQi << ",";
    downlink.dump("nrdl=", os);
    os << ",";
    uplink.dump("nrul=", os);
    os << ",";
    os << qfi << ",";
    os << averagingWindowMs << "}";
  }
};

struct Qos_t {
  std::optional<EpsQos_t> epsQos;
  std::optional<NrQos_t> nrQos;
  void dump(std::string padding, std::ostream& os) const {
    os << padding << "{";
    if(epsQos.has_value()) {epsQos.value().dump("epsqos=", os);} else {os << "epsqos=";}
    os << ",";
    if(nrQos.has_value()) {nrQos.value().dump("nrqos=", os);} else {os << "nrqos=";}
    os << "}";
  }
};

struct Ipv6FlowLabel_t {
  std::optional<uint32_t> value;
  void dump(std::string padding, std::ostream& os) const {
    os << padding << "{";
    if(value.has_value()) {os << value.value();}
    os << "}";
  }
};

struct IpsecSpi_t {
  std::optional<uint32_t> value;
  void dump(std::string padding, std::ostream& os) const {
    os << padding << "{";
    if(value.has_value()) {os << value.value();}
    os << "}";
  }
};

struct TypeOfServiceTrafficClass_t {
  std::optional<uint8_t> value;
  std::optional<uint8_t> mask;
  void dump(std::string padding, std::ostream& os) const {
    os << padding << "{";
    if(value.has_value()) {os << value.value();}
    os << ",";
    if(mask.has_value()) {os << mask.value();}
    os << "}";
  }
};

struct PortRange_t {
  int32_t start;
  int32_t end;
  void dump(std::string padding, std::ostream& os) const {
    os << padding << "{"
       << start << ","
       << end << "}";
  }
};

struct QosFilter_t {
  vector<string> localAddresses;
  vector<string> remoteAddresses;
  QosProtocol_t protocol;
  std::optional<PortRange_t> localPort;
  std::optional<PortRange_t> remotePort;
  TypeOfServiceTrafficClass_t tos;
  Ipv6FlowLabel_t flowLabel;
  IpsecSpi_t spi;
  QosFilterDirection_t direction;
  int32_t precedence;
  void dump(std::string padding, std::ostream& os) const {
    os << padding << "{{";
    for (string la : localAddresses) {
      os << la << " ";
    }
    os << "},{";
    for (string ra : remoteAddresses) {
      os << ra << " ";
    }
    os << "}";
    os << (int)protocol << ",";
    if(localPort.has_value())
      {localPort->dump("lp=", os); os << ",";}
    else {os << "lp=,";}
    if(remotePort.has_value())
      {remotePort->dump("rp=", os); os << ",";}
    else {os << "rp=,";}
    tos.dump("tos=", os);
    flowLabel.dump(",fl=", os);
    spi.dump(",spi=", os);
    os << "," << (int)direction
       << "," << precedence << "},";
  }
};

struct QosSession_t {
  int32_t qosSessionId;
  Qos_t qos;
  vector<QosFilter_t> qosFilters;
  void dump(std::string padding, std::ostream& os) const {
    os << padding << "{";
    os << qosSessionId << ",";
    qos.dump("qos=", os);
    os << ",{";
    for (QosFilter_t qf : qosFilters) {
      qf.dump("qf=", os);
      os << ",";
    }
    os << "}}";
  }
};

struct QosParamResult_t {
  int32_t cid;
  Qos_t qos;
  vector<QosSession_t> qosSessions;
  void dump(std::string padding, std::ostream &os) const {
    os << padding << "{";
    os << "cid: " << "";
    os <<  cid << ",";
    os <<  "defaultQos : " << ",";
    qos.dump("" , os);
    os << "QosSessions " << "";
    for (auto session: qosSessions) {
      session.dump("", os);
      os << ",";
    }
    os << "}";
  }
};

struct DataCallResult_t {
  DataCallFailCause_t cause;
  int32_t suggestedRetryTime;
  int32_t cid;
  int32_t active;
  string type;
  string ifname;
  string addresses;
  vector<LinkAddress_t> linkAddresses;
  string dnses;
  string gateways;
  string pcscf;
  int32_t mtu;
  int32_t mtuV4;
  int32_t mtuV6;
  vector<QosSession_t> qosSessions;
  Qos_t defaultQos;
  void dump(std::string padding, std::ostream& os) const {
    os << padding << "cid=" << cid << "{"
        << (int)cause << ","
        << suggestedRetryTime << ","
        << active << ","
        << type << ","
        << ifname << ","
        << addresses << ",";
    os << "{";
    for (LinkAddress_t addr : linkAddresses) {
      addr.dump("", os);
      os << ",";
    }
    os << "},"
        << dnses << ","
        << gateways << ","
        << pcscf << ","
        << mtu << ","
        << mtuV4 << ","
        << mtuV6 << "}";
    defaultQos.dump("defqos=", os);
    os << ",qoss={";
    for (QosSession_t qs : qosSessions) {
      qs.dump("", os);
      os << ",";
    }
    os << "},";
  }
};

struct SetupDataCallResponse_t {
  ResponseError_t respErr;
  DataCallResult_t call;
  void dump(std::string padding, std::ostream& os) const {
    os << padding << "respErr=" << (int)respErr << " ";
    call.dump("", os);
  }
};

enum class DataCallTimerType : int {
  Handover = 0,
  SetupDataCall = 1,
  PartialRetry = 2,
  PartialRetryResponse = 3,
  DeactivateDataCall = 4,
};

enum HandoffNetworkType_t {
  _eWWAN  = 1 << 0,
  _eWLAN  = 1 << 1,
  _eIWLAN = 1 << 2,
};

enum LinkActiveState {
  _eInactive = 0,
  _eActivePhysicalLinkDown = 1,
  _eActivePhysicalLinkUp = 2
};

struct DataCallListResult_t {
  ResponseError_t respErr;
  vector<DataCallResult_t> call;
  void dump(std::string padding, std::ostream& os) const {
    os << padding << "respErr=" << (int)respErr;
    for (const DataCallResult_t& c : call) {
      c.dump("", os);
      os << ",";
    }
  }
};

struct QualifiedNetwork_t {
  int32_t         apnType;
  vector<int32_t> network;
  void dump(std::string padding, std::ostream& os) const {
    os << padding << "type=" << (ApnTypes_t)apnType << " networks=[";
    for (auto nw: network) {
      os << (AccessNetwork_t)nw << ",";
    }
    os << "]";
  }
};

enum class ScreenState_t : int {
  NONE = 0,
  SCREEN_OFF,
  SCREEN_ON,
};

class NrIconType_t {
public:
    NrIconType_t(NrIconEnum_t setIcon): icon(setIcon) {}
    bool isUwb() { return icon == NrIconEnum_t::UWB; }
    bool isBasic() { return icon == NrIconEnum_t::BASIC; }
    bool isNone() { return icon == NrIconEnum_t::NONE; }

private:
    NrIconEnum_t icon;
};

inline std::ostream& operator<<(std::ostream& os, RatType_t r)
{
  switch (r) {
    case RatType_t::RAT_UNSPECIFIED:
      os << "UNSPECIFIED";
      break;
    case RatType_t::RAT_4G:
      os << "4G";
      break;
    case RatType_t::RAT_5G:
      os << "5G";
      break;
    case RatType_t::RAT_SPLITED:
      os << "SPLIT";
      break;
    default:
      os << std::to_string((int)r);
      break;
  }
  return os;
}

inline std::ostream& operator<<(std::ostream& os, AccessNetwork_t an)
{
  switch (an) {
    case AccessNetwork_t::UNKNOWN:
      os << "UNKNOWN";
      break;
    case AccessNetwork_t::GERAN:
      os << "GERAN";
      break;
    case AccessNetwork_t::UTRAN:
      os << "UTRAN";
      break;
    case AccessNetwork_t::EUTRAN:
      os << "EUTRAN";
      break;
    case AccessNetwork_t::CDMA:
      os << "CDMA";
      break;
    case AccessNetwork_t::IWLAN:
      os << "IWLAN";
      break;
    case AccessNetwork_t::NGRAN:
      os << "NGRAN";
      break;
    default:
      os << std::to_string((int)an);
      break;
  }
  return os;
}

inline std::ostream& operator<<(std::ostream& os, ApnTypes_t type)
{
  switch (type) {
    case ApnTypes_t::NONE:
      os << "NONE";
      break;
    case ApnTypes_t::DEFAULT:
      os << "DEFAULT";
      break;
    case ApnTypes_t::MMS:
      os << "MMS";
      break;
    case ApnTypes_t::SUPL:
      os << "SUPL";
      break;
    case ApnTypes_t::DUN:
      os << "DUN";
      break;
    case ApnTypes_t::HIPRI:
      os << "HIPRI";
      break;
    case ApnTypes_t::FOTA:
      os << "FOTA";
      break;
    case ApnTypes_t::IMS:
      os << "IMS";
      break;
    case ApnTypes_t::CBS:
      os << "CBS";
      break;
    case ApnTypes_t::IA:
      os << "IA";
      break;
    case ApnTypes_t::EMERGENCY:
      os << "EMERGENCY";
      break;
    case ApnTypes_t::MCX:
      os << "MCX";
      break;
    case ApnTypes_t::XCAP:
      os << "XCAP";
      break;
    default:
      os << std::to_string((int)type);
      break;
  }
  return os;
}

inline std::ostream& operator<<(std::ostream& os, RadioAccessFamily_t r)
{
  switch (r) {
    case RadioAccessFamily_t::UNKNOWN:
      os << "UNKNOWN";
      break;
    case RadioAccessFamily_t::GPRS:
      os << "GPRS";
      break;
    case RadioAccessFamily_t::EDGE:
      os << "EDGE";
      break;
    case RadioAccessFamily_t::UMTS:
      os << "UMTS";
      break;
    case RadioAccessFamily_t::IS95A:
      os << "IS95A";
      break;
    case RadioAccessFamily_t::IS95B:
      os << "IS95B";
      break;
    case RadioAccessFamily_t::ONE_X_RTT:
      os << "ONE_X_RTT";
      break;
    case RadioAccessFamily_t::EVDO_0:
      os << "EVDO_0";
      break;
    case RadioAccessFamily_t::EVDO_A:
      os << "EVDO_A";
      break;
    case RadioAccessFamily_t::HSDPA:
      os << "HSDPA";
      break;
    case RadioAccessFamily_t::HSUPA:
      os << "HSUPA";
      break;
    case RadioAccessFamily_t::HSPA:
      os << "HSPA";
      break;
    case RadioAccessFamily_t::EVDO_B:
      os << "EVDO_B";
      break;
    case RadioAccessFamily_t::EHRPD:
      os << "EHRPD";
      break;
    case RadioAccessFamily_t::LTE:
      os << "LTE";
      break;
    case RadioAccessFamily_t::HSPAP:
      os << "HSPAP";
      break;
    case RadioAccessFamily_t::GSM:
      os << "GSM";
      break;
    case RadioAccessFamily_t::TD_SCDMA:
      os << "TD_SCDMA";
      break;
    case RadioAccessFamily_t::LTE_CA:
      os << "LTE_CA";
      break;
    case RadioAccessFamily_t::NR:
      os << "NR";
      break;
    case RadioAccessFamily_t::WLAN:
      os << "WLAN";
      break;
    default:
      os << std::to_string((int)r);
      break;
  }
  return os;
}

inline std::ostream& operator<<(std::ostream& os, FrequencyRange_t fr)
{
  switch (fr) {
    case FrequencyRange_t::UNKNOWN:
      os << "UNKNOWN";
      break;
    case FrequencyRange_t::LOW:
      os << "LOW";
      break;
    case FrequencyRange_t::MID:
      os << "MID";
      break;
    case FrequencyRange_t::HIGH:
      os << "HIGH";
      break;
    case FrequencyRange_t::MMWAVE:
      os << "MMWAVE";
      break;
    default:
      os << std::to_string((int)fr);
  }
  return os;
}

enum HandOverState {
  Failure = 0,
  Pending = 1,
  Success = 2,
};

inline int32_t toRilNgranBand(nas_active_band_enum_v01 qmi_band) {
    switch(qmi_band) {
        case NAS_ACTIVE_BAND_NR5G_BAND_1_V01:
            return NGRAN_BAND_1;
        case NAS_ACTIVE_BAND_NR5G_BAND_2_V01:
            return NGRAN_BAND_2;
        case NAS_ACTIVE_BAND_NR5G_BAND_3_V01:
            return NGRAN_BAND_3;
        case NAS_ACTIVE_BAND_NR5G_BAND_5_V01:
            return NGRAN_BAND_5;
        case NAS_ACTIVE_BAND_NR5G_BAND_7_V01:
            return NGRAN_BAND_7;
        case NAS_ACTIVE_BAND_NR5G_BAND_8_V01:
            return NGRAN_BAND_8;
        case NAS_ACTIVE_BAND_NR5G_BAND_12_V01:
            return NGRAN_BAND_12;
        case NAS_ACTIVE_BAND_NR5G_BAND_14_V01:
            return NGRAN_BAND_14;
        case NAS_ACTIVE_BAND_NR5G_BAND_20_V01:
            return NGRAN_BAND_20;
        case NAS_ACTIVE_BAND_NR5G_BAND_25_V01:
            return NGRAN_BAND_25;
        case NAS_ACTIVE_BAND_NR5G_BAND_28_V01:
            return NGRAN_BAND_28;
        case NAS_ACTIVE_BAND_NR5G_BAND_34_V01:
            return NGRAN_BAND_34;
        case NAS_ACTIVE_BAND_NR5G_BAND_38_V01:
            return NGRAN_BAND_38;
        case NAS_ACTIVE_BAND_NR5G_BAND_39_V01:
            return NGRAN_BAND_39;
        case NAS_ACTIVE_BAND_NR5G_BAND_40_V01:
            return NGRAN_BAND_40;
        case NAS_ACTIVE_BAND_NR5G_BAND_41_V01:
            return NGRAN_BAND_41;
        case NAS_ACTIVE_BAND_NR5G_BAND_48_V01:
            return NGRAN_BAND_48;
        case NAS_ACTIVE_BAND_NR5G_BAND_50_V01:
            return NGRAN_BAND_50;
        case NAS_ACTIVE_BAND_NR5G_BAND_51_V01:
            return NGRAN_BAND_51;
        case NAS_ACTIVE_BAND_NR5G_BAND_65_V01:
            return NGRAN_BAND_65;
        case NAS_ACTIVE_BAND_NR5G_BAND_66_V01:
            return NGRAN_BAND_66;
        case NAS_ACTIVE_BAND_NR5G_BAND_70_V01:
            return NGRAN_BAND_70;
        case NAS_ACTIVE_BAND_NR5G_BAND_71_V01:
            return NGRAN_BAND_71;
        case NAS_ACTIVE_BAND_NR5G_BAND_74_V01:
            return NGRAN_BAND_74;
        case NAS_ACTIVE_BAND_NR5G_BAND_75_V01:
            return NGRAN_BAND_75;
        case NAS_ACTIVE_BAND_NR5G_BAND_76_V01:
            return NGRAN_BAND_76;
        case NAS_ACTIVE_BAND_NR5G_BAND_77_V01:
            return NGRAN_BAND_77;
        case NAS_ACTIVE_BAND_NR5G_BAND_78_V01:
            return NGRAN_BAND_78;
        case NAS_ACTIVE_BAND_NR5G_BAND_79_V01:
            return NGRAN_BAND_79;
        case NAS_ACTIVE_BAND_NR5G_BAND_80_V01:
            return NGRAN_BAND_80;
        case NAS_ACTIVE_BAND_NR5G_BAND_81_V01:
            return NGRAN_BAND_81;
        case NAS_ACTIVE_BAND_NR5G_BAND_82_V01:
            return NGRAN_BAND_82;
        case NAS_ACTIVE_BAND_NR5G_BAND_83_V01:
            return NGRAN_BAND_83;
        case NAS_ACTIVE_BAND_NR5G_BAND_84_V01:
            return NGRAN_BAND_84;
        case NAS_ACTIVE_BAND_NR5G_BAND_85_V01:
            return NGRAN_BAND_85;
        case NAS_ACTIVE_BAND_NR5G_BAND_86_V01:
            return NGRAN_BAND_86;
        case NAS_ACTIVE_BAND_NR5G_BAND_257_V01:
            return NGRAN_BAND_257;
        case NAS_ACTIVE_BAND_NR5G_BAND_258_V01:
            return NGRAN_BAND_258;
        case NAS_ACTIVE_BAND_NR5G_BAND_259_V01:
            return NGRAN_BAND_259;
        case NAS_ACTIVE_BAND_NR5G_BAND_260_V01:
            return NGRAN_BAND_260;
        case NAS_ACTIVE_BAND_NR5G_BAND_261_V01:
            return NGRAN_BAND_261;
        default:
            return std::numeric_limits<int32_t>::max();
    }
}

//Created enum with only 5G Bands
//Add other RAT Bands to this enum, if needed in future
enum class NasActiveBand_t : int32_t
{
    NGRAN_BAND_MIN_ENUM_VAL = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
    NGRAN_BAND_1 = 1,
    NGRAN_BAND_2 = 2,
    NGRAN_BAND_3 = 3,
    NGRAN_BAND_5 = 5,
    NGRAN_BAND_7 = 7,
    NGRAN_BAND_8 = 8,
    NGRAN_BAND_12 = 12,
    NGRAN_BAND_14 = 14,
    NGRAN_BAND_18 = 18,
    NGRAN_BAND_20 = 20,
    NGRAN_BAND_25 = 25,
    NGRAN_BAND_26 = 26,
    NGRAN_BAND_28 = 28,
    NGRAN_BAND_29 = 29,
    NGRAN_BAND_30 = 30,
    NGRAN_BAND_34 = 34,
    NGRAN_BAND_38 = 38,
    NGRAN_BAND_39 = 39,
    NGRAN_BAND_40 = 40,
    NGRAN_BAND_41 = 41,
    NGRAN_BAND_48 = 48,
    NGRAN_BAND_50 = 50,
    NGRAN_BAND_51 = 51,
    NGRAN_BAND_53 = 53,
    NGRAN_BAND_65 = 65,
    NGRAN_BAND_66 = 66,
    NGRAN_BAND_70 = 70,
    NGRAN_BAND_71 = 71,
    NGRAN_BAND_74 = 74,
    NGRAN_BAND_75 = 75,
    NGRAN_BAND_76 = 76,
    NGRAN_BAND_77 = 77,
    NGRAN_BAND_78 = 78,
    NGRAN_BAND_79 = 79,
    NGRAN_BAND_80 = 80,
    NGRAN_BAND_81 = 81,
    NGRAN_BAND_82 = 82,
    NGRAN_BAND_83 = 83,
    NGRAN_BAND_84 = 84,
    NGRAN_BAND_85 = 85,
    NGRAN_BAND_86 = 86,
    NGRAN_BAND_89 = 89,
    NGRAN_BAND_90 = 90,
    NGRAN_BAND_91 = 91,
    NGRAN_BAND_92 = 92,
    NGRAN_BAND_93 = 93,
    NGRAN_BAND_94 = 94,
    NGRAN_BAND_95 = 95,
    NGRAN_BAND_257 = 257,
    NGRAN_BAND_258 = 258,
    NGRAN_BAND_259 = 259,
    NGRAN_BAND_260 = 260,
    NGRAN_BAND_261 = 261,
    NGRAN_BAND_MAX_ENUM_VAL = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
};

struct RilRespParams
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_evt_e_type event_id;
  RIL_Token t;
};
}
#endif
