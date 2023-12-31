/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"
#include "framework/message_translator.h"
#include "framework/legacy.h"
#include <telephony/ril.h>
#include "interfaces/sms/qcril_qmi_sms_types.h"
#include <interfaces/QcRilRequestMessage.h>

/* Request to report SMS memory status Message
   @Receiver: SmsModule

   Response:
    errorCode    : Valid error codes
    responseData : nullptr
*/

class RilRequestReportSmsMemoryStatusMessage
    : public QcRilRequestMessage,
      public add_message_id<RilRequestReportSmsMemoryStatusMessage> {
public:
  static constexpr const char *MESSAGE_NAME = "RIL_REQUEST_REPORT_SMS_MEMORY_STATUS";

  RilRequestReportSmsMemoryStatusMessage() = delete;
  ~RilRequestReportSmsMemoryStatusMessage();

  inline RilRequestReportSmsMemoryStatusMessage(std::shared_ptr<MessageContext> context,
                                                bool available)
      : QcRilRequestMessage(get_class_message_id(), context), mAvailable(available) {
    mName = MESSAGE_NAME;
  }

  bool isAvailable();

  string dump();

private:
  bool mAvailable;
};
