/******************************************************************************
#  Copyright (c) 2013-2014, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/******************************************************************************
  @file    qcril_qmi_pdc.h
  @brief   qcril qmi - PDC

  DESCRIPTION
    Handles RIL requests, Callbacks, indications for QMI NAS.

******************************************************************************/


#ifndef QCRIL_QMI_PDC_H
#define QCRIL_QMI_PDC_H


/*===========================================================================

                           INCLUDE FILES

============================================================================*/

#include "qmi_client.h"
#include "qcril_other.h"
#ifndef QMI_RIL_UTF
#if !defined(LIBXML_UNSUPPORTED) && !defined(__cplusplus)
#include <libxml/parser.h>
#include <libxml/xpath.h>
#else
#define xmlChar unsigned char
#endif
#endif
#include <persistent_device_configuration_v01.h>
#include <interfaces/mbn/QcRilRequestSetMbnConfigMessage.h>
#include <interfaces/mbn/QcRilRequestGetMbnConfigMessage.h>
#include <interfaces/mbn/QcRilRequestGetAvlMbnConfigMessage.h>
#include <interfaces/mbn/QcRilRequestCleanupMbnConfigMessage.h>
#include <interfaces/mbn/QcRilRequestSelectMbnConfigMessage.h>
#include <interfaces/mbn/QcRilRequestGetMetaInfoMessage.h>
#include <interfaces/mbn/QcRilRequestDeactivateMbnConfigMessage.h>
#include <interfaces/mbn/QcRilRequestGetQcVersionOfFileMessage.h>
#include <interfaces/mbn/QcRilRequestValidateMbnConfigMessage.h>
#include <interfaces/mbn/QcRilRequestGetQcVersionOfConfigMessage.h>
#include <interfaces/mbn/QcRilRequestGetOemVersionOfFileMessage.h>
#include <interfaces/mbn/QcRilRequestGetOemVersionOfConfigMessage.h>
#include <interfaces/mbn/QcRilRequestActivateMbnConfigMessage.h>
#include <interfaces/mbn/QcRilRequestEnableModemUpdateMessage.h>
#include <interfaces/mbn/QcRilUnsolMbnConfigResultMessage.h>
#include <interfaces/mbn/QcRilUnsolMbnConfigClearedMessage.h>
#include <interfaces/mbn/QcRilUnsolMbnValidateDumpedMessage.h>
#include <interfaces/mbn/QcRilUnsolMbnConfigListMessage.h>
#include <interfaces/mbn/QcRilUnsolMbnValidateConfigMessage.h>

/*===========================================================================

                        DEFINITIONS AND TYPES

 ===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

#define QCRIL_DUMP_FILE_PREFIX DATA_PARTITION_ROOT "radio/validation_result_sub"
#define QCRIL_DISABLE_MODEM_UPDATE_SLOT_ID  0xFFFFFFFF
#define QCRIL_MBN_FILE_PATH_LEN             255
#define QCRIL_MBN_CONFIG_ID_LEN             124
#define QCRIL_MBN_CONFIG_DESC_LEN           255
#define QCRIL_DUMP_FILE_PATH_LEN            255
#define QCRIL_PDC_ALL_CONFIGS_LEN           ((PDC_CONFIG_ID_SIZE_MAX_V01+1) * PDC_CONFIG_LIST_SIZE_MAX_V01+1)

void qcril_qmi_pdc_set_modem_test_mode
(
  std::shared_ptr<QcRilRequestSetMbnConfigMessage> msg
);

void qcril_qmi_pdc_query_modem_test_mode
(
  std::shared_ptr<QcRilRequestGetMbnConfigMessage> msg
);

void qcril_qmi_pdc_get_available_configs
(
  std::shared_ptr<QcRilRequestGetAvlMbnConfigMessage> msg
);

void qcril_qmi_pdc_enable_modem_update
(
  std::shared_ptr<QcRilRequestEnableModemUpdateMessage> msg
);

qmi_client_error_type qcril_qmi_pdc_init
(
  void
);

void qcril_qmi_pdc_load_configuration
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);
void qcril_qmi_pdc_select_configuration
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_activate_configuration
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_deactivate_current_config
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_cleanup_loaded_configs
(
  std::shared_ptr<QcRilRequestCleanupMbnConfigMessage> msg
);

void qcril_qmi_pdc_select_configs
(
  std::shared_ptr<QcRilRequestSelectMbnConfigMessage> msg
);

void qcril_qmi_pdc_get_meta_info_of_config
(
  std::shared_ptr<QcRilRequestGetMetaInfoMessage> msg
);

void qcril_qmi_pdc_delete_configuration
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_list_configuration_event_hdlr
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_deactivate_configuration
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_deactivate_configs
(
  std::shared_ptr<QcRilRequestDeactivateMbnConfigMessage> msg
);

void qcril_qmi_pdc_get_qc_version_of_file
(
  std::shared_ptr<QcRilRequestGetQcVersionOfFileMessage> msg
);

void qcril_qmi_pdc_get_qc_version_of_configid
(
  std::shared_ptr<QcRilRequestGetQcVersionOfConfigMessage> msg
);

void qcril_qmi_pdc_validate_config
(
  std::shared_ptr<QcRilRequestValidateMbnConfigMessage> msg
);

void qcril_qmi_pdc_parse_diff_result
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_get_oem_version_of_file
(
  std::shared_ptr<QcRilRequestGetOemVersionOfFileMessage> msg
);

void qcril_qmi_pdc_get_oem_version_of_configid
(
  std::shared_ptr<QcRilRequestGetOemVersionOfConfigMessage> msg
);

void qcril_qmi_pdc_activate_configs
(
  std::shared_ptr<QcRilRequestActivateMbnConfigMessage> msg
);

void qcril_mbn_pdc_delete_all_sw_configs
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_mbn_pdc_query_pending_sw_config
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

IxErrnoType qcril_mbn_sw_send_get_config_info_resp
(
    RIL_Errno result,
    boolean has_base_version,
    uint32_t base_version,
    boolean has_config_version,
    uint32_t config_version
);


void qcril_qmi_pdc_retrieve_current_mbn_info
(
    void
);

RIL_Errno qcril_qmi_pdc_get_selected_mbn_config
(
    int                      sub_id,
    pdc_config_type_enum_v01 config_type
);

RIL_Errno qcril_qmi_pdc_disable_modem_update
(
   void
);

void qcril_qmi_pdc_reset_mbn_state();

/*=========================================================================

  FUNCTION:  qcril_qmi_pdc_deactivate_configuration_hdlr

===========================================================================*/
/*!
    @brief
    Deactivate configuration

    @return
    None.
*/
/*=========================================================================*/
RIL_Errno qcril_qmi_pdc_deactivate_configuration_hdlr
(
    uint32_t                 sub_id,
    pdc_config_type_enum_v01 config_type
);

