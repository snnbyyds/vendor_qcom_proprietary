/******************************************************************************
#  Copyright (c) 2018-2021, 2022 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <stdint.h>
#ifndef in_addr_t
typedef uint32_t in_addr_t;
#endif
#include <arpa/inet.h>
#include <cstring>
#include <string.h>
#include "DSDModemEndPointModule.h"
#include "DSDModemEndPoint.h"
#include <modules/qmi/EndpointStatusIndMessage.h>
#include <modules/qmi/QmiIndMessage.h>
#include "UnSolMessages/DsdSystemStatusMessage.h"

#if (!defined(RIL_FOR_LOW_RAM) || defined(RIL_FOR_MDM_LE))
#include "UnSolMessages/DsdSystemStatusFragmentMessage.h"
#include "UnSolMessages/DsdSystemStatusPerApnMessage.h"
#include "UnSolMessages/SetApnPreferredSystemResultMessage.h"
#include "UnSolMessages/IntentToChangeApnPreferredSystemMessage.h"
#include "sync/TurnOnAPAssistIWLANSyncMessage.h"
#include "sync/RegisterForAPAsstIWlanIndsSyncMessage.h"
#include "request/GetDataNrIconTypeMessage.h"
#include "UnSolMessages/UiInfoFragmentMessage.h"
#include "UnSolMessages/SegmentTimeoutMessage.h"
#include "UnSolMessages/DataNrIconTypeIndMessage.h"
#include "request/SetVowifiConfigurationMessage.h"
#endif

#include "sync/RegisterForRoamingStatusChangedMessage.h"
#include "sync/RegisterForSystemStatusSyncMessage.h"
#include "sync/TriggerDDSSwitchSyncMessage.h"
#include "sync/SetCapabilitiesMessage.h"
#include "request/GetDsdSystemStatusMessage.h"
#include "request/SetRatPrefMessage.h"
#include "UnSolMessages/CurrentDDSIndMessage.h"
#include "UnSolMessages/CurrentRoamingStatusChangedMessage.h"
#include "UnSolMessages/DDSSwitchResultIndMessage.h"
#include "MessageCommon.h"
#include "framework/QcrilInitMessage.h"
#include "DataConfig.h"
#define TAG "DSD EndpointM"

using namespace rildata;



DSDModemEndPointModule::DSDModemEndPointModule(string name,
    ModemEndPoint &owner) : ModemEndPointModule(name, owner)
{
  mServiceObject = nullptr;
  usingSystemStatusV2 = false;
  usingUiInfoV2 = false;
  mNrIconReportDisabled = false;
  mLooper = std::unique_ptr<ModuleLooper>(new ModuleLooper);
  homeRatPref = RatPreference::Inactive;
  roamingRatPref = RatPreference::Inactive;
  ratPrefReceived = false;
  isRoaming = false;

  using std::placeholders::_1;
  using std::placeholders::_2;
  using std::placeholders::_3;
  mMessageHandler = {
      {REG_MSG("DSDModemEndPoint_QMI_IND"), std::bind(&DSDModemEndPointModule::handleQmiDsdIndMessage, this, _1)},
      HANDLER(RegisterForSystemStatusSyncMessage, DSDModemEndPointModule::handleRegisterForSystemStatusSync),
      HANDLER(RegisterForRoamingStatusChangedMessage, DSDModemEndPointModule::handleRegisterForCurrentRoamingStatusSync),
      HANDLER(TriggerDDSSwitchSyncMessage, DSDModemEndPointModule::handleTriggerDDSSwitchSyncMessage),
      HANDLER(SetCapabilitiesMessage, DSDModemEndPointModule::handleSetCapabilities),
      HANDLER(GetDsdSystemStatusMessage, DSDModemEndPointModule::handleGetDsdSystemStatus),
      HANDLER(SetRatPrefMessage, DSDModemEndPointModule::handleSetRatPref),
      HANDLER(QcrilInitMessage, DSDModemEndPointModule::handleQcrilInitMessage),
#if (!defined(RIL_FOR_LOW_RAM) || defined(RIL_FOR_MDM_LE))
      {REG_MSG("DSDModemEndPoint_ENDPOINT_STATUS_IND"), std::bind(&DSDModemEndPointModule::handleEndPointStatusIndMessage, this, _1)},
      HANDLER(RegisterForAPAsstIWlanIndsSyncMessage, DSDModemEndPointModule::handleRegisterForAPAsstIWlanIndsSync),
      HANDLER(TurnOnAPAssistIWLANSyncMessage, DSDModemEndPointModule::handleTurnOnAPAssistIWLANSync),
      HANDLER(GetDataNrIconTypeMessage, DSDModemEndPointModule::handleGetDataNrIconType),
      HANDLER(SetVowifiConfigurationMessage, DSDModemEndPointModule::handleSetVowifiConfiguration),
      HANDLER(SegmentTimeoutMessage, DSDModemEndPointModule::handleSegmentTimeout),
#endif
  };

#if (!defined(RIL_FOR_LOW_RAM) || defined(RIL_FOR_MDM_LE))
  std::unordered_map<unsigned int, SegmentTracker::transactionCompleteCb_t> segmentHandlers = {
    {QMI_DSD_SYSTEM_STATUS_V2_IND_V01, std::bind(&DSDModemEndPointModule::processDsdSystemStatusIndComplete, this, _1, _2, _3)},
    {QMI_DSD_GET_SYSTEM_STATUS_V2_RESULT_IND_V01, std::bind(&DSDModemEndPointModule::processDsdSystemStatusResultComplete, this, _1, _2, _3)},
    {QMI_DSD_UI_INFO_IND_V01, std::bind(&DSDModemEndPointModule::processDsdUiInfoIndComplete, this, _1, _2, _3)},
    {QMI_DSD_GET_UI_INFO_V2_RESULT_IND_V01, std::bind(&DSDModemEndPointModule::processDsdUiInfoResultComplete, this, _1, _2, _3)},
  };
  segmentTracker.setHandlers(segmentHandlers);
  segmentTracker.setEndPointName(EndPointName_t::DSD);
#endif
}


void DSDModemEndPointModule::handleQcrilInitMessage(std::shared_ptr<Message> msg)
{
  #ifdef RIL_FOR_MDM_LE
  propertySetMap("persist.vendor.radio.disable_nricon_report",  PERSIST_VENDOR_RADIO_DISABLE_NRICON_REPORT);
  #endif
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  auto qcril_init_msg = std::static_pointer_cast<QcrilInitMessage>(msg);
  if(qcril_init_msg)
  {
    std::string propValue = readProperty("persist.vendor.radio.disable_nricon_report", "false");
    Log::getInstance().d("[" + mName + "]: NrIcon report disabled property is " + propValue);
    if ("true" == propValue) {
      mNrIconReportDisabled = true;
    }
  } else {
    Log::getInstance().d("Improper message received. =" + msg->dump() + "QCRIL DATA Init not triggered!!!");
  }
}

DSDModemEndPointModule::~DSDModemEndPointModule()
{
  mLooper = nullptr;
}

void DSDModemEndPointModule::init()
{
    ModemEndPointModule::init();
    mStackId[0] = 0;
}

qmi_idl_service_object_type DSDModemEndPointModule::getServiceObject()
{
  return dsd_get_service_object_v01();
}

bool DSDModemEndPointModule::handleQmiBinding
(
  qcril_instance_id_e_type instanceId, int8_t stackId)
{
  Log::getInstance().d(
        "[DSDModemEndPointModule] handleQmiBinding");

  (void)instanceId;
  if (stackId < 0)
    return false;

  dsd_bind_subscription_req_msg_v01 bind_request;
  dsd_bind_subscription_resp_msg_v01 bind_resp;
  memset(&bind_request, 0, sizeof(bind_request));
  memset(&bind_resp, 0, sizeof(bind_resp));

  if (stackId == 0) {
    bind_request.bind_subs = DSD_PRIMARY_SUBS_V01;
  } else if (stackId == 1) {
    bind_request.bind_subs = DSD_SECONDARY_SUBS_V01;
  } else if (stackId == 2) {
    bind_request.bind_subs = DSD_TERTIARY_SUBS_V01;
  } else
    return false;

  int ntries = 0;
  do {
    qmi_client_error_type res = qmi_client_send_msg_sync(
         mQmiSvcClient, QMI_DSD_BIND_SUBSCRIPTION_REQ_V01,
         (void*)&bind_request, sizeof(bind_request),
         (void*)&bind_resp, sizeof(bind_resp),
         ModemEndPointModule::DEFAULT_SYNC_TIMEOUT);
    if (QMI_NO_ERR == res && bind_resp.resp.result ==
            QMI_RESULT_SUCCESS_V01) {
      Log::getInstance().d(
        "[DSDModemEndPointModule]: QMI binding succeeds. instanceId: "
        + std::to_string((int)instanceId) + " stackId: " + std::to_string(stackId));
      return true;
    }
    usleep(500*1000);
  } while (++ntries < 10);
  return false;
}

void DSDModemEndPointModule::handleEndPointStatusIndMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d(mName+": Handling msg = " + msg->dump());
  if(!mNrIconReportDisabled) {
    auto shared_indMsg(std::static_pointer_cast<EndpointStatusIndMessage>(msg));
    if(shared_indMsg != nullptr) {
      if( shared_indMsg->getState() == ModemEndPoint::State::OPERATIONAL ) {
        static_cast<DSDModemEndPoint *>(&mOwner)->registerForUiChangeInd();
      } else {
        Log::getInstance().d("[DSDModemEndPointModule] Endpoint is going down"
                             "Not registering for UI indication");
      }
    }
  }
  else {
    Log::getInstance().d("mNrIconReportDisabled = " + std::to_string((int)mNrIconReportDisabled));
  }
}

void DSDModemEndPointModule::handleQmiDsdIndMessage
(
  std::shared_ptr<Message> msg
)
{
  auto shared_indMsg(std::static_pointer_cast<QmiIndMessage>(msg));
  Log::getInstance().d("[DSDModemEndPointModule]: Handling msg = " + msg->dump());

  QmiIndMsgDataStruct *indData = shared_indMsg->getData();
  if (indData != nullptr) {
    indicationHandler(indData->msgId, indData->indData, indData->indSize);
  }
  else {
    Log::getInstance().d("Unexpected, null data from message");
  }
}

void DSDModemEndPointModule::indicationHandler
(
  unsigned int   msg_id,
  unsigned char *decoded_payload,
  uint32_t       decoded_payload_len
)
{
  (void)decoded_payload_len;
  Log::getInstance().d("[DSDModemEndPointModule]: indicationHandler ENTRY");

  if(decoded_payload == nullptr) {
    Log::getInstance().d("decoded_payload is nullptr");
  }
  else {
    Log::getInstance().d("msg_id = "+ std::to_string(msg_id));
    switch(msg_id)
    {
      case QMI_DSD_SYSTEM_STATUS_IND_V01:
        processSystemStatusInd((dsd_system_status_ind_msg_v01 *)decoded_payload);
        break;
#if (!defined(RIL_FOR_LOW_RAM) || defined(RIL_FOR_MDM_LE))
      case QMI_DSD_SYSTEM_STATUS_V2_IND_V01:
        processSystemStatusInd((dsd_system_status_v2_ind_msg_v01 *)decoded_payload);
        break;

      case QMI_DSD_GET_SYSTEM_STATUS_V2_RESULT_IND_V01:
        processSystemStatusInd((dsd_get_system_status_v2_result_ind_msg_v01 *)decoded_payload);
        break;

      case QMI_DSD_INTENT_TO_CHANGE_APN_PREF_SYS_IND_V01:
        processIntentToChangeApnPrefSysInd((dsd_intent_to_change_apn_pref_sys_ind_msg_v01 *)decoded_payload);
        break;

      case QMI_DSD_AP_ASST_APN_PREF_SYS_RESULT_IND_V01:
        processApAsstApnPrefSysResultInd((dsd_ap_asst_apn_pref_sys_result_ind_msg_v01 *)decoded_payload);
        break;

      case QMI_DSD_UI_INFO_IND_V01:
        processUiInfoInd((dsd_ui_info_ind_msg_v01 *)decoded_payload);
        break;

      case QMI_DSD_GET_UI_INFO_V2_RESULT_IND_V01:
        processUiInfoInd((dsd_get_ui_info_v2_result_ind_msg_v01 *)decoded_payload);
        break;
#endif
      case QMI_DSD_SWITCH_DDS_IND_V01:
        processQmiDdsSwitchInd((dsd_switch_dds_ind_msg_v01*)decoded_payload);
        break;

      case QMI_DSD_CURRENT_DDS_IND_V01:
        processQmiCurrentDdsInd((dsd_current_dds_ind_msg_v01*)decoded_payload);
        break;

      case QMI_DSD_ROAMING_STATUS_CHANGE_IND_V01:
        processRoamingStatusChangeInd((dsd_roaming_status_change_ind_msg_v01 *)decoded_payload);
        break;

      default:
        Log::getInstance().d("Unexpected dsd indication, ignoring = "+ std::to_string(msg_id));
        break;
    }
  }
}

/**
 * @brief [brief description]
 * @details [long description]
 *
 * @param ind_data [description]
 */
