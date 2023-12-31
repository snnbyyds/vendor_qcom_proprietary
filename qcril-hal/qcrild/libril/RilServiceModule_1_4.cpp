/******************************************************************************
#  Copyright (c) 2019-2020,2022 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#define TAG "RilServiceModule_1_4"
#include "RilServiceModule_1_4.h"

// Move to common files */
using MessageHandlerTable = std::unordered_map<message_id_t,
        std::function<void(std::shared_ptr<Message>)>>;

extern void updateMessageHandler(MessageHandlerTable& consolidateList,
        MessageHandlerTable& updateList);

RilServiceModule_1_4::RilServiceModule_1_4(qcril_instance_id_e_type instance,
        RadioImpl_1_4 &radioImpl): RilServiceModule_1_3(instance), mRadioImpl_1_4(radioImpl) {
    mName = "RilServiceModule_1_4";

    using std::placeholders::_1;
    MessageHandlerTable updateList = {
        HANDLER(QcrilInitMessage, RilServiceModule_1_4::handleQcrilInit),
        HANDLER(RilUnsolEmergencyListIndMessage, RilServiceModule_1_4::handleEmergencyListIndMessage),
        HANDLER(rildata::RadioDataCallListChangeIndMessage_1_4, RilServiceModule_1_4::handleRadioDataCallListChangeIndMessage),
        HANDLER(rildata::PhysicalConfigStructUpdateMessage, RilServiceModule_1_4::handlePhysicalConfigStructUpdateMessage),
        HANDLER(rildata::RadioKeepAliveStatusIndMessage, RilServiceModule_1_4::handleRadioKeepAliveStatusIndMessage),
        HANDLER(rildata::RilPCODataMessage, RilServiceModule_1_4::handleRilPCODataMessage),
        HANDLER(rildata::CarrierInfoForImsiEncryptionRefreshMessage, RilServiceModule_1_4::handleCarrierInfoForImsiEncryptionRefreshMessage),
        HANDLER(rildata::SetupDataCallRadioResponseIndMessage_1_4, RilServiceModule_1_4::handleSetupDataCallRadioResponseIndMessage),
#ifndef RIL_FOR_LOW_RAM
        HANDLER(rildata::DeactivateDataCallRadioResponseIndMessage, RilServiceModule_1_4::handleDeactivateDataCallRadioResponseIndMessage),
#else
        HANDLER(rildata::DeactivateDataCallRadioResponseIndMessage_1_4, RilServiceModule_1_4::handleDeactivateDataCallRadioResponseIndMessage),
#endif
        HANDLER(rildata::GetDataCallListResponseIndMessage, RilServiceModule_1_4::handleGetDataCallListResponseIndMessage),
    };
    updateMessageHandler(mMessageHandler, updateList);
}

void RilServiceModule_1_4::handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg) {
    QCRIL_LOG_DEBUG("Handling QcrilInitMessage %s",msg->dump().c_str());
    std::shared_ptr<NasSetSignalStrengthCriteriaSupported> cfgMsg =
        std::make_shared<NasSetSignalStrengthCriteriaSupported>(true);
    if (cfgMsg) {
        cfgMsg->dispatch();
    }
}


