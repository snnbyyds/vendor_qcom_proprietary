/**
* Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#ifndef DSDSYSTEMSTATUSFRAGMENTMESSAGE
#define DSDSYSTEMSTATUSFRAGMENTMESSAGE
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "data_system_determination_v01.h"

namespace rildata {

/********************** Class Definitions *************************/
class DsdSystemStatusFragmentMessage: public UnSolicitedMessage,
                           public add_message_id<DsdSystemStatusFragmentMessage> {

private:
  dsd_system_status_v2_ind_msg_v01 mDsdSysStatus;

public:
  static constexpr const char *MESSAGE_NAME = "DsdSystemStatusFragmentMessage";

  DsdSystemStatusFragmentMessage() = delete;
  DsdSystemStatusFragmentMessage(const dsd_system_status_v2_ind_msg_v01 &p)
  : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
    mDsdSysStatus = p;
  }
  ~DsdSystemStatusFragmentMessage(){};

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<DsdSystemStatusFragmentMessage>(mDsdSysStatus);
  }

  dsd_system_status_v2_ind_msg_v01 getParams() {
    return mDsdSysStatus;
  }

  string dump() {
    return MESSAGE_NAME;
  }
};

} //namespace

#endif
