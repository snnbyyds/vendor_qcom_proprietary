/**
* Copyright (c) 2017-2022 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#include <iostream>
#include <cstdlib>
#include "dlfcn.h"
#ifdef RIL_FOR_MDM_LE
#include <signal.h>
#include <time.h>
#include "request/CaptureLogBufferMessage.h"
#include "request/IndicationRegistrationFilterMessage.h"
#endif
#include "qmi_client.h"
#include "wireless_data_service_v01.h"

/* Framework includes */
#include "framework/Dispatcher.h"
#include "framework/Looper.h"
#include "framework/ModuleLooper.h"
#include "framework/QcrilInitMessage.h"
#include "framework/TimeKeeper.h"
#include "modules/android/ril_message_factory.h"
#include "modules/qmi/QmiIndMessage.h"
#include "modules/qmi/EndpointStatusIndMessage.h"
#include "modules/qmi/ModemEndPointFactory.h"
#include "modules/qmi/QmiSetupRequestCallback.h"
#include "qtibus/Messenger.h"
#include "data_system_determination_v01.h"

/* Module includes */
#include "local/DataModule.h"
#include "UnSolMessages/DataInitMessage.h"
#include "request/RilRequestSetupDataCallMessage.h"
#include "request/RilRequestGetDataCallProfileMessage.h"
#include "qcril_reqlist.h"

#ifndef RIL_FOR_LOW_RAM
#include "request/RilRequestDataCallListMessage.h"
#include "request/RilRequestDeactivateDataCallMessage.h"
#include "request/RilRequestEmbmsActivateDeactivateTmgiMessage.h"
#include "request/RilRequestEmbmsActivateTmgiMessage.h"
#include "request/RilRequestEmbmsContentDescUpdateMessage.h"
#include "request/RilRequestEmbmsDeactivateTmgiMessage.h"
#include "request/EmbmsEnableDataReqMessage.h"
#include "request/RilRequestEmbmsGetActiveTmgiMessage.h"
#include "request/RilRequestEmbmsGetAvailTmgiMessage.h"
#include "request/RilRequestEmbmsSendIntTmgiListMessage.h"
#include "request/RilRequestSetDataProfileMessage.h"
#include "sync/RilDataEmbmsActiveMessage.h"
#include "request/RilRequestLastDataCallFailCauseMessage.h"
#include "request/RilRequestPullLceDataMessage.h"
#include "request/RilRequestStartLceMessage.h"
#include "request/RilRequestStopLceMessage.h"
#include "request/RilRequestStartKeepaliveMessage.h"
#include "request/RilRequestStopKeepaliveMessage.h"
#include "request/IWLANCapabilityHandshake.h"
#include "request/GetAllQualifiedNetworkRequestMessage.h"
#include "UnSolMessages/IntentToChangeApnPreferredSystemMessage.h"
#include "request/GetIWlanDataCallListRequestMessage.h"
#include "request/GetIWlanDataRegistrationStateRequestMessage.h"
#include "UnSolMessages/HandoverInformationIndMessage.h"
#include "UnSolMessages/VoiceIndMessage.h"
#include "UnSolMessages/DataAllBearerTypeChangedMessage.h"
#include "UnSolMessages/DataBearerTypeChangedMessage.h"
#include "UnSolMessages/VoiceCallOrigTimeoutMessage.h"
#include "VoiceCallModemEndPoint.h"
#ifdef USE_QCRIL_HAL
#include "modules/nas/NasPhysChanConfigMessage.h"
#else
#include "interfaces/nas/NasPhysChanConfigMessage.h"
#endif
#endif
#include "UnSolMessages/DataNasPsAttachDetachResponse.h"
#include "UnSolMessages/DetachAttachIndTimeoutMessage.h"

#include "request/SetApnInfoMessage.h"
#include "request/SetIsDataEnabledMessage.h"
#include "request/SetIsDataRoamingEnabledMessage.h"
#include "request/RilRequestGoDormantMessage.h"
#include "request/ProcessScreenStateChangeMessage.h"
#include "request/ProcessStackSwitchMessage.h"
#include "request/SetLteAttachProfileMessage.h"
#include "request/SetQualityMeasurementMessage.h"
#include "request/ToggleDormancyIndMessage.h"
#include "request/ToggleLimitedSysIndMessage.h"
#include "request/UpdateMtuMessage.h"
#include "request/GetDdsSubIdMessage.h"
#include "request/RequestDdsSwitchMessage.h"
#include "request/StartKeepAliveRequestMessage.h"
#include "request/StopKeepAliveRequestMessage.h"

#include "request/RilRequestSetCarrierInfoImsiEncryptionMessage.h"
#include "request/SetLinkCapRptCriteriaMessage.h"
#include "request/SetLinkCapFilterMessage.h"
#include "request/SetInitialAttachApnRequestMessage.h"
#include "sync/SetInitialAttachCompletedMessage.h"
#include "request/RilRequestSetInitialAttachApnRequestMessage.h"
#include "request/RegisterBearerAllocationUpdateRequestMessage.h"
#include "request/GetBearerAllocationRequestMessage.h"
#include "request/GetAllBearerAllocationsRequestMessage.h"
#include "request/SetupDataCallRequestMessage.h"
#include "request/DeactivateDataCallRequestMessage.h"
#include "request/GetRadioDataCallListRequestMessage.h"

#include "request/SetCarrierInfoImsiEncryptionMessage.h"

#include "event/RilEventDataCallback.h"
#include "event/LinkPropertiesChangedMessage.h"
#include "request/SetDataProfileRequestMessage.h"
#include "request/SetPreferredDataModemRequestMessage.h"
#include "request/StartLCERequestMessage.h"
#include "request/StopLCERequestMessage.h"
#include "request/PullLCEDataRequestMessage.h"

#include "event/DataGetMccMncCallback.h"
#include "UnSolMessages/SetRadioClientCapUnSolMessage.h"
#include "UnSolMessages/SetLteAttachPdnListActionResultMessage.h"
#include "UnSolMessages/SetApnPreferredSystemResultMessage.h"
#include "UnSolMessages/GetModemAttachParamsRetryMessage.h"

#include "UnSolMessages/DsdSystemStatusPerApnMessage.h"
#include "UnSolMessages/CallStatusMessage.h"
#include "UnSolMessages/CurrentDDSIndMessage.h"
#include "UnSolMessages/CurrentRoamingStatusChangedMessage.h"
#include "UnSolMessages/DDSSwitchResultIndMessage.h"
#include "UnSolMessages/DDSSwitchIPCMessage.h"
#include "UnSolMessages/IAInfoIPCMessage.h"
#include "UnSolMessages/DDSSwitchTimeoutMessage.h"
#include "UnSolMessages/RadioConfigClientConnectedMessage.h"
#include "UnSolMessages/DataCallTimerExpiredMessage.h"
#include "UnSolMessages/NewDDSInfoMessage.h"
#include "UnSolMessages/KeepAliveIndMessage.h"
#include "UnSolMessages/PDCRefreshIndication.h"
#include "DataConfig.h"
#include "UnSolMessages/DsdSystemStatusMessage.h"
#include "UnSolMessages/DsiInitCompletedMessage.h"
#include "UnSolMessages/NasRfBandInfoIndMessage.h"
#include "UnSolMessages/QosInitializeMessage.h"
#include "request/GetQosDataRequestMessage.h"
#include "DataConfig.h"
#include "legacy/qcril_data_qmi_wds.h"
#include "legacy/qcril_data.h"

#include "AuthModemEndPoint.h"
#include "WDSModemEndPoint.h"
#include "DSDModemEndPoint.h"

#include "ProfileHandler.h"
#include "NetworkServiceHandler.h"

#include "telephony/ril.h"
#include "modules/nas/NasSrvDomainPrefIndMessage.h"
#include "modules/nas/NasRequestDataShutdownMessage.h"

#include "modules/android/ClientDisconnectedMessage.h"
#include "modules/android/ClientConnectedMessage.h"

#ifndef QCRIL_RIL_VERSION
#error "undefined QCRIL_RIL_VERSION!"
#endif

#define INVALID_MSG_TOKEN -1
#define PROPERTY_DISABLE_PARTIAL_RETRY_DEFAULT  "false"

extern pthread_mutex_t ddsSubMutex;

#include "modules/uim/UimCardAppStatusIndMsg.h"
#include "modules/uim/UimGetMccMncRequestMsg.h"
#include "modules/uim/UimGetCardStatusRequestSyncMsg.h"

#define MCC_LENGTH 4
#define MNC_LENGTH 4

static load_module<rildata::DataModule> dataModule;
RilRespParams ProfileHandler::m_resp_params={ QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, 0,0};
extern DDSSubIdInfo currentDDSSUB;