void DSDModemEndPointModule::processSystemStatusInd(dsd_system_status_ind_msg_v01 *ind_data)
{
  Log::getInstance().d("[DSDModemEndPointModule]: process dsd_system_status_ind_msg_v01");

  if (!ind_data) {
    Log::getInstance().d("[DSDModemEndPointModule]: invalid input");
    return;
  }

  auto msg = std::make_shared<rildata::DsdSystemStatusMessage>(*ind_data);
  msg->broadcast();

  // TODO: eveluate data registration state
}

void DSDModemEndPointModule::processQmiDdsSwitchInd(dsd_switch_dds_ind_msg_v01 *ind)
{
  if (ind == nullptr) {
    Log::getInstance().d("[DSDModemEndPointModule]: invalid input");
    return;
  }
  Log::getInstance().d("[DSDModemEndPointModule]:"
    "QMI_DSD_SWITCH_DDS_IND_V01: result = " + std::to_string(ind->dds_switch_result));
  auto msg = std::make_shared<DDSSwitchResultIndMessage>();
  switch (ind->dds_switch_result) {
    case DSD_DDS_SWITCH_ALLOWED_V01:
      Log::getInstance().d("[DSDModemEndPointModule]:DsdUnsolicitedIndHdlr: dds switch allowed");
      msg->setAllowed(true);
      break;
    case DSD_DDS_SWITCH_NOT_ALLOWED_V01:
      Log::getInstance().d("[DSDModemEndPointModule]:DsdUnsolicitedIndHdlr: dds switch not allowed");
      msg->setAllowed(false);
      break;
    case DSD_DDS_SWITCH_FAILED_V01:
      Log::getInstance().d("[DSDModemEndPointModule]:DsdUnsolicitedIndHdlr: dds switch failed");
      msg->setFailed(true);
      break;
    default:
      Log::getInstance().d("[DSDModemEndPointModule]:DsdUnsolicitedIndHdlr: dds switch result invalid");
      msg->setFailed(true);
      break;
  }
  msg->broadcast();
}

void DSDModemEndPointModule::processQmiCurrentDdsInd(dsd_current_dds_ind_msg_v01 *ind)
{
  if (ind == nullptr) {
    Log::getInstance().d("[DSDModemEndPointModule]: invalid input");
    return;
  }
  Log::getInstance().d("[DSDModemEndPointModule]:"
        "QMI_DSD_CURRENT_DDS_IND_V01: dds=" + std::to_string(ind->dds));
  dsd_dds_switch_type switchType = static_cast<dsd_dds_switch_type>(ind->dds_switch_type);
  switch (ind->dds) {
    case DSD_PRIMARY_SUBS_V01:
    {
      auto msg = std::make_shared<CurrentDDSIndMessage>(0, switchType);
      msg->broadcast();
      break;
    }
    case DSD_SECONDARY_SUBS_V01:
    {
      auto msg = std::make_shared<CurrentDDSIndMessage>(1, switchType);
      msg->broadcast();
      break;
    }
    default:
      break;
  }
}

bool convertRoamingStatus(uint8_t status) {
  switch (status) {
    case 0:
      return false;
    case 1:
      return true;
    default:
      Log::getInstance().d("[DSDModemEndPointModule]: invalid roaming status " + std::to_string((int)status));
      return false;
  }
}

void DSDModemEndPointModule::processRoamingStatusChangeInd(dsd_roaming_status_change_ind_msg_v01 *ind_data)
{
  Log::getInstance().d("[DSDModemEndPointModule]: process dsd_roaming_status_change_ind_msg_v01 ENTRY");

  if (!ind_data) {
    Log::getInstance().d("[DSDModemEndPointModule]: invalid input");
    return;
  }
  if (ind_data->roaming_status_valid) {
    isRoaming = convertRoamingStatus(ind_data->roaming_status);
    std::shared_ptr<rildata::CurrentRoamingStatusChangedMessage> msg =
        std::make_shared<rildata::CurrentRoamingStatusChangedMessage>(isRoaming);
    if (msg != nullptr) {
      msg->broadcast();
    }

    //Send rat pref again to modem based on current roaming status
    if (ratPrefReceived) {
      sendNotifyDataSettings(isRoaming?roamingRatPref:homeRatPref);
    }

  } else {
    Log::getInstance().d("[DSDModemEndPointModule]: RoamingStatus TLV not valid");
  }
}

