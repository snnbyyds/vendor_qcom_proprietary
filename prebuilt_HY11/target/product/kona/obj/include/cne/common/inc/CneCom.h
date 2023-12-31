#ifndef CNE_COM_H
#define CNE_COM_H

/*==============================================================================
  FILE:         CneCom.h

  OVERVIEW:     Manages communication with file descriptors for CnE

  DEPENDENCIES: Logging

                Copyright (c) 2011-2014, 2016, 2019-2020 Qualcomm Technologies, Inc.
                All Rights Reserved.
                Confidential and Proprietary - Qualcomm Technologies, Inc.
==============================================================================*/

/*=============================================================================
  EDIT HISTORY FOR MODULE

  when        who     what, where, why
  ----------  ---     ---------------------------------------------------------
  04-16-2012  mtony   Removed circular dependency btwn Cne and CneCom
=============================================================================*/
#include <map>
#include <set>
#include <sys/epoll.h>
#include <bits/epoll_event.h>
#include "CneUtils.h"
#include "CneMsg.h"
#include "CneTimer.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmacro-redefined"
#pragma GCC diagnostic pop

#include <vendor/qti/hardware/data/cne/internal/server/1.0/IServiceCallback.h>
using ::vendor::qti::hardware::data::cne::internal::server::V1_0::IServiceCallback;
#include <vendor/qti/hardware/data/cne/internal/api/1.0/INetCfgCallback.h>
using ::vendor::qti::hardware::data::cne::internal::api::V1_0::INetCfgCallback;
#include <vendor/qti/hardware/data/cne/internal/api/1.0/INetReqCallback.h>
using ::vendor::qti::hardware::data::cne::internal::api::V1_0::INetReqCallback;
#include <vendor/qti/hardware/mwqemadapter/1.0/IMwqemAdapterCallback.h>
using ::vendor::qti::hardware::mwqemadapter::V1_0::IMwqemAdapterCallback;

using ::android::sp;
using ::android::hidl::base::V1_0::IBase;
using ::android::hardware::hidl_string;

using vendor::qti::hardware::mwqemadapter::V1_0::MwqemIpTableCmd;
using vendor::qti::hardware::mwqemadapter::V1_0::MwqemIpTableInfo;
typedef union _CasApiCbUnion
{
    sp<INetCfgCallback> netCfgCb;
    sp<INetReqCallback> netReqCb;
    _CasApiCbUnion(): netCfgCb(nullptr) {};
    ~_CasApiCbUnion() {};
} CasApiCbUnion;

/*------------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * ---------------------------------------------------------------------------*/
#undef SUB_TYPE
#define SUB_TYPE CNE_MSG_SUBTYPE_QCNEA_CORE_COM

/*------------------------------------------------------------------------------
 * CLASS         CneCom
 *
 * DESCRIPTION   Handle communications for CnE
 *----------------------------------------------------------------------------*/
class CneCom {

public:

  // signature of callback methods
  typedef void (*ComEventCallback)(int fd, void *data); // fd event
  typedef void (*ComCloseCallback)(int fd, void *data); // fd close event

  /*----------------------------------------------------------------------------
   * FUNCTION      Constructor
   *
   * DESCRIPTION   creates an epoll instance
   *
   * DEPENDENCIES  epoll, eventfd
   *
   * RETURN VALUE  -
   *
   * SIDE EFFECTS  none
   *--------------------------------------------------------------------------*/
  CneCom();

  /*----------------------------------------------------------------------------
   * FUNCTION      processEvents
   *
   * DESCRIPTION   process any ready epoll events, waits up to a maximum of
   *               'waitTime' milliseconds. a wait time of -1 will cause this
   *               method to wait indefinitely.
   *
   * DEPENDENCIES  epoll
   *
   * RETURN VALUE  none
   *
   * SIDE EFFECTS  any events will have their associated callback called
   *--------------------------------------------------------------------------*/
  void processEvents(int waitTime, CneTimer &timer);

