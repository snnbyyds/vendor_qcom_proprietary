/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

  Copyright (c) 2017, 2020 The Linux Foundation. All rights reserved
=============================================================================*/
#ifndef LOC_NET_IFACE_BASE_H
#define LOC_NET_IFACE_BASE_H

#include <IDataItemCore.h>
#include <loc_gps.h>
#include <loc_cfg.h>
#include <algorithm>
#include <vector>
#include <list>
#include <string.h>

using namespace loc_core;

/* Connectivity Type Enum
 *
 * These values are same as we define in case of LA,
 * except for emergency type WWAN which is not defined there. */

typedef enum {
    LOC_NET_CONN_TYPE_INVALID = 0,
    LOC_NET_CONN_TYPE_WLAN = 100,
    LOC_NET_CONN_TYPE_ETHERNET = 101,
    LOC_NET_CONN_TYPE_BLUETOOTH = 102,
    LOC_NET_CONN_TYPE_USB_CRADLE = 103,
    LOC_NET_CONN_TYPE_WWAN_INTERNET = 201,
    LOC_NET_CONN_TYPE_WWAN_SUPL = 205,
    LOC_NET_CONN_TYPE_WWAN_EMERGENCY = 206,
    LOC_NET_CONN_TYPE_MAX
} LocNetConnType;

/* Connection IP type */
typedef enum {
    LOC_NET_CONN_IP_TYPE_INVALID = 0,
    LOC_NET_CONN_IP_TYPE_V4,
    LOC_NET_CONN_IP_TYPE_V6,
    LOC_NET_CONN_IP_TYPE_V4V6,
    LOC_NET_CONN_IP_TYPE_MAX
} LocNetConnIpType;

/* WWAN call event */
typedef enum {
    LOC_NET_WWAN_CALL_EVT_INVALID = 0,
    LOC_NET_WWAN_CALL_EVT_OPEN_SUCCESS,
    LOC_NET_WWAN_CALL_EVT_OPEN_FAILED,
    LOC_NET_WWAN_CALL_EVT_CLOSE_SUCCESS,
    LOC_NET_WWAN_CALL_EVT_CLOSE_FAILED,
    LOC_NET_WWAN_CALL_EVT_MAX
} LocNetWwanCallEvent;

/* Connectivity State */
typedef enum {
    LOC_NET_CONN_STATE_INVALID = 0,
    LOC_NET_CONN_STATE_ENABLED,
    LOC_NET_CONN_STATE_DISABLED,
    LOC_NET_CONN_STATE_CONNECTED,
    LOC_NET_CONN_STATE_DISCONNECTED,
    LOC_NET_CONN_STATE_MAX
} LocNetConnState;

/* WWAN call status callback
 * apn and apnIpType values are valid based on event. */
typedef void (*LocWwanCallStatusCb)(
        void* userDataPtr, LocNetWwanCallEvent event,
        const char* apn, LocNetConnIpType apnIpType);

/* DataItem Notification callback */
typedef void (*LocNetStatusChangeCb)(
        void* userDataPtr, std::list<IDataItemCore*>& itemList);

/*--------------------------------------------------------------------
 * CLASS LocNetIfaceBase
 *
 * Functionality:
 * Interface to OS specific network connection functionality.
 * Currently supported actions:
 * - Register for WLAN/WWAN connectivity indications
 * - Setup / Teardown WWAN data call
 *-------------------------------------------------------------------*/
class LocNetIfaceBase {

public:
    /* To be used for WWAN data call setup
     *
     * Call setup status is notified via Subscriber interface if network info
     * data item is subscribed.
     *
     * Call setup status is also notified via LocWwanCallStatusCb callback
     * function if registered. */
    virtual bool setupWwanCall() = 0;

    /* Stop the ongoing data call */
    virtual bool stopWwanCall() = 0;

    /* Register data call setup callback
     * If callback is registered, we notify back data call status with it. */
    void registerWwanCallStatusCallback(
            LocWwanCallStatusCb wwanCallStatusCb, void* userDataPtr);

    /* Register for data items */
    virtual void subscribe(
            const std::list<DataItemId>& itemListToSubscribe) = 0;

    /* Unregister for data items */
    virtual void unsubscribe(
            const std::list<DataItemId>& itemListToUnsubscribe) = 0;

    /* Unregister all data items */
    virtual void unsubscribeAll() = 0;

    /* Request data items current value */
    virtual void requestData(
            const std::list<DataItemId>& itemListToRequestData) = 0;

    /* Register Notification callback  */
    void registerDataItemNotifyCallback(
            LocNetStatusChangeCb callback, void* userDataPtr);

    /* Virtual destructor since we have other virtual methods */
    virtual ~LocNetIfaceBase() {};

protected:
    /* List of data items subscribed at any instant */
    std::vector<DataItemId> mSubscribedItemList;

    /* Data Item notification callback registered.
     * This information is not instance specific, supports only single
     * client. */
    static LocNetStatusChangeCb sNotifyCb;
    static void* sNotifyCbUserDataPtr;

    /* WWAN data call setup callback */
    LocWwanCallStatusCb mWwanCallStatusCb;
    void* mWwanCbUserDataPtr;

    /* WWAN Call type supported by this instance */
    LocNetConnType mLocNetConnType;

    /* Config items */
    char mApnName[LOC_MAX_PARAM_STRING];
    int  mIpType;

    LocNetIfaceBase(LocNetConnType connType) :
        mSubscribedItemList(), mWwanCallStatusCb(NULL),
        mWwanCbUserDataPtr(NULL), mLocNetConnType(connType),
        mIpType(0) {

        memset(mApnName, 0, LOC_MAX_PARAM_STRING);
        fetchConfigItems();
    }

    /* Utility method to fetch required config items */
    void fetchConfigItems();

    /* Fetch configured APN for specified call type
     * APNs can be configured in gps.conf as:
     * INTERNET_APN = xyz
     * SUPL_APN = xyz */
    char* getApnNameFromConfig();

    /* Fetch configured IP Type for specified call type
     * IP Type can be configured in gps.conf as:
     * INTERNET_IP_TYPE = 4 / 6 / 10
     * SUPL_IP_TYPE = 4 / 6 / 10 */
    LocNetConnIpType getIpTypeFromConfig();

    /* Update the subscribed item list
     * addOrDelete: true = append items to subscribed list
     *              false = delete items from subscribed list
     * Just a utility method to be used from platform specific sub-classes
     * Returns true if any updates are made to the subscription list,
     * or else false. */
    bool updateSubscribedItemList(
            const std::list<DataItemId>& itemList, bool addOrDelete);

    /* Utility method */
    inline bool isItemSubscribed(DataItemId itemId){
        return ( mSubscribedItemList.end() !=
                    std::find( mSubscribedItemList.begin(),
                            mSubscribedItemList.end(), itemId));
    }
};
#endif /* #ifndef LOC_NET_IFACE_BASE_H */