namespace rildata {

DataModule& getDataModule() {
    return dataModule.get_module();
}
#ifndef RIL_FOR_LOW_RAM
bool clearTimeoutForMessage( std::shared_ptr<Message> msg );
#endif

#ifdef RIL_FOR_MDM_LE

int SignalList[] = {SIGBUS, SIGABRT, SIGSEGV};

RIL_Errno handleRilDataDump(string name) {
  time_t system_time = time(0);
  char timestamp[10] = "";
  strftime (timestamp, 10,"%H.%M.%S", localtime(&system_time));
  std::string dumpsys_file_name = "Ril_Data_Dump_" + name + "_" + std::string(timestamp) + ".txt";
  FILE * dumpsys_file = fopen(dumpsys_file_name.c_str(), "w");
  if (dumpsys_file == nullptr)
  {
    fprintf(stderr, "Error cannot open dumpsys file.\n");
    return RIL_E_INTERNAL_ERR;
  }
  fprintf(dumpsys_file, "TimeStamp: %s", asctime(localtime(&system_time)));
  if (name != "CLI_USER") {
    fprintf(dumpsys_file, "Signal Received: %s\n" , name.c_str());
  } else {
    fprintf(dumpsys_file, "Trigerred Via: %s\n" , name.c_str());
  }
  if (fflush(dumpsys_file) != 0) {
    Log::getInstance().d("[DataModule]: Error in file flushing for Data Dump");
  }
  dump_data_module(fileno(dumpsys_file));
  if (dumpsys_file) {
    if (fclose(dumpsys_file) != 0) {
       Log::getInstance().d("[DataModule]: Error in file closing for Data Dump");
    }
    dumpsys_file = nullptr;
  }
  return RIL_E_SUCCESS;
}

void handleSignal(int sig) {
    std::cout << "\nDaemon is Crashing." << std::endl;
    string name = "";
    if (sig == SIGABRT) {
        name = "SIGABRT";
    }
    else if (sig == SIGBUS) {
        name = "SIGBUS";
    }
    else if (sig == SIGSEGV) {
        name = "SIGSEGV";
    }
    std::cout << "\nSignal Received: " + name << std::endl;
    handleRilDataDump(name);
    _exit(EXIT_SUCCESS);
}
#endif


DataModule::DataModule() : mMessageList("DataModule")  {
  mDataQosInit = nullptr;
#ifdef RIL_FOR_MDM_LE
  struct sigaction action = {};
  action.sa_handler = &handleSignal;
  for (int i : SignalList) {
    if (sigaction(i, &action, NULL) != 0) {
        Log::getInstance().d("[DataModule]: Signal Handler Registration Failed for Signal" + std::to_string(i));
    }
  }
#endif
  attachSerialList.clear();
  mName = "DataModule";
  mLooper = std::unique_ptr<ModuleLooper>(new ModuleLooper);
  iwlanHandshakeMsgToken = INVALID_MSG_TOKEN;
  wds_endpoint = ModemEndPointFactory<WDSModemEndPoint>::getInstance().buildEndPoint();
  dsd_endpoint = ModemEndPointFactory<DSDModemEndPoint>::getInstance().buildEndPoint();
  auth_endpoint = ModemEndPointFactory<AuthModemEndPoint>::getInstance().buildEndPoint();
  pdc_endpoint = ModemEndPointFactory<PDCModemEndPoint>::getInstance().buildEndPoint();
  #ifdef FEATURE_DATA_LQE
    ott_endpoint = ModemEndPointFactory<OTTModemEndPoint>::getInstance().buildEndPoint();
  #endif /* FEATURE_DATA_LQE */
  preferred_data_sm = std::make_unique<PreferredDataStateMachine>();
  preferred_data_state_info = std::make_shared<PreferredDataInfo_t>();
  if(preferred_data_state_info != nullptr)
  {
    preferred_data_state_info->isRilIpcNotifier = false;
  }
  else
  {
    Log::getInstance().d("[DataModule]: preferred_data_state_info is NULL");
  }
  preferred_data_state_info->mVoiceCallInfo.voiceSubId = SubId::UNSPECIFIED;
  preferred_data_sm->initialize(preferred_data_state_info);
  preferred_data_state_info->pendingIACb = std::make_shared<std::function<void(std::shared_ptr<Message>)>>(std::bind(&DataModule::processInitialAttachRequestMessage, this, std::placeholders::_1));
  preferred_data_state_info->iaSendResponse = std::make_shared<std::function<void(std::shared_ptr<Message>)>>(std::bind(&DataModule::handleSetInitialAttachCompleted, this, std::placeholders::_1));
  preferred_data_sm->setCurrentState(Initial);

  auth_manager = nullptr;
  call_manager = nullptr;
  profile_handler = nullptr;
  network_service_handler = nullptr;
  memset(&mCachedSystemStatus, 0, sizeof(dsd_system_status_ind_msg_v01));
  keep_alive = std::make_shared<KeepAliveHandler>();
  #ifndef RIL_FOR_LOW_RAM
  voiceCallEndPointSub0 = nullptr;
  voiceCallEndPointSub1 = nullptr;
  #endif

#if !defined(RIL_FOR_LOW_RAM) || defined(RIL_FOR_MDM_LE)
  networkavailability_handler = nullptr;
#endif
  using std::placeholders::_1;

  mMessageHandler = {
#ifndef RIL_FOR_LOW_RAM
                      {RilRequestDeactivateDataCallMessage::get_class_message_id(), std::bind(&DataModule::handleRilRequestDeactivateDataCallMessage, this, _1)},
                      {RilRequestEmbmsActivateDeactivateTmgiMessage::get_class_message_id(), std::bind(&DataModule::handleEmbmsActivateDeactivateTmgiMessage, this, _1)},
                      {RilRequestEmbmsActivateTmgiMessage::get_class_message_id(), std::bind(&DataModule::handleEmbmsActivateTmgiMessage, this, _1)},
                      {RilRequestEmbmsContentDescUpdateMessage::get_class_message_id(), std::bind(&DataModule::handleEmbmsContentDescUpdateMessage, this, _1)},
                      {RilRequestEmbmsDeactivateTmgiMessage::get_class_message_id(), std::bind(&DataModule::handleEmbmsDeactivateTmgiMessage, this, _1)},
                      {EmbmsEnableDataReqMessage::get_class_message_id(), std::bind(&DataModule::handleEmbmsEnableDataReqMessage, this, _1)},
                      {RilRequestEmbmsGetActiveTmgiMessage::get_class_message_id(), std::bind(&DataModule::handleEmbmsGetActiveTmgiMessage, this, _1)},
                      {RilRequestEmbmsGetAvailTmgiMessage::get_class_message_id(), std::bind(&DataModule::handleEmbmsGetAvailTmgiMessage, this, _1)},
                      {RilRequestEmbmsSendIntTmgiListMessage::get_class_message_id(), std::bind(&DataModule::handleEmbmsSendIntTmgiListMessage, this, _1)},
                      {RilRequestLastDataCallFailCauseMessage::get_class_message_id(), std::bind(&DataModule::handleLastDataCallFailCauseMessage, this, _1)},
                      {RilRequestSetupDataCallMessage::get_class_message_id(), std::bind(&DataModule::handleRilRequestSetupDataCallMessage, this, _1)},
                      {RilDataEmbmsActiveMessage::get_class_message_id(), std::bind(&DataModule::handleDataEmbmsActiveMessage, this, _1)},
                      {VoiceIndMessage::get_class_message_id(), std::bind(&DataModule::handleVoiceIndMessage, this, _1)},
                      {VoiceCallOrigTimeoutMessage::get_class_message_id(), std::bind(&DataModule::handleVoiceCallOrigTimeoutMessage, this, _1)},
                      #ifdef QMI_RIL_UTF
                      {REG_MSG("VOICECALL_ENDPOINT_STATUS_IND"), std::bind(&DataModule::handleQmiVoiceEndpointStatusIndMessage, this, _1)},
                      #else
                      {REG_MSG("VOICE_ENDPOINT_STATUS_IND"), std::bind(&DataModule::handleQmiVoiceEndpointStatusIndMessage, this, _1)},
                      #endif
                      {UimCardStatusIndMsg::get_class_message_id(), std::bind(&DataModule::handleUimCardStatusIndMsg, this, _1)},
                      {RilRequestPullLceDataMessage::get_class_message_id(), std::bind(&DataModule::handlePullLceDataMessage, this, _1)},
                      {RilRequestStartLceMessage::get_class_message_id(), std::bind(&DataModule::handleStartLceMessage, this, _1)},
                      {RilRequestStopLceMessage::get_class_message_id(), std::bind(&DataModule::handleStopLceMessage, this, _1)},
                      {RilRequestDataCallListMessage::get_class_message_id(), std::bind(&DataModule::handleDataCallListMessage, this, _1)},
                      {DataBearerTypeChangedMessage::get_class_message_id(), std::bind(&DataModule::handleDataBearerTypeUpdate, this, _1)},
                      {DataAllBearerTypeChangedMessage::get_class_message_id(), std::bind(&DataModule::handleDataAllBearerTypeUpdate, this, _1)},
                      {RegisterBearerAllocationUpdateRequestMessage::get_class_message_id(), std::bind(&DataModule::handleToggleBearerAllocationUpdate, this, _1)},
                      {GetBearerAllocationRequestMessage::get_class_message_id(), std::bind(&DataModule::handleGetBearerAllocation, this, _1)},
                      {GetAllBearerAllocationsRequestMessage::get_class_message_id(), std::bind(&DataModule::handleGetAllBearerAllocations, this, _1)},
                      {IWLANCapabilityHandshake::get_class_message_id(), std::bind(&DataModule::handleIWLANCapabilityHandshake, this, _1)},
                      {GetAllQualifiedNetworkRequestMessage::get_class_message_id(), std::bind(&DataModule::handleGetAllQualifiedNetworksMessage, this, _1)},
                      {IntentToChangeApnPreferredSystemMessage::get_class_message_id(), std::bind(&DataModule::handleIntentToChangeInd, this, _1)},
                      {GetIWlanDataCallListRequestMessage::get_class_message_id(), std::bind(&DataModule::handleGetIWlanDataCallListRequestMessage, this, _1)},
                      {GetIWlanDataRegistrationStateRequestMessage::get_class_message_id(), std::bind(&DataModule::handleGetIWlanDataRegistrationStateRequestMessage, this, _1)},
                      {SetApnPreferredSystemResultMessage::get_class_message_id(), std::bind(&DataModule::handleSetApnPreferredSystemResult, this, _1)},
                      {HandoverInformationIndMessage::get_class_message_id(), std::bind(&DataModule::handleHandoverInformationIndMessage, this, _1)},
                      {NasPhysChanConfigReportingStatus::get_class_message_id(), std::bind(&DataModule::handleNasPhysChanConfigReportingStatusMessage, this, _1)},
                      {NasPhysChanConfigMessage::get_class_message_id(), std::bind(&DataModule::handleNasPhysChanConfigMessage, this, _1)},
                      {DsdSystemStatusPerApnMessage::get_class_message_id(), std::bind(&DataModule::handleDsdSystemStatusPerApn, this, _1)},
#if (QCRIL_RIL_VERSION >= 15)
                      {RilRequestSetDataProfileMessage_v15::get_class_message_id(), std::bind(&DataModule::handleSetDataProfileMessage_v15, this, _1)},
                      {RilRequestStartKeepaliveMessage::get_class_message_id(), std::bind(&DataModule::handleStartKeepaliveMessage, this, _1)},
                      {RilRequestStopKeepaliveMessage::get_class_message_id(), std::bind(&DataModule::handleStopKeepaliveMessage, this, _1)},
#else
                      {RilRequestSetDataProfileMessage::get_class_message_id(), std::bind(&DataModule::handleRilRequestSetDataProfileMessage, this, _1)},
#endif //QCRIL_RIL_VERSION
#endif //RIL_FOR_LOW_RAM
                      {RilRequestGetDataCallProfileMessage::get_class_message_id(), std::bind(&DataModule::handleGetDataCallProfileMessage, this, _1)},
                      {QcrilInitMessage::get_class_message_id(), std::bind(&DataModule::handleQcrilInitMessage, this, _1)},
                      {RilRequestGoDormantMessage::get_class_message_id(), std::bind(&DataModule::handleGoDormantMessage, this, _1)},
                      {ProcessScreenStateChangeMessage::get_class_message_id(), std::bind(&DataModule::handleProcessScreenStateChangeMessage, this, _1)},
                      {ProcessStackSwitchMessage::get_class_message_id(), std::bind(&DataModule::handleProcessStackSwitchMessage, this, _1)},
                      {SetApnInfoMessage::get_class_message_id(), std::bind(&DataModule::handleSetApnInfoMessage, this, _1)},
                      {SetIsDataEnabledMessage::get_class_message_id(), std::bind(&DataModule::handleSetIsDataEnabledMessage, this, _1)},
                      {SetIsDataRoamingEnabledMessage::get_class_message_id(), std::bind(&DataModule::handleSetIsDataRoamingEnabledMessage, this, _1)},
                      {SetQualityMeasurementMessage::get_class_message_id(), std::bind(&DataModule::handleSetQualityMeasurementMessage, this, _1)},
                      {ToggleDormancyIndMessage::get_class_message_id(), std::bind(&DataModule::handleToggleDormancyIndMessage, this, _1)},
                      {ToggleLimitedSysIndMessage::get_class_message_id(), std::bind(&DataModule::handleToggleLimitedSysIndMessage, this, _1)},
                      {UpdateMtuMessage::get_class_message_id(), std::bind(&DataModule::handleUpdateMtuMessage, this, _1)},
                      {GetDdsSubIdMessage::get_class_message_id(), std::bind(&DataModule::handleGetDdsSubIdMessage, this, _1)},
                      {RequestDdsSwitchMessage::get_class_message_id(), std::bind(&DataModule::handleDataRequestDDSSwitchMessage, this, _1)},
#if (QCRIL_RIL_VERSION >= 15)
                      {SetLteAttachProfileMessage_v15::get_class_message_id(), std::bind(&DataModule::handleSetLteAttachProfileMessage_v15, this, _1)},
                      {RilRequestSetCarrierInfoImsiEncryptionMessage::get_class_message_id(), std::bind(&DataModule::handleRilRequestSetCarrierInfoImsiEncryptionMessage, this, _1)},
                      {REG_MSG("AUTH_QMI_IND"), std::bind(&DataModule::handleQmiAuthServiceIndMessage, this, _1)},
                      {REG_MSG("AUTH_ENDPOINT_STATUS_IND"), std::bind(&DataModule::handleQmiAuthEndpointStatusIndMessage, this, _1)},
                      {SetLinkCapFilterMessage::get_class_message_id(), std::bind(&DataModule::handleSetLinkCapFilterMessage, this, _1)},
                      {SetLinkCapRptCriteriaMessage::get_class_message_id(), std::bind(&DataModule::handleSetLinkCapRptCriteriaMessage, this, _1)},
                      {SetDataProfileRequestMessage::get_class_message_id(), std::bind(&DataModule::handleSetDataProfileRequestMessage, this, _1)},
                      {KeepAliveIndMessage::get_class_message_id(), std::bind(&DataModule::handleKeepAliveIndMessage, this, _1)},
                      {StartKeepAliveRequestMessage::get_class_message_id(), std::bind(&DataModule::handleStartKeepAliveRequestMessage, this, _1)},
                      {StopKeepAliveRequestMessage::get_class_message_id(), std::bind(&DataModule::handleStopKeepAliveRequestMessage, this, _1)},
#else
                      {SetLteAttachProfileMessage::get_class_message_id(), std::bind(&DataModule::handleSetLteAttachProfileMessage, this, _1)},
#endif /* QCRIL_RIL_VERSION >= 15 */
                      {RilRequestSetInitialAttachApnRequestMessage::get_class_message_id(), std::bind(&DataModule::handleRilRequestSetInitialAttachApn, this, _1)},
                      {SetInitialAttachApnRequestMessage::get_class_message_id(), std::bind(&DataModule::handleSetInitialAttachApn, this, _1)},
                      {SetInitialAttachCompletedMessage::get_class_message_id(), std::bind(&DataModule::handleSetInitialAttachCompleted, this, _1)},
                      {GetModemAttachParamsRetryMessage::get_class_message_id(), std::bind(&DataModule::handleGetModemAttachParamsRetryMessage, this, _1)},
                      {SetLteAttachPdnListActionResultMessage::get_class_message_id(), std::bind(&DataModule::handleSetLteAttachPdnListActionResult, this, _1)},
                      {NasSrvDomainPrefIndMessage::get_class_message_id(), std::bind(&DataModule::handleNasSrvDomainPrefInd, this, _1)},
                      {DataNasPsAttachDetachResponse::get_class_message_id(), std::bind(&DataModule::handleDataNasPsAttachDetachResponse, this, _1)},
                      {DetachAttachIndTimeoutMessage::get_class_message_id(), std::bind(&DataModule::handleDetachAttachIndTimeoutMessage, this, _1)},
                      {NasRequestDataShutdownMessage::get_class_message_id(), std::bind(&DataModule::handleNasRequestDataShutdown, this, _1)},
                      {REG_MSG("WDSModemEndPoint_ENDPOINT_STATUS_IND"), std::bind(&DataModule::handleQmiWdsEndpointStatusIndMessage, this, _1)},
                      {SetPreferredDataModemRequestMessage::get_class_message_id(), std::bind(&DataModule::handleSetPreferredDataModem, this, _1)},
                      {CurrentDDSIndMessage::get_class_message_id(), std::bind(&DataModule::handleCurrentDDSIndMessage, this, _1)},
                      {CurrentRoamingStatusChangedMessage::get_class_message_id(), std::bind(&DataModule::handleCurrentRoamingStatusChangedMessage, this, _1)},
                      {DDSSwitchResultIndMessage::get_class_message_id(), std::bind(&DataModule::handleDDSSwitchResultIndMessage, this, _1)},
                      {RadioConfigClientConnectedMessage::get_class_message_id(), std::bind(&DataModule::handleRadioConfigClientConnectedMessage, this, _1)},
                      {DDSSwitchTimeoutMessage::get_class_message_id(), std::bind(&DataModule::handleDDSSwitchTimeoutMessage, this, _1)},
                      {DDSSwitchIPCMessage::get_class_message_id(), std::bind(&DataModule::handleDDSSwitchIPCMessage, this, _1)},
                      {IAInfoIPCMessage::get_class_message_id(), std::bind(&DataModule::handleIAInfoIPCMessage, this, _1)},
                      {CallStatusMessage::get_class_message_id(), std::bind(&DataModule::handleDataConnectionStateChangedMessage, this, _1)},
                      {REG_MSG("DSDModemEndPoint_ENDPOINT_STATUS_IND"), std::bind(&DataModule::handleQmiDsdEndpointStatusIndMessage, this, _1)},
                      {DsdSystemStatusMessage::get_class_message_id(), std::bind(&DataModule::handleDsdSystemStatusInd, this, _1)},
                      {RilEventDataCallback::get_class_message_id(), std::bind(&DataModule::handleRilEventDataCallback, this, _1)},
                      {SetupDataCallRequestMessage::get_class_message_id(), std::bind(&DataModule::handleSetupDataCallRequestMessage, this, _1)},
                      {DeactivateDataCallRequestMessage::get_class_message_id(), std::bind(&DataModule::handleDeactivateDataCallRequestMessage, this, _1)},
                      {GetRadioDataCallListRequestMessage::get_class_message_id(), std::bind(&DataModule::handleGetRadioDataCallListRequestMessage, this, _1)},
                      {DataCallTimerExpiredMessage::get_class_message_id(), std::bind(&DataModule::handleDataCallTimerExpiredMessage, this, _1)},
                      {SetRadioClientCapUnSolMessage::get_class_message_id(), std::bind(&DataModule::handleSetRadioClientCapUnSolMessage, this, _1)},
                      {UimCardAppStatusIndMsg::get_class_message_id(), std::bind(&DataModule::handleUimCardAppStatusIndMsg, this, _1)},
                      #ifdef FEATURE_DATA_LQE
                        {REG_MSG("OTTModemEndPoint_ENDPOINT_STATUS_IND"), std::bind(&DataModule::handleQmiOttEndpointStatusIndMessage, this, _1)},
                      #endif /* FEATURE_DATA_LQE */
                      {LinkPropertiesChangedMessage::get_class_message_id(), std::bind(&DataModule::handleLinkPropertiesChangedMessage, this, _1)},
                      {SetCarrierInfoImsiEncryptionMessage::get_class_message_id(), std::bind(&DataModule::handleSetCarrierInfoImsiEncryptionMessage, this, _1)},
                      {RegistrationFailureReportingStatusMessage::get_class_message_id(), std::bind(&DataModule::handleRegistrationFailureReportingStatusMessage, this, _1)},
                      {StartLCERequestMessage::get_class_message_id(), std::bind(&DataModule::handleStartLCERequestMessage, this, _1)},
                      {StopLCERequestMessage::get_class_message_id(), std::bind(&DataModule::handleStopLCERequestMessage, this, _1)},
                      {PullLCEDataRequestMessage::get_class_message_id(), std::bind(&DataModule::handlePullLCEDataRequestMessage, this, _1)},
                      {DsiInitCompletedMessage::get_class_message_id(), std::bind(&DataModule::handleDsiInitCompletedMessage, this, _1)},
                      {ClientDisconnectedMessage::get_class_message_id(), std::bind(&DataModule::handleClientDisconnectedMessage, this, _1)},
                      {ClientConnectedMessage::get_class_message_id(), std::bind(&DataModule::handleClientConnectedMessage, this, _1)},
                      {PDCRefreshIndication::get_class_message_id(), std::bind(&DataModule::handlePDCRefreshIndication, this, _1)},
                      {REG_MSG("PDCModemEndPoint_ENDPOINT_STATUS_IND"), std::bind(&DataModule::handleQmiPDCEndpointStatusIndMessage, this, _1)},
#ifdef RIL_FOR_MDM_LE
                      {CaptureLogBufferMessage::get_class_message_id(), std::bind(&DataModule::handleCaptureLogBufferMessage, this, _1)},
                      {IndicationRegistrationFilterMessage::get_class_message_id(), std::bind(&DataModule::handleIndicationRegistrationFilterMessage, this, _1)},
#endif
                      {NasRfBandInfoIndMessage::get_class_message_id(), std::bind(&DataModule::handleNasRfBandInfoIndMessage, this, _1)},
                      {NasRfBandInfoMessage::get_class_message_id(), std::bind(&DataModule::handleNasRfBandInfoMessage, this, _1)},
                      {QosInitializeMessage::get_class_message_id(), std::bind(&DataModule::handleQosInitMessage, this, _1)},
                      {GetQosDataRequestMessage::get_class_message_id(), std::bind(&DataModule::handleGetQosDataRequestMessage, this, _1)},
                    };

  //Read system property to determine IWLAN mode
  //TODO:: change default value to "1" or legacy mode before initial checkin
  //TODO:: change default value to "0" or default mode when we have code in place
  //to determine Radio hal version
  string propValue = readProperty("ro.telephony.iwlan_operation_mode", "default");
  Log::getInstance().d("[" + mName + "]: IWLAN mode system property is " + propValue);

  if(propValue.compare("default") == 0) {
    propValue = "0";
  }
  else if(propValue.compare("legacy") == 0) {
    propValue = "1";
  }
  else if(propValue.compare("AP-assisted") == 0) {
    propValue = "2";
  }
  else {
    Log::getInstance().d("ro.telephony.iwlan_operation_mode is configured with ["+
                          propValue + "], configuring it to default mode");
    propValue = "0";
  }
#ifdef QMI_RIL_UTF
  mInitTracker.setIWLANMode(rildata::IWLANOperationMode::AP_ASSISTED);
#else
  mInitTracker.setIWLANMode((rildata::IWLANOperationMode)atoi(propValue.c_str()));
#endif
}

DataModule::~DataModule() {
  unloadDataQos();
  mLooper = nullptr;
  //mDsdEndPoint = nullptr;
}

PendingMessageList& DataModule::getPendingMessageList() {
  return mMessageList;
}

void DataModule::init() {
  /* Call base init before doing any other stuff.*/
  Module::init();
}

void DataModule::broadcastReady()
{
    std::shared_ptr<DataInitMessage> data_init_msg =
                       std::make_shared<DataInitMessage>(global_instance_id);
    Dispatcher::getInstance().broadcast(data_init_msg);
}

void DataModule::dump(int fd)
{
  std::ostringstream ss;
  if (call_manager != nullptr) {
    call_manager->dump("", ss);
  }
  ss << "NetworkAvailabilityHandler:" << endl;
#ifndef RIL_FOR_LOW_RAM
  if (networkavailability_handler != nullptr) {
    networkavailability_handler->dump("    ", ss);
  }
  ss << endl;
#endif
  ss << "NetworkServiceHandler:" << endl;
  if (network_service_handler != nullptr) {
    network_service_handler->dump("    ", ss);
  }
  ss << endl;
  ss << "InitTracker:" << endl;
  mInitTracker.dump("    ", ss);
  ss << "    IWlanHandshakeMsgToken=" << iwlanHandshakeMsgToken << endl;
  ss << "    CurrentDDS=" << currentDDSSUB.dds_sub_id << endl;
  ss << "    SwitchType=" << currentDDSSUB.switch_type << endl;
  ss << endl;
  ss << "ProfileHandler:" << endl;
  if (profile_handler != nullptr) {
    profile_handler->dump("    ", ss);
  }

  ss << endl << "Logs:" << endl;
  vector<string> logs = networkAvailabilityLogBuffer.getLogs();
  ss << "NetworkAvailability:" << endl;
  for (const string& msg: logs) {
    ss << msg << endl;
  }
  ss << endl << "DataModule:" << endl;
  logs = logBuffer.getLogs();
  for (const string& msg: logs) {
    ss << msg << endl;
  }
  LocalLogBuffer::toFd(ss.str(), fd);
}

void DataModule::flush() {
  logBuffer.flush();
  networkAvailabilityLogBuffer.flush();
}

void DataModule::handleClientConnectedMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());
  std::shared_ptr<ClientConnectedMessage> m = std::static_pointer_cast<ClientConnectedMessage>(msg);
  if( m != NULL ) {
    if( mInitCompleted && call_manager) {
      call_manager->dataCallListChanged();
    }
  }
}
void DataModule::handleClientDisconnectedMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());
  std::shared_ptr<ClientDisconnectedMessage> m = std::static_pointer_cast<ClientDisconnectedMessage>(msg);
  if( m != NULL ) {
    if( mInitCompleted && call_manager) {
      Log::getInstance().d("[DataModule]: Clean All Calls ");
      call_manager->cleanAllCalls();
    }
  }
}

void DataModule::handleQcrilInitMessage(std::shared_ptr<Message> msg)
{
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
#ifdef QMI_RIL_UTF
  usleep(500000);
#endif
  auto qcril_init_msg = std::static_pointer_cast<QcrilInitMessage>(msg);
  if(qcril_init_msg)
  {
    #ifdef RIL_FOR_MDM_LE
    propertySetMap("persist.vendor.radio.disable_retry",  PERSIST_VENDOR_RADIO_DISABLE_RETRY);
    #endif
    string propValue = readProperty("persist.vendor.radio.disable_retry", PROPERTY_DISABLE_PARTIAL_RETRY_DEFAULT);
    Log::getInstance().d("[" + mName + "]: Partial retry disabled property is " + propValue);
    std::transform( propValue.begin(), propValue.end(), propValue.begin(),
                  [](char c) -> char { return std::tolower(c); });
    if ("true" == propValue) {
        mInitTracker.setPartialRetryEnabled(false);
    }
    global_instance_id = qcril_init_msg->get_instance_id();
    logBuffer.setName("RIL" + std::to_string(global_instance_id));

    QmiSetupRequestCallback callback("data-token");
#ifdef RIL_FOR_DYNAMIC_LOADING
    qcril_instance_id_e_type inst_id = static_cast<qcril_instance_id_e_type>(global_instance_id);
    dsd_endpoint->requestSetup("data-token-client-server", inst_id, &callback);

    wds_endpoint->requestSetup("data-token-client-server", inst_id, &callback);

    auth_endpoint->requestSetup("data-token-client-server", inst_id, &callback);

    pdc_endpoint->requestSetup("data-token-client-server", inst_id, &callback);

    #ifdef FEATURE_DATA_LQE
      ott_endpoint->requestSetup("data-token-client-server", inst_id, &callback);
    #endif
#else
    dsd_endpoint->requestSetup("data-token-client-server", &callback);

    wds_endpoint->requestSetup("data-token-client-server", &callback);

    auth_endpoint->requestSetup("data-token-client-server", &callback);

    pdc_endpoint->requestSetup("data-token-client-server", &callback);

    #ifdef FEATURE_DATA_LQE
      ott_endpoint->requestSetup("data-token-client-server", &callback);
    #endif /* FEATURE_DATA_LQE */
#endif
    broadcastReady();
    Log::getInstance().d("[" + mName + "]: Done msg = " + msg->dump());
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received. =" + msg->dump() + "QCRIL DATA Init not triggered!!!");
  }
}

void DataModule::handleQmiPDCEndpointStatusIndMessage(std::shared_ptr<Message> msg)
{
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  auto m = std::static_pointer_cast<EndpointStatusIndMessage>(msg);
  if(m != nullptr)
  {
    if((m->getState() == ModemEndPoint::State::OPERATIONAL) && pdc_endpoint != nullptr)
    {
      pdc_endpoint->registerForPDCIndication();
    }
    else {
      mIsPdcRefreshInProgress = FALSE;
      Log::getInstance().d("[" + mName + "]: Resetting pdc refresh in progress on receiving modem service down.");
    }
  }
  else
  {
    Log::getInstance().d("[" + mName + "]: Received pdc indication for another sub. Ignoring");
  }
}

void DataModule::handlePDCRefreshIndication(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  auto m = std::static_pointer_cast<PDCRefreshIndication>(msg);
  if(m != nullptr)
  {
      if(m->getEvent() == PDC_EVENT_REFRESH_START_V01)
      {
          mIsPdcRefreshInProgress = TRUE;
          Log::getInstance().d("["+mName + "]: pdc refresh started");
      }
      else if(m->getEvent() == PDC_EVENT_REFRESH_COMPLETE_V01)
      {
          mIsPdcRefreshInProgress = FALSE;
          Log::getInstance().d("["+mName + "]: pdc refresh Completed");
      }
  }
  else
  {
    Log::getInstance().d("[" + mName + "]: Improper message received.");
  }
}

#ifdef RIL_FOR_MDM_LE
void DataModule::handleCaptureLogBufferMessage(std::shared_ptr<Message> msg) {
    if (msg != nullptr) {
        auto m = std::static_pointer_cast<CaptureLogBufferMessage>(msg);
        if (m == nullptr) {
            return;
        }
        RIL_Errno res = handleRilDataDump("CLI_USER");
        auto resp = make_shared<RIL_Errno>(res);
        m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
    }
}
void DataModule::handleIndicationRegistrationFilterMessage(std::shared_ptr<Message> msg) {
    if (msg != nullptr) {
        auto m = std::static_pointer_cast<IndicationRegistrationFilterMessage>(msg);
        if (m == nullptr) {
            return;
        }
        Log::getInstance().d("[" + mName + "]: Processing msg = " + m->dump());
        bool result = true;
        if (dsd_endpoint) {
            bool check = dsd_endpoint->UIChangeInfoRegistrationRequest((m->getFilterValue() & UI_INFO_REGISTRATION_FLAG) != 0);
            result = result & check;
        }
        else {
            Log::getInstance().d("Dsd Endpoint is not Initialized");
            result = false;
        }
        int res = -1;
        if (result == true) {
            res = 0;
        }
        auto resp = make_shared<int>(res);
        m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
    }
    else {
        Log::getInstance().d("Msg is Nullptr");
    }
}
#endif