  /*----------------------------------------------------------------------------
   * FUNCTION      addComEventHandler
   *
   * DESCRIPTION   associate a callback with an epoll event on a file
   *               descriptor; each file descriptor can have only one callback
   *               params:
   *                 fd       - file descriptor to associate with the callback
   *                 callback - pointer to callback method
   *                 data     - additional data to be passed back to callback
   *                 close    - pointer to callback method to be called
   *                            when the fd closes
   *                 events   - epoll events that will trigger the callback,
   *                            will default to EPOLLIN | EPOLLHUP
   *                            but can be overridden
   *
   * DEPENDENCIES  epoll
   *
   * RETURN VALUE  true if the handler was added, otherwise false
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  bool addComEventHandler(int fd, ComEventCallback callback, void *data,
      ComCloseCallback close = NULL, int events = EPOLLIN | EPOLLHUP);

  /*----------------------------------------------------------------------------
   * FUNCTION      removeComEventHandler
   *
   * DESCRIPTION   disassociate a callback with an epoll event on a file
   *               descriptor, if the file descriptor does not have an
   *               associated callback no action is taken
   *
   * DEPENDENCIES  epoll
   *
   * RETURN VALUE  none
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  void removeComEventHandler(int fd);

  /*----------------------------------------------------------------------------
   * FUNCTION      wake
   *
   * DESCRIPTION   stop waiting for events to be processed
   *
   * DEPENDENCIES  epoll, eventfd
   *
   * RETURN VALUE  none
   *
   * SIDE EFFECTS  -
   *--------------------------------------------------------------------------*/
  void wake();

  /*----------------------------------------------------------------------------
   * FUNCTION      isApprovedFd
   *
   * DESCRIPTION   check to see if the given file descriptor is on the list of
   *               FD's approved for I/O.
   *
   * DEPENDENCIES
   *
   * RETURN VALUE  true if approved, otherwise false
   *
   * SIDE EFFECTS
   *--------------------------------------------------------------------------*/
  static bool isApprovedFd(int fd);

    // reads a file descriptor until no data is left, no data is saved/returned
  static void clearRead(int fd, void*);

  /*----------------------------------------------------------------------------
   * FUNCTION      sendUnsolicitedMsg
   *
   * DESCRIPTION   send a message to a file descriptor
   *
   * DEPENDENCIES
   *
   * RETURN VALUE  none
   *
   * SIDE EFFECTS  the message will be parceled and sent to the target
                   file descriptor
   *--------------------------------------------------------------------------*/
  static void sendUnsolicitedMsg(int targetFd, cne_msg_enum_type msgType);

  template <class T>
  static void sendUnsolicitedMsg(int targetFd, cne_msg_enum_type msgType, T const& data) {
    CNE_MSG_VERBOSE("sending unsolicited message. fd:%d type:%s (%d)",
        targetFd, CneUtils::getCneMsgStr(msgType), msgType);

   if (0 == targetFd) {
       sendUnsolicitedMsgServer(msgType, (void*)&data);
   }

  }

  template <class T>
  static void sendUnsolicitedMsg(const CasApiCbUnion& cb, cne_msg_enum_type msgType,
        T const& data, bool notify){
    CNE_MSG_VERBOSE("sending unsolicited message. type:%s (%d), notify: %s",
         CneUtils::getCneMsgStr(msgType), msgType, notify?"true":"false");

    sendUnsolicitedMsgApi(cb, msgType, (void*)&data, notify);
  }

  void setServerCb(const sp<IServiceCallback> serverCb);
  void setMwqemAdapterCb(const sp<IMwqemAdapterCallback> MwqemAdapterCb);
  void clearServerCb(const sp<IServiceCallback> serverCb);
  void resetServerCb();
  void resetMwqemAdapterCb();

private:

  // the max number of epoll events to be returned at one time
  static const unsigned int EPOLL_SIZE = 4;

  // value matched in CNEJ
  static const int UNSOLICITED_MESSAGE = 1;

  // max that send() can write at once
  static const unsigned int MAX_WRITE_SIZE = 1024;

  // file descriptor of the epoll instance
  int epollFd;

  // file descriptor of the eventFd instance (used to wake epoll)
  int eventFd;

  // holds epoll events returned from epoll_wait
  epoll_event events[EPOLL_SIZE];

  // structure for callbacks
  struct ComHandlerObject {
    ComEventCallback callback;
    ComCloseCallback closeCallback;
    void* data;
  };

  // stores mapping of file descriptors to callbacks
  std::map<int, ComHandlerObject> comHandlers;

  // stores list of file descriptors that are ok to work with
  static std::set<int> approvedFd;

  // adds FD to the list of approved file descriptors for I/O
  static void addApprovedFd(int fd);

  // remove FD from the list of approved file descriptors for I/O
  static void remApprovedFd(int fd);

  static sp<IServiceCallback> mServerCb;
  static sp<IMwqemAdapterCallback> mMwqemAdapterCb;
  static void sendUnsolicitedMsgServer(cne_msg_enum_type msgType, void* pData);
  static void sendUnsolicitedMsgApi(const CasApiCbUnion& cb, cne_msg_enum_type msgType, void* pData, bool notify);
  static void transposeMwqemRatInfo(MwqemIfaceInfo_t &mwqemIfaceInfo,MwqemIpTableInfo &mwqemIpTableInfo);
};

#endif /* CNE_COM_H */