void RilServiceModule_1_4::handleEmergencyListIndMessage(
        std::shared_ptr<RilUnsolEmergencyListIndMessage> msg) {
    auto ri = getRadioImpl();
    if (ri) {
        // The ATEL will not acknowledge this UNSOL, hence no wakelock is required

        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        mRadioImpl_1_4.sendcurrentEmergencyNumberList(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}

void RilServiceModule_1_4::handleRadioDataCallListChangeIndMessage(std::shared_ptr<rildata::RadioDataCallListChangeIndMessage_1_4> msg)
{
    if(msg) {
        QCRIL_LOG_DEBUG("Handling handleRadioDataCallListChangeIndMessage %s",msg->dump().c_str());

        auto ri = getRadioImpl();
        if (ri) {
            auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
            radioServiceRwlockPtr->lock_shared();

            std::vector<rildata::DataCallResult_t> dcList = msg->getDCList();
            ::android::hardware::hidl_vec<V1_4::SetupDataCallResult> dcResultList;
            QCRIL_LOG_DEBUG("dcList %d",dcList.size());
            dcResultList.resize(dcList.size());
            int i=0;
            for (rildata::DataCallResult_t entry: dcList) {
                dcResultList[i] = mRadioImpl_1_4.convertDcResultToHidlDcResult_1_4(entry);
                i++;
            }
            mRadioImpl_1_4.dataCallListChanged(RadioIndicationType::UNSOLICITED, dcResultList);

            radioServiceRwlockPtr->unlock_shared();
        }
    }
    else {
        QCRIL_LOG_DEBUG("Handling handleRadioDataCallListChangeIndMessage is null");
    }
}

V1_4::RadioTechnology convertRadioAccessFamilyTypeToRadioTechnology(rildata::RadioAccessFamily_t accessFamily)
{
    V1_4::RadioTechnology radioTech = V1_4::RadioTechnology::UNKNOWN;

    switch(accessFamily) {
        case rildata::RadioAccessFamily_t::GPRS:
            radioTech = V1_4::RadioTechnology::GPRS;
            break;
        case rildata::RadioAccessFamily_t::EDGE:
            radioTech = V1_4::RadioTechnology::EDGE;
            break;
        case rildata::RadioAccessFamily_t::UMTS:
            radioTech = V1_4::RadioTechnology::UMTS;
            break;
        case rildata::RadioAccessFamily_t::IS95A:
            radioTech = V1_4::RadioTechnology::IS95A;
            break;
        case rildata::RadioAccessFamily_t::IS95B:
            radioTech = V1_4::RadioTechnology::IS95B;
            break;
        case rildata::RadioAccessFamily_t::ONE_X_RTT:
            radioTech = V1_4::RadioTechnology::ONE_X_RTT;
            break;
        case rildata::RadioAccessFamily_t::EVDO_0:
            radioTech = V1_4::RadioTechnology::EVDO_0;
            break;
        case rildata::RadioAccessFamily_t::EVDO_A:
            radioTech = V1_4::RadioTechnology::EVDO_A;
            break;
        case rildata::RadioAccessFamily_t::HSDPA:
            radioTech = V1_4::RadioTechnology::HSDPA;
            break;
        case rildata::RadioAccessFamily_t::HSUPA:
            radioTech = V1_4::RadioTechnology::HSUPA;
            break;
        case rildata::RadioAccessFamily_t::HSPA:
            radioTech = V1_4::RadioTechnology::HSPA;
            break;
        case rildata::RadioAccessFamily_t::EVDO_B:
            radioTech = V1_4::RadioTechnology::EVDO_B;
            break;
        case rildata::RadioAccessFamily_t::EHRPD:
            radioTech = V1_4::RadioTechnology::EHRPD;
            break;
        case rildata::RadioAccessFamily_t::LTE:
            radioTech = V1_4::RadioTechnology::LTE;
            break;
        case rildata::RadioAccessFamily_t::HSPAP:
            radioTech = V1_4::RadioTechnology::HSPAP;
            break;
        case rildata::RadioAccessFamily_t::GSM:
            radioTech = V1_4::RadioTechnology::GSM;
            break;
        case rildata::RadioAccessFamily_t::TD_SCDMA:
            radioTech = V1_4::RadioTechnology::TD_SCDMA;
            break;
        case rildata::RadioAccessFamily_t::LTE_CA:
            radioTech = V1_4::RadioTechnology::LTE_CA;
            break;
        case rildata::RadioAccessFamily_t::NR:
            radioTech = V1_4::RadioTechnology::NR;
            break;
        default:
            radioTech = V1_4::RadioTechnology::UNKNOWN;
            break;
    }
    return radioTech;
}

V1_2::CellConnectionStatus convertCellConnStatusToHalType(rildata::CellConnectionStatus_t status)
{
    V1_2::CellConnectionStatus cellConnStatus = V1_2::CellConnectionStatus::NONE;
    switch(status) {
        case rildata::CellConnectionStatus_t::PRIMARY_SERVING:
            cellConnStatus = V1_2::CellConnectionStatus::PRIMARY_SERVING;
            break;
        case rildata::CellConnectionStatus_t::SECONDARY_SERVING:
            cellConnStatus = V1_2::CellConnectionStatus::SECONDARY_SERVING;
            break;
        default:
            break;
    }

    return cellConnStatus;
};

V1_4::FrequencyRange convertFrequencyRangeToHalType(rildata::FrequencyRange_t range)
{
    V1_4::FrequencyRange freqRange = V1_4::FrequencyRange::LOW;
    switch(range) {
        case rildata::FrequencyRange_t::MMWAVE:
            freqRange = V1_4::FrequencyRange::MMWAVE;
            break;
        case rildata::FrequencyRange_t::HIGH:
            freqRange = V1_4::FrequencyRange::HIGH;
            break;
        case rildata::FrequencyRange_t::MID:
            freqRange = V1_4::FrequencyRange::MID;
            break;
        case rildata::FrequencyRange_t::LOW:
            freqRange = V1_4::FrequencyRange::LOW;
            break;
        case rildata::FrequencyRange_t::UNKNOWN:
        default:
            freqRange = (V1_4::FrequencyRange)(rildata::FrequencyRange_t::UNKNOWN);
            break;
    }

    return freqRange;
}

void RilServiceModule_1_4::handlePhysicalConfigStructUpdateMessage(std::shared_ptr<rildata::PhysicalConfigStructUpdateMessage> msg) {
    if(msg) {
        QCRIL_LOG_DEBUG("Handling PhysicalConfigStructUpdateMessage %s",msg->dump().c_str());

        auto ri = getRadioImpl();
        if (ri) {
            std::vector<rildata::PhysicalConfigStructInfo_t> configs = msg->getPhysicalConfigStructInfo();
            ::android::hardware::hidl_vec<V1_4::PhysicalChannelConfig> physChanConfig;
            physChanConfig.resize(configs.size());
            int i=0;
            for (const auto &entry : configs) {
                physChanConfig[i].base.status = convertCellConnStatusToHalType(entry.status);
                physChanConfig[i].base.cellBandwidthDownlink = entry.cellBandwidthDownlink;
                physChanConfig[i].rat = convertRadioAccessFamilyTypeToRadioTechnology(entry.rat);
                V1_4::FrequencyRange range = convertFrequencyRangeToHalType(entry.rfInfo.range);
                physChanConfig[i].rfInfo.range(range);
                int j=0;
                physChanConfig[i].contextIds.resize(entry.contextIds.size());
                for (const auto &cid : entry.contextIds) {
                    physChanConfig[i].contextIds[j] = cid;
                    j++;
                }
                physChanConfig[i].physicalCellId = entry.physicalCellId;
                i++;
            }

            mRadioImpl_1_4.currentPhysicalChannelConfigs(RadioIndicationType::UNSOLICITED, physChanConfig);
        }
    }
    else {
        QCRIL_LOG_DEBUG("Handling PhysicalConfigStructUpdateMessage is null");
    }
}

void RilServiceModule_1_4::handleSetupDataCallRadioResponseIndMessage(std::shared_ptr<rildata::SetupDataCallRadioResponseIndMessage_1_4> msg)
{
    if (msg) {
        QCRIL_LOG_DEBUG("Handling handleSetupDataCallRadioResponseIndMessage %s",msg->dump().c_str());
        auto ri = getRadioImpl();
        if (ri) {
            auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
            radioServiceRwlockPtr->lock_shared();
            auto rsp = msg->getResponse();
            auto status = msg->getStatus();
            auto serial = msg->getSerial();

            RadioResponseInfo responseInfo {.serial = serial, .error = RadioError::NO_MEMORY};
            V1_4::SetupDataCallResult dcResult = {};
            dcResult.cause = V1_4::DataCallFailCause::ERROR_UNSPECIFIED;
            dcResult.suggestedRetryTime = -1;
            RadioError e = RadioError::NONE;
            if (status == Message::Callback::Status::SUCCESS) {
                QCRIL_LOG_DEBUG("setup data call cb invoked status %d respErr %d", status, rsp.respErr);
                QCRIL_LOG_DEBUG("cause = %d", rsp.call.cause);
                QCRIL_LOG_DEBUG("suggestedRetryTime = %d", rsp.call.suggestedRetryTime);
                QCRIL_LOG_DEBUG("cid = %d", rsp.call.cid);
                QCRIL_LOG_DEBUG("active = %d", rsp.call.active);
                QCRIL_LOG_DEBUG("type = %s", rsp.call.type.c_str());
                QCRIL_LOG_DEBUG("ifname = %s", rsp.call.ifname.c_str());
                QCRIL_LOG_DEBUG("addresses = %s", rsp.call.addresses.c_str());
                QCRIL_LOG_DEBUG("dnses = %s", rsp.call.dnses.c_str());
                QCRIL_LOG_DEBUG("gateways = %s", rsp.call.gateways.c_str());
                QCRIL_LOG_DEBUG("pcscf = %s", rsp.call.pcscf.c_str());
                QCRIL_LOG_DEBUG("mtu = %d", rsp.call.mtu);

                dcResult = mRadioImpl_1_4.convertDcResultToHidlDcResult_1_4(rsp.call);
            }
            else {
                switch(rsp.respErr) {
                    case rildata::ResponseError_t::NOT_SUPPORTED: e = RadioError::REQUEST_NOT_SUPPORTED; break;
                    case rildata::ResponseError_t::INVALID_ARGUMENT: e = RadioError::INVALID_ARGUMENTS; break;
                    default: e = RadioError::GENERIC_FAILURE; break;
                }
            }
            responseInfo = {.serial = serial, .error = e};
            mRadioImpl_1_4.setupDataCallResponse_1_4(responseInfo, dcResult);

            radioServiceRwlockPtr->unlock_shared();
        }
    }
}

#ifdef RIL_FOR_LOW_RAM
void RilServiceModule_1_4::handleDeactivateDataCallRadioResponseIndMessage(std::shared_ptr<rildata::DeactivateDataCallRadioResponseIndMessage_1_4> msg)
#else
void RilServiceModule_1_4::handleDeactivateDataCallRadioResponseIndMessage(std::shared_ptr<rildata::DeactivateDataCallRadioResponseIndMessage> msg)
#endif
{
    auto ri = getRadioImpl();
    if (msg) {
        QCRIL_LOG_DEBUG("Handling handleDeactivateDataCallRadioResponseIndMessage %s",msg->dump().c_str());
        if (ri) {
            auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
            radioServiceRwlockPtr->lock_shared();
            auto rsp = msg->getResponse();
            auto status = msg->getStatus();
            auto serial = msg->getSerial();
            RadioResponseInfo responseInfo {.serial = serial, .error = RadioError::NO_MEMORY};
            RadioError e = RadioError::NONE;
            if ((status != Message::Callback::Status::SUCCESS) ||
                (rsp != rildata::ResponseError_t::NO_ERROR)) {
                switch(rsp) {
                    case rildata::ResponseError_t::NOT_SUPPORTED: e = RadioError::REQUEST_NOT_SUPPORTED; break;
                    case rildata::ResponseError_t::INVALID_ARGUMENT: e = RadioError::INVALID_ARGUMENTS; break;
                    case rildata::ResponseError_t::CALL_NOT_AVAILABLE: e = RadioError::INVALID_CALL_ID; break;
                    default: e = RadioError::GENERIC_FAILURE; break;
                }
            }
            responseInfo = {.serial = serial, .error = e};
            QCRIL_LOG_DEBUG("deactivate data call cb invoked status %d respErr %d", status, rsp);
            mRadioImpl_1_4.deactivateDataCallResponse(responseInfo);
            radioServiceRwlockPtr->unlock_shared();
        }
    }
}

void RilServiceModule_1_4::handleRadioKeepAliveStatusIndMessage(std::shared_ptr<rildata::RadioKeepAliveStatusIndMessage> msg) {
    if(msg) {
      QCRIL_LOG_DEBUG("Handling RadioKeepAliveStatusIndMessage %s", msg->dump().c_str());

      auto ri = getRadioImpl();
      if (ri) {
        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        if(radioServiceRwlockPtr)
        {
          radioServiceRwlockPtr->lock_shared();

          V1_1::KeepaliveStatus status;
          status.sessionHandle = msg->getHandle();
          status.code = (::android::hardware::radio::V1_1::KeepaliveStatusCode)(msg->getStatusCode());
          mRadioImpl_1_4.KeepAliveStatusInd(RadioIndicationType::UNSOLICITED, status);
          radioServiceRwlockPtr->unlock_shared();
        } else {
          QCRIL_LOG_DEBUG("radioServiceRwlockPtr is NULL");
        }
      }
    } else {
        QCRIL_LOG_DEBUG("Handling RadioKeepAliveStatusIndicationMessage is null");
    }
}

void RilServiceModule_1_4::handleRilPCODataMessage(std::shared_ptr<rildata::RilPCODataMessage> msg) {
    if(msg) {
      QCRIL_LOG_DEBUG("Handling RilPCODataMessage %s", msg->dump().c_str());

      auto ri = getRadioImpl();
      if (ri) {
        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        if(radioServiceRwlockPtr)
        {
          radioServiceRwlockPtr->lock_shared();

          PcoDataInfo pco = {};
          #ifdef UPDATE_PCO_DATA
          rildata::PcoDataInfo_t pcoData = msg->getPcoDataInfo();
          QCRIL_LOG_DEBUG("pcoData.cid %d", pcoData.cid);
          QCRIL_LOG_DEBUG("pcoData.bearerProto %s", pcoData.bearerProto.c_str());
          QCRIL_LOG_DEBUG("pcoData.pcoId %d", pcoData.pcoId);
          QCRIL_LOG_DEBUG("pcoData.contents size %d", pcoData.contents.size());
          for (const auto &content : pcoData.contents) {
              QCRIL_LOG_DEBUG("pcoData.content %d", content);
          }
          pco.cid = pcoData.cid;
          pco.bearerProto = pcoData.bearerProto;
          pco.pcoId = pcoData.pcoId;
          pco.contents = pcoData.contents;
          #endif
          mRadioImpl_1_4.updatePcoData(RadioIndicationType::UNSOLICITED, pco);
          radioServiceRwlockPtr->unlock_shared();
        } else {
          QCRIL_LOG_DEBUG("radioServiceRwlockPtr is NULL");
        }
      }
    } else {
        QCRIL_LOG_DEBUG("Handling RilPCODataMessage is null");
    }
}

void RilServiceModule_1_4::handleCarrierInfoForImsiEncryptionRefreshMessage(std::shared_ptr<rildata::CarrierInfoForImsiEncryptionRefreshMessage> msg) {
    if(msg) {
      QCRIL_LOG_DEBUG("Handling CarrierInfoForImsiEncryptionRefreshMessage %s", msg->dump().c_str());

      auto ri = getRadioImpl();
      if (ri) {
        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        if(radioServiceRwlockPtr)
        {
          radioServiceRwlockPtr->lock_shared();
          mRadioImpl_1_4.carrierInfoForImsiEncryptionRefresh(RadioIndicationType::UNSOLICITED);
          radioServiceRwlockPtr->unlock_shared();
        } else {
          QCRIL_LOG_DEBUG("radioServiceRwlockPtr is NULL");
        }
      }
    } else {
        QCRIL_LOG_DEBUG("Handling CarrierInfoForImsiEncryptionRefreshMessage is null");
    }
}

void RilServiceModule_1_4::handleGetDataCallListResponseIndMessage(std::shared_ptr<rildata::GetDataCallListResponseIndMessage> msg) {
    if(msg) {
        QCRIL_LOG_DEBUG("Handling handleGetDataCallListResponseIndMessage %s", msg->dump().c_str());
        mRadioImpl_1_4.getDataCallListResponse_1_4(msg->getDCList(), msg->getSerial(), msg->getStatus());
    } else {
        QCRIL_LOG_DEBUG("GetDataCallListResponseIndMessage is null");
    }
}
