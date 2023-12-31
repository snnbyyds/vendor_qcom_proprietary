/******************************************************************************
#  Copyright (c) 2010, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/******************************************************************************
  @file    qcril_qmi_nas2.h
  @brief   qcril qmi - NAS 2nd portion

  DESCRIPTION
    Handles RIL requests, Callbacks, indications for QMI NAS.

******************************************************************************/


#ifndef QCRIL_QMI_NAS2_H
#define QCRIL_QMI_NAS2_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "qmi_client.h"
#include "modules/nas/qcril_qmi_nas.h"
#include "network_access_service_v01.h"
#include "interfaces/nas/RilRequestSet5GStatusMessage.h"
#include "interfaces/nas/RilRequestQuery5GStatusMessage.h"
#include "interfaces/nas/RilRequestEnableEndcMessage.h"
#include "interfaces/nas/RilRequestQueryEndcStatusMessage.h"
#include "interfaces/nas/RilRequestSetNrConfigMessage.h"
#include "interfaces/nas/RilRequestQueryNrConfigMessage.h"

/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

#define QMI_NAS_RAT_MODE_PREF_CDMA                          ( 1 << QMI_NAS_RAT_MODE_PREF_CDMA2000_1X_BIT_V01 )
#define QMI_NAS_RAT_MODE_PREF_HRPD                          ( 1 << QMI_NAS_RAT_MODE_PREF_CDMA2000_HRPD_BIT_V01 )
#define QMI_NAS_RAT_MODE_PREF_GSM                           ( 1 << QMI_NAS_RAT_MODE_PREF_GSM_BIT_V01 )
#define QMI_NAS_RAT_MODE_PREF_UMTS                          ( 1 << QMI_NAS_RAT_MODE_PREF_UMTS_BIT_V01 )
#define QMI_NAS_RAT_MODE_PREF_TDSCDMA                       ( 1 << QMI_NAS_RAT_MODE_PREF_TDSCDMA_BIT_V01 )
#define QMI_NAS_RAT_MODE_PREF_LTE                           ( 1 << QMI_NAS_RAT_MODE_PREF_LTE_BIT_V01 )
#define QMI_NAS_RAT_MODE_PREF_NR5G                          ( 1 << QMI_NAS_RAT_MODE_PREF_NR5G_BIT_V01 )
#define QMI_NAS_RAT_MODE_PREF_CDMA_HRPD                     ( QMI_NAS_RAT_MODE_PREF_CDMA | QMI_NAS_RAT_MODE_PREF_HRPD )
#define QMI_NAS_RAT_MODE_PREF_GSM_UMTS                      ( QMI_NAS_RAT_MODE_PREF_GSM | QMI_NAS_RAT_MODE_PREF_UMTS )
#define QMI_NAS_RAT_MODE_PREF_UMTS_TDSCDMA                  ( QMI_NAS_RAT_MODE_PREF_UMTS | QMI_NAS_RAT_MODE_PREF_TDSCDMA )
#define QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA                   ( QMI_NAS_RAT_MODE_PREF_GSM | QMI_NAS_RAT_MODE_PREF_TDSCDMA )
#define QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_LTE               ( QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA | QMI_NAS_RAT_MODE_PREF_LTE )
#define QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA              ( QMI_NAS_RAT_MODE_PREF_GSM | QMI_NAS_RAT_MODE_PREF_UMTS | QMI_NAS_RAT_MODE_PREF_TDSCDMA )
#define QMI_NAS_RAT_MODE_PREF_UMTS_LTE                      ( QMI_NAS_RAT_MODE_PREF_UMTS | QMI_NAS_RAT_MODE_PREF_LTE )
#define QMI_NAS_RAT_MODE_PREF_TDSCDMA_LTE                   ( QMI_NAS_RAT_MODE_PREF_TDSCDMA | QMI_NAS_RAT_MODE_PREF_LTE )
#define QMI_NAS_RAT_MODE_PREF_GSM_UMTS_LTE                  ( QMI_NAS_RAT_MODE_PREF_GSM_UMTS | QMI_NAS_RAT_MODE_PREF_LTE )
#define QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_LTE          ( QMI_NAS_RAT_MODE_PREF_GSM_UMTS | QMI_NAS_RAT_MODE_PREF_TDSCDMA | QMI_NAS_RAT_MODE_PREF_LTE )
#define QMI_NAS_RAT_MODE_PREF_UMTS_TDSCDMA_LTE              ( QMI_NAS_RAT_MODE_PREF_UMTS | QMI_NAS_RAT_MODE_PREF_TDSCDMA_LTE )
#define QMI_NAS_RAT_MODE_PREF_GSM_UMTS_CDMA_HRPD            ( QMI_NAS_RAT_MODE_PREF_GSM_UMTS | QMI_NAS_RAT_MODE_PREF_CDMA_HRPD )
#define QMI_NAS_RAT_MODE_PREF_CDMA_HRPD_LTE                 ( QMI_NAS_RAT_MODE_PREF_CDMA_HRPD | QMI_NAS_RAT_MODE_PREF_LTE )
#define QMI_NAS_RAT_MODE_PREF_GSM_CDMA_HRPD_LTE             ( QMI_NAS_RAT_MODE_PREF_GSM | QMI_NAS_RAT_MODE_PREF_CDMA_HRPD | QMI_NAS_RAT_MODE_PREF_LTE )
#define QMI_NAS_RAT_MODE_PREF_GSM_UMTS_CDMA_HRPD_LTE        ( QMI_NAS_RAT_MODE_PREF_GSM_UMTS_LTE | QMI_NAS_RAT_MODE_PREF_CDMA_HRPD )
#define QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_CDMA_HRPD_LTE     ( QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_LTE | QMI_NAS_RAT_MODE_PREF_CDMA_HRPD )
#define QMI_NAS_RAT_MODE_PREF_GSM_TDSCDMA_CDMA_HRPD_UMTS     ( QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA | QMI_NAS_RAT_MODE_PREF_CDMA_HRPD )
#define QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_CDMA_HRPD_LTE ( QMI_NAS_RAT_MODE_PREF_GSM_UMTS_TDSCDMA_LTE | QMI_NAS_RAT_MODE_PREF_CDMA_HRPD )
#define QMI_NAS_RAT_MODE_PREF_LTE_NR5G                       (QMI_NAS_RAT_MODE_PREF_LTE | QMI_NAS_RAT_MODE_PREF_NR5G)


