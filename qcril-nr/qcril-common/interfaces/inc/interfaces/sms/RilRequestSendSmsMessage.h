/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"
#include "framework/legacy.h"
#include "interfaces/sms/qcril_qmi_sms_types.h"
#include <interfaces/QcRilRequestMessage.h>

/* Request to send a SMS message.
   @Receiver: SmsModule

   Response:
    errorCode    : Valid error codes
    responseData : std::shared_ptr<RilSendSmsResult_t>
*/
class RilRequestSendSmsMessage : public QcRilRequestMessage,
                                 public add_message_id<RilRequestSendSmsMessage> {
public:
  static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_SEND_SMS";

  RilRequestSendSmsMessage() = delete;
  ~RilRequestSendSmsMessage() {}

#if 0
    inline RilRequestSendSmsMessage(string&& smscPdu, string&& pdu):
        SolicitedMessage<RilSendSmsResult_t>(get_class_message_id()) {
    }

    inline RilRequestSendSmsMessage(const string& smscPdu, const string& pdu):
        SolicitedMessage<RilSendSmsResult_t>(get_class_message_id()) {
    }
#else
  template <typename T1, typename T2>
  explicit RilRequestSendSmsMessage(std::shared_ptr<MessageContext> context, T1 &&smscPdu, T2 &&pdu,
                                    bool linkCtlEnable = false)
      : QcRilRequestMessage(get_class_message_id(), context),
        mSmscPdu(std::forward<T1>(smscPdu)), mPdu(std::forward<T2>(pdu)),
        mLinkCtlEnable(linkCtlEnable) {}
#endif

  bool getLinkCtlEnable();
  const string &getSmscPdu();
  const string &getPdu();

  string dump();

private:
  const string mSmscPdu;
  const string mPdu;
  bool mLinkCtlEnable;
};