/*=========================================================================

  FUNCTION:  qcril_qmi_pdc_activate_configuration_hdlr

===========================================================================*/
/*!
    @brief
    Activate configuration

    @return
    None.
*/
/*=========================================================================*/
RIL_Errno qcril_qmi_pdc_activate_configuration_hdlr
(
    uint32_t                 sub_id,
    pdc_config_type_enum_v01 config_type
);

/*=========================================================================

  FUNCTION:  qcril_qmi_pdc_delete_configuration_hdlr

===========================================================================*/
/*!
    @brief
    Delete configuration

    @return
    None.
*/
/*=========================================================================*/
RIL_Errno qcril_qmi_pdc_delete_configuration_hdlr
(
    uint8_t                 *config_id,
    uint32_t                 config_id_len,
    pdc_config_type_enum_v01 config_type
);

/*=========================================================================

  FUNCTION:  qcril_qmi_pdc_set_select_configuration

===========================================================================*/
/*!
    @brief
    Select a configuration

    @return
    None.
*/
/*=========================================================================*/
RIL_Errno qcril_qmi_pdc_set_select_configuration
(
    uint32_t                 sub_id,
    pdc_config_type_enum_v01 config_type,
    uint8_t                 *config_id,
    uint32_t                 config_id_len
);

RIL_Errno qcril_qmi_pdc_load_config_file
(
    uint32_t sub_id,
    const char *file_path,
    const char *config_id,
    pdc_config_type_enum_v01 config_type
);

void qcril_qmi_pdc_unsol_ind_cb_hndlr
(
    unsigned int   msg_id,
    unsigned char * decoded_payload,
    uint32_t decoded_payload_len
);

#ifndef QMI_RIL_UTF
void qcril_qmi_mbn_diff_send_unsol_msg
(
  int result,
  int index,
  xmlChar* id,
  xmlChar* ref_val,
  xmlChar* dev_val
);
#endif

/*=========================================================================

  FUNCTION:  qcril_qmi_pdc_acquire_pdc_ownership_for_oem_hook

===========================================================================*/
/*!
    @brief
    Acquire pdc ownership

    @return
    True if success, else failure
*/
/*=========================================================================*/
boolean qcril_qmi_pdc_acquire_pdc_ownership_for_oem_hook
(
    void
);

/*===========================================================================

  FUNCTION:  qcril_qmi_pdc_set_sim_info

===========================================================================*/
/*!
    @brief
    Set pdc iccid, mcc, mnc

    @return
    None.
*/
 /*=========================================================================*/

void qcril_qmi_pdc_set_sim_info
(
    char *iccid,
    char *mcc,
    char *mnc
);


/*===========================================================================

  FUNCTION:  qcril_qmi_pdc_pre_init

===========================================================================*/
/*!
    @brief
    Pre init pdc

    @return
    None.
*/
 /*=========================================================================*/
void qcril_qmi_pdc_pre_init
(
  void
);


/*===========================================================================

  FUNCTION: qcril_qmi_pdc_get_sim_info

===========================================================================*/
/*!
    @brief
    Get pdc iccid, mcc, mnc

    @return
    None.
*/
 /*=========================================================================*/

void qcril_qmi_pdc_get_sim_info
(
    int   instance_id,
    char *iccid,
    int   icc_id_len,
    char  *mcc,
    char  mcc_len,
    char  *mnc,
    char  mnc_len
);


/*===========================================================================

  FUNCTION:  qcril_qmi_pdc_get_sub_id

===========================================================================*/
/*!
    @brief
    Get pdc sub_id

    @return
    None.
*/
 /*=========================================================================*/
int qcril_qmi_pdc_get_sub_id
(
    int instance_id
);


/*===========================================================================

  FUNCTION:  qcril_qmi_pdc_set_sub_id

===========================================================================*/
/*!
    @brief
    Initialize the PDC subsystem of the RIL.

    @return
    None.
*/
 /*=========================================================================*/
void qcril_qmi_pdc_set_sub_id
(
  int sub_id
);


//===========================================================================
// qcril_qmi_pdc_get_config_info
//===========================================================================
RIL_Errno qcril_qmi_pdc_get_config_info
(
    const char    *config_id,
    int      config_id_len,
    pdc_config_type_enum_v01 config_type
);

#ifdef QMI_RIL_UTF
void qcril_qmi_hal_pdc_module_cleanup();
#endif

#ifdef __cplusplus
}
#endif


#endif
