/******************************************************************************
  @file    qcril_qmi_oemhook_service.cc
  @brief   qcril qmi - oemhook_service

  DESCRIPTION
    Implements the server side of the IQtiOemHook interface. Handles RIL
    requests and responses and indications to be received and sent to client
    respectively

  ---------------------------------------------------------------------------

  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/

#define TAG "RILQ"

#include <modules/oemhook/qcril_qmi_oemhook_service.h>
#include "qcril_qmi_oem_reqlist.h"

extern "C" {
    #include "qcril_other.h"
}

#include <modules/oemhook/OemHookServiceModule.h>
#include "qcril_qmi_oem_events.h"

#include <modules/uim/UimVoltageStatusRequestMsg.h>
#include <modules/uim/UimCardPowerReqMsg.h>
#include <modules/uim/UimPersoRequestMsg.h>
#include <modules/uim/UimGetPersoStatusRequestMsg.h>
#include <modules/uim/UimEnterDePersoRequestMsg.h>
#include <modules/uim/UimGetCardStatusRequestMsg.h>
#include <modules/uim/UimGetAtrRequestMsg.h>
#include <modules/uim/UimTransmitAPDURequestMsg.h>
#include <modules/uim/UimSwitchSlotRequestMsg.h>
#include <modules/uim/UimGetPhysicalSlotMappingRequestMsg.h>
#include <modules/uim/UimSetAPDUBehaviorRequestMsg.h>
#include <modules/uim/UimGetSlotStatusRequestMsg.h>
#include <modules/uim/UimGetLPATermialCapabilitesRequestMsg.h>
#include <modules/uim/UimSetLPATermialCapabilitesRequestMsg.h>
#include <interfaces/voice/QcRilRequestRejectIncomingCallMessage.h>
#include <interfaces/voice/QcRilRequestGetCurrentSetupCallsMessage.h>
#include <interfaces/voice/QcRilRequestSetupAnswerMessage.h>
#include <interfaces/voice/QcRilRequestSetLocalCallHoldMessage.h>
#ifdef FEATURE_QCRIL_LTE_DIRECT
#include <interfaces/lte_direct/QcRilRequestCancelPublishMessage.h>
#include <interfaces/lte_direct/QcRilRequestCancelSubscribeMessage.h>
#include <interfaces/lte_direct/QcRilRequestGetDeviceCapabilityMessage.h>
#include <interfaces/lte_direct/QcRilRequestGetServiceStatusMessage.h>
#include <interfaces/lte_direct/QcRilRequestLteDirectInitializeMessage.h>
#include <interfaces/lte_direct/QcRilRequestPublishMessage.h>
#include <interfaces/lte_direct/QcRilRequestSubscribeMessage.h>
#include <interfaces/lte_direct/QcRilRequestTerminateMessage.h>
#include <interfaces/lte_direct/lte_direct.h>
#include "lte_direct/qcril_qmi_lte_direct_disc_packing.h"
#include "lte_direct/qcril_qmi_lte_direct_disc_msg_meta.h"
#endif

using namespace vendor::qti::hardware::radio::qcrilhook::V1_0::implementation;
using ::android::hardware::Void;

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qcrilhook {
namespace V1_0 {
namespace implementation {

uint8_t* constructOemHookRaw
(
  uint32_t* length,
  int unsol_event,
  uint8_t* data,
  uint32_t data_len
);

static sp<OemHookImpl> oemHook = NULL;
#define QCRIL_QMI_OEM_INT_VAL_LEN               4

OemHookImpl::OemHookImpl(qcril_instance_id_e_type oemSocketInstanceId) {
    mResponseCb = NULL;
    mIndicationCb = NULL;
    mInstanceId = oemSocketInstanceId;
    mModule = new OemHookServiceModule(oemSocketInstanceId);
    if (mModule != nullptr) {
        mModule->init();
    }
#ifdef FEATURE_QCRIL_LTE_DIRECT
    qcril_qmi_lted_init_msg_meta();
#endif
    QCRIL_LOG_INFO("OemHookImpl()");
}

/*
 * Service Died callback.
 */
void OemHookImpl::serviceDied(
    uint64_t, const ::android::wp<::android::hidl::base::V1_0::IBase> &) {
  QCRIL_LOG_DEBUG("OemHookImpl::serviceDied: Client died. Cleaning up callbacks");
  clearCallbacks();
}

/*
 * Resets indication callback upon client's death
 */
void OemHookImpl::clearCallbacks() {
  std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  if (mResponseCb != nullptr) {
    mResponseCb = nullptr;
  }
  if (mIndicationCb != nullptr) {
    mIndicationCb = nullptr;
  }
}

//===========================================================================
// oem_hook_impl::setCallback
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Registers the callbacks for IQtiOemHookResponse and IQtiOemHookIndication
    passed by the client as a parameter

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::setCallback(const sp<IQtiOemHookResponse> &responseCallback,
                                      const sp<IQtiOemHookIndication> &indicationCallback) {
  QCRIL_LOG_DEBUG("OemHookImpl::setCallback");
  std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  if (mResponseCb != nullptr) {
    mResponseCb->unlinkToDeath(this);
  }
  mResponseCb = responseCallback;
  mIndicationCb = indicationCallback;
  mResponseCb->linkToDeath(this, 0);

  return Void();
}

sp<IQtiOemHookResponse> OemHookImpl::getResponseCallback() {
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mResponseCb;
}

sp<IQtiOemHookIndication> OemHookImpl::getIndicationCallback() {
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mIndicationCb;
}

