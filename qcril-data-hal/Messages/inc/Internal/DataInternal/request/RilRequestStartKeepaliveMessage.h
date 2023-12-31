/*===========================================================================

  Copyright (c) 2018, 2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#ifndef RILREQUESTSTARTKEEPALIVEMESSAGE
#define RILREQUESTSTARTKEEPALIVEMESSAGE
#include <framework/legacy.h>
#include "framework/Message.h"
#include "framework/SolicitedMessage.h"
#include "framework/GenericCallback.h"
#include "framework/add_message_id.h"
#include "framework/message_translator.h"
#include "DataCommon.h"
#include <modules/android/RilRequestMessage.h>

namespace rildata {

class RilRequestStartKeepaliveMessage : public SolicitedMessage<generic_callback_payload>,
                                  public add_message_id<RilRequestStartKeepaliveMessage> {
    private:

    legacy_request_payload params;

    public:

    static constexpr const char* MESSAGE_NAME = "RIL_REQUEST_START_KEEPALIVE";
    RilRequestStartKeepaliveMessage() = delete;
    RilRequestStartKeepaliveMessage(const qcril_request_params_type &p)
          :SolicitedMessage<generic_callback_payload>(get_class_message_id()), params(p) {
      mName = MESSAGE_NAME;
    }
    ~RilRequestStartKeepaliveMessage();

    qcril_request_params_type &get_params() {
      return params.get_params();
    }
    string dump();
};

}//namespace

#endif