#if (!defined(RIL_FOR_LOW_RAM) || defined(RIL_FOR_MDM_LE))
rildata::NrIconEnum_t convertUiMask(const dsd_sys_ui_mask_v01& ui_mask) {
  if (0 != (QMI_DSD_UI_MASK_3GPP_5G_UWB_V01 & ui_mask)) {
    Log::getInstance().d("[DSDModemEndPointModule]: convertUiMask UWB");
    return rildata::NrIconEnum_t::UWB;
  } else if (0 != (QMI_DSD_UI_MASK_3GPP_5G_BASIC_V01 & ui_mask)) {
    Log::getInstance().d("[DSDModemEndPointModule]: convertUiMask BASIC");
    return rildata::NrIconEnum_t::BASIC;
  } else {
    Log::getInstance().d("[DSDModemEndPointModule]: convertUiMask NONE");
    return rildata::NrIconEnum_t::NONE;
  }
}

dsd_apn_avail_sys_info_type_v01 convertApnAvailSystoV1(const dsd_apn_system_status_info_type_v01& apn_avail_sys_v2) {
  dsd_apn_avail_sys_info_type_v01 apn_avail_sys_v1 = {
    .apn_avail_sys_len = apn_avail_sys_v2.apn_avail_sys_len
  };
  memcpy(&apn_avail_sys_v1.apn_name, &apn_avail_sys_v2.apn_name, sizeof(apn_avail_sys_v1.apn_name));
  for (int i = 0; i < apn_avail_sys_v2.apn_avail_sys_len; i++) {
    apn_avail_sys_v1.apn_avail_sys[i] = apn_avail_sys_v2.apn_avail_sys[i];
  }
  return apn_avail_sys_v1;
}

void DSDModemEndPointModule::processUiInfoInd(dsd_ui_info_ind_msg_v01 *ind_data)
{
  Log::getInstance().d("[DSDModemEndPointModule]: process dsd_ui_info_ind_msg_v01 ENTRY");

  if (!ind_data) {
    Log::getInstance().d("[DSDModemEndPointModule]: invalid input");
    return;
  }
  if (ind_data->tx_info_valid) {
    std::shared_ptr<rildata::UiInfoFragmentMessage> msg =
        std::make_shared<rildata::UiInfoFragmentMessage>();
    if (ind_data->global_ui_info_valid && msg != nullptr) {
      msg->setIcon(convertUiMask(ind_data->global_ui_info.ui_mask));
    }
    segmentTracker.addSegment(QMI_DSD_UI_INFO_IND_V01, ind_data->tx_info.transaction_id,
                              ind_data->tx_info.segment_index, ind_data->tx_info.num_segments, msg);
  } else {
    if (ind_data->global_ui_info_valid) {
      rildata::NrIconEnum_t iconType = convertUiMask(ind_data->global_ui_info.ui_mask);
      std::shared_ptr<rildata::DataNrIconTypeIndMessage> msg =
          std::make_shared<rildata::DataNrIconTypeIndMessage>(iconType);
      if (msg != nullptr)
        msg->broadcast();
    }
  }
}

void DSDModemEndPointModule::processUiInfoInd(dsd_get_ui_info_v2_result_ind_msg_v01 *ind_data)
{
  Log::getInstance().d("[DSDModemEndPointModule]: process dsd_get_ui_info_v2_result_ind_msg_v01 ENTRY");

  if (!ind_data) {
    Log::getInstance().d("[DSDModemEndPointModule]: invalid input");
    return;
  }
  std::shared_ptr<rildata::UiInfoFragmentMessage> msg =
      std::make_shared<rildata::UiInfoFragmentMessage>();
  if (ind_data->global_ui_info_valid && msg != nullptr) {
    msg->setIcon(convertUiMask(ind_data->global_ui_info.ui_mask));
  }
  segmentTracker.addSegment(QMI_DSD_GET_UI_INFO_V2_RESULT_IND_V01, ind_data->tx_info.transaction_id,
                            ind_data->tx_info.segment_index, ind_data->tx_info.num_segments, msg);
}

void DSDModemEndPointModule::processDsdSystemStatusIndComplete(SegmentationStatus_t status,
  uint16_t, std::vector<std::shared_ptr<Message>> segments) {
  switch (status) {
    case SegmentationStatus_t::Success:
    {
      vector<dsd_apn_avail_sys_info_type_v01> apn_sys_status;
      dsd_system_status_ind_msg_v01 sys_status_v1 = { .avail_sys_valid = false };
      // consolidate the segments, and split it into two messages
      for (auto msg : segments) {
        auto m = std::static_pointer_cast<DsdSystemStatusFragmentMessage>(msg);
        if (m != nullptr) {
          dsd_system_status_v2_ind_msg_v01 indMsg = m->getParams();
          if (indMsg.apn_avail_sys_valid) {
            for (int i = 0; i < indMsg.apn_avail_sys_len; i++) {
              dsd_apn_avail_sys_info_type_v01 apn_avail_sys_v1 = convertApnAvailSystoV1(indMsg.apn_avail_sys[i]);
              apn_sys_status.push_back(apn_avail_sys_v1);
            }
          }
          if (!sys_status_v1.avail_sys_valid && indMsg.avail_sys_valid) {
            sys_status_v1.avail_sys_valid = true;
            sys_status_v1.avail_sys_len = indMsg.avail_sys_len;
            int source_size = sizeof(indMsg.avail_sys) / sizeof(dsd_system_status_info_type_v01);
            int dest_size = sizeof(sys_status_v1.avail_sys) / sizeof(dsd_system_status_info_type_v01);
            int arr_size = MIN(dest_size, source_size);
            for (int i = 0; i < arr_size; i++) {
                sys_status_v1.avail_sys[i] = indMsg.avail_sys[i];
            }
          }
          if(indMsg.null_bearer_reason_valid) {
            sys_status_v1.null_bearer_reason_valid = true;
            sys_status_v1.null_bearer_reason = indMsg.null_bearer_reason;
          }
        }
      }
      Log::getInstance().d("[DSDModemEndPointModule]: processDsdSystemStatusIndComplete broadcasting sys status v2");
      //Broadcast system status first before the per APN msg

      auto dsdSysStatus = std::make_shared<DsdSystemStatusMessage>(sys_status_v1);
      if (dsdSysStatus) {
        dsdSysStatus->broadcast();
      }

      auto perApnInd = std::make_shared<DsdSystemStatusPerApnMessage>(apn_sys_status);
      if (perApnInd) {
        perApnInd->broadcast();
      }

      break;
    }
    default:
      break;
  }
}