//===========================================================================
// oem_hook_impl::oemHookRawRequest
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the oemhook request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::oemHookRawRequest(int32_t serial, const hidl_vec<uint8_t>& data) {
    uint32_t cmd_id = 0;
    uint32_t reqLen = 0;
    uint8_t *reqData = NULL;

    size_t dataLen = data.size();
    const uint8_t* dataPtr = data.data();
    QCRIL_LOG_INFO("OemHookImpl::oemHookRawRequest serial=%d length=%d", serial, dataLen);

    if (!dataPtr) {
      QCRIL_LOG_ERROR("Invalid parameters; data is null");
      OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
          serial, RadioError::REQUEST_NOT_SUPPORTED, NULL);
      return Void();
    }

    // [OEMNAME (8 bytes MANDATORY)]
    if (dataLen < QCRIL_OTHER_OEM_NAME_LENGTH) {
      QCRIL_LOG_ERROR("Invalid parameters; invalid data length");
      OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
          serial, RadioError::INVALID_ARGUMENTS, NULL);
      return Void();
    }
    dataLen -= QCRIL_OTHER_OEM_NAME_LENGTH;
    dataPtr += QCRIL_OTHER_OEM_NAME_LENGTH;

    // [COMMAND ID (4 bytes MANDATORY)]
    if (dataLen < QCRIL_OTHER_OEM_REQUEST_ID_LEN) {
      QCRIL_LOG_ERROR("Invalid parameters; invalid data length");
      OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
          serial, RadioError::INVALID_ARGUMENTS, NULL);
      return Void();
    }
    memcpy(&cmd_id, dataPtr, QCRIL_OTHER_OEM_REQUEST_ID_LEN);
    dataLen -= QCRIL_OTHER_OEM_REQUEST_ID_LEN;
    dataPtr += QCRIL_OTHER_OEM_REQUEST_ID_LEN;

    // [REQ DATA LENGTH (4 bytes OPTIONAL)]
    if (dataLen > QCRIL_OTHER_OEM_REQUEST_DATA_LEN) {
      memcpy(&reqLen, dataPtr, QCRIL_OTHER_OEM_REQUEST_DATA_LEN);
      dataLen -= QCRIL_OTHER_OEM_REQUEST_DATA_LEN;
      dataPtr += QCRIL_OTHER_OEM_REQUEST_DATA_LEN;
      // [REQ DATA (REQ DATA LENGTH bytes OPTIONAL)]
      reqData = const_cast<uint8_t *>(dataPtr);
    }

    // Return failure if REQ DATA LENGTH is larger than the actual data length.
    if (reqLen > dataLen) {
      QCRIL_LOG_ERROR("Invalid parameters; invalid data length");
      OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
          serial, RadioError::INVALID_ARGUMENTS, NULL);
      return Void();
    }

    switch ( cmd_id )
    {
      case QCRIL_REQ_HOOK_GET_ADN_RECORD:
        getAdnRecord(serial);
        break;

      case QCRIL_REQ_HOOK_UPDATE_ADN_RECORD:
        updateAdnRecord(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_SET_MODEM_TEST_MODE:
        setMbnConfig(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_QUERY_MODEM_TEST_MODE:
        getMbnConfig(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_GET_AVAILABLE_CONFIGS:
        getAvailableMbnConfig(serial);
        break;

      case QCRIL_REQ_HOOK_CLEANUP_LOADED_CONFIGS:
        cleanupMbnConfig(serial);
        break;

      case QCRIL_REQ_HOOK_SEL_CONFIG:
        selectMbnConfig(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_GET_META_INFO:
        getMetaInfo(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_DEACTIVATE_CONFIGS:
        deactivateMbnConfig(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_GET_QC_VERSION_OF_FILE:
        getQcVersionOfFile(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_VALIDATE_CONFIG:
        validateMbnConfig(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_GET_QC_VERSION_OF_CONFIGID:
        getQcVersionOfConfig(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_GET_OEM_VERSION_OF_FILE:
        getOemVersionOfFile(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_GET_OEM_VERSION_OF_CONFIGID:
        getOemVersionOfConfig(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_ACTIVATE_CONFIGS:
        activateMbnConfig(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_ENABLE_MODEM_UPDATE:
        enableModemUpdate(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_UICC_VOLTAGE_STATUS_REQ:
        uimGetVoltageStatus(serial);
        break;

      case QCRIL_REQ_HOOK_UICC_POWER_REQ:
      case QCRIL_REQ_HOOK_SLOT_CARD_POWER_REQ:
        uimSendCardPowerReq(serial, cmd_id, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_PERSONALIZATION_REACTIVATE_REQ:
        uimSendPersoReq(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_PERSONALIZATION_STATUS_REQ:
        uimGetPersoStatus(serial);
        break;

      case QCRIL_REQ_HOOK_ENTER_DEPERSONALIZATION_REQ:
        uimSendDePersoReq(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_SLOT_GET_CARD_STATE_REQ:
        uimGetCardStateReq(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_SLOT_GET_ATR_REQ:
        uimGetCardAtrReq(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_SLOT_SEND_APDU_REQ:
        uimSendApduReq(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_SWITCH_SLOT_REQ:
        uimSwitchSlotReq(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_GET_PHYSICAL_SLOT_MAPPING_REQ:
        uimGetSlotMapping(serial);
        break;

      case QCRIL_REQ_HOOK_SLOT_SET_APDU_BEHAVIOR_REQ:
        uimSetApduBehaviorReq(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_GET_SLOTS_STATUS_REQ:
        uimGetSlotStatus(serial);
        break;

      case QCRIL_REQ_HOOK_GET_TERMINAL_CAPABILITY_LPA_TLV_REQ:
        uimGetLPATC(serial);
        break;

      case QCRIL_REQ_HOOK_SET_TERMINAL_CAPABILITY_LPA_TLV_REQ:
        uimSetLPATC(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_REJECT_INCOMING_CALL_CAUSE_21:
        rejectIncomingCallWithCause21(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_GET_CURRENT_SETUP_CALLS:
        getCurrentSetupCalls(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_REQUEST_SETUP_ANSWER:
        requestSetupAnswer(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_SET_LOCAL_CALL_HOLD:
      case QCRIL_REQ_HOOK_SET_LOCAL_CALL_HOLD_v01:
        requestSetLocalCallHold(serial, reqData, reqLen);
        break;

      case QCRIL_REQ_HOOK_PERFORM_INCREMENTAL_NW_SCAN:
        startIncrementalScanRequest(serial);
        break;

      case QCRIL_REQ_HOOK_CANCEL_QUERY_AVAILABLE_NETWORK:
        cancelIncrementalScanRequest(serial);
        break;

#ifdef FEATURE_QCRIL_LTE_DIRECT
      case QCRIL_REQ_HOOK_LTE_DIRECT_DISC_REQ:
        processLteDirectDiscRequest(serial, reqData, reqLen);
        break;
#endif

      default:
        processIncomingOemhookMessage(mInstanceId, serial, const_cast<unsigned char*>(data.data()),
                                      data.size());
        break;
    }

    return Void();
}

/*
 * Sends oemHook indication/unsol to telephony via callback
 */
void OemHookImpl::sendIndication(unsigned char* buf, size_t bufLen) {
    QCRIL_LOG_INFO("OemHookImpl::sendIndication length=%d", bufLen);
    sp<IQtiOemHookIndication> indCb = getIndicationCallback();
    if (indCb == NULL) {
      QCRIL_LOG_ERROR("OemHookImpl::sendIndication indicationCb is null");
      return;
    }
    hidl_vec<uint8_t> data;
    data.setToExternal((uint8_t*)buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    Return<void> ret = indCb->oemHookRawIndication(data);
    if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
    }
}

/*
 * Sends oemHook response to telephony via callback
 */
void OemHookImpl::sendResponse(int32_t serial, RIL_Errno error, unsigned char* buf, size_t bufLen) {
    sp<IQtiOemHookResponse> respCb = getResponseCallback();
    if (respCb == NULL) {
      QCRIL_LOG_ERROR("OemHookImpl::sendResponse responseCb is null");
      return;
    }
    hidl_vec<uint8_t> data;
    data.setToExternal((uint8_t*)buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    QCRIL_LOG_INFO("OemHookImpl::sendResponse serial=%d error=%d length=%d", serial, error, bufLen);
    Return<void> ret = respCb->oemHookRawResponse(serial, (RadioError)error, data);
    if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
}

//===========================================================================
// oem_hook_impl::getAdnRecord
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_GET_ADN_RECORD request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::getAdnRecord(int32_t serial) {
    QCRIL_LOG_INFO("OemHookImpl::getAdnRecord on slot %d", mInstanceId);
    auto msg = std::make_shared<QcRilRequestGetAdnRecordMessage>(getContext(serial));
    if (msg) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
                QCRIL_NOTUSED( status );
                if (solicitedMsg && responseDataPtr) {
                  if(responseDataPtr->data) {
                    qcril::interfaces::qcril_pbm_adn_count_info adnCountInfo =
                        std::static_pointer_cast<qcril::interfaces::AdnCountInfoResp>(
                        responseDataPtr->data)->getAdnCountInfo();
                    hidl_vec<uint8_t> data;
                    data.setToExternal((uint8_t*) &adnCountInfo, sizeof(adnCountInfo));
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(),
                            oemHookRawResponse, serial, (RadioError)responseDataPtr->errorCode, data);
                  } else {
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(),
                            oemHookRawResponse, serial, (RadioError)responseDataPtr->errorCode, NULL);
                  }
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, RadioError::NO_MEMORY, NULL);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

bool dataLengthCheck(uint32_t data_index, uint32_t data_len) {
  if( data_index >= data_len) {
    QCRIL_LOG_ERROR("Invalid parameters; invalid data length");
    return false;
  }
  return true;
}

qcril::interfaces::AdnRecordInfo* constructAdnRecord
(
  uint8_t *data,
  uint32_t data_len,
  qcril::interfaces::AdnRecordInfo* record_data_ptr
)
{
    uint32_t data_index;
    uint16_t email_index, anr_index, email_elements, anr_elements;
    uint16_t name_length, number_length, email_length, anr_length;

    if ((NULL != data) && (1 < data_len))
    {
      data_index = 0;

      record_data_ptr->record_id = data[data_index++];
      record_data_ptr->record_id += data[data_index++]<<8;
      QCRIL_LOG_INFO("record_id is %d", record_data_ptr->record_id);

      if (!dataLengthCheck(data_index + 1, data_len)) {
        return NULL;
      }

      name_length = data[data_index++];
      name_length += data[data_index++]<<8;
      QCRIL_LOG_INFO("name_length is %d", name_length);

      if(name_length > 0 && dataLengthCheck(data_index + name_length, data_len))
      {
          record_data_ptr->name = std::string(static_cast<const char*>((char *)data + data_index));
          name_length ++;
      }

      data_index += name_length;
      QCRIL_LOG_INFO("name is %s", record_data_ptr->name.c_str());

      if (!dataLengthCheck(data_index, data_len)) {
        return NULL;
      }

      number_length = data[data_index++];
      number_length += data[data_index++]<<8;
      QCRIL_LOG_INFO("number_length is %d", number_length);

      if(number_length > 0 && dataLengthCheck(data_index + number_length, data_len))
      {
          record_data_ptr->number = std::string(static_cast<const char*>((char *)data + data_index));
          number_length ++;
      }

      data_index += number_length;
      QCRIL_LOG_INFO("number is %s", record_data_ptr->number.c_str());

      if (!dataLengthCheck(data_index + 1, data_len)) {
        return NULL;
      }

      email_elements = data[data_index++];
      email_elements += data[data_index++]<<8;
      if (!dataLengthCheck(data_index, data_len)) {
        return NULL;
      }
      record_data_ptr->email_elements = email_elements;
      QCRIL_LOG_INFO("email_elements is %d", email_elements);
      for(email_index = 0; email_index < email_elements; email_index++)
      {
        if (!dataLengthCheck(data_index + 1, data_len)) {
          return NULL;
        }
        email_length = data[data_index++];
        email_length += data[data_index++]<<8;

        if (!dataLengthCheck(data_index + email_length, data_len)) {
          return NULL;
        }

        record_data_ptr->email[email_index] =
                std::string(static_cast<const char*>((char *)data + data_index));
        data_index += email_length + 1;
      }

      if (!dataLengthCheck(data_index + 1, data_len)) {
        return NULL;
      }

      anr_elements = data[data_index++];
      anr_elements += data[data_index++]<<8;
      if (!dataLengthCheck(data_index, data_len)) {
        return NULL;
      }
      record_data_ptr->anr_elements = anr_elements;
      QCRIL_LOG_INFO("anr_elements is %d", anr_elements);
      for(anr_index = 0; anr_index < anr_elements; anr_index++)
      {
        if (!dataLengthCheck(data_index + 1, data_len)) {
          return NULL;
        }
        anr_length = data[data_index++];
        anr_length += data[data_index++]<<8;

        if (!dataLengthCheck(data_index + anr_length, data_len)) {
          return NULL;
        }

        record_data_ptr->ad_number[anr_index] =
                std::string(static_cast<const char*>((char *)data + data_index));
        data_index += anr_length + 1;
      }

      return record_data_ptr;
    }
    else
    {
      QCRIL_LOG_ERROR("parameter error");
      return NULL;
    }
}

//===========================================================================
// oem_hook_impl::updateAdnRecord
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_UPDATE_ADN_RECORD request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::updateAdnRecord(int32_t serial, uint8_t *data, uint32_t dataLen) {
    QCRIL_LOG_INFO("OemHookImpl::updateAdnRecord on slot %d", mInstanceId);

    qcril::interfaces::AdnRecordInfo     recordData;

    if (constructAdnRecord(data, dataLen, &recordData) == NULL)
    {
      QCRIL_LOG_ERROR( "%s", " Invalid input for data \n");

      OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
      return Void();
    }

    auto msg = std::make_shared<QcRilRequestUpdateAdnRecordMessage>(getContext(serial), recordData);
    if (msg) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
                QCRIL_NOTUSED( status );
                if (solicitedMsg && responseDataPtr) {
                  if(responseDataPtr->data) {
                    int32_t recordIndex =
                        std::static_pointer_cast<qcril::interfaces::AdnRecordUpdatedResp>(
                        responseDataPtr->data)->getRecordIndex();
                    hidl_vec<uint8_t> data;
                    data.setToExternal((uint8_t*) &recordIndex, sizeof(recordIndex));
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, (RadioError)responseDataPtr->errorCode, data);
                  } else {
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(),
                            oemHookRawResponse, serial, (RadioError)responseDataPtr->errorCode, NULL);
                  }
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, RadioError::NO_MEMORY, NULL);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

//===========================================================================
// oem_hook_impl::setMbnConfig
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_SET_MODEM_TEST_MODE request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::setMbnConfig(int32_t serial, uint8_t *data, uint32_t dataLen) {
    uint16_t dataIndex = 0;
    char mbnFilePath[QCRIL_MBN_FILE_PATH_LEN];
    size_t fileLen;

    QCRIL_LOG_INFO("OemHookImpl::setMbnConfig on slot %d", mInstanceId);

    if (NULL == data || dataLen < (1 + sizeof(qcril::interfaces::qcril_pdc_mbn_type)))
    {
        QCRIL_LOG_ERROR("invalid parameter: data is empty");
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);

        return Void();
    }

    // fetch sub ID
    uint32_t subId = data[dataIndex];
    dataIndex ++;

    // fetch mbn type
    qcril::interfaces::qcril_pdc_mbn_type mbnType = (
            qcril::interfaces::qcril_pdc_mbn_type)data[dataIndex];
    dataIndex += sizeof(mbnType);
    QCRIL_LOG_INFO("mbn type is %d", mbnType);

    // fetch mbn file path
    fileLen = strlcpy(mbnFilePath, (const char*)(data + dataIndex),
                        std::min((int)(dataLen - dataIndex), (int)(sizeof(mbnFilePath))));
    dataIndex += fileLen + 1;

    QCRIL_LOG_INFO("mbn file path is %s", mbnFilePath);

    if ( fileLen >= QCRIL_MBN_FILE_PATH_LEN )
    {
        QCRIL_LOG_ERROR("invalid parameter: file name length too long");

        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }
    if ( dataIndex >= dataLen )
    {
        QCRIL_LOG_ERROR("invalid parameter: no config ID specified");
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    // fetch config ID
    std::vector<uint8_t> configId(data + dataIndex, data + dataLen - 1);
    configId.insert(configId.end(), '\0');
    QCRIL_LOG_INFO("config ID is %s", configId.data());

    if ( configId.size() > QCRIL_MBN_CONFIG_ID_LEN )
    {
        QCRIL_LOG_ERROR("invalid paramter: config ID length too long");
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    auto msg = std::make_shared<QcRilRequestSetMbnConfigMessage>(
                getContext(serial), subId, mbnType,
                std::string(static_cast<const char*>((char *)mbnFilePath)),
                configId);
    if (msg) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
                QCRIL_NOTUSED( status );
                if (solicitedMsg && responseDataPtr) {
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, (RadioError)responseDataPtr->errorCode, NULL);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, RadioError::NO_MEMORY, NULL);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

//===========================================================================
// oem_hook_impl::getMbnConfig
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_QUERY_MODEM_TEST_MODE request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::getMbnConfig(int32_t serial, uint8_t *data, uint32_t dataLen) {
    uint16_t dataIndex = 0;

    QCRIL_LOG_INFO("OemHookImpl::getMbnConfig on slot %d", mInstanceId);

    if ((NULL == data) || (1 > dataLen))
    {
        QCRIL_LOG_ERROR("invalid parameter: data is empty");
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    // fetch sub ID
    uint32_t subId = data[dataIndex];
    dataIndex ++;

    // fetch mbn type
    qcril::interfaces::qcril_pdc_mbn_type mbnType = (
            qcril::interfaces::qcril_pdc_mbn_type)data[dataIndex];
    dataIndex += sizeof(mbnType);
    QCRIL_LOG_INFO("mbn type is %d", mbnType);

    auto msg = std::make_shared<QcRilRequestGetMbnConfigMessage>(getContext(serial), subId, mbnType);
    if (msg) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
                QCRIL_NOTUSED( status );
                if (solicitedMsg && responseDataPtr) {
                  if(responseDataPtr->data) {
                    int32_t configLen =
                        std::static_pointer_cast<qcril::interfaces::MbnConfigResp>(responseDataPtr->data)->getLength();
                    std::vector<uint8_t> configId =
                        std::static_pointer_cast<qcril::interfaces::MbnConfigResp>(responseDataPtr->data)->getConfig();
                    QCRIL_LOG_INFO("Sending response with config ID is %s", configId.data());
                    hidl_vec<uint8_t> data;
                    data.setToExternal((uint8_t*)configId.data(), configLen);
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, (RadioError)responseDataPtr->errorCode, data);
                  } else {
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                              serial, (RadioError)responseDataPtr->errorCode, NULL);
                }
              }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, RadioError::NO_MEMORY, NULL);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

//===========================================================================
// oem_hook_impl::getAvailableMbnConfig
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_GET_AVAILABLE_CONFIGS request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::getAvailableMbnConfig(int32_t serial) {
    QCRIL_LOG_INFO("OemHookImpl::getAvailableMbnConfig on slot %d", mInstanceId);

    auto msg = std::make_shared<QcRilRequestGetAvlMbnConfigMessage>(getContext(serial));
    if (msg) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
                QCRIL_NOTUSED( status );
                if (solicitedMsg && responseDataPtr) {
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, (RadioError)responseDataPtr->errorCode, NULL);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, RadioError::NO_MEMORY, NULL);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

//===========================================================================
// oem_hook_impl::cleanupMbnConfig
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_CLEANUP_LOADED_CONFIGS request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::cleanupMbnConfig(int32_t serial) {
    QCRIL_LOG_INFO("OemHookImpl::cleanupMbnConfig on slot %d", mInstanceId);

    auto msg = std::make_shared<QcRilRequestCleanupMbnConfigMessage>(getContext(serial));
    if (msg) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
                QCRIL_NOTUSED( status );
                if (solicitedMsg && responseDataPtr) {
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, (RadioError)responseDataPtr->errorCode, NULL);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, RadioError::NO_MEMORY, NULL);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

//===========================================================================
// oem_hook_impl::selectMbnConfig
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_SEL_CONFIG request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::selectMbnConfig(int32_t serial, uint8_t *data, uint32_t dataLen) {
    uint16_t dataIndex = 0;

    QCRIL_LOG_INFO("OemHookImpl::selectMbnConfig on slot %d", mInstanceId);

    if ((NULL == data) || (1 > dataLen))
    {
        QCRIL_LOG_ERROR("invalid parameter: data is empty");
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    // fetch sub ID
    uint32_t subId = data[dataIndex];
    dataIndex ++;

    // fetch mbn type
    qcril::interfaces::qcril_pdc_mbn_type mbnType = (
            qcril::interfaces::qcril_pdc_mbn_type)data[dataIndex];
    dataIndex += sizeof(mbnType);
    QCRIL_LOG_INFO("mbn type is %d", mbnType);

    if ( dataIndex > dataLen )
    {
        QCRIL_LOG_ERROR("invalid parameter: no config ID specified");
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    // fetch config ID
    std::vector<uint8_t> configId(data + dataIndex, data + dataLen);
    configId.insert(configId.end(), '\0');
    QCRIL_LOG_INFO("config ID is %s", configId.data());

    if ( configId.size() > QCRIL_MBN_CONFIG_ID_LEN )
    {
        QCRIL_LOG_ERROR("invalid paramter: config ID length too long");
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    auto msg = std::make_shared<QcRilRequestSelectMbnConfigMessage>(
                getContext(serial), subId, mbnType, configId);
    if (msg) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
                QCRIL_NOTUSED( status );
                if (solicitedMsg && responseDataPtr) {
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, (RadioError)responseDataPtr->errorCode, NULL);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, RadioError::NO_MEMORY, NULL);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

//===========================================================================
// oem_hook_impl::getMetaInfo
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_GET_META_INFO request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::getMetaInfo(int32_t serial, uint8_t *data, uint32_t dataLen) {
    uint16_t dataIndex = 0;

    QCRIL_LOG_INFO("OemHookImpl::getMetaInfo on slot %d", mInstanceId);

    if ((NULL == data) || (1 > dataLen))
    {
        QCRIL_LOG_ERROR("invalid parameter: data is empty");
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    // fetch mbn type
    qcril::interfaces::qcril_pdc_mbn_type mbnType = (
            qcril::interfaces::qcril_pdc_mbn_type)data[dataIndex];
    dataIndex += sizeof(mbnType);
    QCRIL_LOG_INFO("mbn type is %d", mbnType);

    if ( dataIndex > dataLen )
    {
        QCRIL_LOG_ERROR("invalid parameter: no config ID specified");
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    // fetch config ID
    std::vector<uint8_t> configId(data + dataIndex, data + dataLen);
    configId.insert(configId.end(), '\0');
    QCRIL_LOG_INFO("config ID is %s", configId.data());

    if ( configId.size() > QCRIL_MBN_CONFIG_ID_LEN )
    {
        QCRIL_LOG_ERROR("invalid paramter: config ID length too long");
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    auto msg = std::make_shared<QcRilRequestGetMetaInfoMessage>(
            getContext(serial), mbnType, configId);
    if (msg) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
                QCRIL_NOTUSED( status );
                if (solicitedMsg && responseDataPtr) {
                  if(responseDataPtr->data) {
                    int32_t configLen =
                        std::static_pointer_cast<qcril::interfaces::MbnConfigResp>(responseDataPtr->data)->getLength();
                    std::vector<uint8_t> configInfo =
                        std::static_pointer_cast<qcril::interfaces::MbnConfigResp>(responseDataPtr->data)->getConfig();
                    QCRIL_LOG_INFO("Sending response with config info is %s", configInfo.data());
                    hidl_vec<uint8_t> data;
                    data.setToExternal((uint8_t*)configInfo.data(), configLen);
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, (RadioError)responseDataPtr->errorCode, data);
                  } else {
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                              serial, (RadioError)responseDataPtr->errorCode, NULL);
                  }
              }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, RadioError::NO_MEMORY, NULL);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

//===========================================================================
// oem_hook_impl::deactivateMbnConfig
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_DEACTIVATE_CONFIGS request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::deactivateMbnConfig(int32_t serial, uint8_t *data, uint32_t dataLen) {
    QCRIL_LOG_INFO("OemHookImpl::deactivateMbnConfig on slot %d", mInstanceId);

    if ((NULL == data) || (1 > dataLen))
    {
        QCRIL_LOG_ERROR("invalid parameter: data is empty");
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    // fetch mbn type
    qcril::interfaces::qcril_pdc_mbn_type mbnType = (qcril::interfaces::qcril_pdc_mbn_type)*data;
    QCRIL_LOG_INFO("mbn type is %d", mbnType);

    auto msg = std::make_shared<QcRilRequestDeactivateMbnConfigMessage>(getContext(serial), mbnType);
    if (msg) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
                QCRIL_NOTUSED( status );
                if (solicitedMsg && responseDataPtr) {
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, (RadioError)responseDataPtr->errorCode, NULL);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, RadioError::NO_MEMORY, NULL);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

//===========================================================================
// oem_hook_impl::getQcVersionOfFile
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_GET_QC_VERSION_OF_FILE request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::getQcVersionOfFile(int32_t serial, uint8_t *data, uint32_t dataLen) {
    char mbnFilePath[QCRIL_MBN_FILE_PATH_LEN];

    QCRIL_LOG_INFO("OemHookImpl::getQcVersionOfFile on slot %d", mInstanceId);

    if ((NULL == data) || (1 > dataLen))
    {
        QCRIL_LOG_ERROR("invalid parameter: data is empty");
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    if((dataLen > QCRIL_MBN_FILE_PATH_LEN) || (strlen((char *)data) >= QCRIL_MBN_FILE_PATH_LEN))
    {
        QCRIL_LOG_ERROR("invalid parameter: file name length too long");

        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    // fetch mbn file path
    memset(mbnFilePath, 0, sizeof(mbnFilePath));
    strlcpy(mbnFilePath, (const char *)data,
        std::min((int)(dataLen+1), (int)(sizeof(mbnFilePath))));

    QCRIL_LOG_INFO("mbn file path is %s", mbnFilePath);

    auto msg = std::make_shared<QcRilRequestGetQcVersionOfFileMessage>(getContext(serial), mbnFilePath);
    if (msg) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
                QCRIL_NOTUSED( status );
                if (solicitedMsg && responseDataPtr) {
                  if(responseDataPtr->data) {
                    int32_t versionLen =
                        std::static_pointer_cast<qcril::interfaces::FileVersionResp>(responseDataPtr->data)->getLength();
                    std::vector<uint8_t> fileVersion =
                        std::static_pointer_cast<qcril::interfaces::FileVersionResp>(responseDataPtr->data)->getVersion();
                    hidl_vec<uint8_t> data;
                    data.setToExternal((uint8_t*)fileVersion.data(), versionLen);
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, (RadioError)responseDataPtr->errorCode, data);
                  } else {
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                              serial, (RadioError)responseDataPtr->errorCode, NULL);
                  }
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, RadioError::NO_MEMORY, NULL);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

//===========================================================================
// oem_hook_impl::validateMbnConfig
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_VALIDATE_CONFIG request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::validateMbnConfig(int32_t serial, uint8_t *data, uint32_t dataLen) {
    uint16_t dataIndex = 0;

    QCRIL_LOG_INFO("OemHookImpl::validateMbnConfig on slot %d", mInstanceId);

    if ((NULL == data) || (1 > dataLen))
    {
        QCRIL_LOG_ERROR("invalid parameter: data is empty");
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    // fetch sub ID
    uint32_t subId = data[dataIndex];
    dataIndex ++;

    if ( dataIndex > dataLen )
    {
        QCRIL_LOG_ERROR("invalid parameter: no config ID specified");
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    // fetch config ID
    std::vector<uint8_t> configId(data + dataIndex, data + dataLen);
    configId.insert(configId.end(), '\0');
    QCRIL_LOG_INFO("config ID is %s", configId.data());

    if ( configId.size() > QCRIL_MBN_CONFIG_ID_LEN )
    {
        QCRIL_LOG_ERROR("invalid paramter: config ID length too long");
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    auto msg = std::make_shared<QcRilRequestValidateMbnConfigMessage>(getContext(serial), subId, configId);
    if (msg) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
                QCRIL_NOTUSED( status );
                if (solicitedMsg && responseDataPtr) {
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, (RadioError)responseDataPtr->errorCode, NULL);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, RadioError::NO_MEMORY, NULL);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

//===========================================================================
// oem_hook_impl::getQcVersionOfConfig
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_GET_QC_VERSION_OF_CONFIGID request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::getQcVersionOfConfig(int32_t serial, uint8_t *data, uint32_t dataLen) {

    QCRIL_LOG_INFO("OemHookImpl::getQcVersionOfConfig on slot %d", mInstanceId);

    if ((NULL == data) || (1 > dataLen))
    {
        QCRIL_LOG_ERROR("invalid parameter: data is empty");
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    // fetch config ID
    std::vector<uint8_t> configId(data, data + dataLen);
    configId.insert(configId.end(), '\0');
    QCRIL_LOG_INFO("config ID is %s", configId.data());

    if ( configId.size() > QCRIL_MBN_CONFIG_ID_LEN )
    {
        QCRIL_LOG_ERROR("invalid parameter: config ID length too long");

        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    auto msg = std::make_shared<QcRilRequestGetQcVersionOfConfigMessage>(getContext(serial), configId);
    if (msg) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
                QCRIL_NOTUSED( status );
                if (solicitedMsg && responseDataPtr) {
                  if(responseDataPtr->data) {
                    int configVersion =
                        std::static_pointer_cast<qcril::interfaces::ConfigVersionResp>(responseDataPtr->data)->getVersion();
                    hidl_vec<uint8_t> data;
                    data.setToExternal((uint8_t*)&(configVersion), sizeof(configVersion));
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, (RadioError)responseDataPtr->errorCode, data);
                  } else {
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, (RadioError)responseDataPtr->errorCode, NULL);
                  }
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, RadioError::NO_MEMORY, NULL);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

//===========================================================================
// oem_hook_impl::getOemVersionOfFile
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_FILE request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::getOemVersionOfFile(int32_t serial, uint8_t *data, uint32_t dataLen) {
    char mbnFilePath[QCRIL_MBN_FILE_PATH_LEN];

    QCRIL_LOG_INFO("OemHookImpl::getOemVersionOfFile on slot %d", mInstanceId);

    if ((NULL == data) || (1 > dataLen))
    {
        QCRIL_LOG_ERROR("invalid parameter: data is empty");
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    if((dataLen > QCRIL_MBN_FILE_PATH_LEN) || (strlen((char *)data) >= QCRIL_MBN_FILE_PATH_LEN))
    {
        QCRIL_LOG_ERROR("invalid parameter: file name length too long");
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    // fetch mbn file path
    memset(mbnFilePath, 0, sizeof(mbnFilePath));
    strlcpy(mbnFilePath, (char *)data, sizeof(mbnFilePath));

    QCRIL_LOG_INFO("mbn file path is %s", mbnFilePath);

    auto msg = std::make_shared<QcRilRequestGetOemVersionOfFileMessage>(getContext(serial), mbnFilePath);
    if (msg) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                    std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
                QCRIL_NOTUSED( status );
                if (solicitedMsg && responseDataPtr) {
                  if(responseDataPtr->data) {
                    int32_t versionLen =
                        std::static_pointer_cast<qcril::interfaces::FileVersionResp>(responseDataPtr->data)->getLength();
                    std::vector<uint8_t> fileVersion =
                        std::static_pointer_cast<qcril::interfaces::FileVersionResp>(responseDataPtr->data)->getVersion();
                    hidl_vec<uint8_t> data;
                    data.setToExternal((uint8_t*)fileVersion.data(), versionLen);
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, (RadioError)responseDataPtr->errorCode, data);
                  } else {
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                              serial, (RadioError)responseDataPtr->errorCode, NULL);
                  }
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, RadioError::NO_MEMORY, NULL);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

//===========================================================================
// oem_hook_impl::getOemVersionOfConfig
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_CONFIGID request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::getOemVersionOfConfig(int32_t serial, uint8_t *data, uint32_t dataLen) {

    QCRIL_LOG_INFO("OemHookImpl::getOemVersionOfConfig on slot %d", mInstanceId);

    if ((NULL == data) || (1 > dataLen))
    {
        QCRIL_LOG_ERROR("invalid parameter: data is empty");
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    // fetch config ID
    std::vector<uint8_t> configId(data, data + dataLen);
    configId.insert(configId.end(), '\0');
    QCRIL_LOG_INFO("config ID is %s", configId.data());

    if ( configId.size() > QCRIL_MBN_CONFIG_ID_LEN )
    {
        QCRIL_LOG_ERROR("invalid parameter: config ID length too long");

        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    auto msg = std::make_shared<QcRilRequestGetOemVersionOfConfigMessage>(getContext(serial), configId);
    if (msg) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
                QCRIL_NOTUSED( status );
                if (solicitedMsg && responseDataPtr) {
                  if(responseDataPtr->data) {
                    int configVersion =
                        std::static_pointer_cast<qcril::interfaces::ConfigVersionResp>(
                        responseDataPtr->data)->getVersion();
                    hidl_vec<uint8_t> data;
                    data.setToExternal((uint8_t*)&(configVersion), sizeof(configVersion));
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, (RadioError)responseDataPtr->errorCode, data);
                  } else {
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, (RadioError)responseDataPtr->errorCode, NULL);
                  }
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, RadioError::NO_MEMORY, NULL);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

//===========================================================================
// oem_hook_impl::activateMbnConfig
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_ACTIVATE_CONFIGS request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::activateMbnConfig(int32_t serial, uint8_t *data, uint32_t dataLen) {
    uint16_t dataIndex = 0;
    QCRIL_LOG_INFO("OemHookImpl::activateMbnConfig on slot %d", mInstanceId);

    if ((NULL == data) || (1 > dataLen))
    {
        QCRIL_LOG_ERROR("invalid parameter: data is empty");
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    // fetch sub ID
    uint32_t subId = data[dataIndex];
    dataIndex ++;

    // fetch mbn type
    qcril::interfaces::qcril_pdc_mbn_type mbnType = (qcril::interfaces::qcril_pdc_mbn_type)data[dataIndex];
    dataIndex += sizeof(mbnType);
    QCRIL_LOG_INFO("mbn type is %d", mbnType);

    auto msg = std::make_shared<QcRilRequestActivateMbnConfigMessage>(getContext(serial),subId, mbnType);
    if (msg) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
                QCRIL_NOTUSED( status );
                if (solicitedMsg && responseDataPtr) {
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, (RadioError)responseDataPtr->errorCode, NULL);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, RadioError::NO_MEMORY, NULL);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

//===========================================================================
// oem_hook_impl::enableModemUpdate
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_EVT_HOOK_ENABLE_MODEM_UPDATE request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::enableModemUpdate(int32_t serial, uint8_t *data, uint32_t dataLen) {
    QCRIL_LOG_INFO("OemHookImpl::enableModemUpdate on slot %d", mInstanceId);

    if ((NULL == data) || (1 > dataLen))
    {
        QCRIL_LOG_ERROR("invalid parameter: data is empty");
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::INVALID_ARGUMENTS, NULL);
        return Void();
    }

    // fetch sub ID
    uint32_t subId = data[0];

    auto msg = std::make_shared<QcRilRequestEnableModemUpdateMessage>(getContext(serial), subId);
    if (msg) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> responseDataPtr) -> void {
                QCRIL_NOTUSED( status );
                if (solicitedMsg && responseDataPtr) {
                    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, (RadioError)responseDataPtr->errorCode, NULL);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                                serial, RadioError::NO_MEMORY, NULL);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

//===========================================================================
// oem_hook_impl::uimGetVoltageStatus
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_UICC_VOLTAGE_STATUS_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::uimGetVoltageStatus(int32_t serial)
{
  QCRIL_LOG_INFO("OemHookImpl::uimGetVoltageStatus on slot %d", mInstanceId);

  auto msg = std::make_shared<UimVoltageStatusRequestMsg>(mInstanceId);
  if (msg)
  {
    GenericCallback<RIL_UIM_Errno> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_Errno> responseDataPtr) -> void
    {
      if (solicitedMsg && responseDataPtr &&
          status == Message::Callback::Status::SUCCESS)
      {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, (RadioError)*responseDataPtr, NULL);
      }
      else if (solicitedMsg)
      {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, RadioError::INTERNAL_ERR, NULL);
      }
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

//===========================================================================
// oem_hook_impl::uimSendCardPowerReq
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_UICC_POWER_REQ and QCRIL_REQ_HOOK_SLOT_CARD_POWER_REQ
    request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::uimSendCardPowerReq(int32_t serial, uint32_t cmd_id,
                                              uint8_t *data, uint32_t dataLen)
{
  RIL_UIM_CardPowerState card_power = RIL_UIM_CARD_POWER_DOWN;
  uint8_t                slot       = mInstanceId;

  QCRIL_LOG_INFO("OemHookImpl::uimSendCardPowerReq on slot %d", mInstanceId);

  if (data == NULL || dataLen == 0)
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::INVALID_ARGUMENTS, NULL);

    QCRIL_LOG_FUNC_RETURN();
    return Void();
  }

  if (cmd_id == QCRIL_REQ_HOOK_SLOT_CARD_POWER_REQ)
  {
    RIL_SlotCardPower *in_ptr = (RIL_SlotCardPower *)data;

    card_power = (RIL_UIM_CardPowerState)(in_ptr->card_power);
    slot = in_ptr->slot_id;
  }
  else
  {
    card_power = (RIL_UIM_CardPowerState)*data;
  }

  auto msg = std::make_shared<UimCardPowerReqMsg>(slot, card_power);
  if (msg)
  {
    GenericCallback<RIL_UIM_Errno> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_Errno> responseDataPtr) -> void
    {
      if (solicitedMsg && responseDataPtr &&
          status == Message::Callback::Status::SUCCESS)
      {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, (RadioError)*responseDataPtr, NULL);
      }
      else if (solicitedMsg)
      {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, RadioError::INTERNAL_ERR, NULL);
      }
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                            serial, RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

//===========================================================================
// convertOemHookToRilPersoSubState
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Utility function to convert OemHook to RIL PersoSubState.

    @return
    RIL_UIM_PersoSubstate
*/
/*=========================================================================*/
RIL_UIM_PersoSubstate convertOemHookToRilPersoSubState(int32_t persoSubState){
  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_LOG_INFO("convertOemHookToRilPersoSubState: persoSubState %d ", persoSubState);

  switch(persoSubState) {
    case 25 :
      return RIL_UIM_PERSOSUBSTATE_SIM_SPN;
    case 26 :
      return RIL_UIM_PERSOSUBSTATE_SIM_SPN_PUK;
    case 27 :
      return RIL_UIM_PERSOSUBSTATE_SIM_SP_EHPLMN;
    case 28 :
      return RIL_UIM_PERSOSUBSTATE_SIM_SP_EHPLMN_PUK;
    case 29 :
      return RIL_UIM_PERSOSUBSTATE_SIM_ICCID;
    case 30 :
      return RIL_UIM_PERSOSUBSTATE_SIM_ICCID_PUK;
    case 31 :
      return RIL_UIM_PERSOSUBSTATE_SIM_IMPI;
    case 32 :
      return RIL_UIM_PERSOSUBSTATE_SIM_IMPI_PUK;
    case 33 :
      return RIL_UIM_PERSOSUBSTATE_SIM_NS_SP;
    case 34 :
      return RIL_UIM_PERSOSUBSTATE_SIM_NS_SP_PUK;
    default:
      return (RIL_UIM_PersoSubstate)persoSubState;
      /* Only above values require conversion, others are 1:1 mapped */
  }
}

//===========================================================================
// oem_hook_impl::uimSendPersoReq
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_PERSONALIZATION_REACTIVATE_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::uimSendPersoReq(int32_t serial, uint8_t *data, uint32_t dataLen)
{
  RIL_UIM_PersonalizationReq   mPersoData   = {};
  uint32_t                     ctrl_key_len = 0;
  int32_t                      tempPersoType = 0;

  QCRIL_LOG_INFO("OemHookImpl::uimSendPersoReq on slot %d", mInstanceId);

  if (data == NULL || dataLen <= (strlen((char *)data) + 1))
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::INVALID_ARGUMENTS, NULL);

    QCRIL_LOG_FUNC_RETURN();
    return Void();
  }

  ctrl_key_len = strlen((char *)data) + 1;

  mPersoData.controlKey = (char *)data;
  mPersoData.persoType = (RIL_UIM_PersoSubstate)*((uint8_t *)data + ctrl_key_len);
  tempPersoType = (int32_t) mPersoData.persoType;
  mPersoData.persoType = convertOemHookToRilPersoSubState(tempPersoType);

  auto msg = std::make_shared<UimPersoRequestMsg>(mPersoData);
  if (msg)
  {
    GenericCallback<RIL_UIM_PersoResponse> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_PersoResponse> responseDataPtr) -> void
    {
      if (solicitedMsg && responseDataPtr &&
          status == Message::Callback::Status::SUCCESS)
      {
        int               retries = responseDataPtr->no_of_retries;
        hidl_vec<uint8_t> data    = {};

        data.setToExternal((uint8_t*)&retries, sizeof(retries), false);

        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, (RadioError)responseDataPtr->err, data);
      }
      else if (solicitedMsg)
      {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, RadioError::INTERNAL_ERR, NULL);
      }
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

//===========================================================================
// oem_hook_impl::uimGetPersoStatus
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_PERSONALIZATION_STATUS_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::uimGetPersoStatus(int32_t serial)
{
  QCRIL_LOG_INFO("OemHookImpl::uimGetPersoStatus on slot %d", mInstanceId);

  auto msg = std::make_shared<UimGetPersoStatusRequestMsg>();
  if (msg)
  {
    GenericCallback<RIL_UIM_PersonalizationStatusResp> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_PersonalizationStatusResp> rsp_ptr) -> void
    {
      if (solicitedMsg && rsp_ptr &&
          status == Message::Callback::Status::SUCCESS)
      {
        RIL_UIM_PersonalizationStatusResp  rsp = {};
        hidl_vec<uint8_t> data    = {};

        if (rsp_ptr->has_gwNWPersoStatus)
        {
          rsp.has_gwNWPersoStatus = TRUE;
          rsp.gwNWPersoStatus.verifyAttempts = rsp_ptr->gwNWPersoStatus.verifyAttempts;
        }
        else if (rsp_ptr->has_gwNWSubsetPersoStatus)
        {
          rsp.has_gwNWSubsetPersoStatus = TRUE;
          rsp.gwNWSubsetPersoStatus.verifyAttempts = rsp_ptr->gwNWSubsetPersoStatus.verifyAttempts;
        }
        else if (rsp_ptr->has_gwSPPersoStatus)
        {
          rsp.has_gwSPPersoStatus = TRUE;
          rsp.gwSPPersoStatus.verifyAttempts = rsp_ptr->gwSPPersoStatus.verifyAttempts;
        }
        else if (rsp_ptr->has_gwCPPersoStatus)
        {
          rsp.has_gwCPPersoStatus = TRUE;
          rsp.gwCPPersoStatus.verifyAttempts = rsp_ptr->gwCPPersoStatus.verifyAttempts;
        }
        else if (rsp_ptr->has_gwSPNPersoStatus)
        {
          rsp.has_gwSPNPersoStatus = TRUE;
          rsp.gwSPNPersoStatus.verifyAttempts = rsp_ptr->gwSPNPersoStatus.verifyAttempts;
        }
        else if (rsp_ptr->has_gwSPEhplmnPersoStatus)
        {
          rsp.has_gwSPEhplmnPersoStatus = TRUE;
          rsp.gwSPEhplmnPersoStatus.verifyAttempts = rsp_ptr->gwSPEhplmnPersoStatus.verifyAttempts;
        }
        else if (rsp_ptr->has_gwICCIDPersoStatus)
        {
          rsp.has_gwICCIDPersoStatus = TRUE;
          rsp.gwICCIDPersoStatus.verifyAttempts = rsp_ptr->gwICCIDPersoStatus.verifyAttempts;
        }
        else if (rsp_ptr->has_gwIMPIPersoStatus)
        {
          rsp.has_gwIMPIPersoStatus = TRUE;
          rsp.gwIMPIPersoStatus.verifyAttempts = rsp_ptr->gwIMPIPersoStatus.verifyAttempts;
        }
        else if (rsp_ptr->has_gwSIMPersoStatus)
        {
          rsp.has_gwSIMPersoStatus = TRUE;
          rsp.gwSIMPersoStatus.verifyAttempts = rsp_ptr->gwSIMPersoStatus.verifyAttempts;
        }
        else if (rsp_ptr->has_gwNSSPPersoStatus)
        {
          rsp.has_gwNSSPPersoStatus = TRUE;
          rsp.gwNSSPPersoStatus.verifyAttempts = rsp_ptr->gwNSSPPersoStatus.verifyAttempts;
        }
        else if (rsp_ptr->has_gw1xNWType1PersoStatus)
        {
          rsp.has_gw1xNWType1PersoStatus = TRUE;
          rsp.gw1xNWType1PersoStatus.verifyAttempts = rsp_ptr->gw1xNWType1PersoStatus.verifyAttempts;
        }
        else if (rsp_ptr->has_gw1xNWType2PersoStatus)
        {
          rsp.has_gw1xNWType2PersoStatus = TRUE;
          rsp.gw1xNWType2PersoStatus.verifyAttempts = rsp_ptr->gw1xNWType2PersoStatus.verifyAttempts;
        }
        else if (rsp_ptr->has_gw1xRUIMPersoStatus)
        {
          rsp.has_gw1xRUIMPersoStatus = TRUE;
          rsp.gw1xRUIMPersoStatus.verifyAttempts = rsp_ptr->gw1xRUIMPersoStatus.verifyAttempts;
        }

        data.setToExternal((uint8_t*)&rsp, sizeof(rsp), false);

        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, (RadioError)rsp_ptr->err, data);
      }
      else if (solicitedMsg)
      {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, RadioError::INTERNAL_ERR, NULL);
      }
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

//===========================================================================
// oem_hook_impl::uimSendDePersoReq
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_ENTER_DEPERSONALIZATION_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::uimSendDePersoReq(int32_t serial, uint8_t *data, uint32_t dataLen)
{
  RIL_UIM_PersoSubstate  perso_state   = {};
  uint32_t               code_len      = 0;
  int32_t                tempPersoState = 0;

  QCRIL_LOG_INFO("OemHookImpl::uimSendDePersoReq on slot %d", mInstanceId);

  if (data == NULL || dataLen <= (strlen((char *)data) + 1))
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::INVALID_ARGUMENTS, NULL);

    QCRIL_LOG_FUNC_RETURN();
    return Void();
  }

  code_len = strlen((char *)data) + 1;

  perso_state = (RIL_UIM_PersoSubstate)(atoi((char *)data));
  tempPersoState = (int32_t)perso_state;
  perso_state = convertOemHookToRilPersoSubState(tempPersoState);
  data += code_len;

  if (data == NULL || dataLen <= (strlen((char *)data) + 1))
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::INVALID_ARGUMENTS, NULL);

    QCRIL_LOG_FUNC_RETURN();
    return Void();
  }

  auto msg = std::make_shared<UimEnterDePersoRequestMsg>((char *)data, perso_state);
  if (msg)
  {
    GenericCallback<RIL_UIM_PersoResponse> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_PersoResponse> responseDataPtr) -> void
    {
      if (solicitedMsg && responseDataPtr &&
          status == Message::Callback::Status::SUCCESS)
      {
        int               retries = responseDataPtr->no_of_retries;
        hidl_vec<uint8_t> data    = {};

        data.setToExternal((uint8_t*)&retries, sizeof(retries), false);

        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, (RadioError)responseDataPtr->err, data);
      }
      else if (solicitedMsg)
      {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, RadioError::INTERNAL_ERR, NULL);
      }
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

//===========================================================================
// oem_hook_impl::uimGetCardStateReq
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_SLOT_GET_CARD_STATE_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::uimGetCardStateReq(int32_t serial, uint8_t *data, uint32_t dataLen)
{
  QCRIL_LOG_INFO("OemHookImpl::uimGetCardStateReq on slot %d", mInstanceId);

  if (data == NULL || dataLen == 0)
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::INVALID_ARGUMENTS, NULL);

    QCRIL_LOG_FUNC_RETURN();
    return Void();
  }

  auto msg = std::make_shared<UimGetCardStatusRequestMsg>((uint8_t)(*((int *)data)));
  if (msg)
  {
    GenericCallback<RIL_UIM_CardStatus> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_CardStatus> responseDataPtr) -> void
    {
      if (solicitedMsg && responseDataPtr &&
          status == Message::Callback::Status::SUCCESS)
      {
        RIL_UIM_CardState card_state = responseDataPtr->card_state;
        hidl_vec<uint8_t> data       = {};

        data.setToExternal((uint8_t*)&card_state, sizeof(card_state), false);

        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, (RadioError)responseDataPtr->err, data);
      }
      else if (solicitedMsg)
      {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, RadioError::INTERNAL_ERR, NULL);
      }
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

//===========================================================================
// oem_hook_impl::uimGetCardAtrReq
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_SLOT_GET_ATR_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::uimGetCardAtrReq(int32_t serial, uint8_t *data, uint32_t dataLen)
{
  QCRIL_LOG_INFO("OemHookImpl::uimGetCardAtrReq on slot %d", mInstanceId);

  if (data == NULL || dataLen == 0)
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::INVALID_ARGUMENTS, NULL);

    QCRIL_LOG_FUNC_RETURN();
    return Void();
  }

  auto msg = std::make_shared<UimGetAtrRequestMsg>((uint8_t)(*((int *)data)));
  if (msg)
  {
    GenericCallback<UimAtrRspParam> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<UimAtrRspParam> rsp_ptr) -> void
    {
      if (solicitedMsg && rsp_ptr &&
          status == Message::Callback::Status::SUCCESS)
      {
        hidl_vec<uint8_t> data       = {};

        data.setToExternal((uint8_t*)(rsp_ptr->atr.data()), rsp_ptr->atr.length(), false);

        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, (RadioError)rsp_ptr->err, data);
      }
      else if (solicitedMsg)
      {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, RadioError::INTERNAL_ERR, NULL);
      }
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

//===========================================================================
// oem_hook_impl::uimSendApduReq
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_SLOT_SEND_APDU_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::uimSendApduReq(int32_t serial, uint8_t *data, uint32_t dataLen)
{
  RIL_SlotStreamApduReq  * apdu_ptr  = NULL;
  RIL_UIM_SIM_APDU         apdu_data = {};

  QCRIL_LOG_INFO("OemHookImpl::uimSendApduReq on slot %d", mInstanceId);

  if (data == NULL || dataLen == 0)
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::INVALID_ARGUMENTS, NULL);

    QCRIL_LOG_FUNC_RETURN();
    return Void();
  }

  apdu_ptr = (RIL_SlotStreamApduReq *)data;

  apdu_data.sessionid = apdu_ptr->apdu_data.sessionid;
  apdu_data.cla = apdu_ptr->apdu_data.cla;
  apdu_data.instruction = apdu_ptr->apdu_data.instruction;
  apdu_data.p1 = apdu_ptr->apdu_data.p1;
  apdu_data.p2 = apdu_ptr->apdu_data.p2;
  apdu_data.p3 = apdu_ptr->apdu_data.p3;
  apdu_data.data = apdu_ptr->apdu_data.data;

  auto msg = std::make_shared<UimTransmitAPDURequestMsg>(apdu_ptr->slot_id, false, false, apdu_data);
  if (msg)
  {
    GenericCallback<RIL_UIM_SIM_IO_Response> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_SIM_IO_Response> rsp_ptr) -> void
    {
      if (solicitedMsg && rsp_ptr &&
          status == Message::Callback::Status::SUCCESS)
      {
        RIL_SlotSendApduResp  apdu_rsp = {};
        hidl_vec<uint8_t>     data     = {};

        apdu_rsp.slot_id = rsp_ptr->slot_id;
        apdu_rsp.ril_err = (RIL_Errno)rsp_ptr->err;
        apdu_rsp.sw1 = rsp_ptr->sw1;
        apdu_rsp.sw2 = rsp_ptr->sw2;
        if (rsp_ptr->simResponse.length() < sizeof(apdu_rsp.simResponse))
        {
          rsp_ptr->simResponse.copy(apdu_rsp.simResponse, rsp_ptr->simResponse.length());
        }
        data.setToExternal((uint8_t*)&apdu_rsp, sizeof(apdu_rsp), false);

        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, (RadioError)rsp_ptr->err, data);
      }
      else if (solicitedMsg)
      {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, RadioError::INTERNAL_ERR, NULL);
      }
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

//===========================================================================
// oem_hook_impl::uimSwitchSlotReq
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_SWITCH_SLOT_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::uimSwitchSlotReq(int32_t serial, uint8_t *data, uint32_t dataLen)
{
  RIL_PhySlotMap         * slot_map_ptr = NULL;
  std::vector<uint32_t>    mSlotMap     = {};

  QCRIL_LOG_INFO("OemHookImpl::uimSwitchSlotReq on slot %d", mInstanceId);

  if (data == NULL || dataLen == 0)
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::INVALID_ARGUMENTS, NULL);

    QCRIL_LOG_FUNC_RETURN();
    return Void();
  }

  slot_map_ptr = (RIL_PhySlotMap *)data;
  if (slot_map_ptr->array_len > UIM_OEM_HOOK_MAX_CARD_COUNT)
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::INVALID_ARGUMENTS, NULL);
    QCRIL_LOG_FUNC_RETURN();
    return Void();
  }
  mSlotMap.assign(slot_map_ptr->slot_map, slot_map_ptr->slot_map + slot_map_ptr->array_len);

  auto msg = std::make_shared<UimSwitchSlotRequestMsg>(mSlotMap);
  if (msg)
  {
    GenericCallback<RIL_UIM_Errno> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_Errno> responseDataPtr) -> void
    {
      if (solicitedMsg && responseDataPtr &&
          status == Message::Callback::Status::SUCCESS)
      {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, (RadioError)*responseDataPtr, NULL);
      }
      else if (solicitedMsg)
      {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, RadioError::INTERNAL_ERR, NULL);
      }
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

//===========================================================================
// oem_hook_impl::uimGetSlotMapping
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_GET_PHYSICAL_SLOT_MAPPING_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::uimGetSlotMapping(int32_t serial)
{
  QCRIL_LOG_INFO("OemHookImpl::uimGetSlotMapping on slot %d", mInstanceId);

  auto msg = std::make_shared<UimGetPhysicalSlotMappingRequestMsg>();
  if (msg)
  {
    GenericCallback<RIL_UIM_PhySlotMap> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_PhySlotMap> responseDataPtr) -> void
    {
      if (solicitedMsg && responseDataPtr &&
          status == Message::Callback::Status::SUCCESS)
      {
        RIL_PhySlotMap      slot_map = {};
        hidl_vec<uint8_t>   data     = {};

        if (responseDataPtr->slotMap.size() <= UIM_OEM_HOOK_MAX_CARD_COUNT)
        {
          slot_map.array_len = responseDataPtr->slotMap.size();
          for (uint32_t i = 0; i < slot_map.array_len; i++)
          {
            slot_map.slot_map[i] = responseDataPtr->slotMap[i];
          }
        }

        data.setToExternal((uint8_t*)&slot_map, sizeof(slot_map), false);
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, (RadioError)responseDataPtr->err, data);
      }
      else if (solicitedMsg)
      {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, RadioError::INTERNAL_ERR, NULL);
      }
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

//===========================================================================
// oem_hook_impl::uimSetApduBehaviorReq
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_SLOT_SET_APDU_BEHAVIOR_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::uimSetApduBehaviorReq(int32_t serial, uint8_t *data, uint32_t dataLen)
{
  RIL_SetAPDUBehavior      * data_ptr    = NULL;
  RIL_UIM_SetAPDUBehavior    apdu_behav  = {};

  QCRIL_LOG_INFO("OemHookImpl::uimSetApduBehaviorReq on slot %d", mInstanceId);

  if (data == NULL || dataLen == 0)
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::INVALID_ARGUMENTS, NULL);

    QCRIL_LOG_FUNC_RETURN();
    return Void();
  }

  data_ptr = (RIL_SetAPDUBehavior *)data;
  apdu_behav.channel_id = data_ptr->channel_id;
  apdu_behav.apdu_behavior = (RIL_UIM_APDU_Behavior)data_ptr->apdu_behavior;

  auto msg = std::make_shared<UimSetAPDUBehaviorRequestMsg>(mInstanceId, apdu_behav);
  if (msg)
  {
    GenericCallback<RIL_UIM_Errno> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_Errno> responseDataPtr) -> void
    {
      if (solicitedMsg && responseDataPtr &&
          status == Message::Callback::Status::SUCCESS)
      {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, (RadioError)*responseDataPtr, NULL);
      }
      else if (solicitedMsg)
      {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, RadioError::INTERNAL_ERR, NULL);
      }
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

//===========================================================================
// oem_hook_impl::uimGetSlotStatus
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_GET_SLOTS_STATUS_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::uimGetSlotStatus(int32_t serial)
{
  QCRIL_LOG_INFO("OemHookImpl::uimGetSlotStatus on slot %d", mInstanceId);

  auto msg = std::make_shared<UimGetSlotStatusRequestMsg>();
  if (msg)
  {
    GenericCallback<RIL_UIM_SlotsStatusInfo> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_SlotsStatusInfo> resp_ptr) -> void
    {
      if (solicitedMsg && resp_ptr &&
          status == Message::Callback::Status::SUCCESS)
      {
        RIL_SlotsStatus     status   = {};
        hidl_vec<uint8_t>   data     = {};

        if (resp_ptr->slot_status.size() <= UIM_OEM_HOOK_MAX_CARD_COUNT)
        {
          status.no_of_slots = resp_ptr->slot_status.size();

          for (uint8_t i = 0; i < status.no_of_slots && i < UIM_OEM_HOOK_MAX_CARD_COUNT; i++)
          {
            status.slot_status[i].card_state = (Ril_CardPresenceState)resp_ptr->slot_status[i].card_state;
            status.slot_status[i].slot_state = (Ril_SlotState)resp_ptr->slot_status[i].slot_state;
            status.slot_status[i].logical_slot = (uint32_t)resp_ptr->slot_status[i].logical_slot;

            if (resp_ptr->slot_status[i].iccid.size() <= UIM_OEM_HOOK_MAX_ICCID_LEN)
            {
              status.slot_status[i].iccid_len = resp_ptr->slot_status[i].iccid.size();
              memcpy(status.slot_status[i].iccid, resp_ptr->slot_status[i].iccid.data(), status.slot_status[i].iccid_len);
            }
          }
        }
        data.setToExternal((uint8_t*)&status, sizeof(status), false);
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, (RadioError)resp_ptr->err, data);
      }
      else if (solicitedMsg)
      {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, RadioError::INTERNAL_ERR, NULL);
      }
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

//===========================================================================
// oem_hook_impl::uimGetLPATC
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_GET_TERMINAL_CAPABILITY_LPA_TLV_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::uimGetLPATC(int32_t serial)
{
  QCRIL_LOG_INFO("OemHookImpl::uimGetLPATC on slot %d", mInstanceId);

  auto msg = std::make_shared<UimGetLPATermialCapabilitesRequestMsg>(mInstanceId);
  if (msg)
  {
    GenericCallback<RIL_UIM_TerminalCapabilityLPATLV> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_TerminalCapabilityLPATLV> resp_ptr) -> void
    {
      if (solicitedMsg && resp_ptr &&
          status == Message::Callback::Status::SUCCESS)
      {
        RIL_TerminalCapabilityLPATLV     rsp   = {};
        hidl_vec<uint8_t>                data  = {};

        rsp.tlv_present = resp_ptr->tlv_present;
        rsp.value = resp_ptr->value;

        data.setToExternal((uint8_t*)&rsp, sizeof(rsp), false);
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, (RadioError)resp_ptr->err, data);
      }
      else if (solicitedMsg)
      {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, RadioError::INTERNAL_ERR, NULL);
      }
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

//===========================================================================
// oem_hook_impl::uimSetLPATC
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_UICC_VOLTAGE_STATUS_REQ request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::uimSetLPATC(int32_t serial, uint8_t *data, uint32_t dataLen)
{
  RIL_TerminalCapabilityLPATLV  * data_ptr    = NULL;

  QCRIL_LOG_INFO("OemHookImpl::uimSetLPATC on slot %d", mInstanceId);

  if (data == NULL || dataLen == 0)
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::INVALID_ARGUMENTS, NULL);

    QCRIL_LOG_FUNC_RETURN();
    return Void();
  }

  data_ptr = (RIL_TerminalCapabilityLPATLV *)data;

  auto msg = std::make_shared<UimSetLPATermialCapabilitesRequestMsg>(mInstanceId, data_ptr->tlv_present, data_ptr->value);
  if (msg)
  {
    GenericCallback<RIL_UIM_Errno> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_UIM_Errno> responseDataPtr) -> void
    {
      if (solicitedMsg && responseDataPtr &&
          status == Message::Callback::Status::SUCCESS)
      {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, (RadioError)*responseDataPtr, NULL);
      }
      else if (solicitedMsg)
      {
        OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                    serial, RadioError::INTERNAL_ERR, NULL);
      }
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse,
                               serial, RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

//===========================================================================
// OemHookImpl::rejectIncomingCallWithCause21
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_REJECT_INCOMING_CALL_CAUSE_21 request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::rejectIncomingCallWithCause21(int32_t serial, uint8_t * /*data*/,
                                                        uint32_t /*dataLen*/) {
  QCRIL_LOG_INFO("OemHookImpl::rejectIncomingCallWithCause21 on slot %d", mInstanceId);

  auto msg = std::make_shared<QcRilRequestRejectIncomingCallMessage>(getContext(serial));
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                                     (RadioError)errorCode, NULL);  // TODO - map radio error
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                               RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

//===========================================================================
// OemHookImpl::getCurrentSetupCalls
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_GET_CURRENT_SETUP_CALLS request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::getCurrentSetupCalls(int32_t serial, uint8_t * /*data*/,
                                               uint32_t /*dataLen*/) {
  QCRIL_LOG_INFO("OemHookImpl::getCurrentSetupCalls on slot %d", mInstanceId);

  auto msg = std::make_shared<QcRilRequestGetCurrentSetupCallsMessage>(getContext(serial));
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          hidl_vec<uint8_t> data;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
            auto rilCurrentCalls =
                std::static_pointer_cast<qcril::interfaces::GetCurrentSetupCallsRespData>(
                    resp->data);
            if (rilCurrentCalls) {
              qcril_qmi_voice_setup_call_info callInfo = {};
              if (rilCurrentCalls->hasIndex()) {
                callInfo.index = rilCurrentCalls->getIndex();
              }
              if (rilCurrentCalls->hasToa()) {
                callInfo.toa = rilCurrentCalls->getToa();
              }
              if (rilCurrentCalls->hasAls()) {
                callInfo.als = rilCurrentCalls->getAls();
              }
              if (rilCurrentCalls->hasIsVoice()) {
                callInfo.isVoice = rilCurrentCalls->getIsVoice();
              }
              if (rilCurrentCalls->hasNumber() && !rilCurrentCalls->getNumber().empty()) {
                strlcpy(callInfo.number, (const char *)rilCurrentCalls->getNumber().c_str(),
                        sizeof(callInfo.number));
              }
              if (rilCurrentCalls->hasNumberPresentation()) {
                callInfo.numberPresentation = rilCurrentCalls->getNumberPresentation();
              }
              if (rilCurrentCalls->hasName() && !rilCurrentCalls->getName().empty()) {
                strlcpy(callInfo.name, (const char *)rilCurrentCalls->getName().c_str(),
                        sizeof(callInfo.name));
              }
              if (rilCurrentCalls->hasNamePresentation()) {
                callInfo.namePresentation = rilCurrentCalls->getNamePresentation();
              }

              size_t send_buffer_len = sizeof(callInfo);
              unsigned char *send_buffer = new unsigned char[send_buffer_len];
              if (send_buffer) {
                memcpy(send_buffer, &callInfo, send_buffer_len);
              }
              data.setToExternal((uint8_t *)send_buffer, send_buffer_len,
                                 true); /* Send true to allow hidl_vec to delete memory*/
            }
          }
          OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                                     (RadioError)errorCode, data);  // TODO - map radio error
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                               RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

//===========================================================================
// OemHookImpl::requestSetupAnswer
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_REQUEST_SETUP_ANSWER request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::requestSetupAnswer(int32_t serial, uint8_t *data, uint32_t dataLen) {
  QCRIL_LOG_INFO("OemHookImpl::requestSetupAnswer on slot %d", mInstanceId);

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty");
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                               RadioError::INVALID_ARGUMENTS, NULL);
    return Void();
  }

  qcril_qmi_voice_setup_answer_data_type *setupAnswerReq =
      (qcril_qmi_voice_setup_answer_data_type *)data;

  auto msg = std::make_shared<QcRilRequestSetupAnswerMessage>(getContext(serial));
  if (msg) {
    msg->setRejectSetup(setupAnswerReq->rejection ? true : false);
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                                     (RadioError)errorCode, NULL);  // TODO - map radio error
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                               RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

//===========================================================================
// OemHookImpl::requestSetLocalCallHold
//
//===========================================================================
//===========================================================================
/*!
    @brief
    Processes the QCRIL_REQ_HOOK_SET_LOCAL_CALL_HOLD / QCRIL_REQ_HOOK_SET_LOCAL_CALL_HOLD_v01
    request sent by client

    @return
    None
*/
/*=========================================================================*/
Return<void> OemHookImpl::requestSetLocalCallHold(int32_t serial, uint8_t *data, uint32_t dataLen) {
  QCRIL_LOG_INFO("OemHookImpl::requestSetLocalCallHold on slot %d", mInstanceId);

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty");
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                               RadioError::INVALID_ARGUMENTS, NULL);
    return Void();
  }

  int lch = *((int *)data);

  auto msg = std::make_shared<QcRilRequestSetLocalCallHoldMessage>(getContext(serial));
  if (msg) {
    msg->setLocalHoldType(lch ? QcRilRequestSetLocalCallHoldMessage::LocalHoldType::HOLD
                              : QcRilRequestSetLocalCallHoldMessage::LocalHoldType::UNHOLD);
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                                     (RadioError)errorCode, NULL);  // TODO - map radio error
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                               RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> OemHookImpl::startIncrementalScanRequest(int32_t serial) {
  QCRIL_LOG_INFO("OemHookImpl::startIncrementalScanRequest on slot %d, serial %d ",
                                                        mInstanceId, serial);

  auto msg = std::make_shared<RilRequestStartNetworkScanMessage>(getContext(serial));
  msg->setRequestSource(RilRequestStartNetworkScanMessage::RequestSource::OEM);
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                                     (RadioError)errorCode, NULL);  // TODO - map radio error
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                               RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> OemHookImpl::cancelIncrementalScanRequest(int32_t serial) {
  QCRIL_LOG_INFO("OemHookImpl::cancelIncrementalScanRequest on slot %d, serial %d ",
                                                        mInstanceId, serial);

  auto msg = std::make_shared<RilRequestStopNetworkScanMessage>(getContext(serial));

  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                                     (RadioError)errorCode, NULL);  // TODO - map radio error
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                               RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

#ifdef FEATURE_QCRIL_LTE_DIRECT
static qcril::interfaces::lte_direct::DiscoveryType
convertDiscoveryType(LteDirectDiscovery_DiscoveryType in) {
  switch (in) {
  case LteDirectDiscovery_DiscoveryType_INVALID:
    return qcril::interfaces::lte_direct::DiscoveryType::UNKNOWN;
  case LteDirectDiscovery_DiscoveryType_OPEN:
    return qcril::interfaces::lte_direct::DiscoveryType::OPEN;
  case LteDirectDiscovery_DiscoveryType_RESTRICTED:
    return qcril::interfaces::lte_direct::DiscoveryType::RESTRICTED;
  }
  return qcril::interfaces::lte_direct::DiscoveryType::UNKNOWN;
}
static LteDirectDiscovery_Result convertLteDirectResult(qcril::interfaces::lte_direct::Result in) {
  switch (in) {
  case qcril::interfaces::lte_direct::Result::SUCCESS:
    return LteDirectDiscovery_Result_SUCCESS;
  case qcril::interfaces::lte_direct::Result::GENERIC_FAILURE:
    return LteDirectDiscovery_Result_GENERIC_FAILURE;
  case qcril::interfaces::lte_direct::Result::IN_PROGRESS:
    return LteDirectDiscovery_Result_IN_PROGRESS;
  case qcril::interfaces::lte_direct::Result::INVALID_EXPRESSION_SCOPE:
    return LteDirectDiscovery_Result_INVALID_EXPRESSION_SCOPE;
  case qcril::interfaces::lte_direct::Result::UNKNOWN_EXPRESSION:
    return LteDirectDiscovery_Result_UNKNOWN_EXPRESSION;
  case qcril::interfaces::lte_direct::Result::INVALID_DISCOVERY_TYPE:
    return LteDirectDiscovery_Result_INVALID_DISCOVERY_TYPE;
  case qcril::interfaces::lte_direct::Result::SERVICE_NOT_AVAILABLE:
    return LteDirectDiscovery_Result_SERVICE_NOT_AVAILABLE;
  case qcril::interfaces::lte_direct::Result::APP_AUTH_FAILURE:
    return LteDirectDiscovery_Result_APP_AUTH_FAILURE;
  case qcril::interfaces::lte_direct::Result::NOT_SUPPORTED:
  case qcril::interfaces::lte_direct::Result::UNKNOWN:
    return LteDirectDiscovery_Result_NOT_SUPPORTED;
  }
  return LteDirectDiscovery_Result_NOT_SUPPORTED;
}

Return<void> OemHookImpl::lteDirectDiscoveryInitializeRequest(int32_t serial) {
  QCRIL_LOG_INFO("on slot %d, serial %d ", mInstanceId, serial);

  auto msg = std::make_shared<QcRilRequestLteDirectInitializeMessage>(getContext(serial));

  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          size_t packed_msg_len = 0;
          uint8_t send_buffer[8 * 1024] = {0};
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
              nullptr, 0, serial, LteDirectDiscovery_MsgType_RESPONSE,
              LteDirectDiscovery_MsgId_REQUEST_INITIALIZE, LteDirectDiscovery_Error_E_SUCCESS,
              send_buffer, sizeof(send_buffer));
          hidl_vec<uint8_t> respData;
          respData.setToExternal((uint8_t *)send_buffer, packed_msg_len,
                                 false); /* set false as local buffer */
          OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                                     (RadioError)errorCode, respData);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                               RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> OemHookImpl::lteDirectDiscoveryGetDeviceCapabilityRequest(int32_t serial) {
  QCRIL_LOG_INFO("on slot %d, serial %d ", mInstanceId, serial);
  auto msg = std::make_shared<QcRilRequestGetDeviceCapabilityMessage>(getContext(serial));
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          LteDirectDiscovery_DeviceCapability outData = {.has_capability = FALSE};
          size_t packed_msg_len = 0;
          uint8_t send_buffer[8 * 1024] = {0};
          void *respData = nullptr;
          size_t respDataLen = 0;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
            if (errorCode == RIL_E_SUCCESS) {
              auto rilData =
                  std::static_pointer_cast<qcril::interfaces::lte_direct::DeviceCapability>(
                      resp->data);
              if (rilData) {
                if (rilData->hasCapability()) {
                  outData.has_capability = TRUE;
                  outData.capability = rilData->getCapability();
                }
                respData = &outData;
                respDataLen = sizeof(outData);
              }
            }
          }
          packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
              respData, respDataLen, serial, LteDirectDiscovery_MsgType_RESPONSE,
              LteDirectDiscovery_MsgId_REQUEST_GET_DEVICE_CAPABILITY,
              LteDirectDiscovery_Error_E_SUCCESS, send_buffer, sizeof(send_buffer));
          hidl_vec<uint8_t> oemHookRespData;
          oemHookRespData.setToExternal((uint8_t *)send_buffer, packed_msg_len,
                                        false); /* set false as local buffer */
          OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                                     (RadioError)errorCode, oemHookRespData);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                               RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> OemHookImpl::lteDirectDiscoveryTerminateRequest(int32_t serial,
                                                             LteDirectDiscovery_Terminate *inData) {
  QCRIL_LOG_INFO("on slot %d, serial %d ", mInstanceId, serial);
  auto msg = std::make_shared<QcRilRequestTerminateMessage>(getContext(serial));
  if (msg && inData) {
    if (inData->osAppId.arg) {
      msg->setOsAppId((char *)inData->osAppId.arg);
    } else {
      QCRIL_LOG_ERROR("Invalid OsAppId");
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          size_t packed_msg_len = 0;
          uint8_t send_buffer[8 * 1024] = {0};
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
              nullptr, 0, serial, LteDirectDiscovery_MsgType_RESPONSE,
              LteDirectDiscovery_MsgId_REQUEST_TERMINATE, LteDirectDiscovery_Error_E_SUCCESS,
              send_buffer, sizeof(send_buffer));
          hidl_vec<uint8_t> oemHookRespData;
          oemHookRespData.setToExternal((uint8_t *)send_buffer, packed_msg_len,
                                        false); /* set false as local buffer */
          OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                                     (RadioError)errorCode, oemHookRespData);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                               RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> OemHookImpl::lteDirectDiscoveryGetServiceStatusRequest(int32_t serial) {
  QCRIL_LOG_INFO("on slot %d, serial %d ", mInstanceId, serial);
  auto msg = std::make_shared<QcRilRequestGetServiceStatusMessage>(getContext(serial));
  if (msg) {
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          LteDirectDiscovery_ServiceStatus outData = {.has_publishAllowed = FALSE,
                                                      .has_subscribeAllowed = FALSE};
          size_t packed_msg_len = 0;
          uint8_t send_buffer[8 * 1024] = {0};
          void *respData = nullptr;
          size_t respDataLen = 0;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
            if (errorCode == RIL_E_SUCCESS) {
              auto rilData = std::static_pointer_cast<qcril::interfaces::lte_direct::ServiceStatus>(
                  resp->data);
              if (rilData) {
                if (rilData->hasPublishAllowed()) {
                  outData.has_publishAllowed = TRUE;
                  outData.publishAllowed = rilData->getPublishAllowed();
                }
                if (rilData->hasSubscribeAllowed()) {
                  outData.has_subscribeAllowed = TRUE;
                  outData.subscribeAllowed = rilData->getSubscribeAllowed();
                }
                respData = &outData;
                respDataLen = sizeof(outData);
              }
            }
          }
          packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
              respData, respDataLen, serial, LteDirectDiscovery_MsgType_RESPONSE,
              LteDirectDiscovery_MsgId_REQUEST_GET_SERVICE_STATUS,
              LteDirectDiscovery_Error_E_SUCCESS, send_buffer, sizeof(send_buffer));
          hidl_vec<uint8_t> oemHookRespData;
          oemHookRespData.setToExternal((uint8_t *)send_buffer, packed_msg_len,
                                        false); /* set false as local buffer */
          OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                                     (RadioError)errorCode, oemHookRespData);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                               RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> OemHookImpl::lteDirectDiscoveryPublishRequest(int32_t serial,
                                                           LteDirectDiscovery_Publish *inData) {
  QCRIL_LOG_INFO("on slot %d, serial %d ", mInstanceId, serial);
  auto msg = std::make_shared<QcRilRequestPublishMessage>(getContext(serial));
  if (msg && inData) {
    if (inData->osAppId.arg) {
      msg->setOsAppId((char *)inData->osAppId.arg);
    }
    if (inData->expression.arg) {
      msg->setExpression((char *)inData->expression.arg);
    }
    if (inData->has_expressionValidityTime) {
      msg->setExpressionValidityTime(inData->expressionValidityTime);
    }
    if (inData->metaData.arg) {
      msg->setMetaData((char *)inData->metaData.arg);
    }
    if (inData->has_discoveryType) {
      msg->setDiscoveryType(convertDiscoveryType(inData->discoveryType));
    }
    if (inData->has_duration) {
      msg->setDuration(inData->duration);
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          size_t packed_msg_len = 0;
          uint8_t send_buffer[8 * 1024] = {0};
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
              nullptr, 0, serial, LteDirectDiscovery_MsgType_RESPONSE,
              LteDirectDiscovery_MsgId_REQUEST_PUBLISH, LteDirectDiscovery_Error_E_SUCCESS,
              send_buffer, sizeof(send_buffer));
          hidl_vec<uint8_t> oemHookRespData;
          oemHookRespData.setToExternal((uint8_t *)send_buffer, packed_msg_len,
                                        false); /* set false as local buffer */
          OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                                     (RadioError)errorCode, oemHookRespData);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                               RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void>
OemHookImpl::lteDirectDiscoveryCancelPublishRequest(int32_t serial,
                                                    LteDirectDiscovery_CancelPublish *inData) {
  QCRIL_LOG_INFO("on slot %d, serial %d ", mInstanceId, serial);
  auto msg = std::make_shared<QcRilRequestCancelPublishMessage>(getContext(serial));
  if (msg && inData) {
    if (inData->osAppId.arg) {
      msg->setOsAppId((char *)inData->osAppId.arg);
    }
    if (inData->expression.arg) {
      msg->setExpression((char *)inData->expression.arg);
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          size_t packed_msg_len = 0;
          uint8_t send_buffer[8 * 1024] = {0};
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
              nullptr, 0, serial, LteDirectDiscovery_MsgType_RESPONSE,
              LteDirectDiscovery_MsgId_REQUEST_CANCEL_PUBLISH, LteDirectDiscovery_Error_E_SUCCESS,
              send_buffer, sizeof(send_buffer));
          hidl_vec<uint8_t> oemHookRespData;
          oemHookRespData.setToExternal((uint8_t *)send_buffer, packed_msg_len,
                                        false); /* set false as local buffer */
          OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                                     (RadioError)errorCode, oemHookRespData);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                               RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> OemHookImpl::lteDirectDiscoverySubscribeRequest(int32_t serial,
                                                             LteDirectDiscovery_Subscribe *inData) {
  QCRIL_LOG_INFO("on slot %d, serial %d ", mInstanceId, serial);
  auto msg = std::make_shared<QcRilRequestSubscribeMessage>(getContext(serial));
  if (msg && inData) {
    if (inData->osAppId.arg) {
      msg->setOsAppId((char *)inData->osAppId.arg);
    }
    if (inData->expression.arg) {
      msg->setExpression((char *)inData->expression.arg);
    }
    if (inData->has_expressionValidityTime) {
      msg->setExpressionValidityTime(inData->expressionValidityTime);
    }
    if (inData->has_discoveryType) {
      msg->setDiscoveryType(convertDiscoveryType(inData->discoveryType));
    }
    if (inData->has_duration) {
      msg->setDuration(inData->duration);
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          size_t packed_msg_len = 0;
          uint8_t send_buffer[8 * 1024] = {0};
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
              nullptr, 0, serial, LteDirectDiscovery_MsgType_RESPONSE,
              LteDirectDiscovery_MsgId_REQUEST_SUBSCRIBE, LteDirectDiscovery_Error_E_SUCCESS,
              send_buffer, sizeof(send_buffer));
          hidl_vec<uint8_t> oemHookRespData;
          oemHookRespData.setToExternal((uint8_t *)send_buffer, packed_msg_len,
                                        false); /* set false as local buffer */
          OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                                     (RadioError)errorCode, oemHookRespData);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                               RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void>
OemHookImpl::lteDirectDiscoveryCancelSubscribeRequest(int32_t serial,
                                                      LteDirectDiscovery_CancelSubscribe *inData) {
  QCRIL_LOG_INFO("on slot %d, serial %d ", mInstanceId, serial);
  auto msg = std::make_shared<QcRilRequestCancelSubscribeMessage>(getContext(serial));
  if (msg && inData) {
    if (inData->osAppId.arg) {
      msg->setOsAppId((char *)inData->osAppId.arg);
    }
    if (inData->expression.arg) {
      msg->setExpression((char *)inData->expression.arg);
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        ([this, serial](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                        std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          size_t packed_msg_len = 0;
          uint8_t send_buffer[8 * 1024] = {0};
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
              nullptr, 0, serial, LteDirectDiscovery_MsgType_RESPONSE,
              LteDirectDiscovery_MsgId_REQUEST_CANCEL_SUBSCRIBE, LteDirectDiscovery_Error_E_SUCCESS,
              send_buffer, sizeof(send_buffer));
          hidl_vec<uint8_t> oemHookRespData;
          oemHookRespData.setToExternal((uint8_t *)send_buffer, packed_msg_len,
                                        false); /* set false as local buffer */
          OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                                     (RadioError)errorCode, oemHookRespData);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  } else {
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                               RadioError::NO_MEMORY, NULL);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> OemHookImpl::processLteDirectDiscRequest(int32_t serial, uint8_t *data,
                                                      uint32_t dataLen) {
  QCRIL_LOG_INFO("slot %d", mInstanceId);

  if ((NULL == data) || (1 > dataLen)) {
    QCRIL_LOG_ERROR("invalid parameter: data is empty: data = %p, dataLen = %d", data, dataLen);
    OEMHOOK_HIDL_SEND_RESPONSE(mInstanceId, getResponseCallback(), oemHookRawResponse, serial,
                               RadioError::INVALID_ARGUMENTS, NULL);
    return Void();
  }

  size_t unpacked_msg_size = 0;
  LteDirectDiscovery_MsgHeader *msg_tag_ptr = NULL;
  void *msg_data_ptr = NULL;

  QCRIL_LOG_INFO("unpacking Message tag");

  msg_tag_ptr = qcril_qmi_lte_direct_disc_unpack_msg_tag(data, dataLen);

  if (msg_tag_ptr) {
    QCRIL_LOG_INFO("msg: %s, type: %d, message_id: %d, error: %d",
                   qcril_qmi_lte_direct_disc_get_msg_log_str(msg_tag_ptr->id, msg_tag_ptr->type),
                   msg_tag_ptr->type, msg_tag_ptr->id, msg_tag_ptr->error);

    qcril_qmi_lte_direct_disc_parse_packed_msg(msg_tag_ptr->type, msg_tag_ptr->id,
                                               msg_tag_ptr->payload, dataLen, &msg_data_ptr,
                                               &unpacked_msg_size);

    switch (msg_tag_ptr->id) {
    case LteDirectDiscovery_MsgId_REQUEST_INITIALIZE:
      lteDirectDiscoveryInitializeRequest(serial);
      break;
    case LteDirectDiscovery_MsgId_REQUEST_GET_DEVICE_CAPABILITY:
      lteDirectDiscoveryGetDeviceCapabilityRequest(serial);
      break;
    case LteDirectDiscovery_MsgId_REQUEST_TERMINATE:
      lteDirectDiscoveryTerminateRequest(serial, (LteDirectDiscovery_Terminate *)msg_data_ptr);
      break;
    case LteDirectDiscovery_MsgId_REQUEST_GET_SERVICE_STATUS:
      lteDirectDiscoveryGetServiceStatusRequest(serial);
      break;
    case LteDirectDiscovery_MsgId_REQUEST_PUBLISH:
      lteDirectDiscoveryPublishRequest(serial, (LteDirectDiscovery_Publish *)msg_data_ptr);
      break;
    case LteDirectDiscovery_MsgId_REQUEST_CANCEL_PUBLISH:
      lteDirectDiscoveryCancelPublishRequest(serial,
                                             (LteDirectDiscovery_CancelPublish *)msg_data_ptr);
      break;
    case LteDirectDiscovery_MsgId_REQUEST_SUBSCRIBE:
      lteDirectDiscoverySubscribeRequest(serial, (LteDirectDiscovery_Subscribe *)msg_data_ptr);
      break;
    case LteDirectDiscovery_MsgId_REQUEST_CANCEL_SUBSCRIBE:
      lteDirectDiscoveryCancelSubscribeRequest(serial,
                                               (LteDirectDiscovery_CancelSubscribe *)msg_data_ptr);
      break;
    default:
      break;
    }
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

void OemHookImpl::sendLteDirectUnsolAuthorizationResult(
    std::shared_ptr<QcRilUnsolAuthorizationResultMessage> msg) {
  sp<IQtiOemHookIndication> indCb = getIndicationCallback();
  QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true");
  if (indCb && msg) {
    LteDirectDiscovery_AuthorizationResult outData = {};

    do {
      if (msg->hasOsAppId() && msg->getOsAppId().size()) {
        outData.osAppId.arg = (void *)strdup(msg->getOsAppId().c_str());
      } else {
        QCRIL_LOG_ERROR("OS App ID is not present");
        break;
      }
      if (msg->hasResult()) {
        outData.has_result = TRUE;
        outData.result = convertLteDirectResult(msg->getResult());
      } else {
        QCRIL_LOG_ERROR("Result is not present");
        break;
      }

      size_t packed_msg_len = 0;
      uint8_t send_buffer[8 * 1024] = {0};
      packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
          &outData, sizeof(outData),
          10, // TODO
          LteDirectDiscovery_MsgType_UNSOL_RESPONSE,
          LteDirectDiscovery_MsgId_UNSOL_RESPONSE_AUTHORIZATION_RESULT,
          LteDirectDiscovery_Error_E_SUCCESS, send_buffer, sizeof(send_buffer));

      uint32_t bufLen = 0;
      uint8_t *buf = constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC,
                                         (uint8_t *)send_buffer, packed_msg_len);

      hidl_vec<uint8_t> data;
      data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
      Return<void> ret = indCb->oemHookRawIndication(data);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
      }
    } while (FALSE);

    if (outData.osAppId.arg) {
      free(outData.osAppId.arg);
    }
  }
}
void OemHookImpl::sendLteDirectUnsolDeviceCapabilityChanged(
    std::shared_ptr<QcRilUnsolDeviceCapabilityChangedMessage> msg) {
  sp<IQtiOemHookIndication> indCb = getIndicationCallback();
  QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true");
  if (indCb && msg) {
    LteDirectDiscovery_DeviceCapability outData = {};

    do {
      if (msg->hasCapability()) {
        outData.has_capability = TRUE;
        outData.capability = msg->getCapability();
      } else {
        QCRIL_LOG_ERROR("Capability is not present");
        break;
      }

      size_t packed_msg_len = 0;
      uint8_t send_buffer[8 * 1024] = {0};
      packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
          &outData, sizeof(outData),
          10, // TODO
          LteDirectDiscovery_MsgType_UNSOL_RESPONSE,
          LteDirectDiscovery_MsgId_UNSOL_RESPONSE_DEVICE_CAPABILITY_CHANGED,
          LteDirectDiscovery_Error_E_SUCCESS, send_buffer, sizeof(send_buffer));

      uint32_t bufLen = 0;
      uint8_t *buf = constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC,
                                         (uint8_t *)send_buffer, packed_msg_len);

      hidl_vec<uint8_t> data;
      data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
      Return<void> ret = indCb->oemHookRawIndication(data);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
      }
    } while (FALSE);
  }
}

void OemHookImpl::sendLteDirectUnsolExpressionStatus(
    std::shared_ptr<QcRilUnsolExpressionStatusMessage> msg) {
  sp<IQtiOemHookIndication> indCb = getIndicationCallback();
  QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true");
  if (indCb && msg) {
    LteDirectDiscovery_ExpressionStatus outData = {};

    do {
      if (msg->hasOsAppId() && msg->getOsAppId().size()) {
        outData.osAppId.arg = (void *)strdup(msg->getOsAppId().c_str());
      } else {
        QCRIL_LOG_ERROR("OS App ID is not present");
        break;
      }
      if (msg->hasExpression() && msg->getExpression().size()) {
        outData.expression.arg = (void *)strdup(msg->getExpression().c_str());
      } else {
        QCRIL_LOG_ERROR("OS App ID is not present");
        break;
      }
      if (msg->hasResult()) {
        outData.has_result = TRUE;
        outData.result = convertLteDirectResult(msg->getResult());
      } else {
        QCRIL_LOG_ERROR("Result is not present");
        break;
      }

      size_t packed_msg_len = 0;
      uint8_t send_buffer[8 * 1024] = {0};
      packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
          &outData, sizeof(outData),
          10, // TODO
          LteDirectDiscovery_MsgType_UNSOL_RESPONSE,
          LteDirectDiscovery_MsgId_UNSOL_RESPONSE_EXPRESSION_STATUS,
          LteDirectDiscovery_Error_E_SUCCESS, send_buffer, sizeof(send_buffer));

      uint32_t bufLen = 0;
      uint8_t *buf = constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC,
                                         (uint8_t *)send_buffer, packed_msg_len);

      hidl_vec<uint8_t> data;
      data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
      Return<void> ret = indCb->oemHookRawIndication(data);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
      }
    } while (FALSE);
    if (outData.osAppId.arg) {
      free(outData.osAppId.arg);
    }
    if (outData.expression.arg) {
      free(outData.expression.arg);
    }
  }
}
void OemHookImpl::sendLteDirectUnsolMatchEvent(std::shared_ptr<QcRilUnsolMatchEventMessage> msg) {
  sp<IQtiOemHookIndication> indCb = getIndicationCallback();
  QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true");
  if (indCb && msg) {
    LteDirectDiscovery_MatchEvent outData = {};

    do {
      if (msg->hasOsAppId() && msg->getOsAppId().size()) {
        outData.osAppId.arg = (void *)strdup(msg->getOsAppId().c_str());
      } else {
        QCRIL_LOG_ERROR("OS App ID is not present");
        break;
      }
      if (msg->hasExpression() && msg->getExpression().size()) {
        outData.expression.arg = (void *)strdup(msg->getExpression().c_str());
      } else {
        QCRIL_LOG_ERROR("Expression is not present");
        break;
      }
      if (msg->hasMatchedExpression() && msg->getMatchedExpression().size()) {
        outData.matchedExpression.arg = (void *)strdup(msg->getMatchedExpression().c_str());
      } else {
        QCRIL_LOG_ERROR("MatchedExpression is not present");
        break;
      }
      if (msg->hasState()) {
        outData.has_state = TRUE;
        outData.state = msg->getState();
      } else {
        QCRIL_LOG_ERROR("state is not present");
        break;
      }
      if (msg->hasMetaDataIndex()) {
        outData.has_metaDataIndex = TRUE;
        outData.metaDataIndex = msg->getMetaDataIndex();
      } else {
        QCRIL_LOG_ERROR("MetaDataIndex is not present");
        break;
      }
      if (msg->hasMetaData() && msg->getMetaData().size()) {
        outData.metaData.arg = (void *)strdup(msg->getMetaData().c_str());
      } else {
        QCRIL_LOG_ERROR("MetaData is not present");
        break;
      }

      size_t packed_msg_len = 0;
      uint8_t send_buffer[8 * 1024] = {0};
      packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
          &outData, sizeof(outData),
          10, // TODO
          LteDirectDiscovery_MsgType_UNSOL_RESPONSE,
          LteDirectDiscovery_MsgId_UNSOL_RESPONSE_MATCH_EVENT, LteDirectDiscovery_Error_E_SUCCESS,
          send_buffer, sizeof(send_buffer));

      uint32_t bufLen = 0;
      uint8_t *buf = constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC,
                                         (uint8_t *)send_buffer, packed_msg_len);

      hidl_vec<uint8_t> data;
      data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
      Return<void> ret = indCb->oemHookRawIndication(data);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
      }
    } while (FALSE);

    if (outData.osAppId.arg) {
      free(outData.osAppId.arg);
    }
    if (outData.expression.arg) {
      free(outData.expression.arg);
    }
    if (outData.matchedExpression.arg) {
      free(outData.matchedExpression.arg);
    }
    if (outData.metaData.arg) {
      free(outData.metaData.arg);
    }
  }
}

void OemHookImpl::sendLteDirectUnsolPskExpirted(
    std::shared_ptr<QcRilUnsolPskExpirtedMessage> /*msg*/) {
  QCRIL_LOG_INFO("dummy implementation");
}


void OemHookImpl::sendLteDirectUnsolTransmissionStatus(
    std::shared_ptr<QcRilUnsolTransmissionStatusMessage> msg) {
  sp<IQtiOemHookIndication> indCb = getIndicationCallback();
  QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true");
  if (indCb && msg) {
    LteDirectDiscovery_TransmissionStatus outData = {};

    do {
      if (msg->hasOsAppId() && msg->getOsAppId().size()) {
        outData.osAppId.arg = (void *)strdup(msg->getOsAppId().c_str());
      } else {
        QCRIL_LOG_ERROR("OS App ID is not present");
        break;
      }
      if (msg->hasExpression() && msg->getExpression().size()) {
        outData.expression.arg = (void *)strdup(msg->getExpression().c_str());
      } else {
        QCRIL_LOG_ERROR("Expression is not present");
        break;
      }
      if (msg->hasStatus()) {
        outData.has_status = TRUE;
        outData.status = msg->getStatus();
      } else {
        QCRIL_LOG_ERROR("Status is not present");
        break;
      }

      size_t packed_msg_len = 0;
      uint8_t send_buffer[8 * 1024] = {0};
      packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
          &outData, sizeof(outData),
          10, // TODO
          LteDirectDiscovery_MsgType_UNSOL_RESPONSE,
          LteDirectDiscovery_MsgId_UNSOL_RESPONSE_TRANSMISSION_STATUS,
          LteDirectDiscovery_Error_E_SUCCESS, send_buffer, sizeof(send_buffer));

      uint32_t bufLen = 0;
      uint8_t *buf = constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC,
                                         (uint8_t *)send_buffer, packed_msg_len);

      hidl_vec<uint8_t> data;
      data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
      Return<void> ret = indCb->oemHookRawIndication(data);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
      }
    } while (FALSE);

    if (outData.osAppId.arg) {
      free(outData.osAppId.arg);
    }
    if (outData.expression.arg) {
      free(outData.expression.arg);
    }
  }
}
void OemHookImpl::sendLteDirectUnsolReceptionStatus(
    std::shared_ptr<QcRilUnsolReceptionStatusMessage> msg) {
  sp<IQtiOemHookIndication> indCb = getIndicationCallback();
  QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true");
  if (indCb && msg) {
    LteDirectDiscovery_ReceptionStatus outData = {};

    do {
      if (msg->hasOsAppId() && msg->getOsAppId().size()) {
        outData.osAppId.arg = (void *)strdup(msg->getOsAppId().c_str());
      } else {
        QCRIL_LOG_ERROR("OS App ID is not present");
        break;
      }
      if (msg->hasExpression() && msg->getExpression().size()) {
        outData.expression.arg = (void *)strdup(msg->getExpression().c_str());
      } else {
        QCRIL_LOG_ERROR("Expression is not present");
        break;
      }
      if (msg->hasStatus()) {
        outData.has_status = TRUE;
        outData.status = msg->getStatus();
      } else {
        QCRIL_LOG_ERROR("Status is not present");
        break;
      }

      size_t packed_msg_len = 0;
      uint8_t send_buffer[8 * 1024] = {0};
      packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
          &outData, sizeof(outData),
          10, // TODO
          LteDirectDiscovery_MsgType_UNSOL_RESPONSE,
          LteDirectDiscovery_MsgId_UNSOL_RESPONSE_RECEPTION_STATUS,
          LteDirectDiscovery_Error_E_SUCCESS, send_buffer, sizeof(send_buffer));

      uint32_t bufLen = 0;
      uint8_t *buf = constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC,
                                         (uint8_t *)send_buffer, packed_msg_len);

      hidl_vec<uint8_t> data;
      data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
      Return<void> ret = indCb->oemHookRawIndication(data);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
      }
    } while (FALSE);

    if (outData.osAppId.arg) {
      free(outData.osAppId.arg);
    }
    if (outData.expression.arg) {
      free(outData.expression.arg);
    }
  }
}
void OemHookImpl::sendLteDirectUnsolServiceStatus(
    std::shared_ptr<QcRilUnsolServiceStatusMessage> msg) {
  sp<IQtiOemHookIndication> indCb = getIndicationCallback();
  QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true");
  if (indCb && msg) {
    LteDirectDiscovery_ServiceStatus outData = {};

    do {
      if (msg->hasPublishAllowed()) {
        outData.has_publishAllowed = TRUE;
        outData.publishAllowed = msg->getPublishAllowed();
      } else {
        QCRIL_LOG_ERROR("PublishAllowed is not present");
        break;
      }
      if (msg->hasSubscribeAllowed()) {
        outData.has_subscribeAllowed = TRUE;
        outData.subscribeAllowed = msg->getSubscribeAllowed();
      } else {
        QCRIL_LOG_ERROR("SubscribeAllowed is not present");
        break;
      }

      size_t packed_msg_len = 0;
      uint8_t send_buffer[8 * 1024] = {0};
      packed_msg_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
          &outData, sizeof(outData),
          10, // TODO
          LteDirectDiscovery_MsgType_UNSOL_RESPONSE,
          LteDirectDiscovery_MsgId_UNSOL_RESPONSE_SERVICE_STATUS,
          LteDirectDiscovery_Error_E_SUCCESS, send_buffer, sizeof(send_buffer));

      uint32_t bufLen = 0;
      uint8_t *buf = constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC,
                                         (uint8_t *)send_buffer, packed_msg_len);

      hidl_vec<uint8_t> data;
      data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
      Return<void> ret = indCb->oemHookRawIndication(data);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
      }
    } while (FALSE);
  }
}
#endif