void DataModule::handleGetDataCallProfileMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestGetDataCallProfileMessage> m = std::static_pointer_cast<RilRequestGetDataCallProfileMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_request_omh_profile_info(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleProcessStackSwitchMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<ProcessStackSwitchMessage> m = std::static_pointer_cast<ProcessStackSwitchMessage>(msg);
  if( m != NULL ) {
    ProcessStackSwitchMessage::StackSwitchReq info = m->getParams();
    qcril_data_process_stack_switch( info.oldStackId, info.newStackId, info.instanceId);
    auto resp = std::make_shared<int>(QCRIL_DS_SUCCESS);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

#ifndef RIL_FOR_LOW_RAM
void DataModule::handleDataCallListMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  qcril_request_return_type ret;
  auto m = std::static_pointer_cast<RilRequestDataCallListMessage>(msg);
  if( m != NULL ) {
    qcril_data_request_data_call_list(&(m->get_params()), &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}
void DataModule::handleRilRequestDeactivateDataCallMessage(std::shared_ptr<Message> msg) {

  if( msg == NULL ) {
    Log::getInstance().d("[" + mName + "]: ERROR!!! Msg received is NULL");
    return;
  }
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());

  // Cancel the default timer
  clearTimeoutForMessage(msg);

  //Add a new timeout handler
  setTimeoutForMsg(msg, msg->getMessageExpiryTimer());

  std::shared_ptr<RilRequestDeactivateDataCallMessage> m = std::static_pointer_cast<RilRequestDeactivateDataCallMessage>(msg);

  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_request_deactivate_data_call(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received");
  }
}

void DataModule::handleEmbmsActivateDeactivateTmgiMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestEmbmsActivateDeactivateTmgiMessage> m = std::static_pointer_cast<RilRequestEmbmsActivateDeactivateTmgiMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    call_manager->handleEmbmsActivateDeactivateTmgi(&req);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleEmbmsActivateTmgiMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestEmbmsActivateTmgiMessage> m = std::static_pointer_cast<RilRequestEmbmsActivateTmgiMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    call_manager->handleEmbmsActivateTmgi(&req);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleEmbmsContentDescUpdateMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestEmbmsContentDescUpdateMessage> m = std::static_pointer_cast<RilRequestEmbmsContentDescUpdateMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    call_manager->handleEmbmsContentDescUpdate(&req);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleEmbmsDeactivateTmgiMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestEmbmsDeactivateTmgiMessage> m = std::static_pointer_cast<RilRequestEmbmsDeactivateTmgiMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    call_manager->handleEmbmsDeactivateTmgi(&req);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleEmbmsEnableDataReqMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<EmbmsEnableDataReqMessage> m = std::static_pointer_cast<EmbmsEnableDataReqMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    call_manager->handleEmbmsEnableDataRequestMessage(req.instance_id);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleEmbmsGetActiveTmgiMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestEmbmsGetActiveTmgiMessage> m = std::static_pointer_cast<RilRequestEmbmsGetActiveTmgiMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    call_manager->handleEmbmsGetActiveTmgi(&req);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleEmbmsGetAvailTmgiMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestEmbmsGetAvailTmgiMessage> m = std::static_pointer_cast<RilRequestEmbmsGetAvailTmgiMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    call_manager->handleEmbmsGetAvailableTmgi(&req);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleEmbmsSendIntTmgiListMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestEmbmsSendIntTmgiListMessage> m = std::static_pointer_cast<RilRequestEmbmsSendIntTmgiListMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    call_manager->handleEmbmsSendInterestedList(&req);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleLastDataCallFailCauseMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestLastDataCallFailCauseMessage> m = std::static_pointer_cast<RilRequestLastDataCallFailCauseMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_request_last_data_call_fail_cause(&req,&ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

#if (QCRIL_RIL_VERSION >= 15)
void DataModule::handleSetDataProfileMessage_v15(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestSetDataProfileMessage_v15> m = std::static_pointer_cast<RilRequestSetDataProfileMessage_v15>(msg);
  if ( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_request_set_data_profile(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleStartKeepaliveMessage(std::shared_ptr<Message> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    std::shared_ptr<RilRequestStartKeepaliveMessage> m = std::static_pointer_cast<RilRequestStartKeepaliveMessage>(msg);
    if( m != NULL) {
      qcril_request_params_type req = m->get_params();
      qcril_request_return_type ret;
      qcril_data_start_modem_assist_keepalive(&req, &ret);
    } else {
      Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
    }
}

void DataModule::handleStopKeepaliveMessage(std::shared_ptr<Message> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    std::shared_ptr<RilRequestStopKeepaliveMessage> m = std::static_pointer_cast<RilRequestStopKeepaliveMessage>(msg);
    if( m != NULL ) {
      qcril_request_params_type req = m->get_params();
      qcril_request_return_type ret;
      qcril_data_stop_modem_assist_keepalive(&req, &ret);
    } else {
      Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
    }
}
#else
void DataModule::handleRilRequestSetDataProfileMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestSetDataProfileMessage> m = std::static_pointer_cast<RilRequestSetDataProfileMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_request_set_data_profile(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}
#endif //(QCRIL_RIL_VERSION >= 15)

void DataModule::handleRilRequestSetupDataCallMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  std::shared_ptr<RilRequestSetupDataCallMessage> m = std::static_pointer_cast<RilRequestSetupDataCallMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_request_setup_data_call(&req, &ret);
    // We don't return response here. It will be sent upon
    // success/failure at a later point in time.
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handlePullLceDataMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestPullLceDataMessage> m = std::static_pointer_cast<RilRequestPullLceDataMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_lqe_get_info(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleStartLceMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestStartLceMessage> m = std::static_pointer_cast<RilRequestStartLceMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_lqe_start(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleStopLceMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestStopLceMessage> m = std::static_pointer_cast<RilRequestStopLceMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    qcril_request_return_type ret;
    qcril_data_lqe_stop(&req, &ret);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleDataEmbmsActiveMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilDataEmbmsActiveMessage> m = std::static_pointer_cast<RilDataEmbmsActiveMessage>(msg);
  if( m != NULL && call_manager != NULL ) {
    int is_Embms_Active = call_manager->getEmbmsCallInfo();
    auto resp = std::make_shared<bool>(is_Embms_Active);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleUimCardStatusIndMsg(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: handling msg = " + msg->dump());
  std::shared_ptr<UimCardStatusIndMsg> m = std::static_pointer_cast<UimCardStatusIndMsg>(msg);
  if( m != NULL ) {
    if((m->get_card_status()).status == QCRIL_CARD_STATUS_UP || (m->get_card_status()).status == QCRIL_CARD_STATUS_SIM_READY)
    {
      Log::getInstance().d("[" + mName + "]: Sim is present in the device");
      mSimPresent = TRUE;
    }
    else if((m->get_card_status()).status == QCRIL_CARD_STATUS_DOWN || (m->get_card_status()).status == QCRIL_CARD_STATUS_ABSENT)
    {
      Log::getInstance().d("[" + mName + "]: Sim is removed");
      mSimPresent = FALSE;
    }
  } else {
      Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleQmiVoiceEndpointStatusIndMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  auto shared_indMsg(std::static_pointer_cast<EndpointStatusIndMessage>(msg));

  if (shared_indMsg->getState() == ModemEndPoint::State::NON_OPERATIONAL) {
    Log::getInstance().d("[" + mName + "]: Resetting switch type to Permanent as modem is down");
    preferred_data_state_info->switch_type = DSD_DDS_SWITCH_PERMANENT_V01;
    //resetting the timer
    if (voiceCallEndPointSub0 != nullptr && voiceCallEndPointSub0->mIsVoiceCallOrigTimer) {
      voiceCallEndPointSub0->mIsVoiceCallOrigTimer = FALSE;
      TimeKeeper::getInstance().clear_timer(voiceCallEndPointSub0->mVoiceCallOrigTimer);
    } else if (voiceCallEndPointSub1 != nullptr && voiceCallEndPointSub1->mIsVoiceCallOrigTimer) {
      voiceCallEndPointSub1->mIsVoiceCallOrigTimer = FALSE;
      TimeKeeper::getInstance().clear_timer(voiceCallEndPointSub1->mVoiceCallOrigTimer);
    }
    if (TempddsSwitchRequestPending) {
      TimeKeeper::getInstance().clear_timer(tempDDSSwitchRequestTimer);
    }

    PreferredDataEventType voiceServiceDown;
    voiceServiceDown.event = VoiceServiceDown;
    voiceServiceDown.data = nullptr;
    preferred_data_sm->processEvent(voiceServiceDown);
  }
}

void DataModule::handleVoiceIndMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<VoiceIndMessage> m = std::static_pointer_cast<VoiceIndMessage>(msg);
  if(m != NULL && preferred_data_state_info != nullptr)
  {

    std::vector<VoiceIndResp> voiceInfo = m->getParam();

    /*If there are multiple calls then set the switchtype to tempdds
      when there is at least one call which is not in call_end state*/
    for(auto it = voiceInfo.begin(); it!= voiceInfo.end(); it++)
    {
      if((*it).voiceCallType == VoiceCallTypeEnum::CALL_TYPE_EMERGENCY && !mSimPresent)
      {
        preferred_data_state_info->switch_type = DSD_DDS_SWITCH_PERMANENT_V01;
        Log::getInstance().d("[" + mName + "]: Received voice ind for SIMLess emergency call");
        return;
      }

      if(((*it).voiceSubId) == SubId::PRIMARY && voiceCallEndPointSub0 != nullptr && voiceCallEndPointSub0->mIsVoiceCallOrigTimer)
      {
        voiceCallEndPointSub0->mIsVoiceCallOrigTimer = FALSE;
        TimeKeeper::getInstance().clear_timer(voiceCallEndPointSub0->mVoiceCallOrigTimer);
      }
      else if(((*it).voiceSubId) == SubId::SECONDARY && voiceCallEndPointSub1 != nullptr && voiceCallEndPointSub1->mIsVoiceCallOrigTimer)
      {
        voiceCallEndPointSub1->mIsVoiceCallOrigTimer = FALSE;
        TimeKeeper::getInstance().clear_timer(voiceCallEndPointSub1->mVoiceCallOrigTimer);
      }

      if(((int)((*it).voiceSubId) != preferred_data_state_info->dds || preferred_data_state_info->switch_type == DSD_DDS_SWITCH_TEMPORARY_V01) &&
          (*it).voiceCallState != VoiceCallStateEnum::CALL_STATE_END)
      {
        preferred_data_state_info->switch_type = DSD_DDS_SWITCH_TEMPORARY_V01;
        Log::getInstance().d("Temporary DDS switch on sub" + std::to_string((int)(*it).voiceSubId));

        if((*it).voiceCallState == VoiceCallStateEnum::CALL_STATE_CC_IN_PROGRESS)
        {
          //set the voiceCallOrigTimer
          if((*it).voiceSubId == SubId::PRIMARY)
          {
            voiceCallEndPointSub0->mIsVoiceCallOrigTimer = TRUE;
            voiceCallEndPointSub0->mVoiceCallOrigTimer = TimeKeeper::getInstance().set_timer(std::bind(&DataModule::handleVoiceCallOrigTimerExpired, this, std::placeholders::_1), nullptr, VOICEIND_WAITING_TIMEOUT);
          }
          else
          {
            voiceCallEndPointSub1->mIsVoiceCallOrigTimer = TRUE;
            voiceCallEndPointSub1->mVoiceCallOrigTimer = TimeKeeper::getInstance().set_timer(std::bind(&DataModule::handleVoiceCallOrigTimerExpired, this, std::placeholders::_1), nullptr, VOICEIND_WAITING_TIMEOUT);
          }
        }
        preferred_data_state_info->mVoiceCallInfo.voiceSubId = (*it).voiceSubId;
        preferred_data_state_info->mVoiceCallInfo.voiceCallState = (*it).voiceCallState;
        break;
      }
      if(std::next(it) == voiceInfo.end() && preferred_data_state_info->switch_type == DSD_DDS_SWITCH_TEMPORARY_V01)
      {
        //not setting switch_type to permanent if there is a call on another dds sub with call_state_hold
        if(((*it).voiceCallState == VoiceCallStateEnum::CALL_STATE_END) &&
           (((*it).voiceSubId == SubId::PRIMARY && (voiceCallEndPointSub1->isVoiceCallInActiveState())) ||
            ((*it).voiceSubId == SubId::SECONDARY && (voiceCallEndPointSub0->isVoiceCallInActiveState()))))
        {
          Log::getInstance().d("Active call on another sub");
          return;
        }
        /*If no voicecall found on nondds sub with callstate != call_end, set the switch type to permanent*/
        preferred_data_state_info->switch_type = DSD_DDS_SWITCH_PERMANENT_V01;
        Log::getInstance().d("Permanent DDS switch on sub" + std::to_string((int)preferred_data_state_info->mVoiceCallInfo.voiceSubId));

        if((*it).voiceCallState == VoiceCallStateEnum::CALL_STATE_END && ((int)((*it).voiceSubId) == preferred_data_state_info->dds))
        {
          TempddsSwitchRequestPending = true;
          tempDDSSwitchRequestTimer = TimeKeeper::getInstance().set_timer(std::bind(&DataModule::handleTempDDSSwitchTimerExpired, this, std::placeholders::_1), nullptr, TempDDS_SWITCH_REQUEST_TIMEOUT);
        }
      }
    }
  }
}

void DataModule::handleTempDDSSwitchTimerExpired(void *) {
  Log::getInstance().d("Datamodule: onTempDDSSwitchRequestExpired ENTRY");
  DDSTimeOutSwitchType type = DDSTimeOutSwitchType::TempDDSTimeOutSwitch;
  auto msg = std::make_shared<DDSSwitchTimeoutMessage>(type);
  msg->broadcast();
}

void DataModule::handleVoiceCallOrigTimerExpired(void *) {
  Log::getInstance().d("DataModule: handleVoiceCallOrigTimerExpired");
  auto msg = std::make_shared<VoiceCallOrigTimeoutMessage>();
  msg->broadcast();
}

void DataModule::handleVoiceCallOrigTimeoutMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("DataModule: handleVoiceCallOrigTimeoutMessage");
  std::shared_ptr<VoiceCallOrigTimeoutMessage> m = std::static_pointer_cast<VoiceCallOrigTimeoutMessage>(msg);
  if( m != nullptr)
  {
    bool isCallActive = false;
    if(preferred_data_state_info->mVoiceCallInfo.voiceSubId == SubId::PRIMARY)
    {
      voiceCallEndPointSub0->mIsVoiceCallOrigTimer = FALSE;
      isCallActive = voiceCallEndPointSub0->isVoiceCall();
      if(!isCallActive || preferred_data_state_info->mVoiceCallInfo.voiceCallState == VoiceCallStateEnum::CALL_STATE_CC_IN_PROGRESS)
      {
        preferred_data_state_info->switch_type = DSD_DDS_SWITCH_PERMANENT_V01;
      }
    } else {
      voiceCallEndPointSub1->mIsVoiceCallOrigTimer = FALSE;
      isCallActive = voiceCallEndPointSub1->isVoiceCall();
      if(!isCallActive || preferred_data_state_info->mVoiceCallInfo.voiceCallState == VoiceCallStateEnum::CALL_STATE_CC_IN_PROGRESS)
      {
        preferred_data_state_info->switch_type = DSD_DDS_SWITCH_PERMANENT_V01;
      }
    }
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

bool DataModule::loadDatactl() {
  bool ret = false;
  void *datactlLib;
  do {
    datactlLib = dlopen("libqcrildatactl.so", RTLD_LAZY);
    if( NULL == datactlLib ) {
      Log::getInstance().d("[" + mName + "]: Unable to load libqcrildatactl.so");
      break;
    }
    dlerror();
    dcSymbols.datactlInit = (datactlInitFnPtr)dlsym(datactlLib, "datactlInit");
    const char *dlsym_error = dlerror();
    if( dlsym_error ) {
      Log::getInstance().d("[" + mName + "]: Cannot find datactlInit symbol");
      break;
    }
    dcSymbols.datactlEnableIWlan = (datactlEnableIWlanFnPtr)dlsym(datactlLib, "datactlEnableIWlan");
    dlsym_error = dlerror();
    if( dlsym_error ) {
      Log::getInstance().d("[" + mName + "]: Cannot find datactlEnableIWlan symbol");
      break;
    }
    dcSymbols.datactlDisableIWlan = (datactlDisableIWlanFnPtr)dlsym(datactlLib, "datactlDisableIWlan");
    dlsym_error = dlerror();
    if( dlsym_error ) {
      Log::getInstance().d("[" + mName + "]: Cannot find datactlDisableIWlan symbol");
      break;
    }
    //Successfully reached the end
    ret = true;
  }
  while( 0 );
  if( ret == false ) {
    if( datactlLib ) {
      dlclose(datactlLib);
      memset(&dcSymbols, 0, sizeof(dcSymbols)); //Reset all fn ptrs to 0
    }
  }
  return ret;
}

//Perform intializations that are common to services being initialized, mode
//being determined and IWLAN being enabled
void DataModule::initializeIWLAN() {

    if(iwlanHandshakeMsgToken != INVALID_MSG_TOKEN) {
      Log::getInstance().d("[DataModule] Look for IWLANCapabilityHandshake msg "+
                            std::to_string((int)iwlanHandshakeMsgToken));
      getDataModule().getPendingMessageList().print();
      std::shared_ptr<Message> mmm = getDataModule().getPendingMessageList().find((uint16_t)iwlanHandshakeMsgToken);
      if(mmm!=nullptr) {
        std::shared_ptr<IWLANCapabilityHandshake> iwlanHsMsg = std::static_pointer_cast<IWLANCapabilityHandshake>(mmm);
        if(iwlanHsMsg != nullptr)
        {
          auto resp = std::make_shared<rildata::ModemIWLANCapability_t>(
                                              mInitTracker.getModemCapability() ?
                                              rildata::ModemIWLANCapability_t::present:
                                              rildata::ModemIWLANCapability_t::not_present);
          iwlanHsMsg->sendResponse(mmm, Message::Callback::Status::SUCCESS, resp);
          getDataModule().getPendingMessageList().erase(mmm);
          getDataModule().getPendingMessageList().print();
          iwlanHandshakeMsgToken = INVALID_MSG_TOKEN;
        } else {
          Log::getInstance().d("[DataModule] Invalid IWLANCapabilityHandShake msg. Not sending response");
        }
      }
      else {
        Log::getInstance().d("[DataModule] No IWLANCapabilityHandshake msg found");
        iwlanHandshakeMsgToken = INVALID_MSG_TOKEN;
      }
    }
    else {
      Log::getInstance().d("[DataModule] Invalid IWLANCapabilityHandshake msg token");
    }

    Message::Callback::Status status = dsd_endpoint->registerForAPAsstIWlanIndsSync(true);
    Log::getInstance().d("[DataModule] registerForIndications requst"
                          " result = "+ std::to_string((int) status));

    networkavailability_handler = std::make_unique<NetworkAvailabilityHandler>(networkAvailabilityLogBuffer);
    if (mCachedSystemStatus.apn_avail_sys_info_valid) {
      networkavailability_handler->processQmiDsdSystemStatusInd(mCachedSystemStatus.apn_avail_sys_info,
                                                            mCachedSystemStatus.apn_avail_sys_info_len);
    }

#ifndef QMI_RIL_UTF
    //send message to datactl to enable IWLAN
    if (dcSymbols.datactlEnableIWlan) dcSymbols.datactlEnableIWlan();
#endif
}

void DataModule::deinitializeIWLAN() {

    Message::Callback::Status status = dsd_endpoint->registerForAPAsstIWlanIndsSync(false);
    if (status != Message::Callback::Status::SUCCESS)
    {
       Log::getInstance().d("[DataModule] deregisterForIndications requst failed,"
                          " result = "+ std::to_string((int) status));
    }
    else
    {
       Log::getInstance().d("[DataModule] deregisterForIndications request successful"
                                     ", result = "+ std::to_string((int) status));
    }

    networkavailability_handler.reset();
    networkavailability_handler = nullptr;

    //Send message on datactl to disable IWLAN
    if (dcSymbols.datactlDisableIWlan) dcSymbols.datactlDisableIWlan();
}

/*===========================================================================

  FUNCTION:  handleDataAllBearerTypeUpdate

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when DataBearerTypeChangedMessage is received

    @return
*/
/*=========================================================================*/
void DataModule::handleDataAllBearerTypeUpdate(std::shared_ptr<Message> msg)
{
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());
  std::shared_ptr<DataAllBearerTypeChangedMessage> m = std::static_pointer_cast<DataAllBearerTypeChangedMessage>(msg);
  if (m != NULL)
  {
    AllocatedBearer_t bearerInfo = m->getBearerInfo();
    if (call_manager) {
      call_manager->handleDataAllBearerTypeUpdate(bearerInfo);
    }
    else {
      Log::getInstance().d("call_manager is null");
    }
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

/*===========================================================================

  FUNCTION:  handleDataBearerTypeUpdate

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when DataBearerTypeChangedMessage is received

    @return
*/
/*=========================================================================*/
void DataModule::handleDataBearerTypeUpdate(std::shared_ptr<Message> msg)
{
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());
  std::shared_ptr<DataBearerTypeChangedMessage> m = std::static_pointer_cast<DataBearerTypeChangedMessage>(msg);
  if (m != NULL)
  {
    int32_t cid = m->getCid();
    BearerInfo_t bearer = m->getBearerInfo();
    if (call_manager) {
      call_manager->handleDataBearerTypeUpdate(cid, bearer);
    }
    else {
      Log::getInstance().d("call_manager is null");
    }
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

/*===========================================================================

  FUNCTION:  handleToggleBearerAllocationUpdate

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when RegisterBearerAllocationUpdateRequestMessage is received

    @return
*/
/*=========================================================================*/
void DataModule::handleToggleBearerAllocationUpdate(std::shared_ptr<Message> msg)
{
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());

  std::shared_ptr<RegisterBearerAllocationUpdateRequestMessage> m =
    std::static_pointer_cast<RegisterBearerAllocationUpdateRequestMessage>(msg);
  if (m != NULL)
  {
    bool enable = m->getToggleSwitch();
    ResponseError_t ret;
    if (call_manager) {
      ret = call_manager->handleToggleBearerAllocationUpdate(enable);
    }
    else {
      Log::getInstance().d("call_manager is null");
      ret = ResponseError_t::INTERNAL_ERROR;
    }
    auto resp = std::make_shared<ResponseError_t>(ret);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

/*===========================================================================

  FUNCTION:  handleGetBearerAllocation

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when GetBearerAllocationRequestMessage is received
    Invokes the callback with the allocated bearers that were retrieved

    @return
*/
/*=========================================================================*/
void DataModule::handleGetBearerAllocation(std::shared_ptr<Message> msg)
{
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());

  std::shared_ptr<GetBearerAllocationRequestMessage> m =
    std::static_pointer_cast<GetBearerAllocationRequestMessage>(msg);
  if (m != NULL)
  {
    int32_t cid = m->getCallId();
    Message::Callback::Status status = Message::Callback::Status::SUCCESS;
    AllocatedBearerResult_t bearerAllocations;
    if (call_manager) {
      bearerAllocations = call_manager->handleGetBearerAllocation(cid);
    }
    else {
      Log::getInstance().d("call_manager is null");
      bearerAllocations.error = ResponseError_t::INTERNAL_ERROR;
    }
    auto resp = std::make_shared<AllocatedBearerResult_t>(bearerAllocations);
    m->sendResponse(msg, status, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}
/*===========================================================================

  FUNCTION:  handleGetAllBearerAllocations

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when GetAllBearerAllocationsRequestMessage is received
    Invokes the callback with the allocated bearers that were retrieved

    @return
*/
/*=========================================================================*/
void DataModule::handleGetAllBearerAllocations(std::shared_ptr<Message> msg)
{
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());

  auto m = std::static_pointer_cast<GetAllBearerAllocationsRequestMessage>(msg);
  if (m != NULL)
  {
    Message::Callback::Status status = Message::Callback::Status::SUCCESS;
    AllocatedBearerResult_t bearerAllocations;
    if (call_manager) {
      bearerAllocations = call_manager->handleGetAllBearerAllocations();
    }
    else {
      Log::getInstance().d("call_manager is null");
      bearerAllocations.error = ResponseError_t::INTERNAL_ERROR;
    }
    auto resp = std::make_shared<AllocatedBearerResult_t>(bearerAllocations);
    m->sendResponse(msg, status, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}
/*===========================================================================

  FUNCTION:  handleIWLANCapabilityHandshake

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when IWLANCapabilityHandshake is received.

    @return
*/
/*=========================================================================*/
void DataModule::handleIWLANCapabilityHandshake(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());
  std::shared_ptr<IWLANCapabilityHandshake> m = std::static_pointer_cast<IWLANCapabilityHandshake>(msg);
  if (m != NULL) {
    Log::getInstance().d("[ getPendingMessageList test ]: insert message = " +
                         msg->get_message_name());
    std::pair<uint16_t, bool> result = getDataModule().getPendingMessageList().insert(msg);
    iwlanHandshakeMsgToken = (int32_t)result.first;
    Log::getInstance().d("[ getPendingMessageList test ]: insert result token = " +
                         std::to_string((int)iwlanHandshakeMsgToken));
    getDataModule().getPendingMessageList().print();
    if (m->isIWLANEnabled()) {
      mInitTracker.setIWLANEnabled(true);
      if (mInitTracker.allServicesReady() && mInitTracker.isModeDetermined()) {
        initializeIWLAN();
      }
    } else {
      mInitTracker.setIWLANEnabled(false); //TODO: handled disabled
      deinitializeIWLAN();
    }
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

/*===========================================================================

  FUNCTION:  handleGetAllQualifiedNetworksMessage

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when GetAllQualifiedNetworkRequestMessage is received.

    @return
*/
/*=========================================================================*/
void DataModule::handleGetAllQualifiedNetworksMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());
  std::shared_ptr<GetAllQualifiedNetworkRequestMessage> m = std::static_pointer_cast<GetAllQualifiedNetworkRequestMessage>(msg);
  if (m != NULL) {
    Message::Callback::Status status = Message::Callback::Status::SUCCESS;
    QualifiedNetworkResult_t result;

    if (!mInitTracker.isAPAssistMode()) {
      result.respErr = ResponseError_t::NOT_SUPPORTED;
    } else if (!mInitTracker.isIWLANEnabled() || !mInitTracker.allServicesReady()) {
      result.respErr = ResponseError_t::NOT_AVAILABLE;
    } else {
      if (networkavailability_handler) {
        result.respErr = ResponseError_t::NO_ERROR;
        networkavailability_handler->getQualifiedNetworks(result.qualifiedNetwork);
      }
      else {
        Log::getInstance().d("networkavailability_handler is null");
        result.respErr = ResponseError_t::INTERNAL_ERROR;
      }
    }
    auto resp = std::make_shared<QualifiedNetworkResult_t>(result);
    m->sendResponse(msg, status, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}
void DataModule::handleDsdSystemStatusPerApn(std::shared_ptr<Message> msg)
{
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());
  std::shared_ptr<DsdSystemStatusPerApnMessage> m = std::static_pointer_cast<DsdSystemStatusPerApnMessage>(msg);
  if (m != NULL)
  {
    if ( networkavailability_handler ) {
      auto apnAvailSys = m->getAvailSys();
      networkavailability_handler->processQmiDsdSystemStatusInd(apnAvailSys.data(), apnAvailSys.size());
    }
  }
  else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

/*===========================================================================

  FUNCTION:  handleIntentToChangeInd

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when IntentToChangeApnPreferredSystemMessage is received.

    @return
*/
/*=========================================================================*/
void DataModule::handleIntentToChangeInd(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());
  std::shared_ptr<IntentToChangeApnPreferredSystemMessage> m =
     std::static_pointer_cast<IntentToChangeApnPreferredSystemMessage>(msg);
  if (m != NULL) {
    dsd_intent_to_change_apn_pref_sys_ind_msg_v01 intent = m->getParams();

    dsd_intent_to_change_apn_pref_sys_ind_msg_v01 intentToChangeApnPrefSysInd;
    memset(&intentToChangeApnPrefSysInd, 0, sizeof(intentToChangeApnPrefSysInd));

    for(uint32_t i=0 ; i<intent.apn_pref_sys_len ; i++) {
      string apnName(intent.apn_pref_sys[i].apn_pref_info.apn_name);
      if(intent.apn_status_valid && call_manager!=nullptr &&
        (intent.apn_status[i] & (DSD_APN_PREF_SYS_APN_STATUS_WWAN_CONNECTED_V01 |
                                 DSD_APN_PREF_SYS_APN_STATUS_IWLAN_CONNECTED_V01))) {
        if(call_manager->shouldDeferIntentToChange(apnName)) {
          DeferredIntentToChange_t deferIntent;
          memset(&deferIntent, 0, sizeof(deferIntent));
          memcpy(&deferIntent.apnPrefSys, &intent.apn_pref_sys[i], sizeof(deferIntent.apnPrefSys));
          memcpy(&deferIntent.apnStatus, &intent.apn_status[i], sizeof(deferIntent.apnStatus));
          call_manager->deferToProcessIntentToChange(deferIntent);
        }
        else {
          memcpy(&intentToChangeApnPrefSysInd.apn_pref_sys[intentToChangeApnPrefSysInd.apn_pref_sys_len],
                 &intent.apn_pref_sys[i],
                 sizeof(dsd_apn_pref_sys_type_ex_v01));
          intentToChangeApnPrefSysInd.apn_pref_sys_len++;
          memcpy(&intentToChangeApnPrefSysInd.apn_status[intentToChangeApnPrefSysInd.apn_status_len],
                &intent.apn_pref_sys[i],
                sizeof(dsd_apn_pref_sys_apn_status_mask_v01));
          intentToChangeApnPrefSysInd.apn_status_len++;
        }
      }
      else {
        memcpy(&intentToChangeApnPrefSysInd.apn_pref_sys[intentToChangeApnPrefSysInd.apn_pref_sys_len],
                &intent.apn_pref_sys[i],
                sizeof(dsd_apn_pref_sys_type_ex_v01));
        intentToChangeApnPrefSysInd.apn_pref_sys_len++;
        if(intent.apn_status_valid) {
          memcpy(&intentToChangeApnPrefSysInd.apn_status[intentToChangeApnPrefSysInd.apn_status_len],
                  &intent.apn_pref_sys[i],
                  sizeof(dsd_apn_pref_sys_apn_status_mask_v01));
          intentToChangeApnPrefSysInd.apn_status_len++;
          intentToChangeApnPrefSysInd.apn_status_valid = true;
        }
      }
    }

    if (networkavailability_handler && intentToChangeApnPrefSysInd.apn_pref_sys_len > 0) {
      networkavailability_handler->processQmiDsdIntentToChangeApnPrefSysInd(&intentToChangeApnPrefSysInd);
    }
    if (call_manager && intentToChangeApnPrefSysInd.apn_pref_sys_len > 0) {
      call_manager->processQmiDsdIntentToChangeApnPrefSysInd(intentToChangeApnPrefSysInd);
    }
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}
void DataModule::handleGetIWlanDataCallListRequestMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  std::shared_ptr<GetIWlanDataCallListRequestMessage> m = std::static_pointer_cast<GetIWlanDataCallListRequestMessage>(msg);
  if (m != NULL) {
    Message::Callback::Status status = Message::Callback::Status::SUCCESS;
    DataCallListResult_t result;
    result.respErr = ResponseError_t::NO_ERROR;

    if (call_manager) {
      call_manager->getIWlanDataCallList(result.call);
    } else {
      Log::getInstance().d("call_manager is null");
      result.respErr = ResponseError_t::INTERNAL_ERROR;
    }
    auto resp = std::make_shared<DataCallListResult_t>(result);
    m->sendResponse(msg, status, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleGetIWlanDataRegistrationStateRequestMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<GetIWlanDataRegistrationStateRequestMessage> m = std::static_pointer_cast<GetIWlanDataRegistrationStateRequestMessage>(msg);

  if (m != NULL) {
    Message::Callback::Status status = Message::Callback::Status::SUCCESS;
    IWlanDataRegistrationStateResult_t result;
    result.respErr = ResponseError_t::NO_ERROR;
    result.regState = DataRegState_t::NOT_REG_AND_NOT_SEARCHING;
    result.reasonForDenial = 0;
    if (network_service_handler && mInitTracker.isAPAssistMode()) {
      result.regState = static_cast<ApAssistNetworkServiceHandler *>(network_service_handler.get())->getIWlanDataRegistrationState();
    }
    auto resp = std::make_shared<IWlanDataRegistrationStateResult_t>(result);
    m->sendResponse(msg, status, resp);
  }
}

void DataModule::handleSetApnPreferredSystemResult(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  std::shared_ptr<SetApnPreferredSystemResultMessage> m = std::static_pointer_cast<SetApnPreferredSystemResultMessage>(msg);
  if (m != NULL && call_manager != NULL) {
    Log::getInstance().d("[DataModule]::Invoking processQmiDsdApnPreferredSystemResultInd");
    call_manager->processQmiDsdApnPreferredSystemResultInd(&(m->getParams()));
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleHandoverInformationIndMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  std::shared_ptr<HandoverInformationIndMessage> m = std::static_pointer_cast<HandoverInformationIndMessage>(msg);
  if (call_manager != NULL) {
     call_manager->handleCallEventMessage(CallEventTypeEnum::HandoverInformationInd, msg);
  } else {
    Log::getInstance().d("No call_manager created");
  }
}

void DataModule::handleNasPhysChanConfigReportingStatusMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<NasPhysChanConfigReportingStatus> m = std::static_pointer_cast<NasPhysChanConfigReportingStatus>(msg);
  if (m != nullptr && call_manager != nullptr) {
    call_manager->enablePhysChanConfigReporting(m->isPhysChanConfigReportingEnabled());
  } else {
    Log::getInstance().d("No call_manager created");
  }
}

void DataModule::handleNasPhysChanConfigMessage(std::shared_ptr<Message> msg) {
  if (call_manager != nullptr) {
    call_manager->handleNasPhysChanConfigMessage(msg);
  } else {
    Log::getInstance().d("No call_manager created");
  }
}
/*===========================================================================

  FUNCTION:  setTimeoutForMsg

===========================================================================*/
/*!
    @brief
    API to set the timeout for message

    @return
    TimeKeeper::timer_id value
*/
/*=========================================================================*/
TimeKeeper::timer_id DataModule::setTimeoutForMsg
(
   std::shared_ptr<Message> msg, TimeKeeper::millisec maxTimeout
   ) {
  if (msg == NULL) {
    Log::getInstance().d("[" + mName + "]: ERROR!!! Msg received is NULL");
    return 0; /*'0' is the init value of timer_id parameter */
  }
  Log::getInstance().d("[DataModule: set timeout for " + msg->dump());

  TimeKeeper::timer_id tid = TimeKeeper::getInstance().set_timer(
                                                                 [this, msg](void *user_data){

    QCRIL_NOTUSED(user_data);

        if (!(msg->isCallbackExecuting() || msg->isCallbackExecuted()))
        {
          Log::getInstance().d("[DataModule:: Timer expired for " +
                                     msg->dump());
          msg->cancelling();
          Log::getInstance().d("[DataModule]: calling dispatcher inform fun");
          Dispatcher::getInstance().informMessageDispatchFailure(
              msg, Message::Callback::Status::TIMEOUT);

          Log::getInstance().d("Finished");
          msg->cancelled();

          deleteEntryInReqlist(msg);
        }
  },
  nullptr,
  maxTimeout);

  Log::getInstance().d("[" + msg->to_string() + "]: timer_id = " + std::to_string(tid));
  msg->setTimerId(tid);
  return tid;
}

/*===========================================================================

  FUNCTION:  deleteEntryInReqlist

===========================================================================*/
/*!
    @brief
    API to delete request from reqlist
===========================================================================*/
/*!
    @brief
    API to delete request from reqlist

    @return
    None
*/
/*=========================================================================*/
void DataModule::deleteEntryInReqlist
(
  std::shared_ptr<Message> msg
)
{
  /* Remove entry from reqlist */
  std::shared_ptr<RilRequestDeactivateDataCallMessage> m = std::static_pointer_cast<RilRequestDeactivateDataCallMessage>(msg);

  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();
    IxErrnoType reqlistErrno = qcril_data_reqlist_free(&req);

    if( reqlistErrno == E_SUCCESS)
    {
      Log::getInstance().d("qcril_data_deactivate_timeout_handle::Reqlist Free SUCCESS");
    }
    else
    {
      Log::getInstance().d("qcril_data_deactivate_timeout_handler::Reqlist Free failed!!! with Error code "+ std::to_string(reqlistErrno));
    }
  } else {
      Log::getInstance().d("[" + mName + "]: Message received is not DeactivateDataCall message!!!");
  }
}

/*===========================================================================

FUNCTION:  clearTimeoutForMsg

===========================================================================*/
/*!
  @brief
  API to clear the existing timeout for message

    @return
    TimeKeeper::timer_id value
*/
/*=========================================================================*/
bool clearTimeoutForMessage
(
  std::shared_ptr<Message> msg
)
{
  return TimeKeeper::getInstance().clear_timer(msg->getTimerId());
}
#endif //RIL_FOR_LOW_RAM

#if (!defined(RIL_FOR_LOW_RAM) || defined(RIL_FOR_MDM_LE))
void DataModule::processDeferredIntentToChange(DeferredIntentToChange_t deferIntent) {
  string apn(deferIntent.apnPrefSys.apn_pref_info.apn_name);
  Log::getInstance().d("[DataModule] processDeferredIntentToChange = " + apn);

  dsd_intent_to_change_apn_pref_sys_ind_msg_v01 intentToChangeApnPrefSysInd;
  intentToChangeApnPrefSysInd.apn_pref_sys_len = 1;
  memcpy(&intentToChangeApnPrefSysInd.apn_pref_sys[0], &deferIntent.apnPrefSys, sizeof(intentToChangeApnPrefSysInd.apn_pref_sys[0]));
  intentToChangeApnPrefSysInd.apn_status_valid = true;
  intentToChangeApnPrefSysInd.apn_status_len = 1;
  memcpy(&intentToChangeApnPrefSysInd.apn_status[0], &deferIntent.apnStatus, sizeof(intentToChangeApnPrefSysInd.apn_status[0]));

  if (getDataModule().networkavailability_handler) {
    getDataModule().networkavailability_handler->processQmiDsdIntentToChangeApnPrefSysInd(&intentToChangeApnPrefSysInd);
  }
  if (getDataModule().call_manager) {
    getDataModule().call_manager->processQmiDsdIntentToChangeApnPrefSysInd(intentToChangeApnPrefSysInd);
  }
}
#endif

void DataModule::handleStartLCERequestMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<StartLCERequestMessage> m = std::static_pointer_cast<StartLCERequestMessage>(msg);
  if( m != NULL ) {
    RIL_LceStatusInfo statusInfo;
    memset(&statusInfo,0,sizeof(RIL_LceStatusInfo));
    RIL_Errno err = qcril_data_start_lqe(m->getLCEinterval(),m->getLCEmode(),&statusInfo);
    Message::Callback::Status status = (err == RIL_E_SUCCESS ?
             Message::Callback::Status::SUCCESS : Message::Callback::Status::FAILURE);
    auto resp = std::make_shared<RIL_LceStatusInfo>(statusInfo);
    m->sendResponse(msg, status, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleStopLCERequestMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<StopLCERequestMessage> m = std::static_pointer_cast<StopLCERequestMessage>(msg);
  if( m != NULL ) {
    RIL_LceStatusInfo statusInfo;
    memset(&statusInfo,0,sizeof(RIL_LceStatusInfo));
    RIL_Errno err = qcril_data_stop_lqe(&statusInfo);
    Message::Callback::Status status = (err == RIL_E_SUCCESS ?
             Message::Callback::Status::SUCCESS : Message::Callback::Status::FAILURE);
    auto resp = std::make_shared<RIL_LceStatusInfo>(statusInfo);
    m->sendResponse(msg, status, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handlePullLCEDataRequestMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<PullLCEDataRequestMessage> m = std::static_pointer_cast<PullLCEDataRequestMessage>(msg);
  if( m != NULL ) {
    RIL_LceDataInfo dataInfo;
    memset(&dataInfo,0,sizeof(RIL_LceDataInfo));
    RIL_Errno err = qcril_data_get_lqe_info(&dataInfo);
    Message::Callback::Status status = (err == RIL_E_SUCCESS ?
             Message::Callback::Status::SUCCESS : Message::Callback::Status::FAILURE);
    auto resp = std::make_shared<RIL_LceDataInfo>(dataInfo);
    m->sendResponse(msg, status, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleGoDormantMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RilRequestGoDormantMessage> m = std::static_pointer_cast<RilRequestGoDormantMessage>(msg);
  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();

    qcril_request_resp_params_type resp;
    resp.instance_id        = req.instance_id;
    resp.t                  = req.t;
    resp.request_id         = req.event_id;
    resp.resp_pkt           = NULL;
    resp.resp_len           = 0;
    resp.logstr             = NULL;
    resp.rild_sock_oem_req  = 0;
    resp.ril_err_no         = RIL_E_SUCCESS;

    if(call_manager != nullptr)
    {
      bool status = call_manager->handleGoDormantRequest(req.data, req.datalen);
      if (status == false) {
        resp.ril_err_no = RIL_E_OEM_ERROR_3;
      }
      qcril_send_request_response(&resp);
    }
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleProcessScreenStateChangeMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());
  std::shared_ptr<ProcessScreenStateChangeMessage> m = std::static_pointer_cast<ProcessScreenStateChangeMessage>(msg);
  if( m != NULL ) {
    Message::Callback::Status status = Message::Callback::Status::FAILURE;
    int ret = QCRIL_DS_ERROR;
    if (dsd_endpoint) {
      Log::getInstance().d("[DataModule]: Sending Screen State information to DSD EndPoint");
      status = dsd_endpoint->handleScreenStateChangeInd(m->screenState);
      ret = (status == Message::Callback::Status::SUCCESS ?
            QCRIL_DS_SUCCESS : QCRIL_DS_ERROR);
    }
    auto resp = std::make_shared<int>(ret);
    m->sendResponse(msg, status, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleSetApnInfoMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<SetApnInfoMessage> m = std::static_pointer_cast<SetApnInfoMessage>(msg);
  if( m != NULL ) {
    Log::getInstance().d("[" + mName + "]: Legacy set_apn_info request is not supported");
    RIL_Errno ret = RIL_E_SUCCESS;
    auto resp = std::make_shared<RIL_Errno>(ret);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleSetIsDataEnabledMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<SetIsDataEnabledMessage> m = std::static_pointer_cast<SetIsDataEnabledMessage>(msg);
  if ( m != NULL ) {

#ifdef RIL_FOR_LOW_RAM
    bool is_data_enabled = m->getDataState();
    RIL_Errno ret = dsd_endpoint->setIsDataEnabled(is_data_enabled);
#else
    qcril_request_params_type req = m->get_params();
    RIL_Errno ret = qcril_data_set_is_data_enabled( &req, m->is_data_enabled);
#endif
    auto resp = std::make_shared<RIL_Errno>(ret);
    Message::Callback::Status status = (ret == RIL_E_SUCCESS ?
             Message::Callback::Status::SUCCESS : Message::Callback::Status::FAILURE);
    m->sendResponse(msg, status, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleSetIsDataRoamingEnabledMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<SetIsDataRoamingEnabledMessage> m = std::static_pointer_cast<SetIsDataRoamingEnabledMessage>(msg);
  if( m != NULL ) {

#ifdef RIL_FOR_LOW_RAM
    bool is_data_roaming_enabled = m->getDataRoamingState();
    RIL_Errno ret = dsd_endpoint->setIsDataRoamingEnabled(is_data_roaming_enabled);
#else
    qcril_request_params_type req = m->get_params();
    RIL_Errno ret = qcril_data_set_is_data_roaming_enabled(&req, m->is_data_roaming_enabled);
#endif
    auto resp = std::make_shared<RIL_Errno>(ret);
    Message::Callback::Status status = (ret == RIL_E_SUCCESS ?
             Message::Callback::Status::SUCCESS : Message::Callback::Status::FAILURE);
    m->sendResponse(msg, status, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

#if (QCRIL_RIL_VERSION >= 15)
void DataModule::handleSetLteAttachProfileMessage_v15(std::shared_ptr<Message> msg) {
  std::shared_ptr<SetLteAttachProfileMessage_v15> reqMsg = std::static_pointer_cast<SetLteAttachProfileMessage_v15>(msg);
  if( reqMsg != NULL ) {
    RIL_Errno ret = qcril_data_request_set_lte_attach_profile_v15 ( reqMsg->get_params() );
    auto resp = std::make_shared<RIL_Errno>(ret);
    reqMsg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleKeepAliveIndMessage(std::shared_ptr<Message> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    std::shared_ptr<KeepAliveIndMessage> m = std::static_pointer_cast<KeepAliveIndMessage>(msg);
    if( m!= NULL  && keep_alive) {
      KeepAliveEventType eve;
      keep_alive_ind ind;
      ind.status = m->get_status();
      ind.handle = m->get_handle();

      eve.event = KeepAliveInd;
      eve.data = &ind ;
      keep_alive->processEvent(eve);
    } else {
      Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
    }
}

void DataModule::cleanupKeepAlive(int cid_val) {
  Log::getInstance().d("[" + mName + "]: CleanupKeepAlive");
  if(keep_alive) {
    KeepAliveEventType eve;
    eve.event = KeepAliveCleanCallState;
    int cid = cid_val;
    eve.data = &cid;
    keep_alive->processEvent(eve);
  } else {
    Log::getInstance().d("keep_alive is nullptr ");
  }
}

void DataModule::handleStartKeepAliveRequestMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<StartKeepAliveRequestMessage> m = std::static_pointer_cast<StartKeepAliveRequestMessage>(msg);
  if( m!= NULL ) {
    KeepAliveEventType eve;
    KeepaliveRequest_t request = m->getKeepaliveRequest();
    keep_alive_start_request ka_start_req;
    if (call_manager) {
      std::string apn;
      if(call_manager->getApnByCid(request.cid, apn))
      {
        ka_start_req.apn = apn;
        ka_start_req.ril_ka_req = &request;
        eve.data = &ka_start_req;
      } else {
        Log::getInstance().d("There is no call with given cid " + std::to_string(request.cid));
        eve.data = nullptr;
      }
      eve.event = KeepAliveStartReq;
      eve.msg = msg;
      keep_alive->processEvent(eve);
    }
    else {
        Log::getInstance().d("call_manager is null");
        StartKeepAliveResp_t res = {};
        res.error = ResponseError_t::INTERNAL_ERROR;
        auto resp = std::make_shared<StartKeepAliveResp_t>(res);
        m->sendResponse(m, Message::Callback::Status::SUCCESS, resp);
    }
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleStopKeepAliveRequestMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<StopKeepAliveRequestMessage> m = std::static_pointer_cast<StopKeepAliveRequestMessage>(msg);
  if( m!= NULL && keep_alive ) {
    KeepAliveEventType eve;
    eve.event = KeepAliveStopReq;
    eve.data = m->getHandle();
    eve.msg = msg;
    keep_alive->processEvent(eve);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

/*===========================================================================

  FUNCTION:  handleSetDataProfileRequestMessage

===========================================================================*/
/*!
    @brief
    Handler to handle SetDataProfileRequestMessage message request

    @return
*/
/*=========================================================================*/
void DataModule::handleSetDataProfileRequestMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<SetDataProfileRequestMessage> m =
    std::static_pointer_cast<SetDataProfileRequestMessage>(msg);
    if( m != NULL ) {
      if(profile_handler != nullptr && !mIsPdcRefreshInProgress) {
        profile_handler->handleSetDataProfileRequestMessage(msg);
      }
      else {
        RIL_Errno result = RIL_E_INTERNAL_ERR;
        auto resp = std::make_shared<RIL_Errno>(result);
        m->sendResponse(msg, Message::Callback::Status::FAILURE, resp);
        stringstream ss;
        ss << "[" << mName << "]: " << (int)m->getSerial() << "< setDataProfileResponse resp=";
        ss << (int)result;
        Log::getInstance().d(ss.str());
        logBuffer.addLogWithTimestamp(ss.str());
      }
    } else {
      Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
    }
#ifndef RIL_FOR_LOW_RAM
    if (m != NULL && networkavailability_handler != nullptr && !mIsPdcRefreshInProgress) {
      networkavailability_handler->processSetDataProfileRequest(msg);
    }
#endif
}

#else
void DataModule::handleSetLteAttachProfileMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<SetLteAttachProfileMessage> reqMsg = std::static_pointer_cast<SetLteAttachProfileMessage>(msg);
  if( reqMsg != NULL ) {
    RIL_Errno ret = qcril_data_request_set_lte_attach_profile ( reqMsg->get_params() );
    auto resp = std::make_shared<RIL_Errno>(ret);
    reqMsg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

#endif /* #if QCRIL_RIL_VERSION >= 15 */

void DataModule::handleSetQualityMeasurementMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<SetQualityMeasurementMessage> reqMsg = std::static_pointer_cast<SetQualityMeasurementMessage>(msg);
  if( reqMsg != NULL ) {
    qmi_response_type_v01 resp;
    memset(&resp, 0, sizeof(resp));
    resp.result = QMI_RESULT_FAILURE_V01;
    resp.error = QMI_ERR_INTERNAL_V01;
    dsd_set_quality_measurement_info_req_msg_v01 info = reqMsg->getInfo();
#ifdef RIL_FOR_LOW_RAM
    if (dsd_endpoint) {
      resp = dsd_endpoint->setQualityMeasurement(info);
    }
#else
    resp = qcril_data_set_quality_measurement(&info);
#endif
    auto response = std::make_shared<qmi_response_type_v01>(resp);
    reqMsg->sendResponse(msg, Message::Callback::Status::SUCCESS, response);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleSetRadioClientCapUnSolMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());
  std::shared_ptr<SetRadioClientCapUnSolMessage> m = std::static_pointer_cast<SetRadioClientCapUnSolMessage>(msg);
  if( m != NULL ) {
    bool isAPAssistCapable = m->isAPAssistCapable();
    dataCallListChangedCallback = m->getDcListChangedCallback();
    if (!mInitTracker.isModeDetermined()) {
      mInitTracker.setIWLANMode(isAPAssistCapable ? IWLANOperationMode::AP_ASSISTED : IWLANOperationMode::LEGACY);
      }
    if (call_manager) {
        call_manager->setDataCallListChangedCallback(dataCallListChangedCallback);
        Log::getInstance().d("Setting proper dataCallListChangedCallback");
    }
    else {
        performDataModuleInitialization();
    }
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleToggleDormancyIndMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<ToggleDormancyIndMessage> m = std::static_pointer_cast<ToggleDormancyIndMessage>(msg);
  if( m != NULL ) {
    int ret = QCRIL_DS_ERROR;
    Message::Callback::Status status;
    if (call_manager) {
#ifndef RIL_FOR_LOW_RAM
      ret = qcril_data_toggle_dormancy_indications(m->dormIndSwitch);
#endif
      mLinkStatusReportEnabled = (m->dormIndSwitch == DORMANCY_INDICATIONS_ON)?true:false;
      ret = call_manager->toggleLinkActiveStateChangeReport(mLinkStatusReportEnabled);
      status = (ret == QCRIL_DS_SUCCESS ?
             Message::Callback::Status::SUCCESS : Message::Callback::Status::FAILURE);
    }
    else {
      Log::getInstance().d("call_manager is null");
      status = Message::Callback::Status::SUCCESS;
      ret = QCRIL_DS_ERROR;
    }

    auto resp = std::make_shared<int>(ret);
    m->sendResponse(msg, status, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleToggleLimitedSysIndMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<ToggleLimitedSysIndMessage> m = std::static_pointer_cast<ToggleLimitedSysIndMessage>(msg);
  if( m != NULL ) {
    int ret = QCRIL_DS_ERROR;
#ifndef RIL_FOR_LOW_RAM
    ret = qcril_data_toggle_limited_sys_indications(m->sysIndSwitch);
#endif
    if (dsd_endpoint) {
      mLimitedIndReportEnabled = (m->sysIndSwitch == LIMITED_SYS_INDICATIONS_ON)?true:false;
      ret = dsd_endpoint->toggleLimitedSysIndicationChangeReport(mLimitedIndReportEnabled);
    }
    Message::Callback::Status status = (ret == QCRIL_DS_SUCCESS ?
             Message::Callback::Status::SUCCESS : Message::Callback::Status::FAILURE);
    auto resp = std::make_shared<int>(ret);
    m->sendResponse(msg, status, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleUpdateMtuMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<UpdateMtuMessage> m = std::static_pointer_cast<UpdateMtuMessage>(msg);
  if( m != NULL ) {
    qcril_data_update_mtu(m->Mtu);
    auto resp = std::make_shared<int>(QCRIL_DS_SUCCESS);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}



void DataModule::handleGetDdsSubIdMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<GetDdsSubIdMessage> m = std::static_pointer_cast<GetDdsSubIdMessage>(msg);
  if( m != NULL ) {
    DDSSubIdInfo ddsInfo = qcril_data_get_dds_sub_info();

    LOCK_MUTEX(ddsSubMutex);
    currentDDSSUB.dds_sub_id = ddsInfo.dds_sub_id;
    currentDDSSUB.switch_type = ddsInfo.switch_type;
    UNLOCK_MUTEX(ddsSubMutex);
    Log::getInstance().d("[" + mName + "]:Current DDS is on SUB ="+std::to_string(currentDDSSUB.dds_sub_id)+
                         "switch type = "+std::to_string(currentDDSSUB.switch_type));

    auto resp = std::make_shared<DDSSubIdInfo>(ddsInfo);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleDataRequestDDSSwitchMessage(std::shared_ptr<Message> msg) {
  std::shared_ptr<RequestDdsSwitchMessage> m = std::static_pointer_cast<RequestDdsSwitchMessage>(msg);
  if( m != NULL ) {
    RIL_Errno ret = qcril_data_request_dds_switch(m->dds_sub_info);
    auto resp = std::make_shared<RIL_Errno>(ret);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

#if (QCRIL_RIL_VERSION >= 15)

void DataModule::handleRilRequestSetCarrierInfoImsiEncryptionMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  std::shared_ptr<RilRequestSetCarrierInfoImsiEncryptionMessage> m = std::static_pointer_cast<RilRequestSetCarrierInfoImsiEncryptionMessage>(msg);

  if( m != NULL ) {
    qcril_request_params_type req = m->get_params();

    qcril_request_resp_params_type resp;
    resp.instance_id        = QCRIL_DEFAULT_INSTANCE_ID;
    resp.t                  = req.t;
    resp.request_id         = req.event_id;
    resp.request_id_android = RIL_REQUEST_SET_CARRIER_INFO_IMSI_ENCRYPTION;
    resp.resp_pkt           = NULL;
    resp.resp_len           = 0;
    resp.logstr             = NULL;
    resp.rild_sock_oem_req  = 0;

    if (auth_manager) {
      auth_manager->setCarrierInfoImsiEncryption(&req);
      resp.ril_err_no         = RIL_E_SUCCESS;
    }
    else {
      Log::getInstance().d("auth_manager is null");
      resp.ril_err_no         = RIL_E_INTERNAL_ERR;
    }
    qcril_send_request_response( &resp );
  }else {
    Log::getInstance().d("[" + mName + "]: Improper message received");
  }
}

void DataModule::handleQmiAuthServiceIndMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  auto shared_indMsg(std::static_pointer_cast<QmiIndMessage>(msg));
  QmiIndMsgDataStruct *indData = shared_indMsg->getData();

  if (indData != nullptr && auth_manager!= nullptr) {
    auth_manager->qmiAuthServiceIndicationHandler(indData->msgId, indData->indData,
          indData->indSize);
  } else {
    Log::getInstance().d("[" + mName + "] Unexpected, null data from message");
  }
}

void DataModule::handleQmiAuthEndpointStatusIndMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  auto shared_indMsg(std::static_pointer_cast<EndpointStatusIndMessage>(msg));

  if (shared_indMsg->getState() == ModemEndPoint::State::OPERATIONAL) {
     if (!mInitTracker.getAuthServiceReady()) {
       mInitTracker.setAuthServiceReady(true);
       performDataModuleInitialization();
     }
  }
  else {
    mInitTracker.setAuthServiceReady(false);
    Log::getInstance().d("[" + mName + "]: ModemEndPoint is not operational");
  }
}

void DataModule::handleQmiDsdEndpointStatusIndMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  auto shared_indMsg(std::static_pointer_cast<EndpointStatusIndMessage>(msg));

  if (shared_indMsg->getState() == ModemEndPoint::State::OPERATIONAL) {
    if( !mInitTracker.getDsdServiceReady() ) {
       mInitTracker.setDsdServiceReady(true);
       performDataModuleInitialization();
    }
  }
  else {
    //Reset all interested handlers
    Log::getInstance().d("[" + mName + "]: ModemEndPoint is not operational");
    mInitTracker.setDsdServiceReady(false);
  }
}

void DataModule::logFn(std::string logStr) {
    Log::getInstance().d(logStr);
}

//Perform initialization that are common to services being initialized
//and mode being determined
void DataModule::performDataModuleInitialization() {
  if ( mInitTracker.isModeDetermined() && mInitCompleted &&
          mInitTracker.getDsdServiceReady() ) {
    //Post SSR, prioritizing caching of current DDS &
    //registration of DDS indication from modem

    DDSSubIdInfo ddsInfo;
    Log::getInstance().d("[" + mName + "]:dispatching currentDDSSync message");
    auto status = ModemEndPointFactory<DSDModemEndPoint>::getInstance().buildEndPoint()->getCurrentDDSSync(ddsInfo);
    if(status == Message::Callback::Status::SUCCESS)
    {
      LOCK_MUTEX(ddsSubMutex);
      currentDDSSUB.dds_sub_id = ddsInfo.dds_sub_id;
      currentDDSSUB.switch_type = ddsInfo.switch_type;
      UNLOCK_MUTEX(ddsSubMutex);
      Log::getInstance().d("[" + mName + "]:Current DDS is on Sub ="+std::to_string(currentDDSSUB.dds_sub_id)+
                           " and Switch type = "+std::to_string(currentDDSSUB.switch_type));

      //posting currentDDS
      PreferredDataEventType currentDdsEvent;
      DDSSwitchInfo_t eventData;
      eventData.subId = ddsInfo.dds_sub_id;
      currentDdsEvent.event = CurrentDDSInd;
      currentDdsEvent.data = &eventData;
      preferred_data_sm->processEvent(currentDdsEvent);
    }
    else
    {
      Log::getInstance().d("Failed to get the current DDS information");
    }
    dsd_endpoint->registerForCurrentDDSInd();
  }
  if( mInitTracker.allServicesReady() && mInitTracker.isModeDetermined()) {
    if( !mInitCompleted ) {
      logBuffer.addLogWithTimestamp("[" + mName + "]: performDataModuleInitialization");
      auth_manager = std::make_unique<AuthManager>();
      profile_handler = std::make_unique<ProfileHandler>(logBuffer);
      call_manager = std::make_unique<CallManager>(logBuffer);
      #if (!defined(RIL_FOR_LOW_RAM) || defined(RIL_FOR_MDM_LE))
      call_manager->setProcessDeferredIntentCallback(std::bind(&DataModule::processDeferredIntentToChange, this, std::placeholders::_1));
      #endif
      #ifdef RIL_FOR_MDM_LE
      propertySetMap("persist.vendor.radio.max_retry_timeout", PERSIST_VENDOR_RADIO_MAX_RETRY_TIMEOUT);
      #endif
      string propValue = readProperty("persist.vendor.radio.max_retry_timeout", "");
      Log::getInstance().d("[" + mName + "]: Partial retry max timeout property is " + propValue);
      unsigned long maxTimeout = DEFAULT_MAX_PARTIAL_RETRY_TIMEOUT;
      if (!propValue.empty()) {
        maxTimeout = stoul(propValue);
      }
      BringUpCapability capability = BRING_UP_LEGACY;
      if(wds_endpoint != nullptr) {
        auto cap = std::make_shared<BringUpCapability>();
        Message::Callback::Status status = wds_endpoint->getCallBringUpCapabilitySync(cap);
        if (status != Message::Callback::Status::SUCCESS) {
          Log::getInstance().d("[CallManager] failed to get call bringup capability, result = "
            + std::to_string((int) status));
        } else if(mInitTracker.isAPAssistMode() && cap != nullptr){ /* bringup by type requires Ap-assist mode*/
          capability = *cap;
        }
      }
      Log::getInstance().d("[CallManager]: Using call bringup capability = " + \
          std::to_string((int)capability));
      #ifdef QMI_RIL_UTF
        if(capability == BRING_UP_LEGACY) {
          mInitTracker.setIWLANMode(IWLANOperationMode::LEGACY);
        }
      #endif
      call_manager->init(mInitTracker.isAPAssistMode(), mInitTracker.isPartialRetryEnabled(),
          maxTimeout, capability, !mInitCompleted);

      call_manager->setCleanupKeepAliveCallback(std::bind(&DataModule::cleanupKeepAlive, this, std::placeholders::_1));

      if (dataCallListChangedCallback != nullptr) {
        call_manager->setDataCallListChangedCallback(dataCallListChangedCallback);
      }
      profile_handler->init(mInitTracker.isAPAssistMode());
      qcril_data_init(mInitTracker.isAPAssistMode());
      auth_manager->qmiAuthServiceRegisterIndications(true);
      dsd_endpoint->setV2Capabilities();
      dsd_endpoint->generateDsdSystemStatusInd();
      dsd_endpoint->registerForSystemStatusSync();
      dsd_endpoint->registerForRoamingStatusChanged();
      dsd_endpoint->toggleLimitedSysIndicationChangeReport(mLimitedIndReportEnabled);
      wds_endpoint->registerDataRegistrationRejectCause(mRegistrationFailureCauseReport);

#if (!defined(RIL_FOR_LOW_RAM) || defined(RIL_FOR_MDM_LE))
      #if (defined(RIL_FOR_MDM_LE))
      preferred_data_state_info->isRilIpcNotifier = true;
      #endif
#endif

      #if (QCRIL_RIL_VERSION >= 15)
      /* Register for QMI_WDS_MODEM_ASSISTED_KA_STATUS_IND_V01 */
      Message::Callback::Status status = wds_endpoint->registerForKeepAliveInd(TRUE);
      Log::getInstance().d("[DataModule] getKeepAliveMessageSync request "
                  "result = " + std::to_string((int) status));
      #endif

      preferred_data_state_info->sub = global_instance_id;
      preferred_data_state_info->switch_type = DSD_DDS_SWITCH_PERMANENT_V01;
      Messenger::get().registerForMessage(DDSSwitchIPCMessage::get_class_message_id(),
                                          std::bind(&DataModule::constructDDSSwitchIPCMessage,
                                          this, std::placeholders::_1));
      Messenger::get().registerForMessage(IAInfoIPCMessage::get_class_message_id(),
                                          std::bind(&DataModule::constructIAInfoIPCMessage,
                                           this, std::placeholders::_1));
      PreferredDataEventType initEvent;
      initEvent.event = InitializeSM;
      preferred_data_sm->processEvent(initEvent);

      if( mInitTracker.isAPAssistMode() ) {
#ifndef RIL_FOR_LOW_RAM
          //Create AP assist variant of NetworkServiceHandler
          network_service_handler = std::make_unique<ApAssistNetworkServiceHandler>();
          Message::Callback::Status status = dsd_endpoint->sendAPAssistIWLANSupportedSync();

          Log::getInstance().d("[DataModule] sendAPAssistIWLANSupportedSync request "
                      " result = " + std::to_string((int) status));
          if (status == Message::Callback::Status::SUCCESS)
          {
             mInitTracker.setModemCapability(true);
             //call datactl to enable/disable IWLAN
            if (loadDatactl()) {
              dcSymbols.datactlInit(global_instance_id, logFn);
              if( mInitTracker.isIWLANEnabled() ) {
                 initializeIWLAN();
              }
              else {
                dcSymbols.datactlDisableIWlan();
              }
            }
#ifdef QMI_RIL_UTF
            else {
              if( mInitTracker.isIWLANEnabled() ) {
                 initializeIWLAN();
              }
            }
#endif
          }
          else
          {
            //This should never happen
            mInitTracker.setModemCapability(false);
            if(iwlanHandshakeMsgToken != INVALID_MSG_TOKEN) {
              Log::getInstance().d("[DataModule] Look for IWLANCapabilityHandshake msg "+
                                    std::to_string((int)iwlanHandshakeMsgToken));
              getDataModule().getPendingMessageList().print();
              std::shared_ptr<Message> mmm = getDataModule().getPendingMessageList().find((uint16_t)iwlanHandshakeMsgToken);
              if(mmm!=nullptr) {
                std::shared_ptr<IWLANCapabilityHandshake> iwlanHsMsg = std::static_pointer_cast<IWLANCapabilityHandshake>(mmm);
                if(iwlanHsMsg != nullptr) {
                  auto resp = std::make_shared<rildata::ModemIWLANCapability_t>(rildata::ModemIWLANCapability_t::not_present);
                  iwlanHsMsg->sendResponse(mmm, Message::Callback::Status::SUCCESS, resp);
                  getDataModule().getPendingMessageList().erase(mmm);
                  getDataModule().getPendingMessageList().print();
                  iwlanHandshakeMsgToken = INVALID_MSG_TOKEN;
                } else {
                  Log::getInstance().d("[DataModule] Invalid IWLANCapabilityHandShake msg. Not sending response");
                }
              }
              else {
                Log::getInstance().d("[DataModule] No IWLANCapabilityHandshake msg found");
                iwlanHandshakeMsgToken = INVALID_MSG_TOKEN;
              }
            }
            else {
              Log::getInstance().d("[DataModule] Invalid IWLANCapabilityHandshake msg token");
            }
          }
#endif //RIL_FOR_LOW_RAM
      }
      else { //Legacy mode
        //Create legacy mode variant of NetworkServiceHandler
        network_service_handler = std::make_unique<NetworkServiceHandler>();
        network_service_handler->processQmiDsdSystemStatusInd(&mCachedSystemStatus);
      }
      loadDataQos();
      call_manager->processQmiDsdSystemStatusInd(&mCachedSystemStatus);
      mInitCompleted = TRUE;
    }
    else { //Services ready after SSR
      if( call_manager != nullptr )
      {
        call_manager->triggerDsiInit(!mInitCompleted);
      } else {
        Log::getInstance().d("call_manager is NULL");
      }
      if(auth_manager != nullptr)
      {
        auth_manager->qmiAuthServiceRegisterIndications(true);
        auth_manager->updateModemWithCarrierImsiKeyCache();
      } else {
        Log::getInstance().d(" auth_manager is NULL");
      }
      dsd_endpoint->setV2Capabilities();
      dsd_endpoint->generateDsdSystemStatusInd();
      dsd_endpoint->registerForSystemStatusSync();

      dsd_endpoint->registerForRoamingStatusChanged();
      wds_endpoint->registerDataRegistrationRejectCause(mRegistrationFailureCauseReport);
#if (!defined(RIL_FOR_LOW_RAM) || defined(RIL_FOR_MDM_LE))
#if (QCRIL_RIL_VERSION >= 15)
      /* Register for QMI_WDS_MODEM_ASSISTED_KA_STATUS_IND_V01 */
      auto status = wds_endpoint->registerForKeepAliveInd(TRUE);
      Log::getInstance().d("[DataModule] getKeepAliveMessageSync request "
                  "result = " + std::to_string((int) status));
#endif
#endif

      if( mInitTracker.isAPAssistMode() ) {
#ifndef RIL_FOR_LOW_RAM
        Message::Callback::Status status = dsd_endpoint->sendAPAssistIWLANSupportedSync();

        Log::getInstance().d("[DataModule] sendAPAssistIWLANSupportedSync request "
                    " result = " + std::to_string((int) status));

        //send message to datactl to enable/disable IWLAN
        if( mInitTracker.isIWLANEnabled() ) {
          if (dcSymbols.datactlEnableIWlan) {
            dcSymbols.datactlEnableIWlan();
          }
          status = dsd_endpoint->registerForAPAsstIWlanIndsSync(true);
          Log::getInstance().d("[DataModule] registerForAPAsstPrefSysChgSync request "
                    " result = " + std::to_string((int) status));
        }
        else {
          if (dcSymbols.datactlDisableIWlan) {
            dcSymbols.datactlDisableIWlan();
          }
        }
#endif// RIL_FOR_LOW_RAM
      }
    }
  } else if( (mInitTracker.getDsdServiceReady() ||
               mInitTracker.getWdsServiceReady() || mInitTracker.getAuthServiceReady()) ) {
    if( call_manager )
    {
      if(call_manager->dsiInitStatus == DsiInitStatus_t::PENDING_RELEASE)
      {
        call_manager->triggerDsiRelease();
      }
      call_manager->triggerDsiInit(!mInitCompleted);
    } else {
        Log::getInstance().d("call_manager is NULL");
    }
  }
}

bool DataModule::loadDataQos() {
#ifdef QMI_RIL_UTF
  return true;
#endif
  std::string propValue = readProperty("persist.vendor.rcs.singlereg.feature", "0");
  if (propValue == "0") {
    Log::getInstance().d("[" + mName + "]: rcs single reg is not supported");
    return false;
  }
  bool ret = false;
  do {
    dataQosLibInstance = dlopen("libqcrildataqos.so", RTLD_LAZY);
    if( NULL == dataQosLibInstance ) {
      Log::getInstance().d("[" + mName + "]: Unable to load libqcrildataqos.so");
      break;
    }
    dlerror();
    mDataQosInit = (dataQosInitFnPtr)dlsym(dataQosLibInstance, "dataQosInit");
    const char *dlsym_error = dlerror();
    if( dlsym_error ) {
      Log::getInstance().d("[" + mName + "]: Cannot find dataQosInit symbol");
      break;
    }
    mDataQosCleanup = (dataQosCleanUpFnPtr)dlsym(dataQosLibInstance, "dataQosCleanUp");
    dlsym_error = dlerror();
    if( dlsym_error ) {
      Log::getInstance().d("[" + mName + "]: Cannot find dataQosInit symbol");
      break;
    }
    mDataQosDeInit = (dataQosDeInitFnPtr)dlsym(dataQosLibInstance, "dataQosDeInit");
    dlsym_error = dlerror();
    if(dlsym_error) {
      Log::getInstance().d("["+ mName + "]: Cannot find dataQosDeInit symbol");
      break;
    }
    ret = true;
  }
  while( 0 );
  if( ret == false ) {
    unloadDataQos();
  }
  return ret;
}

void DataModule::unloadDataQos() {
#ifdef QMI_RIL_UTF
// @TODO: uncomment once UTF is ready
//   dataQosCleanUp();
   return;
#endif
  if(dataQosLibInstance != nullptr) {
    if(mDataQosDeInit != nullptr) {
      mDataQosDeInit();
    }
    mDataQosInit = nullptr;
    mDataQosCleanup = nullptr;
    mDataQosDeInit = nullptr;
    dlclose(dataQosLibInstance);
    dataQosLibInstance = nullptr;
  }
}

void DataModule::handleSetLinkCapFilterMessage(std::shared_ptr<Message> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    std::shared_ptr<SetLinkCapFilterMessage> m = std::static_pointer_cast<SetLinkCapFilterMessage>(msg);
    if( m != NULL ) {
      auto rf = m->getParams();
      lceHandler.toggleReporting(rf);
      auto resp = std::make_shared<int>(static_cast<int>(RIL_E_SUCCESS));
      m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
    }
    else {
      Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
    }
}

void DataModule::handleSetLinkCapRptCriteriaMessage(std::shared_ptr<Message> msg)
{
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    std::shared_ptr<SetLinkCapRptCriteriaMessage> m = std::static_pointer_cast<SetLinkCapRptCriteriaMessage>(msg);
    if( m != NULL ) {
      auto rf = m->getParams();
      Message::Callback::Status status = Message::Callback::Status::SUCCESS;
      LinkCapCriteriaResult_t result = lceHandler.setCriteria(rf);
      if (result != rildata::LinkCapCriteriaResult_t::success) {
        status = Message::Callback::Status::FAILURE;
      }
      auto resp = std::make_shared<rildata::LinkCapCriteriaResult_t>(result);
      m->sendResponse(msg, status, resp);
    } else {
      Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
    }
}
#endif

void DataModule::handleRilEventDataCallback(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  logBuffer.addLogWithTimestamp("[" + mName + "]: Handling msg = " + msg->dump());
  if (call_manager != nullptr) {
    call_manager->handleCallEventMessage(CallEventTypeEnum::RilEventDataCallback, msg);
  }
  else {
     Log::getInstance().d("call_manager is null");
  }
}

/*===========================================================================

  FUNCTION:  handleRilRequestSetInitialAttachApn

===========================================================================*/
/*!
    @brief
    Handler to handle RilRequestSetInitialAttachApnRequestMessage message request

    @return
*/
/*=========================================================================*/
void DataModule::handleRilRequestSetInitialAttachApn(std::shared_ptr<Message> msg)
{
  RIL_Errno  res = RIL_E_SUCCESS;
  qcril_request_resp_params_type    resp;
  qcril_reqlist_public_type       qcril_req_info;
  IxErrnoType reqlist_status = E_SUCCESS;
  std::shared_ptr<RilRequestSetInitialAttachApnRequestMessage> m =
       std::static_pointer_cast<RilRequestSetInitialAttachApnRequestMessage>(msg);

  memset(&resp,0,sizeof(qcril_request_resp_params_type));
  memset(&qcril_req_info,0,sizeof(qcril_reqlist_public_type));
  if (m!= nullptr)
  {
    Log::getInstance().d("[DataModule]: Handling msg = " + m->dump());
    const qcril_request_params_type *params_ptr = m->get_params();
    if( params_ptr == NULL )
    {
      Log::getInstance().d("ERROR!!params_ptr is NULL . Returning error response to telephony");
      return;
    }
    do
    {
      Log::getInstance().d("[DataModule]: Inserting"
             " QCRIL_EVT_QMI_REQUEST_INIT_ATTACH_APN event in reqlist ");
      qcril_reqlist_default_entry( params_ptr->t,
                                   params_ptr->event_id,
                                   params_ptr->modem_id,
                                   QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                                   QCRIL_EVT_QMI_REQUEST_INIT_ATTACH_APN,
                                   NULL,
                                   &qcril_req_info );
      if (( reqlist_status = qcril_reqlist_new( params_ptr->instance_id,
                                       &qcril_req_info )) != E_SUCCESS)
      {
        Log::getInstance().d("Reqlist entry failure..status: "+ std::to_string(reqlist_status));
        res = map_internalerr_from_reqlist_new_to_ril_err(reqlist_status);
        break;
      }
      RilRespParams resp_params{ params_ptr->instance_id,
                                                 params_ptr->modem_id,
                                                 params_ptr->event_id,
                                                 params_ptr->t
                                               };
      Log::getInstance().d("[DataModule]: Handling token = " + std::to_string((long long)params_ptr->t)) ;
      if( profile_handler )
      {
        profile_handler->handleInitialAttachRequest(m->get_attach_params(), resp_params);
      } else
      {
        Log::getInstance().d("[DataModule]: Profile Handler is NULL. Returning ERROR response") ;
        qcril_default_request_resp_params( params_ptr->instance_id,
                                           params_ptr->t,
                                           params_ptr->event_id,
                                           RIL_E_INTERNAL_ERR,
                                           &resp );
        qcril_send_request_response( &resp );
      }
    } while(0);

    if (res != RIL_E_SUCCESS)
    {
      Log::getInstance().d("[DataModule]:handleRilRequestSetInitialAttachApn:"
                           " Sending Error response");
      qcril_default_request_resp_params( params_ptr->instance_id,
                                         params_ptr->t,
                                         params_ptr->event_id,
                                         res,
                                         &resp );
      qcril_send_request_response( &resp );
    }
  } else
  {
    Log::getInstance().d("[" + mName + "]: Improper message received for handleRilRequestSetInitialAttachApn");
  }
}

/*===========================================================================

  FUNCTION:  handleSetInitialAttachApn

===========================================================================*/
/*!
    @brief
    Handler to handle handleSetInitialAttachApn message request in LE
    target

    @return
*/
/*=========================================================================*/
void DataModule::handleSetInitialAttachApn(std::shared_ptr<Message> msg)
{
  std::shared_ptr<SetInitialAttachApnRequestMessage> m =
       std::static_pointer_cast<SetInitialAttachApnRequestMessage>(msg);

  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());
  logBuffer.addLogWithTimestamp("[" + mName + "]: Handling msg = " + msg->dump());

  if( m != NULL) {
    if(profile_handler != nullptr &&  !mIsPdcRefreshInProgress) {
      PreferredDataEventType IAEvent;
      DDSSwitchInfo_t eventData;
      eventData.msg = m;
      attachSerialList.insert(m->getSerial());
      //send IAstarted event for masterRil
      IAEvent.event = IAStarted;
      IAEvent.data = &eventData;
      preferred_data_sm->processEvent(IAEvent);
    } else {
      RIL_Errno result = RIL_E_INTERNAL_ERR;
      auto resp = std::make_shared<RIL_Errno>(result);
      m->sendResponse(msg, Message::Callback::Status::FAILURE, resp);
    }
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

/*===========================================================================

FUNCTION:  handleSetInitialAttachApn

===========================================================================*/
/*!
    @brief
    Handler to handle handleSetInitialAttachApn message request in LE
    target

    @return
*/
/*=========================================================================*/
void DataModule::processInitialAttachRequestMessage(std::shared_ptr<Message> msg)
{
  Log::getInstance().d("[" + mName + "] : processInitialAttachRequestMessage");

  std::shared_ptr<SetInitialAttachApnRequestMessage> m =
       std::static_pointer_cast<SetInitialAttachApnRequestMessage>(msg);

  if(m!= nullptr && profile_handler != nullptr) {
    #ifdef QMI_RIL_UTF
    if(!preferred_data_state_info->isRilIpcNotifier)
    {
      global_instance_id = QCRIL_SECOND_INSTANCE_ID;
    }
    #endif
    profile_handler->handleInitialAttachRequest(m);
  } else {
    Log::getInstance().d("Could not process the msg");
  }
}

void DataModule::handleGetModemAttachParamsRetryMessage(std::shared_ptr<Message> msg)
{
  Log::getInstance().d("[" + mName + "] : handleRetryAttachTimeOutMessage");

  std::shared_ptr<GetModemAttachParamsRetryMessage> m =
       std::static_pointer_cast<GetModemAttachParamsRetryMessage>(msg);

  if(m != nullptr && profile_handler) {
    profile_handler->handleGetModemAttachParamsRetryMessage();
  }
}

/*!
    @brief
    Handler to handle SetInitialAttachApnRequestMessage message request

    @return
*/
/*=========================================================================*/
void DataModule::handleSetLteAttachPdnListActionResult(std::shared_ptr<Message> msg)
{
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
    std::shared_ptr<SetLteAttachPdnListActionResultMessage> m = std::static_pointer_cast<SetLteAttachPdnListActionResultMessage>(msg);
    if( m != NULL && profile_handler != NULL ) {
      Log::getInstance().d("[DataModule]::Invoking handleWdsUnSolInd" );
      profile_handler->handleWdsUnSolInd(&(m->getParams()));
    } else {
      Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
    }
}

/*===========================================================================

  FUNCTION:  handleNasSrvDomainPrefInd

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when NasSrvDomainPrefIndMessage is received

    @return
*/
/*=========================================================================*/
void DataModule::handleNasSrvDomainPrefInd(std::shared_ptr<Message> m)
{
  Log::getInstance().d("[DataModule]: Handling msg = " + m->dump());
  std::shared_ptr<NasSrvDomainPrefIndMessage> msg = std::static_pointer_cast<NasSrvDomainPrefIndMessage>(m);
  if (msg != NULL)
  {
    uint8_t domainPrefValid;
    nas_srv_domain_pref_enum_type_v01 domainPref;
    msg->getDomainPref(domainPrefValid,domainPref);
    if(profile_handler)
    {
      Log::getInstance().d("[DataModule]::Invoking qcril_data_nas_detach_attach_ind_hdlr" );
      profile_handler->qcril_data_nas_detach_attach_ind_hdlr(domainPrefValid, domainPref);
    } else
    {
      Log::getInstance().d("[DataModule]::Invalid nas_helper object. Returning");
    }
    Log::getInstance().d("[DataModule]::handleNasSrvDomainPrefInd EXIT" );
  }
}

/*===========================================================================

  FUNCTION:  handleDataNasPsAttachDetachResponse

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when DataNasPsAttachDetachResponse is received

    @return
*/
/*=========================================================================*/
void DataModule::handleDataNasPsAttachDetachResponse(std::shared_ptr<Message> m)
{
  Log::getInstance().d("[DataModule]: Handling msg = " + m->dump());
  std::shared_ptr<DataNasPsAttachDetachResponse> msg = std::static_pointer_cast<DataNasPsAttachDetachResponse>(m);
  if (msg != NULL)
  {
    if(profile_handler)
    {
      Log::getInstance().d("[DataModule]::Invoking processNasPsAttachDetachResp" );
      profile_handler->processNasPsAttachDetachResp(msg->getError());
    }
    else
    {
      Log::getInstance().d("[DataModule]::Invalid profile_handler object. Returning");
    }
    Log::getInstance().d("[DataModule]::handleDataNasPsAttachDetachResponse EXIT" );
  }
}

/*===========================================================================

  FUNCTION:  handleDetachAttachIndTimeoutMessage

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when DetachAttachIndTimeoutMessage is received

    @return
*/
/*=========================================================================*/
void DataModule::handleDetachAttachIndTimeoutMessage(std::shared_ptr<Message> m)
{
  Log::getInstance().d("[DataModule]: Handling msg = " + m->dump());
  std::shared_ptr<DetachAttachIndTimeoutMessage> msg = std::static_pointer_cast<DetachAttachIndTimeoutMessage>(m);
  if (msg != NULL)
  {
    if(profile_handler)
    {
      Log::getInstance().d("[DataModule]::Invoking processNasPsAttachDetachResp" );
      profile_handler->processDetachAttachTimeout(msg);
    }
    else
    {
      Log::getInstance().d("[DataModule]::Invalid profile_handler object. Returning");
    }
    Log::getInstance().d("[DataModule]::handleDetachAttachIndTimeoutMessage EXIT" );
  }
}

/*===========================================================================

  FUNCTION:  handleSetInitialAttachCompleted

===========================================================================*/
/*!
    @brief
    Handler to handle handleSetInitialAttachCompleted message request in LE
    target

    @return
*/
/*=========================================================================*/
void DataModule::handleSetInitialAttachCompleted(std::shared_ptr<Message> msg)
{
  std::shared_ptr<SetInitialAttachCompletedMessage> m =
       std::static_pointer_cast<SetInitialAttachCompletedMessage>(msg);

  if( m != NULL ) {
    Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());
    logBuffer.addLogWithTimestamp("[" + mName + "]: Handling msg = " + msg->dump());
    auto initialAttachMessage = m->getMsg();
    if (initialAttachMessage) {
        if (attachSerialList.find(initialAttachMessage->getSerial()) == attachSerialList.end()) {
            Log::getInstance().d("[DataModule]: Response for this initialAttachMessage is already sent");
            m->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
        }
        else {
            attachSerialList.erase(initialAttachMessage->getSerial());
            bool status = m->getStatus();
            RIL_Errno result = RIL_E_INTERNAL_ERR;
            auto apiStatus = Message::Callback::Status::FAILURE;
            if (status) {
                result = RIL_E_SUCCESS;
                apiStatus = Message::Callback::Status::SUCCESS;
            }
            auto resp = std::make_shared<RIL_Errno>(result);
            initialAttachMessage->sendResponse(initialAttachMessage, apiStatus, resp);
            if (m->getProcessIACompleted()) {
                PreferredDataEventType IAComplete;
                IAComplete.event = IACompleted;
                preferred_data_sm->processEvent(IAComplete);
            }
            m->sendResponse(msg, Message::Callback::Status::SUCCESS, nullptr);
        }
    } else {
        m->sendResponse(msg, Message::Callback::Status::FAILURE, nullptr);
    }
  }
}

/*===========================================================================

  FUNCTION:  handleNasRequestDataShutdown

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when NasRequestDataShutdownMessage is received

    @return
*/
/*=========================================================================*/
void DataModule::handleNasRequestDataShutdown(std::shared_ptr<Message> msg)
{
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());
  std::shared_ptr<NasRequestDataShutdownMessage> m = std::static_pointer_cast<NasRequestDataShutdownMessage>(msg);
  if (m != NULL)
  {
    NasRequestDataShutdownResponse ret = NasRequestDataShutdownResponse::FAILURE;
#ifdef RIL_FOR_LOW_RAM
    if(dsd_endpoint->deviceShutdownRequest()){
      ret = NasRequestDataShutdownResponse::SUCCESS;
    }
#else
    if(qcril_data_device_shutdown()) {
      ret = NasRequestDataShutdownResponse::SUCCESS;
    }
#endif
    auto resp = std::make_shared<NasRequestDataShutdownResponse>(ret);
    m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

/*============================================================================

    retrieveUIMAppStatusFromAppInfo

============================================================================*/
/*!
    @brief
    Retrieve aid buffer application info

    @return
    None
*/
/*=========================================================================*/
int DataModule::retrieveUIMAppStatusFromAppInfo(RIL_UIM_AppStatus *application, string &aid_buffer, RIL_UIM_AppType *app_type)
{
    int res = E_FAILURE;

    Log::getInstance().d("[Datamodule]: handling retrieveUIMAppStatusFromAppInfo");
    if (application && app_type)
    {
        Log::getInstance().d("app type " + std::to_string(application->app_type));
        Log::getInstance().d("app State " + std::to_string(application->app_state));

        if ((application->app_state == RIL_UIM_APPSTATE_READY) &&
            !application->aid_ptr.empty())
        {
          aid_buffer = application->aid_ptr;
          Log::getInstance().d("aid buffer " + aid_buffer);
          *app_type = application->app_type;
          res = E_SUCCESS;
        }
    }
    return res;
}

/*============================================================================

    retrieveUIMCardStatus

============================================================================*/
/*!
    @brief
    Retrieve aid buffer card status info

    @return
    None
*/
/*=========================================================================*/
int DataModule::retrieveUIMCardStatus( std::shared_ptr<RIL_UIM_CardStatus> ril_card_status, string &aid_buffer, RIL_UIM_AppType *app_type)
{
    int               res = E_FAILURE;
    int               index;

    Log::getInstance().d("[Datamodule]: handling retrieveUIMCardStatus");
    if (ril_card_status)
    {
        if (ril_card_status->card_state == RIL_UIM_CARDSTATE_PRESENT)
        {
            Log::getInstance().d("card is present");
            if (ril_card_status->gsm_umts_subscription_app_index != -1)
            {
                index = ril_card_status->gsm_umts_subscription_app_index;
                /* retrieve aid from gsm umts subscription app info */
                res = retrieveUIMAppStatusFromAppInfo(
                                                &ril_card_status->applications[index],
                                                aid_buffer, app_type);
               Log::getInstance().d("res " + std::to_string(res));
            }

            if ((res == E_FAILURE) &&
                (ril_card_status->cdma_subscription_app_index != -1))
            {
                index = ril_card_status->cdma_subscription_app_index;
                /* retrieve aid from cdma subscription app info */
                res = retrieveUIMAppStatusFromAppInfo(
                                                &ril_card_status->applications[index],
                                                aid_buffer, app_type);
            }

            if ((res == E_FAILURE) &&
                (ril_card_status->ims_subscription_app_index != -1))
            {
                index = ril_card_status->ims_subscription_app_index;
                /* retrieve aid from ims subscription app info */
                res = retrieveUIMAppStatusFromAppInfo(
                                                &ril_card_status->applications[index],
                                                aid_buffer, app_type);
            }
        }
    }
    return res;
}
/*===========================================================================

    qcrilDataUimEventAppStatusUpdate

============================================================================*/
/*!
    @brief
    Handles QCRIL_EVT_CM_CARD_APP_STATUS_CHANGED

    @return
    None
*/
/*=========================================================================*/
void DataModule::qcrilDataUimEventAppStatusUpdate ( const qcril_request_params_type *const params_ptr, qcril_request_return_type *const ret_ptr)
{
  RIL_UIM_AppStatus              *card_app_info;
  string                          aid = {};
  RIL_UIM_AppType                 request_app_type;

  Log::getInstance().d("qcrilDataUimEventAppStatusUpdate:: ENTRY");
  QCRIL_NOTUSED(ret_ptr);

  if (!params_ptr)
  {
    Log::getInstance().d("PARAMS ptr is NULL");
    return;
  }

  card_app_info = (RIL_UIM_AppStatus *)(params_ptr->data);

  /* Process only this slots SIM card applications */
  if (card_app_info != NULL &&
      card_app_info->app_state == RIL_UIM_APPSTATE_READY)
  {
    Log::getInstance().d("app type"+std::to_string(card_app_info->app_type)+
                         "app state"+std::to_string(card_app_info->app_state));

    auto card_status = std::make_shared<UimGetCardStatusRequestSyncMsg>(qmi_ril_get_process_instance_id());
    std::shared_ptr<RIL_UIM_CardStatus> ril_card_status = nullptr;

    /* retrieve card status info */
    if (card_status != nullptr && (card_status->dispatchSync(ril_card_status) != Message::Callback::Status::SUCCESS))
    {
      Log::getInstance().d("Get card status request failed");
      return;
    }

    if(ril_card_status != nullptr && ril_card_status->err != RIL_UIM_E_SUCCESS)
    {
      Log::getInstance().d("ril card status failed");
      return;
    }
    /* retrieve aid from card status */
    if ((retrieveUIMCardStatus(ril_card_status, aid, &request_app_type))!= E_SUCCESS)
    {
      Log::getInstance().d("Retrieval of AID from card status failed");
      return;
    }

    Log::getInstance().d("Received SIM aid_buffer="+aid);
    qcril_uim_app_type  app_type = QCRIL_UIM_APP_UNKNOWN;

    //proceed only when memory is allocated
    if(aid.empty())
    {
      Log::getInstance().d("AID Memory allocation failed");
      return;
    }

    switch(request_app_type)
    {
      case RIL_UIM_APPTYPE_SIM:
        app_type = QCRIL_UIM_APP_SIM;
        break;
     case RIL_UIM_APPTYPE_USIM:
        app_type = QCRIL_UIM_APP_USIM;
        break;
     case RIL_UIM_APPTYPE_RUIM:
        app_type = QCRIL_UIM_APP_RUIM;
        break;
     case RIL_UIM_APPTYPE_CSIM:
        app_type = QCRIL_UIM_APP_CSIM;
        break;
     default:
        app_type = QCRIL_UIM_APP_UNKNOWN;
        break;
     }

     DataGetMccMncCallback Cb("set-cb-token");
     std::shared_ptr<UimGetMccMncRequestMsg> req =
     std::make_shared<UimGetMccMncRequestMsg>(aid, app_type, &Cb);
     if(req)
     {
       Log::getInstance().d("Dispatching UimGetMccMncRequestMsg Message");
       req->dispatch();
     }

   } else {
       Log::getInstance().d("Card APP info is NULL or slot id mismatch or Card APP status isn't READY");
   }
}

void DataModule::handleUimCardAppStatusIndMsg(std::shared_ptr<Message> m)
{
  qcril_request_return_type ret_ptr;
  qcril_request_params_type params_ptr;
  Log::getInstance().d("[DataModule]: Handling msg = " + m->dump());

  std::shared_ptr<UimCardAppStatusIndMsg> msg =
       std::static_pointer_cast<UimCardAppStatusIndMsg>(m);
  std::memset(&params_ptr, 0, sizeof(params_ptr));
  std::memset(&ret_ptr, 0, sizeof(ret_ptr));

  if( msg != NULL )
  {
    params_ptr.data = static_cast<void *>(new char[sizeof(RIL_UIM_AppStatus)]);
    if(params_ptr.data)
    {
      std::memcpy(params_ptr.data, &msg->get_app_info(), sizeof(RIL_UIM_AppStatus));
      params_ptr.datalen = sizeof(RIL_UIM_AppStatus);
      params_ptr.modem_id = QCRIL_DEFAULT_MODEM_ID;
#ifndef QMI_RIL_UTF
      qcrilDataUimEventAppStatusUpdate (&params_ptr, &ret_ptr);
#endif
      delete[] (static_cast<char*>(params_ptr.data));
      params_ptr.data = NULL;
    } else
    {
      Log::getInstance().d("[DataModule]: Memory allocation failure");
    }
  } else
  {
    Log::getInstance().d("[" + mName + "]: Improper message received");
  }
}

/*===========================================================================

  FUNCTION:  handleQmiWdsEndpointStatusIndMessage

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when QMI WDS modem endpoint status is changed

    @return
*/
/*=========================================================================*/
void DataModule::handleQmiWdsEndpointStatusIndMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  auto shared_indMsg(std::static_pointer_cast<EndpointStatusIndMessage>(msg));

  if (shared_indMsg->getState() == ModemEndPoint::State::OPERATIONAL) {
    if (!mInitTracker.getWdsServiceReady()) {
      mInitTracker.setWdsServiceReady(true);
      performDataModuleInitialization();
    }
  } else if (shared_indMsg->getState() == ModemEndPoint::State::NON_OPERATIONAL) {
    if (call_manager) {
      call_manager->cleanUpAllBearerAllocation();
      call_manager->triggerDsiRelease();
    } else {
      Log::getInstance().d("call_manager is null");
    }
    //Reset all interested handlers
    mInitTracker.setWdsServiceReady(false);
    // release QDP attach profile
    if(profile_handler) {
      profile_handler->releaseQdpAttachProfile();
    }
    Log::getInstance().d("[" + mName + "]: WDSModemEndPoint is not operational");
  }
}

#ifdef FEATURE_DATA_LQE
/*===========================================================================*/
/*!
    @brief
    Handler which gets invoked when QMI OTT modem endpoint status is changed

    @return
*/
/*=========================================================================*/
void DataModule::handleQmiOttEndpointStatusIndMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  auto shared_indMsg(std::static_pointer_cast<EndpointStatusIndMessage>(msg));

  if (shared_indMsg->getState() == ModemEndPoint::State::OPERATIONAL) {
    if (!mInitTracker.getOttServiceReady()) {
      mInitTracker.setOttServiceReady(true);
      qcril_data_ott_lqe_init();
    }
  } else if (shared_indMsg->getState() == ModemEndPoint::State::NON_OPERATIONAL) {
    mInitTracker.setOttServiceReady(false);
    Log::getInstance().d("[" + mName + "]: OTTModemEndPoint is not operational");
  }
}
#endif /* FEATURE_DATA_LQE */

/*===========================================================================

  FUNCTION:  handleDataConnectionStateChangedMessage

===========================================================================*/
/*!
    @brief
    Handler when data connection state is changed

    @return
*/
/*=========================================================================*/
void DataModule::handleDataConnectionStateChangedMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<CallStatusMessage> m = std::static_pointer_cast<CallStatusMessage>(msg);
  if (m != NULL) {
    const CallStatusParams callParams = m->getCallParams();
    if (callParams.evt == QCRIL_DATA_EVT_CALL_RELEASED) {
      if (call_manager) {
        //call_manager->cleanUpBearerAllocation((int32_t)m->getCallId());
      } else {
        Log::getInstance().d("call_manager is null");
      }
    }
  }
}
/*===========================================================================

  FUNCTION:  handleDsdSystemStatusInd

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when DsdSystemStatusMessage is received.

    @return
*/
/*=========================================================================*/
void DataModule::handleDsdSystemStatusInd(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[DataModule]: Handling msg = " + msg->dump());
  std::shared_ptr<DsdSystemStatusMessage> m = std::static_pointer_cast<DsdSystemStatusMessage>(msg);
  if (m != NULL) {
    mCachedSystemStatus = m->getParams();
    if (network_service_handler) {
      network_service_handler->processQmiDsdSystemStatusInd(&mCachedSystemStatus);
    }
#ifndef RIL_FOR_LOW_RAM
    if (networkavailability_handler) {
      //Update global preferred system in NetworkAvailabilityHander
      if(mCachedSystemStatus.avail_sys_valid && mCachedSystemStatus.avail_sys_len >= 1) {
        networkavailability_handler->setGlobalPreferredSystem(mCachedSystemStatus.avail_sys[0]);
      }
      if (mCachedSystemStatus.apn_avail_sys_info_valid) {
        networkavailability_handler->processQmiDsdSystemStatusInd(mCachedSystemStatus.apn_avail_sys_info,
                                                              mCachedSystemStatus.apn_avail_sys_info_len);
      }
    }
#endif
    if (call_manager) {
      call_manager->processQmiDsdSystemStatusInd(&mCachedSystemStatus);
    }
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

/*===========================================================================

  FUNCTION:  handleSetPreferredDataModem

===========================================================================*/
/*!
    @brief
    Handler for AP triggered DDS switch

    @return
*/
/*=========================================================================*/
void DataModule::handleSetPreferredDataModem(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<SetPreferredDataModemRequestMessage> m = std::static_pointer_cast<SetPreferredDataModemRequestMessage>(msg);
  if (m != NULL) {
    if (TempddsSwitchRequestPending) {
      TimeKeeper::getInstance().clear_timer(tempDDSSwitchRequestTimer);
    }
    PreferredDataEventType setPreferredDataEvent;
    DDSSwitchInfo_t eventData;
    eventData.subId = m->getModemId();
    eventData.msg = m;

    setPreferredDataEvent.event = SetPreferredDataModem;
    setPreferredDataEvent.data = &eventData;
    preferred_data_sm->processEvent(setPreferredDataEvent);
  }
}

/*===========================================================================

  FUNCTION:  handleCurrentDDSIndMessage

===========================================================================*/
/*!
    @brief
    Handler for MP triggered DDS switch

    @return
*/
/*=========================================================================*/
void DataModule::handleCurrentDDSIndMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<CurrentDDSIndMessage> m = std::static_pointer_cast<CurrentDDSIndMessage>(msg);
  if (m != NULL) {
    DDSSubIdInfo ddsInfo;
    ddsInfo.dds_sub_id = m->getSubId();
    ddsInfo.switch_type = m->getSwitchType();

    LOCK_MUTEX(ddsSubMutex)
    currentDDSSUB.dds_sub_id = ddsInfo.dds_sub_id;
    currentDDSSUB.switch_type = ddsInfo.switch_type;
    UNLOCK_MUTEX(ddsSubMutex);

    Log::getInstance().d("[" + mName + "]: Sending response to qcril common");
    auto msg = std::make_shared<rildata::NewDDSInfoMessage>(global_instance_id, ddsInfo);
    msg->broadcast();

    Log::getInstance().d("[" + mName + "]:Current DDS is on SUB =" + std::to_string(currentDDSSUB.dds_sub_id) + "with switch type =  " + std::to_string(currentDDSSUB.switch_type));
    // only listen for current DDS indication on main ril
    #ifdef QMI_RIL_UTF
    if (preferred_data_state_info != nullptr) {
    #else
    if (preferred_data_state_info != nullptr &&
        preferred_data_state_info->isRilIpcNotifier) {
    #endif
      PreferredDataEventType currentDdsEvent;
      DDSSwitchInfo_t eventData;
      eventData.subId = m->getSubId();
      currentDdsEvent.event = CurrentDDSInd;
      currentDdsEvent.data = &eventData;
      preferred_data_sm->processEvent(currentDdsEvent);
    }
  }
}

void DataModule::handleCurrentRoamingStatusChangedMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<CurrentRoamingStatusChangedMessage> m =
    std::static_pointer_cast<CurrentRoamingStatusChangedMessage>(msg);
  if (m != NULL) {
    if (call_manager != nullptr) {
      call_manager->updateCurrentRoamingStatus(m->isRoaming());
    }
  }
}

/*===========================================================================

  FUNCTION:  handleDDSSwitchResultIndMessage

===========================================================================*/
/*!
    @brief
    Handler when DDS switch started by modem

    @return
*/
/*=========================================================================*/
void DataModule::handleDDSSwitchResultIndMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<DDSSwitchResultIndMessage> m = std::static_pointer_cast<DDSSwitchResultIndMessage>(msg);
  if (m != NULL) {
    PreferredDataEventType ddsSwitchEvent;
    DDSSwitchInfo_t eventData;
    eventData.msg = m;

    eventData.switchAllowed = DDSSwitchRes::FAIL;
    if (m->getAllowed()) {
      eventData.switchAllowed = DDSSwitchRes::ALLOWED;
    }

    if (m->getError() != TriggerDDSSwitchError::NO_ERROR) {
      eventData.switchAllowed = DDSSwitchRes::ERROR;
    }

    ddsSwitchEvent.event = DDSSwitchInd;
    ddsSwitchEvent.data = &eventData;
    preferred_data_sm->processEvent(ddsSwitchEvent);
  }
}

/*===========================================================================

  FUNCTION:  handleRadioConfigClientConnectedMessage

===========================================================================*/
/*!
    @brief
    Handler when client registers response functions with RadioConfig HAL

    @return
*/
/*=========================================================================*/
void DataModule::handleRadioConfigClientConnectedMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  Log::getInstance().d("Client connected to ril instance " + std::to_string(global_instance_id));
  // this flag indicates that the current ril instance will be broadcasting IPC messages
  preferred_data_state_info->isRilIpcNotifier = true;
  qcril_data_init_dpm_client_for_legacy_modem( preferred_data_state_info->isRilIpcNotifier );
  QmiSetupRequestCallback callback("voice-token");

  #ifdef QMI_RIL_UTF
  std::shared_ptr<RadioConfigClientConnectedMessage> m = std::static_pointer_cast<RadioConfigClientConnectedMessage>(msg);
  if(m != nullptr)
  {
    preferred_data_state_info->isRilIpcNotifier = m->getRilType();
  }
  #endif

#ifndef RIL_FOR_LOW_RAM
  if (voiceCallEndPointSub0 == nullptr) {
    voiceCallEndPointSub0 = std::shared_ptr<VoiceCallModemEndPoint>(new VoiceCallModemEndPoint("VoiceSub0EndPoint", SubId::PRIMARY));
#ifdef RIL_FOR_DYNAMIC_LOADING
    qcril_instance_id_e_type inst_id = static_cast<qcril_instance_id_e_type>(global_instance_id);
    voiceCallEndPointSub0->requestSetup("voice-token-client-server", inst_id, &callback);
#else
    voiceCallEndPointSub0->requestSetup("voice-token-client-server", &callback);
#endif
  }

  if (voiceCallEndPointSub1 == nullptr) {
    voiceCallEndPointSub1 = std::shared_ptr<VoiceCallModemEndPoint>(new VoiceCallModemEndPoint("VoiceSub1EndPoint", SubId::SECONDARY));
#ifdef RIL_FOR_DYNAMIC_LOADING
    qcril_instance_id_e_type inst_id = static_cast<qcril_instance_id_e_type>(global_instance_id);
    voiceCallEndPointSub1->requestSetup("voice-token-client-server", inst_id, &callback);
#else
    voiceCallEndPointSub1->requestSetup("voice-token-client-server", &callback);
#endif
  }
#endif
}

/*===========================================================================

  FUNCTION:  handleDDSSwitchTimeoutMessage

===========================================================================*/
/*!
    @brief

    @return
*/
/*=========================================================================*/
void DataModule::handleDDSSwitchTimeoutMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<DDSSwitchTimeoutMessage> m = std::static_pointer_cast<DDSSwitchTimeoutMessage>(msg);
  if (m != NULL) {
    PreferredDataEventType timeoutEvent;
    DDSSwitchInfo_t eventData;
    eventData.msg = m;

    switch (m->getType()) {
       case DDSTimeOutSwitchType::DDSTimeOutSwitch:
         timeoutEvent.event = DDSSwitchTimerExpired;
         break;
       case DDSTimeOutSwitchType::TempDDSTimeOutSwitch:
         timeoutEvent.event = TempDDSSwitchTimerExpired;
         break;
    }
    timeoutEvent.data = &eventData;
    preferred_data_sm->processEvent(timeoutEvent);
  }
}

/*===========================================================================

  FUNCTION:  handleDDSSwitchIPCMessage

===========================================================================*/
/*!
    @brief

    @return
*/
/*=========================================================================*/
void DataModule::handleDDSSwitchIPCMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<DDSSwitchIPCMessage> m = std::static_pointer_cast<DDSSwitchIPCMessage>(msg);
  if (m != NULL) {
    // do not listen for IPC messages on main ril
    if (preferred_data_state_info != nullptr &&
        !preferred_data_state_info->isRilIpcNotifier) {
      PreferredDataEventType ipcEvent;
      DDSSwitchInfo_t eventData;
      eventData.subId = m->getSubId();
      ipcEvent.data = &eventData;
      switch (m->getState()) {
         case DDSSwitchIPCMessageState::ApStarted:
           ipcEvent.event = DDSSwitchApStarted;
           preferred_data_sm->processEvent(ipcEvent);
           break;
         case DDSSwitchIPCMessageState::Completed:
           ipcEvent.event = DDSSwitchCompleted;
           preferred_data_sm->processEvent(ipcEvent);
           break;
         case DDSSwitchIPCMessageState::MpStarted:
           ipcEvent.event = DDSSwitchMpStarted;
           preferred_data_sm->processEvent(ipcEvent);
           break;
         default:
           Log::getInstance().d("Invalid DDSSwitchIPCMessage state = " + std::to_string(static_cast<int>(m->getState())));
           break;
      }
    } else {
      Log::getInstance().d("Ignoring IPC message on self");
    }
  }
}

/*===========================================================================

  FUNCTION:  handleIAInfoIPCMessage

===========================================================================*/
/*!
    @brief

    @return
*/
/*=========================================================================*/
void DataModule::handleIAInfoIPCMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<IAInfoIPCMessage> m = std::static_pointer_cast<IAInfoIPCMessage>(msg);
  if (m != NULL) {
    if(preferred_data_state_info != nullptr)
    {
      //Process IACompleted and IAStartRequest on master ril
      PreferredDataEventType ipcEvent;
      if(preferred_data_state_info->isRilIpcNotifier)
      {
        switch(m->getState()) {
          case IAInfoIPCMessageState::IACompleted:
          {
            ipcEvent.event = IACompleted;
            preferred_data_sm->processEvent(ipcEvent);
            break;
          }

          case IAInfoIPCMessageState::IAStartRequest:
          {
            ipcEvent.event = IAStartRequest;
            preferred_data_sm->processEvent(ipcEvent);
            #ifdef QMI_RIL_UTF
            preferred_data_state_info->isRilIpcNotifier = false;
            #endif
            break;
          }
          default:
          break;
        }
      } else if(m->getState() == IAInfoIPCMessageState::IAStartResponse) {
          //handle IAStartResponse on slaveRil
          ipcEvent.event = IAStartResponse;
          preferred_data_sm->processEvent(ipcEvent);
      }
    } else {
      Log::getInstance().d("Ignoring IPC message on self");
    }
  }
}
/*===========================================================================

  FUNCTION:  constructDDSSwitchIPCMessage

===========================================================================*/
/*!
    @brief

    @return
*/
/*=========================================================================*/
std::shared_ptr<IPCMessage> DataModule::constructDDSSwitchIPCMessage(IPCIStream& is) {
  // The message will have its members initialized in deserialize
  std::shared_ptr<DDSSwitchIPCMessage> ipcMessage = std::make_shared<DDSSwitchIPCMessage>(
     DDSSwitchIPCMessageState::ApStarted, -1);
  if (ipcMessage != nullptr) {
    ipcMessage->deserialize(is);
  } else {
    Log::getInstance().d("[DataModule] Invalid DDSSwitchIPCMessage");
  }
  return ipcMessage;
}

/*===========================================================================

  FUNCTION:  constructIAInfoIPCMessage

===========================================================================*/
/*!
    @brief

    @return
*/
/*=========================================================================*/
std::shared_ptr<IPCMessage> DataModule::constructIAInfoIPCMessage(IPCIStream& is) {
  // The message will have its members initialized in deserialize
  std::shared_ptr<IAInfoIPCMessage> ipcMessage = std::make_shared<IAInfoIPCMessage>(
     IAInfoIPCMessageState::IAStartRequest);
  if (ipcMessage != nullptr) {
    ipcMessage->deserialize(is);
  } else {
    Log::getInstance().d("[DataModule] Invalid IAInfoIPCMessage");
  }
  return ipcMessage;
}

/*===========================================================================

  FUNCTION:  map_internalerr_from_reqlist_new_to_ril_err

===========================================================================*/
/*!
    @brief
    Helper API to convert data of IxErrnoType type to RIL_Errno type
    input: data of IxErrnoType type

    @return
*/
/*=========================================================================*/
RIL_Errno DataModule::map_internalerr_from_reqlist_new_to_ril_err(IxErrnoType error) {
  RIL_Errno ret;
  switch (error) {
     case E_SUCCESS:
       ret = RIL_E_SUCCESS;
       break;
     case E_NOT_ALLOWED:
       ret = RIL_E_INVALID_STATE; //needs to be changed after internal discussion
       break;
     case E_NO_MEMORY:
       ret = RIL_E_NO_MEMORY;
       break;
     case E_NO_RESOURCES:
       ret = RIL_E_NO_RESOURCES;
       break;
     case E_RADIO_NOT_AVAILABLE:
       ret = RIL_E_RADIO_NOT_AVAILABLE;
       break;
     case E_INVALID_ARG:
       ret = RIL_E_INVALID_ARGUMENTS;
       break;
     default:
       ret = RIL_E_INTERNAL_ERR;
       break;
  }
  return ret;
}

void DataModule::handleSetupDataCallRequestMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  logBuffer.addLogWithTimestamp("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<SetupDataCallRequestMessage> m = std::static_pointer_cast<SetupDataCallRequestMessage>(msg);
  if (m != NULL) {
    if (call_manager != nullptr && !mIsPdcRefreshInProgress) {
      Dispatcher::getInstance().clearTimeoutForMessage(m);
      call_manager->handleCallEventMessage(CallEventTypeEnum::SetupDataCall, msg);
    } else {
      Log::getInstance().d("call_manager is null");
      SetupDataCallResponse_t result;
      result.respErr = ResponseError_t::INTERNAL_ERROR;
      result.call.cause = DataCallFailCause_t::ERROR_UNSPECIFIED;
      result.call.suggestedRetryTime = -1;
      auto resp = std::make_shared<SetupDataCallResponse_t>(result);
      m->sendResponse(m, Message::Callback::Status::SUCCESS, resp);
    }
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleDeactivateDataCallRequestMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  logBuffer.addLogWithTimestamp("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<DeactivateDataCallRequestMessage> m = std::static_pointer_cast<DeactivateDataCallRequestMessage>(msg);
  if (m != NULL) {
    if (call_manager != nullptr) {
      Dispatcher::getInstance().clearTimeoutForMessage(m);
      call_manager->handleCallEventMessage(CallEventTypeEnum::DeactivateDataCall, msg);
    } else {
      Log::getInstance().d("call_manager is null");
      ResponseError_t result = ResponseError_t::INTERNAL_ERROR;
      auto resp = std::make_shared<ResponseError_t>(result);
      m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
    }
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleGetQosDataRequestMessage(std::shared_ptr<Message> msg) {
  auto m = std::static_pointer_cast<GetQosDataRequestMessage>(msg);
  if (m == nullptr || call_manager == nullptr) {
    Log::getInstance().d("Unable to process the Message");
    return;
  }
  QosParamResult_t result = {};
  Log::getInstance().d("[" + mName + "]: Handling msg = " + m->dump());

  result.cid = m->getCid();
  result.qosSessions = call_manager->getQosSessionsforCid(m->getCid());
  Message::Callback::Status status = Message::Callback::Status::SUCCESS;
  auto resp = std::make_shared<QosParamResult_t>(result);
  m->sendResponse(msg, status, resp);
}

void DataModule::handleGetRadioDataCallListRequestMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  std::shared_ptr<GetRadioDataCallListRequestMessage> m = std::static_pointer_cast<GetRadioDataCallListRequestMessage>(msg);
  if (m != NULL) {
    Message::Callback::Status status = Message::Callback::Status::SUCCESS;
    DataCallListResult_t result;
    result.respErr = ResponseError_t::NO_ERROR;

    if (call_manager) {
      call_manager->getRadioDataCallList(result.call);
    } else {
      Log::getInstance().d("call_manager is null");
      result.respErr = ResponseError_t::INTERNAL_ERROR;
    }
    auto resp = std::make_shared<DataCallListResult_t>(result);
    m->sendResponse(msg, status, resp);
  } else {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump());
  }
}

void DataModule::handleDataCallTimerExpiredMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  logBuffer.addLogWithTimestamp("[" + mName + "]: Handling msg = " + msg->dump());
  if (call_manager != nullptr) {
    call_manager->handleCallEventMessage(CallEventTypeEnum::TimerExpired, msg);
  } else {
    Log::getInstance().d("No call_manager created");
  }
}

void DataModule::handleLinkPropertiesChangedMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (call_manager) {
    call_manager->handleCallEventMessage(CallEventTypeEnum::LinkPropertiesChanged, msg);
  }
  else {
    Log::getInstance().d("call_manager is null");
  }
}

void DataModule::handleSetCarrierInfoImsiEncryptionMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<SetCarrierInfoImsiEncryptionMessage> m = std::static_pointer_cast<SetCarrierInfoImsiEncryptionMessage>(msg);
  if ( m == NULL ) {
    Log::getInstance().d("Msg is null");
    return;
  }
  RIL_Errno ret = RIL_E_INTERNAL_ERR;
  if (auth_manager) {
    auto rc = auth_manager->setCarrierInfoImsiEncryption(m->getImsiEncryptionInfo());
    if( rc )
      ret = RIL_E_SUCCESS;
  } else {
    Log::getInstance().d("auth_manager is null");
  }
  auto resp = std::make_shared<RIL_Errno>(ret);
  m->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
}

void DataModule::handleRegistrationFailureReportingStatusMessage(std::shared_ptr<Message> msg) {
  if (msg != nullptr) {
      std::shared_ptr<RegistrationFailureReportingStatusMessage> m = std::static_pointer_cast<RegistrationFailureReportingStatusMessage>(msg);
      Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
      if (m != nullptr) {
        if (mRegistrationFailureCauseReport != m->getReportingStatus()) {
          mRegistrationFailureCauseReport = m->getReportingStatus();
          if (wds_endpoint) {
            wds_endpoint->registerDataRegistrationRejectCause(mRegistrationFailureCauseReport);
          }
          else {
            Log::getInstance().d("wds_endpoint is null");
          }
          if (call_manager) {
            call_manager->registerDataRegistrationRejectCause(mRegistrationFailureCauseReport);
          }
          else {
            Log::getInstance().d("call_manager is null");
          }
        }
      }
      else {
        Log::getInstance().d("[" + mName + "]: Invalid msg received");
      }
  }
}

/*============================================================================

    qcrilDataprocessMccMncInfo

============================================================================*/
/*!
    @brief
    Process mcc mnc info

    @return
    None
*/
/*=========================================================================*/
void qcrilDataprocessMccMncInfo
(
    const qcril_request_params_type *const params_ptr,
    qcril_request_return_type       *const ret_ptr
)
{
  qcril_uim_mcc_mnc_info_type *uim_mcc_mnc_info = NULL;

  Log::getInstance().d("qcrilDataprocessMccMncInfo: ENTRY");

  if ((params_ptr == NULL) || (ret_ptr == NULL))
  {
    Log::getInstance().d("ERROR!! Invalid input, cannot process request");
    return;
  }

  uim_mcc_mnc_info = (qcril_uim_mcc_mnc_info_type*)params_ptr->data;
  if (uim_mcc_mnc_info == NULL)
  {
    Log::getInstance().d("NULL uim_mcc_mnc_info");
    return;
  }

  if (uim_mcc_mnc_info->err_code != RIL_UIM_E_SUCCESS)
  {
    Log::getInstance().d("uim_get_mcc_mnc_info error:"+ std::to_string(uim_mcc_mnc_info->err_code));
    return;
  }

  //According to the declaration of size in 'UimGetMccMncRequestMsg.h'
  //each of mcc & mnc is 4 bytes, adding the error check based on this size

  if ( (uim_mcc_mnc_info->mcc[MCC_LENGTH - 1] != '\0')
    || (uim_mcc_mnc_info->mnc[MNC_LENGTH - 1] != '\0') )
  {
    Log::getInstance().d("ERROR!! Improper input received. Either of MCC or MNC is not NULL terminated");
    return;
  }
  std::string str = uim_mcc_mnc_info->mcc;
  std::string str1 = uim_mcc_mnc_info->mnc;
  Log::getInstance().d("mcc:"+ str+"mnc="+ str1);

  qdp_compare_and_update(uim_mcc_mnc_info->mcc,
                                 uim_mcc_mnc_info->mnc);
}

void DataModule::handleQosInitMessage(std::shared_ptr<Message> msg)
{
  auto m = std::static_pointer_cast<QosInitializeMessage>(msg);
  if (m == nullptr) {
    return;
  }
  Log::getInstance().d("[" + mName + "]: Handling msg = " + m->dump());

  if(m->isCloseQos() && mDataQosCleanup != nullptr) {
    mDataQosCleanup();
    Log::getInstance().d("[" + mName + "]: dataQos cleanup triggered");
    return;
  }

  dataQosInitParams params;
  params.cid = m->getCid();
  params.muxId = m->getMuxId();
  params.eptype = m->getEpType();
  params.epid = m->getEpId();
  params.slot_id = global_instance_id;
#ifdef QMI_RIL_UTF
// @TODO: uncomment once UTF is ready
//   dataQosInit(&params, logFn);
  return;
#endif
  if(mDataQosInit != nullptr) {
    mDataQosInit(&params, logFn);
  } else {
    Log::getInstance().d("[" + mName + "]: loadDataQos not loaded or encountered error");
  }
}

#if (defined (QMI_RIL_UTF) || defined(RIL_FOR_MDM_LE))
void dump_data_module(int fd) {
  getDataModule().dump(fd);
  getDataModule().flush();
}
#endif

#ifdef QMI_RIL_UTF
void qcril_qmi_hal_data_module_cleanup() {
  getDataModule().cleanup();
}

void DataModule::cleanup()
{
  std::shared_ptr<DSDModemEndPoint> mDsdModemEndPoint =
      ModemEndPointFactory<DSDModemEndPoint>::getInstance().buildEndPoint();
  DSDModemEndPointModule* mDsdModemEndPointModule =
      (DSDModemEndPointModule*)mDsdModemEndPoint->mModule;
  mDsdModemEndPointModule->cleanUpQmiSvcClient();

  std::shared_ptr<AuthModemEndPoint> mAuthModemEndPoint =
      ModemEndPointFactory<AuthModemEndPoint>::getInstance().buildEndPoint();
  AuthModemEndPointModule* mAuthModemEndPointModule =
      (AuthModemEndPointModule*)mAuthModemEndPoint->mModule;
  mAuthModemEndPointModule->cleanUpQmiSvcClient();

  std::shared_ptr<OTTModemEndPoint> mOTTModemEndPoint =
      ModemEndPointFactory<OTTModemEndPoint>::getInstance().buildEndPoint();
  OTTModemEndPointModule* mOTTModemEndPointModule =
      (OTTModemEndPointModule*)mOTTModemEndPoint->mModule;
  mOTTModemEndPointModule->cleanUpQmiSvcClient();

  std::shared_ptr<WDSModemEndPoint> mWDSModemEndPoint =
      ModemEndPointFactory<WDSModemEndPoint>::getInstance().buildEndPoint();
  WDSModemEndPointModule* mWDSModemEndPointModule =
      (WDSModemEndPointModule*)mWDSModemEndPoint->mModule;
  mWDSModemEndPointModule->cleanUpQmiSvcClient();

  std::shared_ptr<PDCModemEndPoint> mPDCModemEndPoint =
      ModemEndPointFactory<PDCModemEndPoint>::getInstance().buildEndPoint();
  PDCModemEndPointModule* mPDCModemEndPointModule =
      (PDCModemEndPointModule*)mPDCModemEndPoint->mModule;
  mPDCModemEndPointModule->cleanUpQmiSvcClient();

  mInitTracker = InitTracker();
  mInitCompleted = false;
  iwlanHandshakeMsgToken = INVALID_MSG_TOKEN;
  preferred_data_sm = std::make_unique<PreferredDataStateMachine>();
  preferred_data_state_info = std::make_shared<PreferredDataInfo_t>();
  preferred_data_state_info->isRilIpcNotifier = false;
  preferred_data_state_info->mVoiceCallInfo.voiceSubId = SubId::UNSPECIFIED;
  preferred_data_sm->initialize(preferred_data_state_info);
  preferred_data_sm->setCurrentState(Initial);
  preferred_data_state_info->pendingIACb = std::make_shared<std::function<void(std::shared_ptr<Message>)>>(std::bind(&DataModule::processInitialAttachRequestMessage, this, std::placeholders::_1));
  preferred_data_state_info->iaSendResponse = std::make_shared<std::function<void(std::shared_ptr<Message>)>>(std::bind(&DataModule::handleSetInitialAttachCompleted, this, std::placeholders::_1));
  #if !defined(RIL_FOR_LOW_RAM) || defined(RIL_FOR_MDM_LE)
  voiceCallEndPointSub0 = nullptr;
  voiceCallEndPointSub1 = nullptr;
  keep_alive = std::make_shared<KeepAliveHandler>();
  #endif
  currentDDSSUB = { QCRIL_INVALID_MODEM_STACK_ID, DSD_DDS_DURATION_PERMANANT_V01 };

  auth_manager = nullptr;
  profile_handler = nullptr;
  call_manager = nullptr;
  network_service_handler = nullptr;
  mIsPdcRefreshInProgress = false;
  memset(&mCachedSystemStatus, 0, sizeof(dsd_system_status_ind_msg_v01));
  iwlanHandshakeMsgToken = INVALID_MSG_TOKEN;
  getPendingMessageList().clear();
  mInitTracker.setIWLANMode(rildata::IWLANOperationMode::AP_ASSISTED);
  qcril_data_qmi_wds_release();
  qcril_data_lqe_release();
  // dump(0);
}
#endif

void DataModule::handleDsiInitCompletedMessage(std::shared_ptr<Message> msg)
{
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  std::shared_ptr<DsiInitCompletedMessage> m = std::static_pointer_cast<DsiInitCompletedMessage>(msg);

  if( (m == NULL) || (call_manager == NULL))
  {
    Log::getInstance().d("[" + mName + "]: Improper message received = " + msg->dump() + "or Callmanager is NULL");
    return;
  }
  if(call_manager->dsiInitStatus  == DsiInitStatus_t::PENDING_RELEASE)
  {
    call_manager->triggerDsiRelease();
    if( (mInitTracker.getDsdServiceReady() ||
              mInitTracker.getWdsServiceReady() || mInitTracker.getAuthServiceReady()) )
    {
      call_manager->triggerDsiInit(!mInitCompleted);
    }
  }
  else
  {
    call_manager->dsiInitStatus = DsiInitStatus_t::COMPLETED;
    Log::getInstance().d("DSI init is completed");
  }
}

void DataModule::handleNasRfBandInfoIndMessage(std::shared_ptr<Message> msg)
{
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (call_manager != nullptr) {
    call_manager->updateNasRfActiveBandInfo(msg);
  } else {
    Log::getInstance().d("No call_manager created");
  }
}

void DataModule::handleNasRfBandInfoMessage(std::shared_ptr<Message> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (call_manager != nullptr) {
    call_manager->handleNasRfBandInfoMessage(msg);
  } else {
    Log::getInstance().d("No call_manager created");
  }
}
}//namespace
