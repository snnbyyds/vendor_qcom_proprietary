/******************************************************************************
#  Copyright (c) 2018,2020,2022 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include <ril_service.h>
#include <framework/Log.h>
#include <framework/Module.h>
#include <framework/QcrilInitMessage.h>
#include <framework/UnSolicitedMessage.h>
#include <interfaces/sms/RilUnsolIncoming3GppSMSMessage.h>
#include <interfaces/sms/RilUnsolIncoming3Gpp2SMSMessage.h>
#include <interfaces/sms/RilUnsolNewSmsStatusReportMessage.h>
#include <interfaces/sms/RilUnsolNewBroadcastSmsMessage.h>
#include <interfaces/sms/RilUnsolStkCCAlphaNotifyMessage.h>
#include <interfaces/sms/RilUnsolImsNetworkStateChangedMessage.h>

#include "interfaces/nas/RilUnsolNetworkStateChangedMessage.h"
#include "interfaces/nas/RilUnsolNitzTimeReceivedMessage.h"
#include "interfaces/nas/RilUnsolVoiceRadioTechChangedMessage.h"
#include "interfaces/nas/RilUnsolNetworkScanResultMessage.h"
#include "interfaces/nas/RilUnsolSignalStrengthMessage.h"
#include "interfaces/nas/RilUnsolEmergencyCallbackModeMessage.h"
#include "interfaces/nas/RilUnsolRadioCapabilityMessage.h"
#include "interfaces/nas/RilUnsolCdmaPrlChangedMessage.h"
#include "interfaces/nas/RilUnsolRestrictedStateChangedMessage.h"
#include "interfaces/nas/RilUnsolUiccSubsStatusChangedMessage.h"
#include "interfaces/nas/RilUnsolRadioStateChangedMessage.h"
#include "interfaces/nas/RilUnsolCdmaSubscriptionSourceChangedMessage.h"
#include "interfaces/RilAcknowledgeRequestMessage.h"

#include <interfaces/uim/UimSimStatusChangedInd.h>
#include <interfaces/uim/UimSimRefreshIndication.h>
#include <interfaces/gstk/GstkUnsolIndMsg.h>

#include <interfaces/voice/QcRilUnsolCallStateChangeMessage.h>
#include <interfaces/voice/QcRilUnsolCallRingingMessage.h>
#include <interfaces/voice/QcRilUnsolSupplementaryServiceMessage.h>
#include <interfaces/voice/QcRilUnsolSrvccStatusMessage.h>
#include <interfaces/voice/QcRilUnsolRingbackToneMessage.h>
#include <interfaces/voice/QcRilUnsolCdmaOtaProvisionStatusMessage.h>
#include <interfaces/voice/QcRilUnsolCdmaCallWaitingMessage.h>
#include <interfaces/voice/QcRilUnsolSuppSvcNotificationMessage.h>
#include <interfaces/voice/QcRilUnsolOnUssdMessage.h>
#include <interfaces/voice/QcRilUnsolCdmaInfoRecordMessage.h>

#ifdef RIL_FOR_LOW_RAM
#include "request/DeactivateDataCallRequestMessage.h"
#include "request/SetupDataCallRequestMessage.h"
#include "UnSolMessages/RadioKeepAliveStatusIndMessage.h"
#include "UnSolMessages/RadioDataCallListChangeIndMessage.h"
#include "UnSolMessages/GetDataCallListResponseIndMessage.h"
#endif

class RilServiceModule: public Module {
    public:
        RilServiceModule(qcril_instance_id_e_type instance);
        virtual void init() {
            Module::init();
        }
    protected:
        sp<RadioImpl> getRadioImpl();
        qcril_instance_id_e_type mInstance;
        virtual void handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg);
    private:
        void handleIncoming3GppSMSMessage(std::shared_ptr<RilUnsolIncoming3GppSMSMessage> msg);
        void handleIncoming3Gpp2SMSMessage(std::shared_ptr<RilUnsolIncoming3Gpp2SMSMessage> msg);
        void handleNewSmsOnSimMessage(std::shared_ptr<RilUnsolNewSmsOnSimMessage> msg);
        void handleNewSmsStatusReportMessage(std::shared_ptr<RilUnsolNewSmsStatusReportMessage> msg);
        void handleNewBroadcastSmsMessage(std::shared_ptr<RilUnsolNewBroadcastSmsMessage> msg);
        void handleStkCCAlphaNotifyMessage(std::shared_ptr<RilUnsolStkCCAlphaNotifyMessage> msg);
        void handleCdmaRuimSmsStorageFullMessage(std::shared_ptr<RilUnsolCdmaRuimSmsStorageFullMessage> msg);
        void handleSimSmsStorageFullMessage(std::shared_ptr<RilUnsolSimSmsStorageFullMessage> msg);
        void handleImsNetworkStateChangedMessage(std::shared_ptr<RilUnsolImsNetworkStateChangedMessage> msg);

        void handleQcRilUnsolCallStateChangeMessage(std::shared_ptr<QcRilUnsolCallStateChangeMessage> msg);
        void handleQcRilUnsolCallRingingMessage(std::shared_ptr<QcRilUnsolCallRingingMessage> msg);
        void handleQcRilUnsolSupplementaryServiceMessage(std::shared_ptr<QcRilUnsolSupplementaryServiceMessage> msg);
        void handleQcRilUnsolSrvccStatusMessage(std::shared_ptr<QcRilUnsolSrvccStatusMessage> msg);
        void handleQcRilUnsolRingbackToneMessage(std::shared_ptr<QcRilUnsolRingbackToneMessage> msg);
        void handleQcRilUnsolCdmaOtaProvisionStatusMessage(std::shared_ptr<QcRilUnsolCdmaOtaProvisionStatusMessage> msg);
        void handleQcRilUnsolCdmaCallWaitingMessage(std::shared_ptr<QcRilUnsolCdmaCallWaitingMessage> msg);
        void handleQcRilUnsolSuppSvcNotificationMessage(std::shared_ptr<QcRilUnsolSuppSvcNotificationMessage> msg);
        void handleQcRilUnsolOnUssdMessage(std::shared_ptr<QcRilUnsolOnUssdMessage> msg);
        void handleQcRilUnsolCdmaInfoRecordMessage(std::shared_ptr<QcRilUnsolCdmaInfoRecordMessage> msg);

        void handleUimSimStatusChangedInd(std::shared_ptr<UimSimStatusChangedInd> msg);
        void handleUimSimRefreshIndication(std::shared_ptr<UimSimRefreshIndication> msg);
        void handleGstkUnsolIndMsg(std::shared_ptr<GstkUnsolIndMsg> msg);

        void handleAcknowledgeRequestMessage(std::shared_ptr<RilAcknowledgeRequestMessage> msg);
        void handleNetworkStateChangedMessage(std::shared_ptr<RilUnsolNetworkStateChangedMessage> msg);
        void handleNitzTimeReceivedMessage(std::shared_ptr<RilUnsolNitzTimeReceivedMessage> msg);
        void handleVoiceRadioTechChangedMessage(std::shared_ptr<RilUnsolVoiceRadioTechChangedMessage> msg);
        void handleNetworkScanResultMessage(std::shared_ptr<RilUnsolNetworkScanResultMessage> msg);
        void handleSignalStrengthMessage(std::shared_ptr<RilUnsolSignalStrengthMessage> msg);
        void handleEmergencyCallbackModeMessage(std::shared_ptr<RilUnsolEmergencyCallbackModeMessage> msg);
        void handlelRadioCapabilityMessage(std::shared_ptr<RilUnsolRadioCapabilityMessage> msg);
        void handleCdmaPrlChangedMessage(std::shared_ptr<RilUnsolCdmaPrlChangedMessage> msg);
        void handleRestrictedStateChangedMessage(std::shared_ptr<RilUnsolRestrictedStateChangedMessage> msg);
        void handleUiccSubsStatusChangedMessage(std::shared_ptr<RilUnsolUiccSubsStatusChangedMessage> msg);
        void handleRadioStateChangedMessage(std::shared_ptr<RilUnsolRadioStateChangedMessage> msg);
        void handleCdmaSubscriptionSourceChangedMessage(
            std::shared_ptr<RilUnsolCdmaSubscriptionSourceChangedMessage> msg);

#ifdef RIL_FOR_LOW_RAM
        void handleSetupDataCallRadioResponseIndMessage(std::shared_ptr<rildata::SetupDataCallRadioResponseIndMessage_1_0> msg);
        void handleDeactivateDataCallRadioResponseIndMessage(std::shared_ptr<rildata::DeactivateDataCallRadioResponseIndMessage_1_0> msg);
        void handleRadioKeepAliveStatusIndMessage(std::shared_ptr<rildata::RadioKeepAliveStatusIndMessage> msg);
        void handleRadioDataCallListChangeIndMessage(std::shared_ptr<rildata::RadioDataCallListChangeIndMessage_1_0> msg);
        void handleGetDataCallListResponseIndMessage(std::shared_ptr<rildata::GetDataCallListResponseIndMessage> msg);
#endif
};