uint8_t* constructOemHookRaw
(
  uint32_t* length,
  int unsol_event,
  uint8_t* data,
  uint32_t data_len
)
{
    size_t payload_len = (QCRIL_QMI_OEM_INT_VAL_LEN + QCRIL_OTHER_OEM_NAME_LENGTH +
                          sizeof(unsol_event) + sizeof(data_len) + data_len);
    uint8_t *payload = new uint8_t[payload_len];
    uint32_t index = 0;
    if ( NULL != payload )
    {
        /* Unsolicited oemhook message has following message format
          [RIL_UNSOL_OEM_HOOK_RAW (4)]
          [OEM_NAME(8)] [Message Id (4) ] [Payload Length (4) ] [Payload]
        */
        int unsol_hook_raw = RIL_UNSOL_OEM_HOOK_RAW;
        memcpy( payload, &(unsol_hook_raw), QCRIL_QMI_OEM_INT_VAL_LEN );
        index += QCRIL_QMI_OEM_INT_VAL_LEN;

        memcpy( &payload[index], QCRIL_HOOK_OEM_NAME, QCRIL_OTHER_OEM_NAME_LENGTH );
        index += QCRIL_OTHER_OEM_NAME_LENGTH;

        memcpy( &payload[index], &unsol_event, sizeof(unsol_event) );
        index += sizeof(unsol_event);

        memcpy( &payload[index], &data_len, sizeof(data_len) );
        index += sizeof(data_len);

        if( data!= NULL && data_len > 0 ) {
            memcpy( &payload[index], data, data_len );
        }
        index += data_len;
    }

    *length = index;
    return payload;
}

