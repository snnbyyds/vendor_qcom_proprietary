/******************************************************************************
#  Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/******************************************************************************
  @file    qcril_qmi_radio_config_imss_v02.h

  DESCRIPTION
    Handles data related radio config req and response handlers

******************************************************************************/
#ifndef QCRIL_QMI_RADIO_CONFIG_IMSS_V02_H
#define QCRIL_QMI_RADIO_CONFIG_IMSS_V02_H

#include "qcrili.h"
#include "qcril_qmi_radio_config_meta.h"

/*===========================================================================
                          GET REQUEST HANDLERS
===========================================================================*/

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_voice_dynamic_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_codec_dynamic_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_sip_new_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_reg_mgr_new_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_ims_service_enable_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_sms_new_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_presence_new_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_rtp_dynamic_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_emer_dynamic_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_video_dynamic_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_rcs_new_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_ims_new_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

/*===========================================================================
                          SET REQUEST HANDLERS
===========================================================================*/

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_voice_dynamic_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_codec_dynamic_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_sip_new_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_reg_mgr_new_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_ims_service_enable_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_sms_new_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_presence_new_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_rtp_dynamic_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_emer_dynamic_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_video_dynamic_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_rcs_new_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_ims_new_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_concurrency_config_req_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);
/*===========================================================================
                           GET RESPONSE HANDLERS
===========================================================================*/

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_voice_dynamic_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_codec_dynamic_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_sip_new_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_reg_mgr_new_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_ims_service_enable_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_sms_new_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_presence_new_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_rtp_dynamic_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_emer_dynamic_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_video_dynamic_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_rcs_new_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_get_ims_new_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

/*===========================================================================
                           SET RESPONSE HANDLERS
===========================================================================*/

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_voice_dynamic_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_codec_dynamic_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_sip_new_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_reg_mgr_new_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_ims_service_enable_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_sms_new_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_presence_new_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_rtp_dynamic_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_emer_dynamic_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_video_dynamic_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_rcs_new_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_ims_new_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);

qcril_qmi_ims_config_error_type qcril_qmi_radio_config_imss_set_concurrency_config_resp_handler
(
  const qcril_qmi_radio_config_params_type *const config_ptr,
  uint16_t req_id
);
#endif /* QCRIL_QMI_RADIO_CONFIG_IMSS_V02_H */