void DSDModemEndPointModule::processDsdSystemStatusResultComplete(SegmentationStatus_t status,
  uint16_t txId, std::vector<std::shared_ptr<Message>> segments) {
  SegmentTracker::KeyType_t key = SegmentTracker::serialize(QMI_DSD_GET_SYSTEM_STATUS_V2_RESULT_IND_V01, txId);
  switch (status) {
    case SegmentationStatus_t::Success:
    {
      vector<dsd_apn_avail_sys_info_type_v01> apn_sys_status;
      dsd_system_status_ind_msg_v01 sys_status_v1 = { .avail_sys_valid = false };
      // consolidate the segments, and split it into two messages
      for (auto msg : segments) {
        auto m = std::static_pointer_cast<DsdSystemStatusFragmentMessage>(msg);
        if (m != nullptr) {
          dsd_system_status_v2_ind_msg_v01 indMsg = m->getParams();
          if (indMsg.apn_avail_sys_valid) {
            for (int i = 0; i < indMsg.apn_avail_sys_len; i++) {
              dsd_apn_avail_sys_info_type_v01 apn_avail_sys_v1 = convertApnAvailSystoV1(indMsg.apn_avail_sys[i]);
              apn_sys_status.push_back(apn_avail_sys_v1);
            }
          }
          if (!sys_status_v1.avail_sys_valid && indMsg.avail_sys_valid) {
            sys_status_v1.avail_sys_valid = true;
            sys_status_v1.avail_sys_len = indMsg.avail_sys_len;
            int source_size = sizeof(indMsg.avail_sys) / sizeof(dsd_system_status_info_type_v01);
            int dest_size = sizeof(sys_status_v1.avail_sys) / sizeof(dsd_system_status_info_type_v01);
            int arr_size = MIN(dest_size, source_size);
            for (int i = 0; i < arr_size; i++) {
                sys_status_v1.avail_sys[i] = indMsg.avail_sys[i];
            }
          }
          if (indMsg.null_bearer_reason_valid) {
           sys_status_v1.null_bearer_reason_valid = true;
           sys_status_v1.null_bearer_reason = indMsg.null_bearer_reason;
          }
        }
      }
      if (pendingSegmentRequests.find(key) != pendingSegmentRequests.end()) {
        // if pending request message exists, respond to it with system status
        DsdSystemStatusResult_t response = {
          .resp_ind = sys_status_v1,
          .apn_sys = apn_sys_status
        };
        auto reqMsg = static_pointer_cast<GetDsdSystemStatusMessage>(pendingSegmentRequests[key]);
        if (reqMsg != nullptr) {
          auto responsePtr = std::make_shared<rildata::DsdSystemStatusResult_t>(response);
          if (responsePtr) {
            reqMsg->sendResponse(reqMsg, Message::Callback::Status::SUCCESS, responsePtr);
          }
          pendingSegmentRequests.erase(key);
          break;
        } else {
          Log::getInstance().d("[DSDModemEndPointModule]: processDsdSystemStatusResultComplete reqMsg is null");
        }
      } else {
        //Broadcast system status first before the per APN msg

        auto dsdSysStatus = std::make_shared<DsdSystemStatusMessage>(sys_status_v1);
        if (dsdSysStatus) {
          dsdSysStatus->broadcast();
        }

        auto perApnInd = std::make_shared<DsdSystemStatusPerApnMessage>(apn_sys_status);
        if (perApnInd) {
          perApnInd->broadcast();
        }

      }
      break;
    }
    default:
    {
      // if segmentation error happened, initiate a new request
      dsd_get_system_status_v2_req_msg_v01  sys_req_msg;
      dsd_get_system_status_v2_resp_msg_v01  sys_resp_msg;

      memset(&sys_req_msg, 0, sizeof(sys_req_msg));
      memset(&sys_resp_msg, 0, sizeof(sys_resp_msg));

      /* Query the current system status from QMI-DSD service */

      int rc = qmi_client_send_msg_sync( mQmiSvcClient,
                                  QMI_DSD_GET_SYSTEM_STATUS_V2_REQ_V01,
                                  &sys_req_msg,
                                  sizeof(sys_req_msg),
                                  &sys_resp_msg,
                                  sizeof(sys_resp_msg),
                                  QCRIL_DATA_QMI_TIMEOUT);
      if (rc != QMI_NO_ERR) {
        Log::getInstance().d("[DSDModemEndPointModule] processDsdSystemStatusResultComplete "
                              "Failed to send QMI_DSD_GET_UI_INFO_REQ_V01 for retry");
      }
      if (pendingSegmentRequests.find(key) != pendingSegmentRequests.end()) {
        // if pending request message exists, respond to it with failure
        DsdSystemStatusResult_t response = {};
        auto reqMsg = static_pointer_cast<GetDsdSystemStatusMessage>(pendingSegmentRequests[key]);
        if (reqMsg != nullptr) {
          auto responsePtr = std::make_shared<rildata::DsdSystemStatusResult_t>(response);
          if (responsePtr) {
            reqMsg->sendResponse(reqMsg, Message::Callback::Status::FAILURE, responsePtr);
          }
          pendingSegmentRequests.erase(key);
          break;
        } else {
          Log::getInstance().d("[DSDModemEndPointModule]: processDsdSystemStatusResultComplete reqMsg is null");
        }
      }
      // No indication to broadcast
    }
  }
}

std::shared_ptr<rildata::UiInfoFragmentMessage> consolidateUiInfo(std::vector<std::shared_ptr<Message>> segments) {
  for (auto msg : segments) {
    auto m = std::static_pointer_cast<rildata::UiInfoFragmentMessage>(msg);
    if (m->hasIcon()) {
      return m;
    }
  }
  Log::getInstance().d("[DSDModemEndPointModule]: consolidateUiInfo icon not found");
  return nullptr;
}

void DSDModemEndPointModule::processDsdUiInfoIndComplete(SegmentationStatus_t status,
  uint16_t, std::vector<std::shared_ptr<Message>> segments) {
  switch (status) {
    case SegmentationStatus_t::Success:
    {
      auto consolidatedMsg = consolidateUiInfo(segments);
      if (consolidatedMsg != nullptr && consolidatedMsg->hasIcon()) {
        auto broadcastMsg = std::make_shared<rildata::DataNrIconTypeIndMessage>(consolidatedMsg->getIcon());
        if (broadcastMsg != nullptr) {
            Log::getInstance().d("[DSDModemEndPointModule]: processDsdUiInfoIndComplete broadcasting " + broadcastMsg->dump());
            broadcastMsg->broadcast();
        }
      }
      return;
    }
    default:
      Log::getInstance().d("[DSDModemEndPointModule]: processDsdUiInfoIndComplete Error occured during segmentation");
      return;
  }
}

void DSDModemEndPointModule::processDsdUiInfoResultComplete(SegmentationStatus_t status,
  uint16_t txId, std::vector<std::shared_ptr<Message>> segments) {
  SegmentTracker::KeyType_t key = SegmentTracker::serialize(QMI_DSD_GET_UI_INFO_V2_RESULT_IND_V01, txId);
  switch (status) {
    case SegmentationStatus_t::Success:
    {
      if (!segments.empty()) {
        auto consolidatedMsg = consolidateUiInfo(segments);
        if (pendingSegmentRequests.find(key) != pendingSegmentRequests.end()) {
          // if pending request message exists, respond to it with icon type
          auto reqMsg = static_pointer_cast<GetDataNrIconTypeMessage>(pendingSegmentRequests[key]);
          if (reqMsg != nullptr) {
            std::shared_ptr<rildata::NrIconType_t> icon;
            if (consolidatedMsg != nullptr && consolidatedMsg->hasIcon()) {
              icon = std::make_shared<rildata::NrIconType_t>(consolidatedMsg->getIcon());
            } else {
              icon = std::make_shared<rildata::NrIconType_t>(NrIconEnum_t::NONE);
            }
            reqMsg->sendResponse(reqMsg, Message::Callback::Status::SUCCESS, icon);
            pendingSegmentRequests.erase(key);
            break;
          } else {
            Log::getInstance().d("[DSDModemEndPointModule]: processDsdUiInfoResultComplete reqMsg is null");
          }
        } else {
          // if no pending request message exists, send indication with icon type
          if (consolidatedMsg != nullptr) {
            NrIconType_t icon{consolidatedMsg->getIcon()};
            auto broadcastMsg = std::make_shared<rildata::DataNrIconTypeIndMessage>(icon);
            if (broadcastMsg != nullptr) {
              Log::getInstance().d("[DSDModemEndPointModule]: processDsdUiInfoResultComplete no pending message..."
                                 " broadcasting" + broadcastMsg->dump());
              broadcastMsg->broadcast();
            }
          }
        }
      } else {
        Log::getInstance().d("[DSDModemEndPointModule]: processDsdUiInfoResultComplete segments is empty");
      }
      break;
    }
    default:
    {
      // if segmentation error happened, initiate a new request
      dsd_get_ui_info_req_msg_v01 qmiReq;
      dsd_get_ui_info_resp_msg_v01 qmiResp;
      memset(&qmiReq, 0, sizeof(qmiReq));
      memset(&qmiResp, 0, sizeof(qmiResp));

      int retVal = qmi_client_send_msg_sync(mQmiSvcClient,
                                QMI_DSD_GET_UI_INFO_REQ_V01,
                                (void *)&qmiReq, sizeof(qmiReq),
                                (void *)&qmiResp, sizeof(qmiResp),
                                QCRIL_DATA_QMI_TIMEOUT);
      if (retVal != QMI_NO_ERR) {
        Log::getInstance().d("[DSDModemEndPointModule] processDsdUiInfoResultComplete "
                              "Failed to send QMI_DSD_GET_UI_INFO_REQ_V01 for retry");
        break;
      }
      if (pendingSegmentRequests.find(key) != pendingSegmentRequests.end()) {
        // if pending request message exists, respond to it with icon type
        auto reqMsg = static_pointer_cast<GetDataNrIconTypeMessage>(pendingSegmentRequests[key]);
        if (reqMsg != nullptr) {
          std::shared_ptr<NrIconType_t> result;
          if (qmiResp.global_ui_info_valid) {
            NrIconEnum_t iconType = convertUiMask(qmiResp.global_ui_info.ui_mask);
            result = std::make_shared<NrIconType_t>(iconType);
          } else {
            result = std::make_shared<NrIconType_t>(NrIconEnum_t::NONE);
          }
          pendingSegmentRequests.erase(key);
          reqMsg->sendResponse(reqMsg, Message::Callback::Status::SUCCESS, result);
        }
      } else {
        if (qmiResp.global_ui_info_valid) {
          NrIconEnum_t iconType = convertUiMask(qmiResp.global_ui_info.ui_mask);
          NrIconType_t icon{iconType};
          auto broadcastMsg = std::make_shared<rildata::DataNrIconTypeIndMessage>(icon);
          if (broadcastMsg != nullptr) {
            Log::getInstance().d("[DSDModemEndPointModule]: processDsdUiInfoResultComplete broadcasting " + broadcastMsg->dump());
            broadcastMsg->broadcast();
          }
        }
      }
    }
  }
}

