/******************************************************************************
#  Copyright (c) 2015, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/*!
  @file
  qcril_qmi_radio_config_data.h

  @brief
  Contains data related radio config structures, req and response handlers

*/


#ifndef QCRIL_QMI_RADIO_CONFIG_DATA_H
#define QCRIL_QMI_RADIO_CONFIG_DATA_H


#include <telephony/ril.h>
#include "qcrili.h"


//----PLACEHOLDERS FOR NOW-----
/* **needs to be discussed**
 * Since this is a sync request, it will call the send response handler directly
 * which will have to take care of mapping the req-resp config items
 */
RIL_Errno qcril_qmi_radio_config_data_set_quality_measurement_req_handler
(
  const void *const config_ptr,
  uint16_t req_id
);

/* **needs to be discussed**
 * Should be called from the req handler..!need to be discussed!
 */
RIL_Errno qcril_qmi_radio_config_data_set_quality_measurement_resp_handler
(
  const void *const config_ptr,
  uint16_t req_id
);

#endif /* QCRIL_QMI_RADIO_CONFIG_IMSS_H */