uint8_t* convertAdnRecords(uint32_t* length, const qcril::interfaces::AdnRecords* adn_record_ptr)
{
    uint8_t* hook_response;
    uint16_t element_index, response_index = 0;
    uint16_t name_length, number_length, email_length, anr_length;
    uint16_t email_elements, email_index, anr_elements, anr_index;
    const uint32_t INT_SIZE = 2;
    uint32_t response_length = 0;

    response_length = INT_SIZE;  //record_elements
    for(element_index = 0; element_index < adn_record_ptr->record_elements; element_index++)
    {
        const qcril::interfaces::AdnRecordInfo* adn_resp_ptr =
                    & adn_record_ptr->adn_record_info[element_index];

        response_length += INT_SIZE;                      //record_id
        response_length += INT_SIZE;                      //name_length
        if(!adn_resp_ptr->name.empty())
        {
            response_length += strlen(adn_resp_ptr->name.c_str());    //name
        }
        response_length += INT_SIZE;                      //number_length
        if(!adn_resp_ptr->number.empty())
        {
            response_length += strlen(adn_resp_ptr->number.c_str());  //number
        }
        response_length += INT_SIZE;                      //email_elements
        for(email_index = 0; email_index < adn_resp_ptr->email_elements; email_index++)
        {
            response_length += INT_SIZE;  //email_length
            response_length += strlen(adn_resp_ptr->email[email_index].c_str()); //email
        }
        response_length += INT_SIZE;                      //anr_elements
        for(anr_index = 0; anr_index < adn_resp_ptr->anr_elements; anr_index++)
        {
            response_length += INT_SIZE;                  //anr_length
            response_length += strlen(adn_resp_ptr->ad_number[anr_index].c_str()); //anr
        }
        response_length ++;
    }

    QCRIL_LOG_INFO("Response length is: %d",response_length);

    hook_response = (uint8_t *)qcril_malloc(response_length);
    if ( !hook_response )
    {
        QCRIL_LOG_ERROR("Failed to alloc memory for oem hook payload");
        return NULL;
    }

    //record_elements
    response_index = 0;
    hook_response[response_index++] = adn_record_ptr->record_elements & 0x00FF;
    hook_response[response_index++] = (adn_record_ptr->record_elements & 0xFF00) >> 8;

    for(element_index = 0; element_index < adn_record_ptr->record_elements; element_index++)
    {
        const qcril::interfaces::AdnRecordInfo* adn_resp_ptr =
                    & adn_record_ptr->adn_record_info[element_index];

        //record_id
        hook_response[response_index++] = adn_resp_ptr->record_id & 0x00FF;
        hook_response[response_index++] = (adn_resp_ptr->record_id & 0xFF00) >> 8;

        //name_length
        if(!adn_resp_ptr->name.empty())
        {
            name_length = strlen(adn_resp_ptr->name.c_str());
        }
        else
        {
            name_length = 0;
        }
        hook_response[response_index++] = name_length & 0x00FF;
        hook_response[response_index++] = (name_length & 0xFF00) >> 8;

        //name
        if(!adn_resp_ptr->name.empty()) {
            memcpy(hook_response + response_index, adn_resp_ptr->name.c_str(), name_length);
        }
        response_index += name_length;

        //number_length
        if(!adn_resp_ptr->number.empty())
        {
            number_length = strlen(adn_resp_ptr->number.c_str());
        }
        else
        {
            number_length = 0;
        }
        hook_response[response_index++] = number_length & 0x00FF;
        hook_response[response_index++] = (number_length & 0xFF00) >> 8;

        //number
        if(!adn_resp_ptr->number.empty()) {
            memcpy(hook_response + response_index, adn_resp_ptr->number.c_str(), number_length);
        }
        response_index += number_length;

        //email_elements
        email_elements = adn_resp_ptr->email_elements;
        hook_response[response_index++] = email_elements & 0x00FF;
        hook_response[response_index++] = (email_elements & 0xFF00) >> 8;

        for(email_index = 0; email_index < email_elements; email_index++)
        {
            //email_length
            email_length = strlen(adn_resp_ptr->email[email_index].c_str());
            hook_response[response_index++] = email_length & 0x00FF;
            hook_response[response_index++] = (email_length & 0xFF00) >> 8;
            //email
            memcpy(hook_response + response_index,
                        adn_resp_ptr->email[email_index].c_str(), email_length);
            response_index += email_length;
        }

        //anr_elements
        anr_elements = adn_resp_ptr->anr_elements;
        hook_response[response_index++] = anr_elements & 0x00FF;
        hook_response[response_index++] = (anr_elements & 0xFF00) >> 8;

        for(anr_index = 0; anr_index < anr_elements; anr_index++)
        {
            //anr_length
            anr_length = strlen(adn_resp_ptr->ad_number[anr_index].c_str());
            hook_response[response_index++] = anr_length & 0x00FF;
            hook_response[response_index++] = (anr_length & 0xFF00) >> 8;
            //anr
            memcpy(hook_response + response_index,
                        adn_resp_ptr->ad_number[anr_index].c_str(), anr_length);
            response_index += anr_length;
        }
    }
    hook_response[response_index] = '\0';

    *length = response_length;
    return hook_response;
}