void DSDModemEndPointModule::getUiInfo(std::shared_ptr<Message> m)
{
  auto msg= std::static_pointer_cast<GetDataNrIconTypeMessage>(m);
  std::shared_ptr<NrIconType_t> result = nullptr;
  dsd_get_ui_info_req_msg_v01 qmiReq;
  dsd_get_ui_info_resp_msg_v01 qmiResp;
  memset(&qmiReq, 0, sizeof(qmiReq));
  memset(&qmiResp, 0, sizeof(qmiResp));

  int retVal = qmi_client_send_msg_sync(mQmiSvcClient,
                            QMI_DSD_GET_UI_INFO_REQ_V01,
                            (void *)&qmiReq, sizeof(qmiReq),
                            (void *)&qmiResp, sizeof(qmiResp),
                            QCRIL_DATA_QMI_TIMEOUT);
  if (retVal != QMI_NO_ERR) {
    Log::getInstance().d("[DSDModemEndPointModule::getUiInfo]"
                          "Failed to send QMI_DSD_GET_UI_INFO_REQ_V01");
    msg->sendResponse(msg, Message::Callback::Status::FAILURE, result);
    return;
  }
  if (qmiResp.global_ui_info_valid) {
    NrIconEnum_t iconType = convertUiMask(qmiResp.global_ui_info.ui_mask);
    result = std::make_shared<NrIconType_t>(iconType);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, result);
  } else if (qmiResp.tx_id_valid) {
    SegmentTracker::KeyType_t key = SegmentTracker::serialize(QMI_DSD_GET_UI_INFO_V2_RESULT_IND_V01, qmiResp.tx_id);
    Log::getInstance().d("[DSDModemEndPointModule::getUiInfo]"
                         " Begin transaction txId=" + key);
    pendingSegmentRequests[key] = msg;
    segmentTracker.startTransactionTimer(QMI_DSD_GET_UI_INFO_V2_RESULT_IND_V01, qmiResp.tx_id);
    // If there is transaction id, delay the response until following QMI segments arrive
  } else {
    result = std::make_shared<NrIconType_t>(NrIconEnum_t::NONE);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, result);
  }
}

