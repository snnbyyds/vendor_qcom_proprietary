/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

GENERAL DESCRIPTION
  Implementation of FrameworkActionRequest

  Copyright  (c) 2017, 2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/


#define LOG_TAG "FrameworkActionRequest"
#define LOG_NDEBUG 0
#ifdef __ANDROID__
#include "utils/Log.h"
#else
#include <cstddef>
#include <stddef.h>
#endif
#include "FrameworkActionRequest.h"
#include "IzatDefines.h"
#include "Subscription.h"
#include "log_util.h"

FrameworkActionRequest* FrameworkActionRequest::mFrameworkActionReqObj = NULL;

using namespace std;

// FrameworkActionRequest class implementation
FrameworkActionRequest* FrameworkActionRequest::getFrameworkActionReqObj()
{
    int result = 0;

    ENTRY_LOG();
    do {
          // already initialized
          BREAK_IF_NON_ZERO(0, mFrameworkActionReqObj);

          mFrameworkActionReqObj = new (std::nothrow) FrameworkActionRequest();
          BREAK_IF_ZERO(2, mFrameworkActionReqObj);
          result = 0;
    } while(0);

    EXIT_LOG_WITH_ERROR("%d", result);
    return mFrameworkActionReqObj;
}

void FrameworkActionRequest::destroyInstance()
{
    ENTRY_LOG();

    delete mFrameworkActionReqObj;
    mFrameworkActionReqObj = NULL;

    EXIT_LOG_WITH_ERROR("%d", 0);
}

//IFrameworkActionReq overrides
void FrameworkActionRequest::turnOn (DataItemId dit, int timeOut)
{
    ENTRY_LOG();

    SubscriptionCallbackIface* cbIface =
            Subscription::getSubscriptionCallback();
    if (cbIface == NULL) {
        LOC_LOGE("Subscription CB null");
        return;
    }
    cbIface->turnOnModule(dit, timeOut);
}

void FrameworkActionRequest::turnOff (DataItemId dit)
{
    ENTRY_LOG();

    SubscriptionCallbackIface* cbIface =
            Subscription::getSubscriptionCallback();
    if (cbIface == NULL) {
        LOC_LOGE("Subscription CB null");
        return;
    }
    cbIface->turnOffModule(dit);
}

#ifdef USE_GLIB
bool FrameworkActionRequest::connectBackhaul(const string& clientName)
{
    bool retVal = false;
#ifdef USE_QCMAP
    LocNetIface* locNetIfaceObj = Subscription::getSubscriptionObj()->getLocNetIfaceObj();
    if (locNetIfaceObj) {
        retVal = locNetIfaceObj->connectBackhaul(clientName);
    }
#else
    LOC_LOGE("QCMAP is not supported");
#endif
    return retVal;
}

bool FrameworkActionRequest::disconnectBackhaul(const string& clientName)
{
    bool retVal = false;
#ifdef USE_QCMAP
    LocNetIface* locNetIfaceObj = Subscription::getSubscriptionObj()->getLocNetIfaceObj();
    if (locNetIfaceObj) {
        retVal = locNetIfaceObj->disconnectBackhaul(clientName);
    }
#else
    LOC_LOGE("QCMAP is not supported");
#endif
   return retVal;
}
#endif
