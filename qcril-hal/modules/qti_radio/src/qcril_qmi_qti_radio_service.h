/******************************************************************************
  @file    qcril_qmi_qti_radio_service.h
  @brief   qcril qmi - qtiRadioService_service
---------------------------------------------------------------------------

  Copyright (c) 2017, 2022-2023 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
  ******************************************************************************/

#ifndef VENDOR_QTI_HARDWARE_QTIRADIO_V2_7_QTIRADIO_H
#define VENDOR_QTI_HARDWARE_QTIRADIO_V2_7_QTIRADIO_H

#include <vendor/qti/hardware/radio/qtiradio/2.7/IQtiRadio.h>
#include <vendor/qti/hardware/radio/qtiradio/2.7/IQtiRadioResponse.h>
#include <vendor/qti/hardware/radio/qtiradio/2.7/IQtiRadioIndication.h>

#include "UnSolMessages/QosDataChangedIndMessage.h"
#include <android/hardware/radio/1.0/types.h>
#include <framework/legacy.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <hidl/HidlSupport.h>
#include <telephony/ril.h>
#include <QtiMutex.h>

#include "QtiRadioContext.h"

using namespace vendor::qti::hardware::radio::qtiradio::V1_0;
using ::android::hardware::Return;
using ::android::hardware::hidl_death_recipient;
using ::android::hardware::hidl_string;
using ::android::sp;
using ::android::wp;
using android::hardware::radio::V1_0::RadioResponseInfo;

