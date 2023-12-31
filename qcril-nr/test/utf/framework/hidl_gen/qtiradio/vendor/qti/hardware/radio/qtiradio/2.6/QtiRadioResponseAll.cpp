/*
  Copyright (c) 2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
#define LOG_TAG "vendor.qti.hardware.radio.qtiradio@2.6::QtiRadioResponse"

#undef UNUSED
#include <vendor/qti/hardware/radio/qtiradio/2.6/IQtiRadioResponse.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V2_6 {

const char* IQtiRadioResponse::descriptor("vendor.qti.hardware.radio.qtiradio@2.6::IQtiRadioResponse");


// Methods from ::vendor::qti::hardware::radio::qtiradio::V1_0::IQtiRadioResponse follow.
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::getAtrResponse(const ::vendor::qti::hardware::radio::qtiradio::V1_0::QtiRadioResponseInfo& info, const ::android::hardware::hidl_string& atr)

// Methods from ::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadioResponse follow.
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::onEnable5gResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_0::Status status)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::onDisable5gResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_0::Status status)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::onEnable5gOnlyResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_0::Status status)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::on5gStatusResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus enabled)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::onNrDcParamResponse(int32_t serial, uint32_t errorCode, const ::vendor::qti::hardware::radio::qtiradio::V2_0::DcParam& dcParam)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::onNrBearerAllocationResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus bearerStatus)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::onSignalStrengthResponse(int32_t serial, uint32_t errorCode, const ::vendor::qti::hardware::radio::qtiradio::V2_0::SignalStrength& signalStrength)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::sendCdmaSmsResponse(const ::vendor::qti::hardware::radio::qtiradio::V1_0::QtiRadioResponseInfo& info, const ::android::hardware::radio::V1_0::SendSmsResult& sms)

// Methods from ::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioResponse follow.
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::onUpperLayerIndInfoResponse(int32_t serial, uint32_t errorCode, const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndInfo& uliInfo)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::onNrBearerAllocationResponse_2_1(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus bearerStatus)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::on5gConfigInfoResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType config)

// Methods from ::vendor::qti::hardware::radio::qtiradio::V2_2::IQtiRadioResponse follow.
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::onNrIconTypeResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType iconType)

// Methods from ::vendor::qti::hardware::radio::qtiradio::V2_3::IQtiRadioResponse follow.
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::onEnableEndcResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_0::Status status)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::onEndcStatusResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus endcStatus)

// Methods from ::vendor::qti::hardware::radio::qtiradio::V2_4::IQtiRadioResponse follow.
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::setCarrierInfoForImsiEncryptionResponse(const ::vendor::qti::hardware::radio::qtiradio::V1_0::QtiRadioResponseInfo& info)

// Methods from ::vendor::qti::hardware::radio::qtiradio::V2_5::IQtiRadioResponse follow.
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::setNrConfigResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_5::Status status)
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::onNrConfigResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_5::NrConfig config)

// Methods from ::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse follow.
// no default implementation for: ::android::hardware::Return<void> IQtiRadioResponse::getQtiRadioCapabilityResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::qtiradio::V2_6::RadioAccessFamily> raf)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IQtiRadioResponse::interfaceChain(interfaceChain_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiRadioResponse::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiRadioResponse::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    _hidl_cb(::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse::descriptor);
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiRadioResponse::getHashChain(getHashChain_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiRadioResponse::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IQtiRadioResponse::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IQtiRadioResponse::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiRadioResponse::getDebugInfo(getDebugInfo_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiRadioResponse::notifySyspropsChanged(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IQtiRadioResponse::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}

::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse>> IQtiRadioResponse::castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse>& parent, bool /* emitError */) {
    return parent;
}
::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse>> IQtiRadioResponse::castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_5::IQtiRadioResponse>& parent, bool /* emitError */) {
    return ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse>(static_cast<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse*>(parent.get()));
}

::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse>> IQtiRadioResponse::castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_4::IQtiRadioResponse>& parent, bool emitError) {
    return ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse>(static_cast<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse*>(parent.get()));
}

::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse>> IQtiRadioResponse::castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_3::IQtiRadioResponse>& parent, bool emitError) {
    return ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse>(static_cast<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse*>(parent.get()));
}

::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse>> IQtiRadioResponse::castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_2::IQtiRadioResponse>& parent, bool emitError) {
    return ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse>(static_cast<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse*>(parent.get()));
}

::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse>> IQtiRadioResponse::castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioResponse>& parent, bool emitError) {
    return ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse>(static_cast<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse*>(parent.get()));

}

::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse>> IQtiRadioResponse::castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadioResponse>& parent, bool emitError) {
    return ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse>(static_cast<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse*>(parent.get()));

}

::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse>> IQtiRadioResponse::castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V1_0::IQtiRadioResponse>& parent, bool emitError) {
    return ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse>(static_cast<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse*>(parent.get()));

}

::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse>> IQtiRadioResponse::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse>(static_cast<::vendor::qti::hardware::radio::qtiradio::V2_6::IQtiRadioResponse*>(parent.get()));
}



::android::sp<IQtiRadioResponse> IQtiRadioResponse::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::sp<IQtiRadioResponse> IQtiRadioResponse::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IQtiRadioResponse::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IQtiRadioResponse::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}


}  // namespace V2_6
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