void DSDModemEndPointModule::handleGetDataNrIconType(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  auto m = std::static_pointer_cast<GetDataNrIconTypeMessage>(msg);
  if (m != NULL)
  {
    if (mNrIconReportDisabled) {
      Log::getInstance().d("handleGetDataNrIconType icontype disabled NONE");
      std::shared_ptr<NrIconType_t> result = std::make_shared<NrIconType_t>(NrIconEnum_t::NONE);
      m->sendResponse(msg, Message::Callback::Status::SUCCESS, result);
    } else {
      getUiInfo(msg);
    }
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

/**
 * @brief [brief description]
 * @details [long description]
 *
 * @param ind_data [description]
 */
void DSDModemEndPointModule::processSystemStatusInd(dsd_system_status_v2_ind_msg_v01 *ind_data)
{
  Log::getInstance().d("[DSDModemEndPointModule]: process dsd_system_status_v2_ind_msg_v01");

  if (!ind_data) {
    Log::getInstance().d("[DSDModemEndPointModule]: invalid input");
    return;
  }

  auto msg = std::make_shared<rildata::DsdSystemStatusFragmentMessage>(*ind_data);
  segmentTracker.addSegment(QMI_DSD_SYSTEM_STATUS_V2_IND_V01, ind_data->tx_info.transaction_id,
                            ind_data->tx_info.segment_index, ind_data->tx_info.num_segments, msg);
}

dsd_system_status_v2_ind_msg_v01 convertToSystemStatusV2Ind(const dsd_get_system_status_v2_result_ind_msg_v01& ind_result)
{
  dsd_system_status_v2_ind_msg_v01 ind_msg = {
    .tx_info = ind_result.tx_info,
    .avail_sys_valid = ind_result.avail_sys_valid,
    .avail_sys_len = ind_result.avail_sys_len,
    .recommended_pref_sys_index_valid = ind_result.recommended_pref_sys_index_valid,
    .recommended_pref_sys_index = ind_result.recommended_pref_sys_index,
    .null_bearer_reason_valid = ind_result.null_bearer_reason_valid,
    .null_bearer_reason = ind_result.null_bearer_reason,
    .apn_avail_sys_valid = ind_result.apn_avail_sys_valid,
    .apn_avail_sys_len = ind_result.apn_avail_sys_len,
  };
  memcpy(&ind_msg.avail_sys, &ind_result.avail_sys, sizeof(ind_msg.avail_sys));
  memcpy(&ind_msg.apn_avail_sys, &ind_result.apn_avail_sys, sizeof(ind_msg.apn_avail_sys));
  return ind_msg;
}

void DSDModemEndPointModule::processSystemStatusInd(dsd_get_system_status_v2_result_ind_msg_v01 *ind_result_data)
{
  Log::getInstance().d("[DSDModemEndPointModule]: process dsd_get_system_status_v2_result_ind_msg_v01");

  if (!ind_result_data) {
    Log::getInstance().d("[DSDModemEndPointModule]: invalid input");
    return;
  }

  dsd_system_status_v2_ind_msg_v01 ind_data = convertToSystemStatusV2Ind(*ind_result_data);
  auto msg = std::make_shared<rildata::DsdSystemStatusFragmentMessage>(ind_data);
  segmentTracker.addSegment(QMI_DSD_GET_SYSTEM_STATUS_V2_RESULT_IND_V01, ind_data.tx_info.transaction_id,
                            ind_data.tx_info.segment_index, ind_data.tx_info.num_segments, msg);
}

/**
 * @brief This indication is from modem that preferred RAT is changed
 * @details Modem wants to handoff to target RAT and inform to QNP to update
 *          preferred RAT change to Transport Manager
 *
 * @param ind_data [description]
 */
void DSDModemEndPointModule::processIntentToChangeApnPrefSysInd(dsd_intent_to_change_apn_pref_sys_ind_msg_v01 *ind_data)
{
  Log::getInstance().d("[DSDModemEndPointModule]: processQmiDsdIntentToChangeApnPrefSysInd = "+std::to_string(ind_data->apn_pref_sys_len));

  int numOfApns = ind_data->apn_pref_sys_len;
  for(int i=0 ; i<numOfApns ; i++) {
    //ind_data->apn_pref_sys[i].apn_name;
    // TODO: Search APN type for apn name
    // if data for APN name is connected, change the state to HANDOVER

    //ind_data->apn_pref_sys[i].pref_sys;
  }

  auto msg = std::make_shared<rildata::IntentToChangeApnPreferredSystemMessage>(*ind_data);
  msg->broadcast();
}

/**
 *
 */
void DSDModemEndPointModule::processApAsstApnPrefSysResultInd(dsd_ap_asst_apn_pref_sys_result_ind_msg_v01 *ind_data)
{
  Log::getInstance().d("[NetworkServiceHandler]: processApAsstApnPrefSysResultInd = "+std::to_string(ind_data->result_info.result));

  auto msg = std::make_shared<rildata::SetApnPreferredSystemResultMessage>(*ind_data);
  msg->broadcast();
}

void DSDModemEndPointModule::handleTurnOnAPAssistIWLANSync(std::shared_ptr<Message> m)
{
  Log::getInstance().d("[DSDModemEndPointModule]: handleTurnOnAPAssistIWLANSync : Handling msg = " + m->dump());

  auto msg= std::static_pointer_cast<TurnOnAPAssistIWLANSyncMessage>(m);

  if (mQmiSvcClient != nullptr)
  {
    do
    {
        dsd_set_capability_req_msg_v01 cap_req;
        dsd_set_capability_resp_msg_v01 cap_resp;

        memset(&cap_req, 0, sizeof(cap_req));
        memset(&cap_resp, 0, sizeof(cap_resp));

        //Send AP capability
        cap_req.ap_asst_mode_valid = true;
        cap_req.ap_asst_mode = DSD_AP_ASST_APN_PREF_SYS_ON_V01;

        int rc = qmi_client_send_msg_sync(mQmiSvcClient,
                                     QMI_DSD_SET_CAPABILITY_REQ_V01,
                                     (void *)&cap_req,
                                     sizeof(cap_req),
                                     (void *)&cap_resp,
                                     sizeof(cap_resp),
                                     QCRIL_DATA_QMI_TIMEOUT);
        if ((rc != QMI_NO_ERR) || (cap_resp.resp.result == QMI_RESULT_FAILURE_V01 )){
          Log::getInstance().d("[DSDModemEndPointModule] Failed to send QMI_DSD_SET_CAPABILITY_REQ_V01");
          break;
        } else
        {
          Log::getInstance().d("[DSDModemEndPointModule] QMI_DSD_SET_CAPABILITY_REQ_V01::Successfully sent");
        }

        msg->sendResponse(msg, Message::Callback::Status::SUCCESS, nullptr);
        return;
    }
    while (0);
    msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
  }

}

void DSDModemEndPointModule::handleGetDsdSystemStatusV2(std::shared_ptr<Message> m) {
  Log::getInstance().d("[DSDModemEndPointModule]: handleGetDsdSystemStatusV2 ENTRY");
  auto msg= std::static_pointer_cast<GetDsdSystemStatusMessage>(m);
  if (msg != nullptr) {
    dsd_get_system_status_v2_req_msg_v01  sys_req_msg;
    dsd_get_system_status_v2_resp_msg_v01  sys_resp_msg;

    memset(&sys_req_msg, 0, sizeof(sys_req_msg));
    memset(&sys_resp_msg, 0, sizeof(sys_resp_msg));

    /* Query the current system status from QMI-DSD service */
    int rc = qmi_client_send_msg_sync( mQmiSvcClient,
                                QMI_DSD_GET_SYSTEM_STATUS_V2_REQ_V01,
                                &sys_req_msg,
                                sizeof(sys_req_msg),
                                &sys_resp_msg,
                                sizeof(sys_resp_msg),
                                QCRIL_DATA_QMI_TIMEOUT);

    if (QMI_NO_ERR != rc || QMI_NO_ERR != sys_resp_msg.resp.result) {
      QCRIL_LOG_ERROR("failed to send dsd_get_system_status_v2, rc=%d err=%d",rc,
                                sys_resp_msg.resp.error);
      msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
      return;
    }

    if (sys_resp_msg.transaction_id_valid) {
      SegmentTracker::KeyType_t key = SegmentTracker::serialize(QMI_DSD_GET_SYSTEM_STATUS_V2_RESULT_IND_V01, sys_resp_msg.transaction_id);
      Log::getInstance().d("[DSDModemEndPointModule::handleGetDsdSystemStatusV2]"
                          " Begin transaction tx=" + key);
      pendingSegmentRequests[key] = msg;
      // expect an indication later to respond to the message
      segmentTracker.startTransactionTimer(QMI_DSD_GET_SYSTEM_STATUS_V2_RESULT_IND_V01, sys_resp_msg.transaction_id);
    } else {
      Log::getInstance().d("[DSDModemEndPointModule::handleGetDsdSystemStatusV2] invalid transaction id");
      msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
    }
  }
  else {
    msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
  }
}

void DSDModemEndPointModule::handleRegisterForAPAsstIWlanIndsSync(std::shared_ptr<Message> m)
{
  Log::getInstance().d("[DSDModemEndPointModule]: handleRegisterForAPAsstIWlanIndsSysSync : Handling msg = " + m->dump());

  auto msg= std::static_pointer_cast<RegisterForAPAsstIWlanIndsSyncMessage>(m);

  if (mQmiSvcClient != nullptr)
  {
    do
    {
        dsd_indication_register_req_msg_v01 ind_req;
        dsd_indication_register_resp_msg_v01 ind_resp;

        memset(&ind_req, 0, sizeof(ind_req));
        memset(&ind_resp, 0, sizeof(ind_resp));

        ind_req.report_intent_to_change_pref_sys_valid = true;
        ind_req.report_intent_to_change_pref_sys = msg->getParams();

        ind_req.report_ap_asst_apn_pref_sys_result_valid = true;
        ind_req.report_ap_asst_apn_pref_sys_result = msg->getParams();

        int rc = qmi_client_send_msg_sync(mQmiSvcClient,
                                     QMI_DSD_INDICATION_REGISTER_REQ_V01,
                                     (void *)&ind_req,
                                     sizeof(ind_req),
                                     (void *)&ind_resp,
                                     sizeof(ind_resp),
                                     QCRIL_DATA_QMI_TIMEOUT);

        if ((rc != QMI_NO_ERR) || (ind_resp.resp.result == QMI_RESULT_FAILURE_V01 )) {
          Log::getInstance().d("[DSDModemEndPointModule] Failed to send QMI_DSD_INDICATION_REGISTER_REQ_V01");
          break;
        } else
        {
          Log::getInstance().d("[DSDModemEndPointModule] QMI_DSD_INDICATION_REGISTER_REQ_V01::Successfully sent");
        }
        msg->sendResponse(msg, Message::Callback::Status::SUCCESS, nullptr);
        return;
    }
    while (0);
    msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
  }

}

void DSDModemEndPointModule::handleSetVowifiConfiguration(std::shared_ptr<Message> m) {
  Log::getInstance().d("[DSDModemEndPointModule]: Handling msg = " + m->dump());
  std::shared_ptr<SetVowifiConfigurationMessage> msg = std::static_pointer_cast<SetVowifiConfigurationMessage>(m);
  if (msg != nullptr) {
    qmi_dsd_configure_vowifi_service_req_msg_v01 req_msg;
    qmi_dsd_configure_vowifi_service_resp_msg_v01 resp_msg;
    memset(&req_msg, 0, sizeof(req_msg));
    memset(&resp_msg, 0, sizeof(resp_msg));

    RIL_Errno retVal = RIL_E_SUCCESS;
    if (mQmiSvcClient == nullptr) {
      retVal = RIL_E_OEM_ERROR_1;
    } else {
      if (msg->hasFqdn()) {
        req_msg.fqdn_valid = true;
        strlcpy(req_msg.fqdn, msg->getFqdn().c_str(), std::min((size_t)QMI_DSD_MAX_FQDN_LEN_V01, msg->getFqdn().length()) + 1);
      }
      if (msg->hasV4Address()) {
        in_addr buffer{};
        int convertResult = inet_pton(AF_INET, msg->getV4Address().c_str(), &buffer);
        if (convertResult == 1) {
          req_msg.vowifi_ipv4_addr_valid = true;
          assert(sizeof(buffer) == sizeof(req_msg.vowifi_ipv4_addr));
          memcpy((void *)&req_msg.vowifi_ipv4_addr, (void *)&buffer, sizeof(buffer));
        } else {
          Log::getInstance().d("[DSDModemEndPointModule] V4 conversion failed");
        }
      }
      if (msg->hasV6Address()) {
        in6_addr buffer{};
        int convertResult = inet_pton(AF_INET6, msg->getV6Address().c_str(), &buffer);
        if (convertResult == 1) {
          req_msg.vowifi_ipv6_addr_valid = true;
          assert(sizeof(buffer) == sizeof(req_msg.vowifi_ipv6_addr));
          memcpy((void *)req_msg.vowifi_ipv6_addr, (void *)&buffer, sizeof(buffer));
        } else {
          Log::getInstance().d("[DSDModemEndPointModule] V6 conversion failed");
        }
      }
      int rc = qmi_client_send_msg_sync(mQmiSvcClient,
                                    QMI_DSD_CONFIGURE_VOWIFI_SERVICE_REQ_V01,
                                    (void *)&req_msg,
                                    sizeof(req_msg),
                                    (void *)&resp_msg,
                                    sizeof(resp_msg),
                                    QCRIL_DATA_QMI_TIMEOUT);
      if ((rc != QMI_NO_ERR) || (resp_msg.resp.result == QMI_RESULT_FAILURE_V01 )) {
        Log::getInstance().d("[DSDModemEndPointModule] Failed to send QMI_DSD_CONFIGURE_VOWIFI_SERVICE_REQ_V01");
        retVal = RIL_E_OEM_ERROR_2;
      } else{
        Log::getInstance().d("[DSDModemEndPointModule] QMI_DSD_CONFIGURE_VOWIFI_SERVICE_REQ_V01::Successfully sent");
      }
    }
    std::shared_ptr<RIL_Errno> result = std::make_shared<RIL_Errno>(retVal);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, result);
  } else {
    Log::getInstance().d("[DSDModemEndPointModule]: Improper message received = " + msg->dump());
  }
}

void DSDModemEndPointModule::handleSegmentTimeout(std::shared_ptr<Message> m)
{
  Log::getInstance().d("[DSDModemEndPointModule]: handleSegmentTimeout : Handling msg = " + m->dump());

  auto msg= std::static_pointer_cast<SegmentTimeoutMessage>(m);
  if(msg != nullptr && msg->getEndPointName() == EndPointName_t::DSD)
  {
    segmentTracker.handleSegmentTimeout(msg->getMessageId(), msg->getTransactionId());
  }
}

#endif //RIL_FOR_LOW_RAM

void DSDModemEndPointModule::handleTriggerDDSSwitchSyncMessage(std::shared_ptr<Message> m)
{
  Log::getInstance().d("[DSDModemEndPointModule]: handleTriggerDDSSwitchSyncMessage : Handling msg = " + m->dump());
  auto msg= std::static_pointer_cast<TriggerDDSSwitchSyncMessage>(m);
  if (msg != NULL)
  {
    dsd_switch_dds_resp_msg_v01 respMsg = qmiDDSSwitchRequest(msg->getSubId(), msg->getSwitchType());
    SetPreferredDataModemResult_t resp;
    switch (respMsg.resp.error) {
      case QMI_ERR_NONE_V01:
        resp = SetPreferredDataModemResult_t::NO_ERROR;
        break;
      case QMI_ERR_INVALID_ARG_V01:
        resp = SetPreferredDataModemResult_t::INVALID_ARG;
        break;
      case QMI_ERR_MISSING_ARG_V01:
        resp = SetPreferredDataModemResult_t::INVALID_ARG;
        break;
      case QMI_ERR_INVALID_OPERATION_V01:
        resp = SetPreferredDataModemResult_t::INVALID_OPERATION;
        break;
      default:
        resp = SetPreferredDataModemResult_t::QMI_ERROR;
        break;
    }
    if (resp == SetPreferredDataModemResult_t::NO_ERROR) {
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, std::make_shared<SetPreferredDataModemResult_t>(resp));
    } else {
      Log::getInstance().d("Failed to switch dds");
      msg->sendResponse(msg, Message::Callback::Status::FAILURE, std::make_shared<SetPreferredDataModemResult_t>(resp));
    }
  } else
  {
    Log::getInstance().d("Msg received is NULL");
  }
}