/*
 * Sends oemHook QCRIL_EVT_HOOK_UNSOL_ADN_RECORDS_IND to telephony via callback
 */
void OemHookImpl::sendAdnRecords(std::shared_ptr<QcRilUnsolAdnRecordsOnSimMessage> msg) {
    sp<IQtiOemHookIndication> indCb = getIndicationCallback();
    QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true" );
    if (indCb) {
        uint32_t recordsLen = 0;
        uint8_t* records = convertAdnRecords(&recordsLen, msg->getAdnRecords());

        uint32_t bufLen = 0;
        uint8_t* buf = constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_ADN_RECORDS_IND, records, recordsLen);

        hidl_vec<uint8_t> data;
        data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
        Return<void> ret = indCb->oemHookRawIndication(data);
        qcril_free(records);
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    }
}

/*
 * Sends oemHook QCRIL_EVT_HOOK_UNSOL_ADN_INIT_DONE to telephony via callback
 */
void OemHookImpl::sendAdnInitDone(std::shared_ptr<QcRilUnsolAdnInitDoneMessage> msg) {
    sp<IQtiOemHookIndication> indCb = getIndicationCallback();
    QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true" );
    if (indCb) {
        QCRIL_NOTUSED( msg );

        uint32_t bufLen = 0;
        uint8_t* buf = constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_ADN_INIT_DONE, NULL, 0);

        hidl_vec<uint8_t> data;
        data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
        Return<void> ret = indCb->oemHookRawIndication(data);
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    }
}