extern void onRequest(int request, void *data, size_t datalen, RIL_Token t);
extern void constructCdmaSms(RIL_CDMA_SMS_Message &rcsm, const android::hardware::radio::V1_0::CdmaSmsMessage& sms);
namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V1_0 {
namespace implementation {

RIL_Token qcril_qmi_qti_convert_radio_token_to_ril_token(uint32_t oem_token);
uint32_t qcril_qmi_qti_free_and_convert_ril_token_to_radio_token(RIL_Token ril_token);
void qcril_qmi_qti_radio_service_init(int instanceId);
void qtiGetAtrResponse(int token, RIL_Errno error, const char *buf, int bufLen);
V2_0::EnableStatus convert_five_g_status(five_g_status status);
V2_0::DcParam convert_five_g_endc_dcnr(five_g_endc_dcnr &endc_dcnr_info);
void initialize(V2_0::DcParam &dc_param);
V2_0::SignalStrength convert_five_g_signal_strength(five_g_signal_strength &signal_strength);
void initialize(V2_0::SignalStrength &ss);
V2_0::BearerStatus convert_five_g_bearer_status(five_g_bearer_status bearer_status);
V2_1::BearerStatus convert_five_g_bearer_status_2_1(five_g_bearer_status bearer_status);
V2_1::PlmnInfoListStatus convert_five_g_plmn_list_info_status(five_g_plmn_info_list_status plmn_list_status);
V2_1::UpperLayerIndStatus convert_five_g_upper_layer_ind_status(five_g_upper_layer_ind_status upli_status);
V2_1::UpperLayerIndInfo convert_five_g_upper_layer_ind_info(five_g_upper_layer_ind_info &five_g_upli_info);
void initialize(V2_1::UpperLayerIndInfo &upli_info);
V2_1::ConfigType convert_five_g_config_info(five_g_config_type config);
V2_2::NrIconType convert_five_g_icon_type(five_g_icon_type config);

void on5gStatusChange(five_g_status status);
void onNrDcParamChange(five_g_endc_dcnr dcParam);
void onNrBearerAllocationChange(five_g_bearer_status bearerStatus);
void onSignalStrengthChange(five_g_signal_strength signalStrength);
void onUpperLayerIndInfoChange(five_g_upper_layer_ind_info upli_info);
void on5gConfigInfoChange(five_g_config_type config);
void queryNrIconTypeResponse(RIL_Token token, RIL_Errno err_num, five_g_icon_type config);
void onNrIconTypeChange(five_g_icon_type config);
void onQosDataChanged(rildata::QosParamResult_t &result);
void populateResponseInfo(QtiRadioResponseInfo& responseInfo, int serial,
                          QtiRadioResponseType responseType, RIL_Errno e);

void registerService(qcril_instance_id_e_type instanceId);

class QtiRadioImpl : public V2_4::IQtiRadio, public hidl_death_recipient {
protected:
  sp<V1_0::IQtiRadioResponse> mResponseCb;
  sp<V1_0::IQtiRadioIndication> mIndicationCb;

  sp<V2_0::IQtiRadioResponse> mResponseCbV2_0;
  sp<V2_0::IQtiRadioIndication> mIndicationCbV2_0;

  sp<V2_1::IQtiRadioResponse> mResponseCbV2_1;
  sp<V2_1::IQtiRadioIndication> mIndicationCbV2_1;

  sp<V2_2::IQtiRadioResponse> mResponseCbV2_2;
  sp<V2_2::IQtiRadioIndication> mIndicationCbV2_2;

  sp<V2_3::IQtiRadioResponse> mResponseCbV2_3;

  sp<V2_4::IQtiRadioResponse> mResponseCbV2_4;
  sp<V2_5::IQtiRadioResponse> mResponseCbV2_5;
  sp<V2_6::IQtiRadioResponse> mResponseCbV2_6;
  sp<V2_7::IQtiRadioResponse> mResponseCbV2_7;
  sp<V2_7::IQtiRadioIndication> mIndicationCbV2_7;
  qtimutex::QtiSharedMutex mCallbackLock;


  qcril_instance_id_e_type mInstanceId;

  sp<V1_0::IQtiRadioResponse> getResponseCallback();
  sp<V1_0::IQtiRadioIndication> getIndicationCallback();
  sp<V2_0::IQtiRadioResponse> getResponseCallbackV2_0();
  sp<V2_0::IQtiRadioIndication> getIndicationCallbackV2_0();
  sp<V2_1::IQtiRadioResponse> getResponseCallbackV2_1();
  sp<V2_1::IQtiRadioIndication> getIndicationCallbackV2_1();
  sp<V2_2::IQtiRadioResponse> getResponseCallbackV2_2();
  sp<V2_2::IQtiRadioIndication> getIndicationCallbackV2_2();
  sp<V2_3::IQtiRadioResponse> getResponseCallbackV2_3();
  sp<V2_4::IQtiRadioResponse> getResponseCallbackV2_4();
  // Function from hidl_death_recipient
  void serviceDied(uint64_t, const ::android::wp<::android::hidl::base::V1_0::IBase> &);

#ifdef QMI_RIL_UTF
public:
#endif

  // Functions from IQtiRadio
  Return<void> getAtr(int32_t serial);
  Return<void> enable5g(int32_t serial);
  Return<void> disable5g(int32_t serial);
  Return<void> enable5gOnly(int32_t serial);
  Return<void> query5gStatus(int32_t serial);
  Return<void> enableEndc(int32_t serial, bool enable);
  Return<void> queryEndcStatus(int32_t serial);
  Return<void> queryNrDcParam(int32_t serial);
  Return<void> queryNrBearerAllocation(int32_t serial);
  Return<void> queryNrSignalStrength(int32_t serial);
  Return<void> queryUpperLayerIndInfo(int32_t serial);
  Return<void> query5gConfigInfo(int32_t serial);
  Return<void> sendCdmaSms(int32_t serial,
                           const android::hardware::radio::V1_0::CdmaSmsMessage& sms,
                           bool expectMore);
  Return<int32_t> getPropertyValueInt(const hidl_string& prop, int32_t def);
  Return<bool> getPropertyValueBool(const hidl_string& prop, bool def);
  Return<void> getPropertyValueString(const hidl_string& prop, const hidl_string& def,
                                      getPropertyValueString_cb _hidl_cb);
  Return<void> queryNrIconType(int32_t serial);
  Return<void> setCarrierInfoForImsiEncryption(int32_t serial,
        const V2_4::ImsiEncryptionInfo& data);

  // Functions from IQtiRadio
  Return<void> setCallback(const sp<IQtiRadioResponse> &responseCallback,
                           const sp<IQtiRadioIndication> &indicationCallback);
  void setCallBackNoLock(const sp<IQtiRadioResponse> &responseCallback,
                         const sp<IQtiRadioIndication> &indicationCallback);
  void clearCallbacks();
public:
  QtiRadioImpl();
  virtual void setInstanceId(qcril_instance_id_e_type instanceId);
  virtual void getAtrResponse(int token, RIL_Errno error, const char *buf, int bufLen);
  virtual void sendQtiIndication();
  virtual void on5gStatusChange(five_g_status status);
  virtual void onNrDcParamChange(five_g_endc_dcnr dcParam);
  virtual void onNrBearerAllocationChange(five_g_bearer_status bearerStatus);
  virtual void onSignalStrengthChange(five_g_signal_strength signalStrength);
  virtual void onUpperLayerIndInfoChange(five_g_upper_layer_ind_info upli_info);
  virtual void on5gConfigInfoChange(five_g_config_type config);
  virtual void queryNrIconTypeResponse(RIL_Token token, RIL_Errno err_num, five_g_icon_type config);
  virtual void onNrIconTypeChange(five_g_icon_type config);
  virtual void onQosDataChanged(rildata::QosParamResult_t &) {}
  virtual std::shared_ptr<QtiRadioContext> getContext(uint32_t serial) {
    return std::make_shared<QtiRadioContext>(mInstanceId, serial);
  }
};

class QtiRadioImpl_2_5 : public QtiRadioImpl {
protected:
  sp<V2_5::IQtiRadioResponse> getResponseCallbackV2_5();
  Return<void> setCallback(const sp<IQtiRadioResponse> &responseCallback,
                           const sp<IQtiRadioIndication> &indicationCallback);
  void setCallBackNoLock(const sp<IQtiRadioResponse> &responseCallback,
                         const sp<IQtiRadioIndication> &indicationCallback);
public:
  Return<void> setNrConfig(int32_t serial, const V2_5::NrConfig config);
  Return<void> queryNrConfig(int32_t serial);
};

class QtiRadioImpl_2_6 : public QtiRadioImpl_2_5 {
protected:
  sp<V2_6::IQtiRadioResponse> getResponseCallbackV2_6();
  Return<void> setCallback(const sp<IQtiRadioResponse> &responseCallback,
                           const sp<IQtiRadioIndication> &indicationCallback);
  void setCallBackNoLock(const sp<IQtiRadioResponse> &responseCallback,
                         const sp<IQtiRadioIndication> &indicationCallback);
public:
  Return<void> getQtiRadioCapability(int32_t serial);
};

class QtiRadioImpl_2_7 : public QtiRadioImpl_2_6 {
protected:
  sp<V2_7::IQtiRadioResponse> getResponseCallbackV2_7();
  sp<V2_7::IQtiRadioIndication> getIndicationCallbackV2_7();
  Return<void> setCallback(const sp<IQtiRadioResponse> &responseCallback,
                           const sp<IQtiRadioIndication> &indicationCallback);
  void sendGetQosParametersResponse(RadioResponseInfo &responseInfo,
                                    V2_7::QosParametersResult &result);
  void setCallBackNoLock(const sp<IQtiRadioResponse> &responseCallback,
                         const sp<IQtiRadioIndication> &indicationCallback);
public:
  Return<void> getQosParameters(int32_t serial, int32_t cid);
  virtual void onQosDataChanged(rildata::QosParamResult_t &result);
};

} // namespace implementation
} // namespace V2_0
} // namespace qtiradio
} // namespace radio
} // namespace hardware
} // namespace qti
} // namespace vendor
#endif // VENDOR_QTI_HARDWARE_QTIRADIO_V2_7_QTIRADIO_H