void DSDModemEndPointModule::handleSetCapabilities(std::shared_ptr<Message> m)
{
  Log::getInstance().d("[DSDModemEndPointModule]: Handling msg = " + m->dump());

  auto msg= std::static_pointer_cast<SetCapabilitiesMessage>(m);
  if (msg != nullptr && mQmiSvcClient != nullptr)
  {
    dsd_set_capability_req_msg_v01 cap_req;
    dsd_set_capability_resp_msg_v01 cap_resp;
    memset(&cap_req, 0, sizeof(cap_req));
    memset(&cap_resp, 0, sizeof(cap_resp));

    if (msg->getSystemStatusV2()) {
      cap_req.system_status_cap_valid = true;
      cap_req.system_status_cap = DSD_CAPABILITY_SYSTEM_STATUS_V2_V01;
    }
    if (msg->getUiInfoV2()) {
      cap_req.ui_info_cap_valid = true;
      cap_req.ui_info_cap = DSD_CAPABILITY_UI_INFO_V2_V01;
    }

    int rc = qmi_client_send_msg_sync(mQmiSvcClient,
                        QMI_DSD_SET_CAPABILITY_REQ_V01,
                        (void *)&cap_req, sizeof(cap_req),
                        (void *)&cap_resp, sizeof(cap_resp),
                        QCRIL_DATA_QMI_TIMEOUT);

    if ((rc != QMI_NO_ERR) || (cap_resp.resp.result == QMI_RESULT_FAILURE_V01 )) {
      Log::getInstance().d("[DSDModemEndPointModule] Failed to send QMI_DSD_SET_CAPABILITY_REQ_V01");
      msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
      return;
    }
    if (cap_resp.system_status_cap_valid &&
        cap_resp.system_status_cap == DSD_CAPABILITY_SYSTEM_STATUS_V2_V01) {
      usingSystemStatusV2 = true;
    }
    if (cap_resp.ui_info_cap_valid &&
        cap_resp.ui_info_cap == DSD_CAPABILITY_UI_INFO_V2_V01) {
      usingUiInfoV2 = true;
    }
    stringstream ss;
    ss << "[DSDModemEndPointModule] QMI_DSD_SET_CAPABILITY_REQ_V01::Successfully sent. " << std::boolalpha <<
      "systemStatusV2=" << usingSystemStatusV2 << " uiInfoV2=" << usingUiInfoV2;
    Log::getInstance().d(ss.str());
    int status = usingSystemStatusV2 && usingUiInfoV2;
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, std::make_shared<int>(status));
  } else {
    msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
  }
}

void DSDModemEndPointModule::handleGetDsdSystemStatusV1(std::shared_ptr<Message> m) {
  Log::getInstance().d("[DSDModemEndPointModule]: handleGetDsdSystemStatusV1 ENTRY");
  auto msg= std::static_pointer_cast<GetDsdSystemStatusMessage>(m);
  if (msg != nullptr) {
    do
    {
      dsd_get_system_status_req_msg_v01  sys_req_msg;
      dsd_get_system_status_resp_msg_v01  sys_resp_msg;
      dsd_system_status_ind_msg_v01 *ind_data = nullptr;

      memset(&sys_req_msg, 0, sizeof(sys_req_msg));
      memset(&sys_resp_msg, 0, sizeof(sys_resp_msg));

      /* Query the current system status from QMI-DSD service */

      int rc = qmi_client_send_msg_sync( mQmiSvcClient,
                                  QMI_DSD_GET_SYSTEM_STATUS_REQ_V01,
                                  &sys_req_msg,
                                  sizeof(sys_req_msg),
                                  &sys_resp_msg,
                                  sizeof(sys_resp_msg),
                                  QCRIL_DATA_QMI_TIMEOUT);

      if (QMI_NO_ERR != rc) {
        QCRIL_LOG_ERROR("failed to send qmi_dsd_get_system_status, err=%d",rc);
        break;
      }
      else if (QMI_NO_ERR != sys_resp_msg.resp.result) {
        QCRIL_LOG_ERROR("failed to send qmi_dsd_get_system_status, err=%d",
                                  sys_resp_msg.resp.error);
        break;
      }

      /* Process the system status response */
      ind_data = (dsd_system_status_ind_msg_v01 *)((char *)&sys_resp_msg +
                                                offsetof(dsd_get_system_status_resp_msg_v01,
                                                avail_sys_valid));
      DsdSystemStatusResult_t result_v1 = {};
      if (ind_data != nullptr) {
        result_v1.resp_ind = *ind_data;
      }
      auto result = std::make_shared<DsdSystemStatusResult_t>(result_v1);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, result);
      return;
    }
    while (0);
    msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
  }
}

void DSDModemEndPointModule::handleRegisterForCurrentRoamingStatusSync(std::shared_ptr<Message> m) {
  Log::getInstance().d("[DSDModemEndPointModule]: handleRegisterForCurrentRoamingStatusSync : Handling msg = " + m->dump());

  auto msg= std::static_pointer_cast<RegisterForRoamingStatusChangedMessage>(m);

  if (mQmiSvcClient != nullptr)
  {
    do
    {
        dsd_indication_register_req_msg_v01 ind_req;
        dsd_indication_register_resp_msg_v01 ind_resp;

        memset(&ind_req, 0, sizeof(ind_req));
        memset(&ind_resp, 0, sizeof(ind_resp));

        ind_req.report_roaming_status_change_valid = true;
        ind_req.report_roaming_status_change = 1;

        int rc = qmi_client_send_msg_sync(mQmiSvcClient,
                                     QMI_DSD_INDICATION_REGISTER_REQ_V01,
                                     (void *)&ind_req,
                                     sizeof(ind_req),
                                     (void *)&ind_resp,
                                     sizeof(ind_resp),
                                     QCRIL_DATA_QMI_TIMEOUT);

        if ((rc != QMI_NO_ERR) || (ind_resp.resp.result == QMI_RESULT_FAILURE_V01 )) {
          Log::getInstance().d("[DSDModemEndPointModule] Failed to send QMI_DSD_INDICATION_REGISTER_REQ_V01");
        } else
        {
          Log::getInstance().d("[DSDModemEndPointModule] QMI_DSD_INDICATION_REGISTER_REQ_V01::Successfully sent");
        }
        //Query the modem for initial roaming status
        dsd_get_current_roaming_status_info_req_msg_v01 req_msg;
        dsd_get_current_roaming_status_info_resp_msg_v01 resp_msg;
        memset(&req_msg, 0, sizeof(req_msg));
        memset(&resp_msg, 0, sizeof(resp_msg));

        rc = qmi_client_send_msg_sync(mQmiSvcClient,
                                          QMI_DSD_GET_CURRENT_ROAMING_STATUS_INFO_REQ_V01,
                                          (void *)&req_msg,
                                          sizeof(req_msg),
                                          (void *)&resp_msg,
                                          sizeof(resp_msg),
                                          QCRIL_DATA_QMI_TIMEOUT);

        if ((rc != QMI_NO_ERR) || (resp_msg.resp.result == QMI_RESULT_FAILURE_V01 )) {
          Log::getInstance().d("[DSDModemEndPointModule] Failed to send QMI_DSD_GET_CURRENT_ROAMING_STATUS_INFO_REQ_V01");
        } else {
          Log::getInstance().d("[DSDModemEndPointModule] QMI_DSD_GET_CURRENT_ROAMING_STATUS_INFO_REQ_V01::Successfully sent");

          //Pretend its an indication to broadcast the info and send rat pref to modem etc
          dsd_roaming_status_change_ind_msg_v01 ind_msg;
          memset(&ind_msg, 0, sizeof(ind_msg));
          ind_msg.roaming_status_valid = resp_msg.roaming_status_valid;
          ind_msg.roaming_status = resp_msg.roaming_status;
          processRoamingStatusChangeInd(&ind_msg);
        }

        msg->sendResponse(msg, Message::Callback::Status::SUCCESS, nullptr);
        return;
    }
    while (0);
  }
  msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);

}