/*
 * Sends oemHook QCRIL_EVT_HOOK_UNSOL_PDC_CONFIG to telephony via callback
 */
void OemHookImpl::sendMbnConfigResult(std::shared_ptr<QcRilUnsolMbnConfigResultMessage> msg) {
    sp<IQtiOemHookIndication> indCb = getIndicationCallback();
    QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true" );
    if (indCb) {
        char result = msg->getResult();
        uint32_t bufLen = 0;
        uint8_t* buf = constructOemHookRaw(
                &bufLen, QCRIL_REQ_HOOK_UNSOL_PDC_CONFIG, (uint8_t*)&result, sizeof(result));

        hidl_vec<uint8_t> data;
        data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
        Return<void> ret = indCb->oemHookRawIndication(data);
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    }
}

/*
 * Sends oemHook QCRIL_EVT_HOOK_UNSOL_PDC_CLEAR_CONFIGS to telephony via callback
 */
void OemHookImpl::sendMbnConfigCleared(std::shared_ptr<QcRilUnsolMbnConfigClearedMessage> msg) {
    sp<IQtiOemHookIndication> indCb = getIndicationCallback();
    QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true" );
    if (indCb) {
        char result = msg->getResult();
        uint32_t bufLen = 0;
        uint8_t* buf = constructOemHookRaw(
                &bufLen, QCRIL_REQ_HOOK_UNSOL_PDC_CLEAR_CONFIGS, (uint8_t*)&result, sizeof(result));

        hidl_vec<uint8_t> data;
        data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
        Return<void> ret = indCb->oemHookRawIndication(data);
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    }
}