void qcril_qmi_nas2_find_static_operator_name
(
  const char * mcc_str,
  const char * mnc_str,
  const char * mcc_mnc_str_ref,
  const char **long_ons_ptr,
  const char **short_ons_ptr
);

int qcril_qmi_nas2_find_3gpp2_static_operator_name
(
  char * mcc_str,
  char * mnc_str,
  uint16_t sid,
  uint16_t nid,
  char **long_ons_ptr,
  char **short_ons_ptr
);

void qcril_qmi_nas2_find_static_cc_operator_name
(
  const char *sim_mcc_str,
  const char *mcc_str,
  const char *mnc_str,
  const char **long_ons_ptr,
  const char **short_ons_ptr
);

void qcril_qmi_nas2_find_elaboration_static_name(const char * mcc_mnc_str, char** long_name_res, char ** short_name_res );

void qcril_qmi_nas2_set_max_transmit_power
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
);

void qcril_qmi_nas2_get_sar_rev_key
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

mode_pref_mask_type_v01 qcril_qmi_nas2_convert_rat_to_mode_pref(int rat);

nas_radio_if_enum_v01
    qcril_qmi_nas2_convert_qcril_rat_to_qmi_rat(RIL_RadioTechnology qcril_rat);

RIL_Errno qmi_ril_nwreg_request_mode_pref( int android_mode_pref, uint8_t *is_change );

char* qcril_qmi_nas2_retrieve_mcc_from_iccid(char *iccid);

unsigned int qcril_qmi_nas_get_radio_tech(uint16_t mode_pref);

void qcril_qmi_nas_request_set_preferred_network_band_pref
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_request_set_preferred_network_acq_order
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_request_get_preferred_network_band_pref
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_nas_request_get_preferred_network_acq_order(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qmi_ril_nas_cache_deferred_acq_order
(
    uint32_t acq_order_len,
    qcril_qmi_acq_order_e_type acq_order_map,
    nas_radio_if_enum_v01 *acq_order
);
uint8_t qmi_ril_nas_get_deferred_acq_order_map( qcril_qmi_acq_order_e_type *acq_order_map );
uint8_t qcril_qmi_nas_get_gw_acq_order_pref (uint16_t *gw_acq_order_pref);
uint8_t qcril_qmi_nas_get_acq_order(uint32_t *acq_order_len, nas_radio_if_enum_v01 *acq_order);
uint8_t qmi_ril_nas_get_deferred_acq_order( uint32_t *acq_order_len, nas_radio_if_enum_v01 *acq_order );
uint8_t qcril_qmi_nas_check_is_indication_received();
RIL_Errno qcril_qmi_nas2_create_reqlist_setup_timer_helper(std::shared_ptr<RilRequestSetPrefNetworkTypeMessage> msg);

void qcril_qmi_nas_request_enable_5g
(
    std::shared_ptr<RilRequestSet5GStatusMessage> msg
);

void qcril_qmi_nas_request_enable_5g_only
(
    std::shared_ptr<RilRequestSet5GStatusMessage> msg
);

void qcril_qmi_nas_request_disable_5g
(
    std::shared_ptr<RilRequestSet5GStatusMessage> msg
);

void qcril_qmi_nas_request_query_5g_status
(
    std::shared_ptr<RilRequestQuery5GStatusMessage> msg
);

void qcril_qmi_nas_request_enable_endc
(
    std::shared_ptr<RilRequestEnableEndcMessage> msg
);

void qcril_qmi_nas_request_query_endc_status
(
    std::shared_ptr<RilRequestQueryEndcStatusMessage> msg
);

void qcril_qmi_nas_request_set_nr_config
(
    std::shared_ptr<RilRequestSetNrConfigMessage> msg
);

void qcril_qmi_nas_request_query_nr_config
(
    std::shared_ptr<RilRequestQueryNrConfigMessage> msg
);
#endif /* QCRIL_QMI_NAS2_H */

