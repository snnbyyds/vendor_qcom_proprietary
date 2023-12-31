/******************************************************************************
#  Copyright (c) 2018,2020 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#ifndef TURNONAPASSISTIWLANSYNCMESSAGE
#define TURNONAPASSISTIWLANSYNCMESSAGE
#include "framework/SolicitedSyncMessage.h"
#include "framework/Util.h"
#include "framework/add_message_id.h"
#include "framework/Dispatcher.h"
#include <list>

class TurnOnAPAssistIWLANSyncMessage : public SolicitedSyncMessage<bool>,
                                    public add_message_id<TurnOnAPAssistIWLANSyncMessage>
{
public:
  static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.TurnOnAPAssistIWLANSyncMessage";
  inline TurnOnAPAssistIWLANSyncMessage(GenericCallback<bool> *callback) :
    SolicitedSyncMessage<bool>(get_class_message_id())
  {
    (void)callback;
    mName = MESSAGE_NAME;
  }
  ~TurnOnAPAssistIWLANSyncMessage() {}

  string dump(){
    return mName;
  }
};

#endif