/*
 * Sends oemHook QCRIL_EVT_HOOK_UNSOL_PDC_VALIDATE_DUMPED to telephony via callback
 */
void OemHookImpl::sendMbnValidateDumped(std::shared_ptr<QcRilUnsolMbnValidateDumpedMessage> msg) {
    sp<IQtiOemHookIndication> indCb = getIndicationCallback();
    QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true" );
    if (indCb) {
        std::string dumpFile = msg->getDumpFile();
        uint32_t bufLen = 0;
        uint8_t* buf = constructOemHookRaw(
                &bufLen, QCRIL_REQ_HOOK_UNSOL_PDC_VALIDATE_DUMPED,
                (uint8_t*)dumpFile.c_str(), dumpFile.length());

        hidl_vec<uint8_t> data;
        data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
        Return<void> ret = indCb->oemHookRawIndication(data);
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    }
}

/*
 * Sends oemHook QCRIL_EVT_HOOK_UNSOL_PDC_LIST_CONFIG to telephony via callback
 */
void OemHookImpl::sendMbnConfigList(std::shared_ptr<QcRilUnsolMbnConfigListMessage> msg) {
    sp<IQtiOemHookIndication> indCb = getIndicationCallback();
    QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true" );
    if (indCb) {
        uint32_t bufLen = 0;
        uint8_t* buf = constructOemHookRaw(
            &bufLen, QCRIL_REQ_HOOK_UNSOL_PDC_LIST_CONFIG,
            (uint8_t*)msg->getResult().data(), msg->getResult().size());

        hidl_vec<uint8_t> data;
        data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
        Return<void> ret = indCb->oemHookRawIndication(data);
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    }
}

/*
 * Sends oemHook QCRIL_EVT_HOOK_UNSOL_PDC_VALIDATE_CONFIGS to telephony via callback
 */
