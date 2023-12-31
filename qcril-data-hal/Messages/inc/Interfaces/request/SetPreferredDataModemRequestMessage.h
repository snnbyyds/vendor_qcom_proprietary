/**
* Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"
#include "MessageCommon.h"

namespace rildata {

struct SetPreferredDataModemResponse_t {
  SetPreferredDataModemResult_t errCode;
  ResponseError_t toResponseError();
};

class SetPreferredDataModemRequestMessage : public SolicitedMessage<SetPreferredDataModemResponse_t>,
                                            public add_message_id<SetPreferredDataModemRequestMessage> {
  private:
    int modemId;
    int32_t mSerial;
    std::shared_ptr<std::function<void(uint16_t)>> mAcknowlegeRequestCb;

  public:
    static constexpr const char *MESSAGE_NAME = "SetPreferredDataModemRequestMessage";
    SetPreferredDataModemRequestMessage(int p):SolicitedMessage<SetPreferredDataModemResponse_t>(get_class_message_id()), modemId(p) {
      mName = MESSAGE_NAME;
    }
    SetPreferredDataModemRequestMessage( const int32_t serial,int32_t modemid,
        const std::shared_ptr<std::function<void(uint16_t)>> ackCb
    ):SolicitedMessage<SetPreferredDataModemResponse_t>(get_class_message_id()), modemId(modemid) {
        mName = MESSAGE_NAME;
        mSerial = serial;
        mAcknowlegeRequestCb = ackCb;
    }
    ~SetPreferredDataModemRequestMessage() {
        if (mCallback) {
            delete mCallback;
            mCallback = nullptr;
        }
    }
    int32_t getSerial() {
        return mSerial;
    }
    int getModemId() { return modemId; }
    string dump() {
        return mName + "{ " + std::to_string(modemId) + " }";
    }
    std::shared_ptr<std::function<void(uint16_t)>> getAcknowlegeRequestCb() {
        return mAcknowlegeRequestCb;
    }
};

inline ResponseError_t SetPreferredDataModemResponse_t::toResponseError() {
  switch (errCode) {
    case SetPreferredDataModemResult_t::NO_ERROR:
      return ResponseError_t::NO_ERROR;
    case SetPreferredDataModemResult_t::DDS_SWITCH_NOT_ALLOWED:
      return ResponseError_t::INTERNAL_ERROR;
    case SetPreferredDataModemResult_t::DDS_SWITCH_TIMEOUT:
      return ResponseError_t::INTERNAL_ERROR;
    case SetPreferredDataModemResult_t::REQUEST_ALREADY_PENDING:
      return ResponseError_t::NOT_AVAILABLE;
    case SetPreferredDataModemResult_t::INVALID_ARG:
      return ResponseError_t::INVALID_ARGUMENT;
    case SetPreferredDataModemResult_t::INVALID_OPERATION:
      return ResponseError_t::NOT_AVAILABLE;
    case SetPreferredDataModemResult_t::QMI_ERROR:
      return ResponseError_t::INTERNAL_ERROR;
    case SetPreferredDataModemResult_t::MODEM_ENDPOINT_NOT_OPERATIONAL:
      return ResponseError_t::INTERNAL_ERROR;
    default:
      return ResponseError_t::INTERNAL_ERROR;
  }
}

}
