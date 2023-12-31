/******************************************************************************
#  Copyright (c) 2018,2020,2022 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <memory>
#include <android/hardware/radio/1.2/IRadio.h>
#include <android/hardware/radio/1.2/IRadioResponse.h>
#include <android/hardware/radio/1.2/IRadioIndication.h>
#include <android/hardware/radio/1.2/types.h>
#include <telephony/ril.h>
#include "ril_internal.h"
#include "ril_service.h"
#include "RadioContext.h"
#include <utils/SystemClock.h>
#include <inttypes.h>
#include <request/LinkCapIndMessage.h>
#include <interfaces/nas/NasPhysChanConfigMessage.h>
#include <interfaces/nas/NasSetSignalStrengthCriteria.h>
#include "MessageCommon.h"

#ifdef RIL_FOR_LOW_RAM
#include "MessageCommon.h"
#include "UnSolMessages/RadioDataCallListChangeIndMessage.h"
#endif

using namespace android::hardware::radio;
using namespace android::hardware::radio::V1_0;
using namespace android::hardware::radio::deprecated::V1_0;
using ::android::hardware::Return;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_array;
using ::android::hardware::Void;
using android::CommandInfo;
using android::RequestInfo;
using android::requestToString;
using android::sp;

void convertRilCellInfoListToHal_1_2(int slotId, void *response, size_t responseLen, hidl_vec<V1_2::CellInfo>& records);
void convertRilCellInfoListToHal_1_2(const std::vector<RIL_CellInfo_v12>& rillCellInfo, hidl_vec<V1_2::CellInfo>& records);

// REVISIT: the need for virtual inheritance, since V1_2::IRadio is a pure virtual class and
// V1_0:IRadio is "virtual" inherited from IBase
// If RTTI is not enabled, seems we could not upcast "virtual base class" to derived classes
//class RadioImpl_1_2 : virtual public RadioImpl, public V1_2::IRadio {
class RadioImpl_1_2 : public RadioImpl, public V1_2::IRadio {
private:
    sp<V1_2::IRadioResponse> mRadioResponseV1_2;
    sp<V1_2::IRadioIndication> mRadioIndicationV1_2;

protected:
    Module* mModule;

    Return<void> setResponseFunctions_nolock(
        const ::android::sp<IRadioResponse>& radioResponseParam,
        const ::android::sp<IRadioIndication>& radioIndicationParam);

    void fillInSignalStrengthCriteria(std::vector<SignalStrengthCriteriaEntry> &out,
            int32_t hysteresisMs,
            int32_t hysteresisDb,
            const hidl_vec<int32_t> &thresholdsDbm,
            V1_2::AccessNetwork ran);

    RIL_RadioAccessNetworks convertHidlAccessNetworkToRilAccessNetwork(V1_2::AccessNetwork ran);
#ifdef HAS_QCRIL_DATA_1_5_RESPONSE_TYPES
    rildata::AccessNetwork_t convertHidlAccessNetworkToDataAccessNetwork(V1_2::AccessNetwork ran);
#endif
    RadioError convertMsgToRadioError(Message::Callback::Status status, RIL_Errno e);

public:
    using RadioContext = RadioContextClass<RadioImpl_1_2>;

    RadioImpl_1_2(qcril_instance_id_e_type instance);

    virtual Module* getRilServiceModule() override;

    virtual void createRilServiceModule() override;

    virtual int sendNetworkScanResult(std::shared_ptr<RilUnsolNetworkScanResultMessage> msg) override;
    virtual int sendSignalStrength(std::shared_ptr<RilUnsolSignalStrengthMessage> msg) override;
    virtual void clearCallbacks();

    Return<void> setResponseFunctions(
        const ::android::sp<IRadioResponse>& radioResponseParam,
        const ::android::sp<IRadioIndication>& radioIndicationParam);

    Return<void> setIndicationFilter_1_2(int32_t serial, ::android::hardware::hidl_bitfield<V1_2::IndicationFilter> indicationFilter);
    Return<void> setSignalStrengthReportingCriteria(int32_t serial, int32_t hysteresisMs, int32_t hysteresisDb, const hidl_vec<int32_t>& thresholdsDbm, V1_2::AccessNetwork ran);

    Return<void> setLinkCapacityReportingCriteria(int32_t serial, int32_t hysteresisMs, int32_t hysteresisDlKbps, int32_t hysteresisUlKbps, const hidl_vec<int32_t>& thresholdsDownlinkKbps, const hidl_vec<int32_t>& thresholdsUplinkKbps, V1_2::AccessNetwork ran);

    Return<void> setupDataCall_1_2(int32_t serial, V1_2::AccessNetwork accessNetwork, const V1_0::DataProfileInfo& dataProfileInfo, bool modemCognitive, bool roamingAllowed, bool isRoaming, V1_2::DataRequestReason reason, const hidl_vec<hidl_string>& addresses, const hidl_vec<hidl_string>& dnses);

    Return<void> deactivateDataCall_1_2(int32_t serial, int32_t cid, V1_2::DataRequestReason reason);
    Return<void> startNetworkScan_1_2(int32_t serial, const ::android::hardware::radio::V1_2::NetworkScanRequest& request);

    // override to use 1_2 callback
    virtual Return<void> getVoiceRegistrationState(int32_t serial) override;
    virtual Return<void> getDataRegistrationState(int32_t serial) override;
    virtual Return<void> getSignalStrength(int32_t serial) override;

    // Forward these to RadioImpl
    // This is needed because the interface files generated by the hidl compiler do not use virtual inheritance.
    // So, we need to provide a concrete implementation for all virtual methods inherited.
    Return<void> getIccCardStatus(int32_t serial);
    Return<void> supplyIccPinForApp(int32_t serial, const ::android::hardware::hidl_string& pin, const ::android::hardware::hidl_string& aid) {
        return RadioImpl::supplyIccPinForApp(serial,
                pin,
                aid);
    }
    Return<void> supplyIccPukForApp(int32_t serial, const ::android::hardware::hidl_string& puk, const ::android::hardware::hidl_string& pin, const ::android::hardware::hidl_string& aid) {
        return RadioImpl::supplyIccPukForApp(serial, puk, pin, aid);
    }
    Return<void> supplyIccPin2ForApp(int32_t serial, const ::android::hardware::hidl_string& pin2, const ::android::hardware::hidl_string& aid) {
        return RadioImpl::supplyIccPin2ForApp(serial, pin2, aid);
    }
    Return<void> supplyIccPuk2ForApp(int32_t serial, const ::android::hardware::hidl_string& puk2, const ::android::hardware::hidl_string& pin2, const ::android::hardware::hidl_string& aid) {
        return RadioImpl::supplyIccPuk2ForApp(serial, puk2, pin2, aid);
    }
    Return<void> changeIccPinForApp(int32_t serial, const ::android::hardware::hidl_string& oldPin, const ::android::hardware::hidl_string& newPin, const ::android::hardware::hidl_string& aid) {
        return RadioImpl::changeIccPinForApp(serial, oldPin, newPin, aid);
    }
    Return<void> changeIccPin2ForApp(int32_t serial, const ::android::hardware::hidl_string& oldPin2, const ::android::hardware::hidl_string& newPin2, const ::android::hardware::hidl_string& aid) {
        return RadioImpl::changeIccPin2ForApp(serial, oldPin2, newPin2, aid);
    }
    Return<void> supplyNetworkDepersonalization(int32_t serial, const ::android::hardware::hidl_string& netPin) {
        return RadioImpl::supplyNetworkDepersonalization(serial, netPin);
    }

    Return<void> getCurrentCalls(int32_t serial);

    Return<void> dial(int32_t serial, const ::android::hardware::radio::V1_0::Dial& dialInfo) {
        return RadioImpl::dial(serial, dialInfo);
    }
    Return<void> getImsiForApp(int32_t serial, const ::android::hardware::hidl_string& aid) {
        return RadioImpl::getImsiForApp(serial, aid);
    }
    Return<void> hangup(int32_t serial, int32_t gsmIndex) {
        return RadioImpl::hangup(serial, gsmIndex);
    }
    Return<void> hangupWaitingOrBackground(int32_t serial) {
        return RadioImpl::hangupWaitingOrBackground(serial);
    }
    Return<void> hangupForegroundResumeBackground(int32_t serial) {
        return RadioImpl::hangupForegroundResumeBackground(serial);
    }
    Return<void> switchWaitingOrHoldingAndActive(int32_t serial) {
        return RadioImpl::switchWaitingOrHoldingAndActive(serial);
    }
    Return<void> conference(int32_t serial) {
        return RadioImpl::conference(serial);
    }
    Return<void> rejectCall(int32_t serial) {
        return RadioImpl::rejectCall(serial);
    }
    Return<void> getLastCallFailCause(int32_t serial) {
        return RadioImpl::getLastCallFailCause(serial);
    }
    Return<void> getOperator(int32_t serial) {
        return RadioImpl::getOperator(serial);
    }
    Return<void> setRadioPower(int32_t serial, bool on) {
        return RadioImpl::setRadioPower(serial, on);
    }
    Return<void> sendDtmf(int32_t serial, const ::android::hardware::hidl_string& s) {
        return RadioImpl::sendDtmf(serial,  s);
    }
    Return<void> sendSms(int32_t serial, const ::android::hardware::radio::V1_0::GsmSmsMessage& message) {
        return RadioImpl::sendSms(serial,  message);
    }
    Return<void> sendSMSExpectMore(int32_t serial, const ::android::hardware::radio::V1_0::GsmSmsMessage& message) {
        return RadioImpl::sendSMSExpectMore(serial,  message);
    }
    Return<void> setupDataCall(int32_t serial, ::android::hardware::radio::V1_0::RadioTechnology radioTechnology, const ::android::hardware::radio::V1_0::DataProfileInfo& dataProfileInfo, bool modemCognitive, bool roamingAllowed, bool isRoaming) {
        return RadioImpl::setupDataCall(serial, radioTechnology, dataProfileInfo, modemCognitive, roamingAllowed, isRoaming);
    }
    Return<void> iccIOForApp(int32_t serial, const ::android::hardware::radio::V1_0::IccIo& iccIo) {
        return RadioImpl::iccIOForApp(serial,  iccIo);
    }
    Return<void> sendUssd(int32_t serial, const ::android::hardware::hidl_string& ussd) {
        return RadioImpl::sendUssd(serial,  ussd);
    }
    Return<void> cancelPendingUssd(int32_t serial) {
        return RadioImpl::cancelPendingUssd(serial);
    }
    Return<void> getClir(int32_t serial) {
        return RadioImpl::getClir(serial);
    }
    Return<void> setClir(int32_t serial, int32_t status) {
        return RadioImpl::setClir(serial, status);
    }
    Return<void> getCallForwardStatus(int32_t serial, const ::android::hardware::radio::V1_0::CallForwardInfo& callInfo) {
        return RadioImpl::getCallForwardStatus(serial,  callInfo);
    }
    Return<void> setCallForward(int32_t serial, const ::android::hardware::radio::V1_0::CallForwardInfo& callInfo) {
        return RadioImpl::setCallForward(serial,  callInfo);
    }
    Return<void> getCallWaiting(int32_t serial, int32_t serviceClass) {
        return RadioImpl::getCallWaiting(serial, serviceClass);
    }
    Return<void> setCallWaiting(int32_t serial, bool enable, int32_t serviceClass) {
        return RadioImpl::setCallWaiting(serial, enable, serviceClass);
    }
    Return<void> acknowledgeLastIncomingGsmSms(int32_t serial, bool success, ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause cause) {
        return RadioImpl::acknowledgeLastIncomingGsmSms(serial, success, cause);
    }
    Return<void> acceptCall(int32_t serial) {
        return RadioImpl::acceptCall(serial);
    }
    Return<void> deactivateDataCall(int32_t serial, int32_t cid, bool reasonRadioShutDown) {
        return RadioImpl::deactivateDataCall(serial, cid, reasonRadioShutDown);
    }
    Return<void> getFacilityLockForApp(int32_t serial, const ::android::hardware::hidl_string& facility, const ::android::hardware::hidl_string& password, int32_t serviceClass, const ::android::hardware::hidl_string& appId) {
        return RadioImpl::getFacilityLockForApp(serial,  facility,  password, serviceClass,  appId);
    }
    Return<void> setFacilityLockForApp(int32_t serial, const ::android::hardware::hidl_string& facility, bool lockState, const ::android::hardware::hidl_string& password, int32_t serviceClass, const ::android::hardware::hidl_string& appId) {
        return RadioImpl::setFacilityLockForApp(serial,  facility, lockState,  password, serviceClass,  appId);
    }
    Return<void> setBarringPassword(int32_t serial, const ::android::hardware::hidl_string& facility, const ::android::hardware::hidl_string& oldPassword, const ::android::hardware::hidl_string& newPassword) {
        return RadioImpl::setBarringPassword(serial,  facility,  oldPassword,  newPassword);
    }
    Return<void> getNetworkSelectionMode(int32_t serial) {
        return RadioImpl::getNetworkSelectionMode(serial);
    }
    Return<void> setNetworkSelectionModeAutomatic(int32_t serial) {
        return RadioImpl::setNetworkSelectionModeAutomatic(serial);
    }
    Return<void> setNetworkSelectionModeManual(int32_t serial, const ::android::hardware::hidl_string& operatorNumeric) {
        return RadioImpl::setNetworkSelectionModeManual(serial,  operatorNumeric);
    }
    Return<void> getAvailableNetworks(int32_t serial) {
        return RadioImpl::getAvailableNetworks(serial);
    }
    Return<void> startDtmf(int32_t serial, const ::android::hardware::hidl_string& s) {
        return RadioImpl::startDtmf(serial,  s);
    }
    Return<void> stopDtmf(int32_t serial) {
        return RadioImpl::stopDtmf(serial);
    }
    Return<void> getBasebandVersion(int32_t serial) {
        return RadioImpl::getBasebandVersion(serial);
    }
    Return<void> separateConnection(int32_t serial, int32_t gsmIndex) {
        return RadioImpl::separateConnection(serial, gsmIndex);
    }
    Return<void> setMute(int32_t serial, bool enable) {
        return RadioImpl::setMute(serial, enable);
    }
    Return<void> getMute(int32_t serial) {
        return RadioImpl::getMute(serial);
    }
    Return<void> getClip(int32_t serial) {
        return RadioImpl::getClip(serial);
    }
    Return<void> getDataCallList(int32_t serial) {
        return RadioImpl::getDataCallList(serial);
    }
    Return<void> getDataCallListResponse(std::shared_ptr<rildata::DataCallListResult_t> responseDataPtr, int32_t serial, Message::Callback::Status status) {
        return RadioImpl::getDataCallListResponse(responseDataPtr, serial, status);
    }
    Return<void> setSuppServiceNotifications(int32_t serial, bool enable) {
        return RadioImpl::setSuppServiceNotifications(serial, enable);
    }
    Return<void> writeSmsToSim(int32_t serial, const ::android::hardware::radio::V1_0::SmsWriteArgs& smsWriteArgs) {
        return RadioImpl::writeSmsToSim(serial,  smsWriteArgs);
    }
    Return<void> deleteSmsOnSim(int32_t serial, int32_t index) {
        return RadioImpl::deleteSmsOnSim(serial, index);
    }
    Return<void> setBandMode(int32_t serial, ::android::hardware::radio::V1_0::RadioBandMode mode) {
        return RadioImpl::setBandMode(serial, mode);
    }
    Return<void> getAvailableBandModes(int32_t serial) {
        return RadioImpl::getAvailableBandModes(serial);
    }
    Return<void> sendEnvelope(int32_t serial, const ::android::hardware::hidl_string& command) {
        return RadioImpl::sendEnvelope(serial,  command);
    }
    Return<void> sendTerminalResponseToSim(int32_t serial, const ::android::hardware::hidl_string& commandResponse) {
        return RadioImpl::sendTerminalResponseToSim(serial,  commandResponse);
    }
    Return<void> handleStkCallSetupRequestFromSim(int32_t serial, bool accept) {
        return RadioImpl::handleStkCallSetupRequestFromSim(serial, accept);
    }
    Return<void> explicitCallTransfer(int32_t serial) {
        return RadioImpl::explicitCallTransfer(serial);
    }
    Return<void> setPreferredNetworkType(int32_t serial, ::android::hardware::radio::V1_0::PreferredNetworkType nwType) {
        return RadioImpl::setPreferredNetworkType(serial, nwType);
    }
    Return<void> getPreferredNetworkType(int32_t serial) {
        return RadioImpl::getPreferredNetworkType(serial);
    }
    Return<void> getNeighboringCids(int32_t serial) {
        return RadioImpl::getNeighboringCids(serial);
    }
    Return<void> setLocationUpdates(int32_t serial, bool enable) {
        return RadioImpl::setLocationUpdates(serial, enable);
    }
    Return<void> setCdmaSubscriptionSource(int32_t serial, ::android::hardware::radio::V1_0::CdmaSubscriptionSource cdmaSub) {
        return RadioImpl::setCdmaSubscriptionSource(serial, cdmaSub);
    }
    Return<void> setCdmaRoamingPreference(int32_t serial, ::android::hardware::radio::V1_0::CdmaRoamingType type) {
        return RadioImpl::setCdmaRoamingPreference(serial, type);
    }
    Return<void> getCdmaRoamingPreference(int32_t serial) {
        return RadioImpl::getCdmaRoamingPreference(serial);
    }
    Return<void> setTTYMode(int32_t serial, ::android::hardware::radio::V1_0::TtyMode mode) {
        return RadioImpl::setTTYMode(serial, mode);
    }
    Return<void> getTTYMode(int32_t serial) {
        return RadioImpl::getTTYMode(serial);
    }
    Return<void> setPreferredVoicePrivacy(int32_t serial, bool enable) {
        return RadioImpl::setPreferredVoicePrivacy(serial, enable);
    }
    Return<void> getPreferredVoicePrivacy(int32_t serial) {
        return RadioImpl::getPreferredVoicePrivacy(serial);
    }
    Return<void> sendCDMAFeatureCode(int32_t serial, const ::android::hardware::hidl_string& featureCode) {
        return RadioImpl::sendCDMAFeatureCode(serial,  featureCode);
    }
    Return<void> sendBurstDtmf(int32_t serial, const ::android::hardware::hidl_string& dtmf, int32_t on, int32_t off) {
        return RadioImpl::sendBurstDtmf(serial,  dtmf, on, off);
    }
    Return<void> sendCdmaSms(int32_t serial, const ::android::hardware::radio::V1_0::CdmaSmsMessage& sms) {
        return RadioImpl::sendCdmaSms(serial,  sms);
    }
    Return<void> acknowledgeLastIncomingCdmaSms(int32_t serial, const ::android::hardware::radio::V1_0::CdmaSmsAck& smsAck) {
        return RadioImpl::acknowledgeLastIncomingCdmaSms(serial,  smsAck);
    }
    Return<void> getGsmBroadcastConfig(int32_t serial) {
        return RadioImpl::getGsmBroadcastConfig(serial);
    }
    Return<void> setGsmBroadcastConfig(int32_t serial, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::GsmBroadcastSmsConfigInfo>& configInfo) {
        return RadioImpl::setGsmBroadcastConfig(serial, configInfo);
    }
    Return<void> setGsmBroadcastActivation(int32_t serial, bool activate) {
        return RadioImpl::setGsmBroadcastActivation(serial, activate);
    }
    Return<void> getCdmaBroadcastConfig(int32_t serial) {
        return RadioImpl::getCdmaBroadcastConfig(serial);
    }
    Return<void> setCdmaBroadcastConfig(int32_t serial, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CdmaBroadcastSmsConfigInfo>& configInfo) {
        return RadioImpl::setCdmaBroadcastConfig(serial, configInfo);
    }
    Return<void> setCdmaBroadcastActivation(int32_t serial, bool activate) {
        return RadioImpl::setCdmaBroadcastActivation(serial, activate);
    }
    Return<void> getCDMASubscription(int32_t serial) {
        return RadioImpl::getCDMASubscription(serial);
    }
    Return<void> writeSmsToRuim(int32_t serial, const ::android::hardware::radio::V1_0::CdmaSmsWriteArgs& cdmaSms) {
        return RadioImpl::writeSmsToRuim(serial,  cdmaSms);
    }
    Return<void> deleteSmsOnRuim(int32_t serial, int32_t index) {
        return RadioImpl::deleteSmsOnRuim(serial, index);
    }
    Return<void> getDeviceIdentity(int32_t serial) {
        return RadioImpl::getDeviceIdentity(serial);
    }
    Return<void> exitEmergencyCallbackMode(int32_t serial) {
        return RadioImpl::exitEmergencyCallbackMode(serial);
    }
    Return<void> getSmscAddress(int32_t serial) {
        return RadioImpl::getSmscAddress(serial);
    }
    Return<void> setSmscAddress(int32_t serial, const ::android::hardware::hidl_string& smsc) {
        return RadioImpl::setSmscAddress(serial,  smsc);
    }
    Return<void> reportSmsMemoryStatus(int32_t serial, bool available) {
        return RadioImpl::reportSmsMemoryStatus(serial, available);
    }
    Return<void> reportStkServiceIsRunning(int32_t serial) {
        return RadioImpl::reportStkServiceIsRunning(serial);
    }
    Return<void> getCdmaSubscriptionSource(int32_t serial) {
        return RadioImpl::getCdmaSubscriptionSource(serial);
    }
    Return<void> requestIsimAuthentication(int32_t serial, const ::android::hardware::hidl_string& challenge) {
        return RadioImpl::requestIsimAuthentication(serial,  challenge);
    }
    Return<void> acknowledgeIncomingGsmSmsWithPdu(int32_t serial, bool success, const ::android::hardware::hidl_string& ackPdu) {
        return RadioImpl::acknowledgeIncomingGsmSmsWithPdu(serial, success,  ackPdu);
    }
    Return<void> sendEnvelopeWithStatus(int32_t serial, const ::android::hardware::hidl_string& contents) {
        return RadioImpl::sendEnvelopeWithStatus(serial,  contents);
    }
    Return<void> getVoiceRadioTechnology(int32_t serial) {
        return RadioImpl::getVoiceRadioTechnology(serial);
    }
    Return<void> getCellInfoList(int32_t serial) {
        return RadioImpl::getCellInfoList(serial);
    }
    Return<void> setCellInfoListRate(int32_t serial, int32_t rate) {
        return RadioImpl::setCellInfoListRate(serial, rate);
    }
    Return<void> setInitialAttachApn(int32_t serial, const ::android::hardware::radio::V1_0::DataProfileInfo& dataProfileInfo, bool modemCognitive, bool isRoaming) {
        return RadioImpl::setInitialAttachApn(serial,  dataProfileInfo, modemCognitive, isRoaming);
    }
    Return<void> getImsRegistrationState(int32_t serial) {
        return RadioImpl::getImsRegistrationState(serial);
    }
    Return<void> sendImsSms(int32_t serial, const ::android::hardware::radio::V1_0::ImsSmsMessage& message) {
        return RadioImpl::sendImsSms(serial,  message);
    }
    Return<void> iccTransmitApduBasicChannel(int32_t serial, const ::android::hardware::radio::V1_0::SimApdu& message) {
        return RadioImpl::iccTransmitApduBasicChannel(serial,  message);
    }
    Return<void> iccOpenLogicalChannel(int32_t serial, const ::android::hardware::hidl_string& aid, int32_t p2) {
        return RadioImpl::iccOpenLogicalChannel(serial,  aid, p2);
    }
    Return<void> iccCloseLogicalChannel(int32_t serial, int32_t channelId) {
        return RadioImpl::iccCloseLogicalChannel(serial, channelId);
    }
    Return<void> iccTransmitApduLogicalChannel(int32_t serial, const ::android::hardware::radio::V1_0::SimApdu& message) {
        return RadioImpl::iccTransmitApduLogicalChannel(serial,  message);
    }
    Return<void> nvReadItem(int32_t serial, ::android::hardware::radio::V1_0::NvItem itemId) {
        return RadioImpl::nvReadItem(serial, itemId);
    }
    Return<void> nvWriteItem(int32_t serial, const ::android::hardware::radio::V1_0::NvWriteItem& item) {
        return RadioImpl::nvWriteItem(serial,  item);
    }
    Return<void> nvWriteCdmaPrl(int32_t serial, const ::android::hardware::hidl_vec<uint8_t>& prl) {
        return RadioImpl::nvWriteCdmaPrl(serial,  prl);
    }
    Return<void> nvResetConfig(int32_t serial, ::android::hardware::radio::V1_0::ResetNvType resetType) {
        return RadioImpl::nvResetConfig(serial, resetType);
    }
    Return<void> setUiccSubscription(int32_t serial, const ::android::hardware::radio::V1_0::SelectUiccSub& uiccSub) {
        return RadioImpl::setUiccSubscription(serial,  uiccSub);
    }
    Return<void> setDataAllowed(int32_t serial, bool allow) {
        return RadioImpl::setDataAllowed(serial, allow);
    }
    Return<void> getHardwareConfig(int32_t serial) {
        return RadioImpl::getHardwareConfig(serial);
    }
    Return<void> requestIccSimAuthentication(int32_t serial, int32_t authContext, const ::android::hardware::hidl_string& authData, const ::android::hardware::hidl_string& aid) {
        return RadioImpl::requestIccSimAuthentication(serial, authContext,  authData,  aid);
    }
    Return<void> setDataProfile(int32_t serial, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::DataProfileInfo>& profiles, bool isRoaming) {
        return RadioImpl::setDataProfile(serial, profiles, isRoaming);
    }
    Return<void> requestShutdown(int32_t serial) {
        return RadioImpl::requestShutdown(serial);
    }
    Return<void> getRadioCapability(int32_t serial) {
        return RadioImpl::getRadioCapability(serial);
    }
    Return<void> setRadioCapability(int32_t serial, const ::android::hardware::radio::V1_0::RadioCapability& rc) {
        return RadioImpl::setRadioCapability(serial,  rc);
    }
    Return<void> startLceService(int32_t serial, int32_t reportInterval, bool pullMode);
    Return<void> stopLceService(int32_t serial);
    Return<void> pullLceData(int32_t serial);
    Return<void> getModemActivityInfo(int32_t serial) {
        return RadioImpl::getModemActivityInfo(serial);
    }
    Return<void> setAllowedCarriers(int32_t serial, bool allAllowed, const ::android::hardware::radio::V1_0::CarrierRestrictions& carriers) {
        return RadioImpl::setAllowedCarriers(serial, allAllowed,  carriers);
    }
    Return<void> getAllowedCarriers(int32_t serial) {
        return RadioImpl::getAllowedCarriers(serial);
    }
    Return<void> sendDeviceState(int32_t serial, ::android::hardware::radio::V1_0::DeviceStateType deviceStateType, bool state) {
        return RadioImpl::sendDeviceState(serial, deviceStateType, state);
    }
    Return<void> setIndicationFilter(int32_t serial, ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::IndicationFilter> indicationFilter) {
        return RadioImpl::setIndicationFilter(serial, indicationFilter);
    }
    Return<void> setSimCardPower(int32_t serial, bool powerUp) {
        return RadioImpl::setSimCardPower(serial, powerUp);
    }
    Return<void> responseAcknowledgement() {
        return RadioImpl::responseAcknowledgement();
    }
    Return<void> setCarrierInfoForImsiEncryption(int32_t serial, const ::android::hardware::radio::V1_1::ImsiEncryptionInfo& imsiEncryptionInfo) {
        return RadioImpl::setCarrierInfoForImsiEncryption(serial,  imsiEncryptionInfo);
    }
    Return<void> setSimCardPower_1_1(int32_t serial, ::android::hardware::radio::V1_1::CardPowerState powerUp) {
        return RadioImpl::setSimCardPower_1_1(serial, powerUp);
    }
    Return<void> startNetworkScan(int32_t serial, const ::android::hardware::radio::V1_1::NetworkScanRequest& request) {
        return RadioImpl::startNetworkScan(serial,  request);
    }
    Return<void> stopNetworkScan(int32_t serial) {
        return RadioImpl::stopNetworkScan(serial);
    }
    Return<void> startKeepalive(int32_t serial, const ::android::hardware::radio::V1_1::KeepaliveRequest& keepalive) {
        return RadioImpl::startKeepalive(serial,  keepalive);
    }
    Return<void> stopKeepalive(int32_t serial, int32_t sessionHandle) {
        return RadioImpl::stopKeepalive(serial, sessionHandle);
    }
    std::shared_ptr<RadioContext> getContext(int32_t serial);

    virtual ::android::status_t registerAsService( const std::string &serviceName);
    static const QcrildServiceVersion &getVersion();
    virtual const char *getDescriptor() {
        return V1_2::IRadio::descriptor;
    }
    virtual int sendCellInfoListInd(int slotId,
            int indicationType,
            int token,
            RIL_Errno e,
            void *response,
            size_t responseLen);

    virtual int sendGetCellInfoListResponse(int slotId,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responseLen);
    virtual int sendLinkCapInd(std::shared_ptr<rildata::LinkCapIndMessage> msg);
    virtual int sendCurrentPhysicalChannelConfigs(std::shared_ptr<NasPhysChanConfigMessage> msg);
    virtual void setLinkCapacityReportingCriteriaResponse_1_2(RadioResponseInfo responseInfo);

#ifdef RIL_FOR_LOW_RAM
    void setupDataCallResponse(RadioResponseInfo responseInfo, V1_0::SetupDataCallResult dcResult);
    void deactivateDataCallResponse(RadioResponseInfo responseInfo);
    void dataCallListChanged(RadioIndicationType indType, hidl_vec<V1_0::SetupDataCallResult> dcResultList);
#endif

protected:
    int convertToHidl(V1_2::AudioQuality &aq, enum RIL_AudioQuality rilAudioQuality);
    int convertToHidl(V1_2::Call &out, qcril::interfaces::CallInfo &in);
    virtual RIL_Errno convertLcResultToRilError(rildata::LinkCapCriteriaResult_t result);
    virtual bool checkThresholdAndHysteresis(int32_t hysteresisMs, int32_t hysteresisDb,
            const hidl_vec<int32_t> &thresholdsDbm);
private:
    RadioError sanityCheckSignalStrengthCriteriaParams(int32_t hysteresisMs, int32_t hysteresisDb,
            const hidl_vec<int32_t> &thresholdsDbm, V1_2::AccessNetwork ran);
};

int convertToHal(WcdmaSignalStrength &out, const RIL_WCDMA_SignalStrength &in);
int convertToHal(V1_2::WcdmaSignalStrength &out, const RIL_WCDMA_SignalStrength &in);
void convertToHal(V1_2::TdscdmaSignalStrength &out, const RIL_TD_SCDMA_SignalStrength &in);
int convertRilSignalStrengthToHal(void *response, size_t responseLen,
        V1_2::SignalStrength& signalStrength);

template <>
void fillCellIdentityGsm(V1_2::CellIdentityGsm &out, const RIL_CellIdentityGsm_v12 &in);
template <>
void fillCellIdentityWcdma(V1_2::CellIdentityWcdma &out, const RIL_CellIdentityWcdma_v12 &in);
template <>
void fillCellIdentityCdma(V1_2::CellIdentityCdma &out, const RIL_CellIdentityCdma &in);
template <>
void fillCellIdentityLte(V1_2::CellIdentityLte &out, const RIL_CellIdentityLte_v12 &in);
template <>
void fillCellIdentityTdscdma(V1_2::CellIdentityTdscdma &out, const RIL_CellIdentityTdscdma &in);

RadioError fillNetworkScanRequest_1_2(const V1_2::NetworkScanRequest& request, RIL_NetworkScanRequest &scanRequest);