void OemHookImpl::sendMbnValidateConfig(std::shared_ptr<QcRilUnsolMbnValidateConfigMessage> msg) {
    sp<IQtiOemHookIndication> indCb = getIndicationCallback();
    QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true" );
    if (indCb) {
        uint32_t bufLen = 0;
        uint8_t* buf = constructOemHookRaw(
                &bufLen, QCRIL_REQ_HOOK_UNSOL_PDC_VALIDATE_CONFIGS,
                (uint8_t*)msg->getMbnDiff().data(), msg->getMbnDiff().size());

        hidl_vec<uint8_t> data;
        data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
        Return<void> ret = indCb->oemHookRawIndication(data);
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_SIMLOCK_TEMP_UNLOCK_EXPIRED to telephony via callback
 */
void OemHookImpl::uimSimlockTempUnlockExpireInd(std::shared_ptr<UimSimlockTempUnlockExpireInd> msg) {
    sp<IQtiOemHookIndication> indCb = getIndicationCallback();
    QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true" );
    if (indCb) {
        QCRIL_NOTUSED( msg );

        uint32_t bufLen = 0;
        uint8_t* buf = constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_SIMLOCK_TEMP_UNLOCK_EXPIRED, NULL, 0);

        hidl_vec<uint8_t> data;
        data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
        Return<void> ret = indCb->oemHookRawIndication(data);
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_CARD_STATE_CHANGE_IND to telephony via callback
 */
void OemHookImpl::uimCardStateChangeInd(std::shared_ptr<UimCardStateChangeInd> msg) {
    sp<IQtiOemHookIndication> indCb = getIndicationCallback();
    QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true" );
    if (indCb && msg) {
        RIL_SlotCardState card_state = {};
        uint32_t          bufLen     = 0;
        hidl_vec<uint8_t> data       = {};
        uint8_t*          buf        = NULL;

        card_state.slot_id = (uint32_t)msg->get_slot();
        card_state.card_state = (RIL_CardState)msg->get_card_state();

        buf = constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_CARD_STATE_CHANGE_IND, (uint8_t *)&card_state, sizeof(card_state));

        data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
        Return<void> ret = indCb->oemHookRawIndication(data);
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_SLOT_STATUS_CHANGE_IND to telephony via callback
 */
void OemHookImpl::uimSlotStatusInd(std::shared_ptr<UimSlotStatusInd> msg) {
    sp<IQtiOemHookIndication> indCb = getIndicationCallback();
    QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true" );
    if (indCb && msg) {
        RIL_SlotsStatus                 status = {};
        uint32_t                        bufLen = 0;
        hidl_vec<uint8_t>               data   = {};
        uint8_t*                        buf    = NULL;
        std::vector<RIL_UIM_SlotStatus> ind    = msg->get_status();

        if (ind.size() <= UIM_OEM_HOOK_MAX_CARD_COUNT)
        {
          status.no_of_slots = ind.size();

          for (uint8_t i = 0; i < status.no_of_slots && i < UIM_OEM_HOOK_MAX_CARD_COUNT; i++)
          {
            status.slot_status[i].card_state = (Ril_CardPresenceState)ind[i].card_state;
            status.slot_status[i].slot_state = (Ril_SlotState)ind[i].slot_state;
            status.slot_status[i].logical_slot = (uint32_t)ind[i].logical_slot;

            if (ind[i].iccid.size() <= UIM_OEM_HOOK_MAX_ICCID_LEN)
            {
              status.slot_status[i].iccid_len = ind[i].iccid.size();
              memcpy(status.slot_status[i].iccid, ind[i].iccid.data(), status.slot_status[i].iccid_len);
            }
          }
        }

        buf = constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_SLOT_STATUS_CHANGE_IND, (uint8_t *)&status, sizeof(status));

        data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
        Return<void> ret = indCb->oemHookRawIndication(data);
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_SIM_REFRESH to telephony via callback
 */
void OemHookImpl::uimSimRefreshIndication(std::shared_ptr<UimSimRefreshIndication> msg) {
    sp<IQtiOemHookIndication> indCb = getIndicationCallback();
    QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true" );
    if (indCb && msg) {
        RIL_Hook_SimRefreshResponse    refresh = {};
        uint32_t                       bufLen  = 0;
        hidl_vec<uint8_t>              data    = {};
        uint8_t*                       buf     = NULL;
        RIL_UIM_SIM_RefreshIndication  ind     = msg->get_refresh_ind();

        refresh.result = (RIL_SimRefreshResult)ind.result;
        refresh.ef_id = ind.ef_id;
        refresh.app_type = (RIL_AppType)ind.app_type;

        if (ind.aid.length() <= (UIM_OEM_HOOK_MAX_AID_SIZE * 2))
        {
          refresh.aid_len = ind.aid.length();
          ind.aid.copy(refresh.aid, refresh.aid_len);
        }

        buf = constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_SIM_REFRESH, (uint8_t *)&refresh, sizeof(refresh));

        data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
        Return<void> ret = indCb->oemHookRawIndication(data);
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_UICC_VOLTAGE_STATUS to telephony via callback
 */
void OemHookImpl::uimVoltageStatusInd(std::shared_ptr<UimVoltageStatusInd> msg) {
    sp<IQtiOemHookIndication> indCb = getIndicationCallback();
    QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true" );
    if (indCb) {
        RIL_UiccVoltageStatus   status  = (RIL_UiccVoltageStatus)msg->get_status();
        uint32_t                bufLen  = 0;
        hidl_vec<uint8_t>       data    = {};
        uint8_t*                buf     = NULL;

        buf = constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_UICC_VOLTAGE_STATUS, (uint8_t *)status, sizeof(status));

        data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
        Return<void> ret = indCb->oemHookRawIndication(data);
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_REMOTE_SIM_STATUS_IND to telephony via callback
 */
void OemHookImpl::uimRmtRemoteSimStatusIndMsg(std::shared_ptr<UimRmtRemoteSimStatusIndMsg> msg) {
    sp<IQtiOemHookIndication> indCb = getIndicationCallback();
    QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true" );
    if (indCb) {
        RIL_HookRemoteSimInfo          status  = {};
        uint32_t                       bufLen  = 0;
        hidl_vec<uint8_t>              data    = {};
        uint8_t*                       buf     = NULL;
        RIL_UIM_RMT_RemoteSIMInfo      ind     = msg->get_status();

        status.isEnable = ind.isEnable;
        status.transportType = (RIL_TransportType)ind.transportType;
        status.usage = (RIL_Usage)ind.usage;
        status.remoteEvent = (RIL_remoteEvent)ind.remoteEvent;

        buf = constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_REMOTE_SIM_STATUS_IND, (uint8_t *)&status, sizeof(status));

        data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
        Return<void> ret = indCb->oemHookRawIndication(data);
        if (!ret.isOk()) {
            QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
        }
    }
}

/*=========================================================================
  HELPER FUNCTION: on_length_enum_to_str
===========================================================================*/
void on_length_enum_to_str(qcril::interfaces::DtmfOnLength on_enum, char *str, int len) {
  if (len >= 4) {
    switch (on_enum) {
      case qcril::interfaces::DtmfOnLength::ONLENGTH_95MS:
        strlcpy(str, "95", len);
        break;
      case qcril::interfaces::DtmfOnLength::ONLENGTH_150MS:
        strlcpy(str, "150", len);
        break;
      case qcril::interfaces::DtmfOnLength::ONLENGTH_200MS:
        strlcpy(str, "200", len);
        break;
      case qcril::interfaces::DtmfOnLength::ONLENGTH_250MS:
        strlcpy(str, "250", len);
        break;
      case qcril::interfaces::DtmfOnLength::ONLENGTH_300MS:
        strlcpy(str, "300", len);
        break;
      case qcril::interfaces::DtmfOnLength::ONLENGTH_350MS:
        strlcpy(str, "350", len);
        break;
      case qcril::interfaces::DtmfOnLength::ONLENGTH_SMS:
        strlcpy(str, "SMS", len);
        break;
      default:
        break;
    }
  }
}

/*=========================================================================
  HELPER FUNCTION: off_length_enum_to_str
===========================================================================*/
void off_length_enum_to_str(qcril::interfaces::DtmfOffLength off_enum, char *str, int len) {
  if (len >= 4) {
    switch (off_enum) {
      case qcril::interfaces::DtmfOffLength::OFFLENGTH_60MS:
        strlcpy(str, "60", len);
        break;
      case qcril::interfaces::DtmfOffLength::OFFLENGTH_100MS:
        strlcpy(str, "100", len);
        break;
      case qcril::interfaces::DtmfOffLength::OFFLENGTH_150MS:
        strlcpy(str, "150", len);
        break;
      case qcril::interfaces::DtmfOffLength::OFFLENGTH_200MS:
        strlcpy(str, "200", len);
        break;
      default:
        break;
    }
  }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_CDMA_BURST_DTMF to telephony via callback
 */
void OemHookImpl::sendUnsolCdmaBurstDtmf(std::shared_ptr<QcRilUnsolDtmfMessage> msg) {
  sp<IQtiOemHookIndication> indCb = getIndicationCallback();
  QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true");
  if (indCb && msg) {
    char payload[QCRIL_QMI_VOICE_DTMF_FWD_BURST_PAYLOAD_LENGTH];
    char on_length[4];
    char off_length[4];
    int digit_buf_len = 0;

    memset(payload, 0, sizeof(payload));
    memset(on_length, 0, sizeof(on_length));
    memset(off_length, 0, sizeof(off_length));

    if (msg->hasOnLength()) {
      on_length_enum_to_str(msg->getOnLength(), on_length, sizeof(on_length));
    }
    if (msg->hasOffLength()) {
      off_length_enum_to_str(msg->getOffLength(), off_length, sizeof(off_length));
    }

    if (!msg->getDigitBuffer().empty()) {
      digit_buf_len = std::min(msg->getDigitBuffer().size(),
                               (sizeof(payload) - (sizeof(on_length) + sizeof(off_length))));
    }
    memcpy(payload, on_length, sizeof(on_length));
    memcpy(payload + sizeof(on_length), off_length, sizeof(off_length));
    if (digit_buf_len) {
      memcpy(payload + sizeof(on_length) + sizeof(off_length),
             (void *)msg->getDigitBuffer().c_str(), digit_buf_len);
    }

    uint32_t bufLen = 0;
    uint8_t *buf = constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_CDMA_BURST_DTMF,
                                       (uint8_t *)payload, sizeof(payload));

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    Return<void> ret = indCb->oemHookRawIndication(data);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
    }
  }
}
/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_CDMA_CONT_DTMF_START to telephony via callback
 */
void OemHookImpl::sendUnsolCdmaContDtmfStart(std::shared_ptr<QcRilUnsolDtmfMessage> msg) {
  sp<IQtiOemHookIndication> indCb = getIndicationCallback();
  QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true");
  if (indCb && msg) {
    char payload = 0;
    if (!msg->getDigitBuffer().empty()) {
      payload = msg->getDigitBuffer()[0];
    }

    uint32_t bufLen = 0;
    uint8_t *buf = constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_CDMA_CONT_DTMF_START,
                                       (uint8_t *)&payload, sizeof(payload));

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    Return<void> ret = indCb->oemHookRawIndication(data);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
    }
  }
}
/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_CDMA_CONT_DTMF_STOP to telephony via callback
 */
void OemHookImpl::sendUnsolCdmaContDtmfStop(std::shared_ptr<QcRilUnsolDtmfMessage> msg) {
  sp<IQtiOemHookIndication> indCb = getIndicationCallback();
  QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true");
  if (indCb && msg) {
    uint32_t bufLen = 0;
    uint8_t *buf = constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_CDMA_CONT_DTMF_STOP, NULL, 0);

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    Return<void> ret = indCb->oemHookRawIndication(data);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
    }
  }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_EXTENDED_DBM_INTL to telephony via callback
 */
void OemHookImpl::sendUnsolExtendedDbmIntl(std::shared_ptr<QcRilUnsolExtBurstIntlMessage> msg) {
  sp<IQtiOemHookIndication> indCb = getIndicationCallback();
  QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true");
  if (indCb && msg) {
    int payload[QCRIL_QMI_VOICE_EXT_BRST_INTL_PAYLOAD_LENGTH];
    memset(payload, 0, sizeof(payload));

    payload[0] = msg->getMcc();
    payload[1] = msg->getDataBurstSubType();
    payload[2] = msg->getChargeInd();
    payload[3] = msg->getSubUnit();
    payload[4] = msg->getUnit();

    uint32_t bufLen = 0;
    uint8_t *buf = constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_EXTENDED_DBM_INTL,
                                       (uint8_t *)payload, sizeof(payload));

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    Return<void> ret = indCb->oemHookRawIndication(data);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
    }
  }
}

/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_NSS_RELEASE to telephony via callback
 */
void OemHookImpl::sendUnsolNssRelease(std::shared_ptr<QcRilUnsolNssReleaseMessage> msg) {
  sp<IQtiOemHookIndication> indCb = getIndicationCallback();
  QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true");
  if (indCb && msg) {
    uint8_t callId = msg->getCallId();
    voice_nss_release_enum_v02 nssRelease = msg->getNssRelease();

    size_t nss_release_size = sizeof(nssRelease);
    size_t call_id_size = sizeof(callId);
    char payload[(nss_release_size + call_id_size)];
    memcpy(payload, &nssRelease, nss_release_size);
    memcpy((payload + nss_release_size), &callId, call_id_size);

    uint32_t bufLen = 0;
    uint8_t *buf = constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_NSS_RELEASE,
                                       (uint8_t *)payload, sizeof(payload));

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    Return<void> ret = indCb->oemHookRawIndication(data);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
    }
  }
}
/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_SS_ERROR_CODE to telephony via callback
 */
void OemHookImpl::sendUnsolSsErrorCode(std::shared_ptr<QcRilUnsolSuppSvcErrorCodeMessage> msg) {
  sp<IQtiOemHookIndication> indCb = getIndicationCallback();
  QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true");
  if (indCb && msg) {
    int payload[2] = {0};

    payload[0] = msg->getCallId();
    payload[1] = msg->getFailureCause();

    uint32_t bufLen = 0;
    uint8_t *buf = constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_SS_ERROR_CODE,
                                       (uint8_t *)payload, sizeof(payload));

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    Return<void> ret = indCb->oemHookRawIndication(data);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
    }
  }
}
/*
 * Sends oemHook QCRIL_REQ_HOOK_UNSOL_SPEECH_CODEC_INFO to telephony via callback
 */
void OemHookImpl::sendUnsolSpeechCodecInfo(std::shared_ptr<QcRilUnsolSpeechCodecInfoMessage> msg) {
  sp<IQtiOemHookIndication> indCb = getIndicationCallback();
  QCRIL_LOG_INFO("ind null: %s", indCb ? "false" : "true");
  if (indCb && msg) {
    int payload[3] = {0};

    payload[0] = msg->getCallId();
    payload[1] = msg->getSpeechCodec();
    payload[2] = msg->getNetworkMode();

    uint32_t bufLen = 0;
    uint8_t *buf = constructOemHookRaw(&bufLen, QCRIL_REQ_HOOK_UNSOL_SPEECH_CODEC_INFO,
                                       (uint8_t *)payload, sizeof(payload));

    hidl_vec<uint8_t> data;
    data.setToExternal(buf, bufLen, true); /* Send true to allow hidl_vec to delete memory*/
    Return<void> ret = indCb->oemHookRawIndication(data);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send indication. Exception : %s", ret.description().c_str());
    }
  }
}

/*
 * Register oemhook service with Service Manager
 */
uint8_t oemHookRegisterService(qcril_instance_id_e_type oemSocketInstanceId) {
    android::status_t ret = android::OK;
    if (oemHook == NULL) {
        oemHook = new OemHookImpl(oemSocketInstanceId);
        std::string serviceName = "oemhook";
        ret = oemHook->registerAsService(serviceName + std::to_string(oemSocketInstanceId));
        QCRIL_LOG_INFO("OemHookRegisterService instanceId=%d status=%d", oemSocketInstanceId, ret);
    }
    return ret == android::OK;
}

sp<OemHookImpl> getOemHookService(qcril_instance_id_e_type oemSocketInstanceId)
{
    QCRIL_LOG_INFO("OemHookImpl::getOemHookService: instanceId=%d", oemSocketInstanceId);
    return oemHook;
}

Module* OemHookImpl::getOemHookServiceModule() {
    return mModule;
}

void sendOemhookIndication(qcril_instance_id_e_type oemSocketInstanceId,
    unsigned char* buf, size_t bufLen) {
    if (oemHook != NULL) {
        oemHook->sendIndication(buf, bufLen);
    } else {
        QCRIL_LOG_ERROR("OemHookImpl::sendOemhookIndication:OemHook service is null instanceId=%d",
            oemSocketInstanceId);
    }
}

void sendOemhookResponse(qcril_instance_id_e_type oemSocketInstanceId, int serial,
    RIL_Errno error, unsigned char* buf, size_t bufLen) {
    if (oemHook != NULL) {
        oemHook->sendResponse(serial, error, buf, bufLen);
    } else {
        QCRIL_LOG_ERROR("OemHookImpl::sendOemhookResponse:OemHook service is null instanceId=%d",
            oemSocketInstanceId);
    }
}

} // namespace implementation
} // namespace V1_0
} // namespace qcrilhook
} // namespace radio
} // namespace hardware
} // namespace qti
} // namespace vendor