dsd_rat_preference_enum_v01 convertRatPreferenceToDsd(RatPreference pref) {
  switch (pref) {
    case RatPreference::Inactive:
      return DSD_RAT_PREFERENCE_INACTIVE_V01;
    case RatPreference::CellularOnly:
      return DSD_RAT_PREFERENCE_CELLULAR_ONLY_V01;
    case RatPreference::WifiOnly:
      return DSD_RAT_PREFERENCE_WIFI_ONLY_V01;
    case RatPreference::CellularPreferred:
      return DSD_RAT_PREFERENCE_CELLULAR_PREFERRED_V01;
    case RatPreference::WifiPreferred:
      return DSD_RAT_PREFERENCE_WIFI_PREFERRED_V01;
    case RatPreference::ImsPreferred:
      return DSD_RAT_PREFERENCE_IMS_PREFERRED_V01;
    default:
      Log::getInstance().d("[DSDModemEndPointModule]: received invalid rat pref=" + to_string((int)pref));
      return DSD_RAT_PREFERENCE_ENUM_MIN_ENUM_VAL_V01;
  }
}

void DSDModemEndPointModule::handleSetRatPref(std::shared_ptr<Message> m) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + m->dump());
  std::shared_ptr<SetRatPrefMessage> msg = std::static_pointer_cast<SetRatPrefMessage>(m);
  if (msg != nullptr) {

    RIL_Errno retVal = RIL_E_SUCCESS;
    //Check for validity of RIL values
    if ((msg->getRatPreference() >= RatPreference::Inactive &&
         msg->getRatPreference() <= RatPreference::ImsPreferred) &&
        (msg->getRoamingRatPreference() >= RatPreference::Inactive &&
         msg->getRoamingRatPreference() <= RatPreference::ImsPreferred)) {

      //Cache the latest values from the message
      homeRatPref = msg->getRatPreference();
      roamingRatPref = msg->getRoamingRatPreference();
      ratPrefReceived = true;
      if (mQmiSvcClient == nullptr) {
        retVal = RIL_E_OEM_ERROR_1;
      }
      else {
        if (!sendNotifyDataSettings(isRoaming? roamingRatPref:homeRatPref)) {
          retVal = RIL_E_OEM_ERROR_2;
        }
      }
    }
    else {
      retVal = RIL_E_INVALID_ARGUMENTS;
    }

    std::shared_ptr<RIL_Errno> result = std::make_shared<RIL_Errno>(retVal);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, result);
  } else {
    Log::getInstance().d("[DSDModemEndPointModule]: Improper message received = " + msg->dump());
  }
}

bool DSDModemEndPointModule::sendNotifyDataSettings(RatPreference ratPref) {
  bool retVal = true;
  dsd_notify_data_settings_req_msg_v01 req_msg;
  dsd_notify_data_settings_resp_msg_v01 resp_msg;
  memset(&req_msg, 0, sizeof(req_msg));
  memset(&resp_msg, 0, sizeof(resp_msg));

  req_msg.rat_preference_valid = true;
  req_msg.rat_preference = convertRatPreferenceToDsd(ratPref);

  int rc = qmi_client_send_msg_sync(mQmiSvcClient,
                                    QMI_DSD_NOTIFY_DATA_SETTING_REQ_V01,
                                    (void *)&req_msg,
                                    sizeof(req_msg),
                                    (void *)&resp_msg,
                                    sizeof(resp_msg),
                                    QCRIL_DATA_QMI_TIMEOUT);
      if ((rc != QMI_NO_ERR) || (resp_msg.resp.result == QMI_RESULT_FAILURE_V01 )) {
        Log::getInstance().d("[DSDModemEndPointModule] Failed to send QMI_DSD_NOTIFY_DATA_SETTING_REQ_V01");
     retVal = false;
      } else{
        Log::getInstance().d("[DSDModemEndPointModule] QMI_DSD_NOTIFY_DATA_SETTING_REQ_V01::Successfully sent");
      }
  return retVal;
}

void DSDModemEndPointModule::handleGetDsdSystemStatus(std::shared_ptr<Message> m)
{
  Log::getInstance().d("[DSDModemEndPointModule]: Handling msg = " + m->dump());
  auto msg= std::static_pointer_cast<GetDsdSystemStatusMessage>(m);

  if (mQmiSvcClient != nullptr)
  {
#if (!defined(RIL_FOR_LOW_RAM) || defined(RIL_FOR_MDM_LE))
    if (usingSystemStatusV2) {
      handleGetDsdSystemStatusV2(m);
    } else 
#endif
    {
      handleGetDsdSystemStatusV1(m);
        }
      }
}

void DSDModemEndPointModule::handleRegisterForSystemStatusSync(std::shared_ptr<Message> m)
{
  Log::getInstance().d("[DSDModemEndPointModule]: handleRegisterForSystemStatusSync : Handling msg = " + m->dump());

  auto msg= std::static_pointer_cast<RegisterForSystemStatusSyncMessage>(m);

  if (mQmiSvcClient != nullptr)
  {
    do
    {
        dsd_system_status_change_req_msg_v01 system_status_change_req;
        dsd_system_status_change_resp_msg_v01 system_status_change_resp;

        memset(&system_status_change_req, 0, sizeof(system_status_change_req));
        memset(&system_status_change_resp, 0, sizeof(system_status_change_resp));
        system_status_change_req.report_null_bearer_reason_valid = true;
        system_status_change_req.report_null_bearer_reason = true;

        auto rc = qmi_client_send_msg_sync(mQmiSvcClient,
                                     QMI_DSD_SYSTEM_STATUS_CHANGE_REQ_V01,
                                     (void *)&system_status_change_req,
                                     sizeof(system_status_change_req),
                                     (void *)&system_status_change_resp,
                                     sizeof(system_status_change_resp),
                                    QCRIL_DATA_QMI_TIMEOUT);
        if ((rc != QMI_NO_ERR) || (system_status_change_resp.resp.result == QMI_RESULT_FAILURE_V01 )) {
          Log::getInstance().d("[DSDModemEndPointModule] Failed to send QMI_DSD_SYSTEM_STATUS_CHANGE_REQ_V01");
          break;
        } else
        {
          Log::getInstance().d("[DSDModemEndPointModule] QMI_DSD_SYSTEM_STATUS_CHANGE_REQ_V01::Successfully sent");
      }

        msg->sendResponse(msg, Message::Callback::Status::SUCCESS, nullptr);
        return;
    }
    while (0);
    msg->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
  }

}

dsd_switch_dds_resp_msg_v01 DSDModemEndPointModule::qmiDDSSwitchRequest(int subId, dsd_dds_switch_type_enum_v01 switch_type)
{
  Log::getInstance().d("DSDModemEndPointModule::qmiDDSSwitchRequest ENTRY");

  dsd_switch_dds_req_msg_v01 reqMsg;
  dsd_switch_dds_resp_msg_v01 respMsg;

  memset(&reqMsg, 0, sizeof(reqMsg));
  memset(&respMsg, 0, sizeof(respMsg));

  switch (subId) {
    case 0:
      reqMsg.subscription = DSD_PRIMARY_SUBS_V01;
      break;
    case 1:
      reqMsg.subscription = DSD_SECONDARY_SUBS_V01;
      break;
    default:
      respMsg.resp.error = QMI_ERR_INVALID_ARG_V01;
      return respMsg;
  }

  reqMsg.dds_switch_type_valid = true;
  reqMsg.dds_switch_type = switch_type;

  if (mQmiSvcClient != nullptr)
  {

    int rc = qmi_client_send_msg_sync(mQmiSvcClient,
                                      QMI_DSD_SWITCH_DDS_REQ_V01,
                                      (void *)&reqMsg,
                                      sizeof(reqMsg),
                                      (void *)&respMsg,
                                      sizeof(respMsg),
                                      QCRIL_DATA_QMI_TIMEOUT);
    if (rc != QMI_NO_ERR) {
      Log::getInstance().d("[DSDModemEndPointModule] Failed to send"
        "QMI_DSD_SWITCH_DDS_REQ_V01 with rc =" + std::to_string(rc));
    } else
    {
      Log::getInstance().d("[DSDModemEndPointModule] qmiDDSSwitchRequest::Successfully sent"
        "QMI_DSD_SWITCH_DDS_REQ_V01");
    }
  }
  return respMsg;
}

#ifdef QMI_RIL_UTF
void DSDModemEndPointModule::cleanUpQmiSvcClient()
{
  ModemEndPointModule::cleanUpQmiSvcClient();
  usingSystemStatusV2 = false;
  usingUiInfoV2 = false;
  ratPrefReceived = false;
  static_cast<DSDModemEndPoint *>(&mOwner)->cleanup();
}
#endif
