/******************************************************************************
#  Copyright (c) 2018-2020   Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#ifndef WDSMODEMENDPOINT
#define WDSMODEMENDPOINT
#include "modules/qmi/ModemEndPoint.h"
#include "WDSModemEndPointModule.h"
#include "framework/Log.h"
#include "MessageCommon.h"

class WDSModemEndPoint : public ModemEndPoint
{
private:
  bool mIsDataRegistered;
  bool mIsReportDataRegistrationRejectCause;

public:
  static constexpr const char *NAME = "WDSModemEndPoint";
  WDSModemEndPoint() : ModemEndPoint(NAME) {
    mModule = new WDSModemEndPointModule("WDSModemEndPointModule", *this);
    mModule->init();
    mIsDataRegistered = false;
    mIsReportDataRegistrationRejectCause = false;
    Log::getInstance().d("[WDSModemEndPoint]: xtor");
  }
  ~WDSModemEndPoint() {
      Log::getInstance().d("[WDSModemEndPoint]: destructor");
    //mModule->killLooper();
    delete mModule;
    mModule = nullptr;
  }

  int getLteAttachParams(wds_get_lte_attach_params_resp_msg_v01 *attach_param);
#ifdef RIL_FOR_DYNAMIC_LOADING
  void requestSetup(string clientToken, qcril_instance_id_e_type id, GenericCallback<string>* cb);
#else
  void requestSetup(string clientToken, GenericCallback<string>* cb);
#endif
  /**
   * @brief      Gets the attach list.
   *
   * @param      attach_list  The attach list
   *
   * @return     Status
   */
  Message::Callback::Status getAttachListSync(
    std::shared_ptr<std::list<uint16_t>>& attach_list
  );

  /**
   * @brief      Set Attach list with desired action
   *
   * @param[in]  attach_list  The attach list
   * @param[in]  action       The action
   */
  Message::Callback::Status setAttachListSync(
    const std::shared_ptr<std::list<uint16_t>>& attach_list,
   const SetAttachListSyncMessage::AttachListAction action);

  Message::Callback::Status getAttachListCapabilitySync(
    std::shared_ptr<AttachListCap>& cap);

  Message::Callback::Status getApnTypesForName(
    string apnName,
    std::shared_ptr<std::list<int32_t>>& apnTypes
  );

  /**
   * @brief Posts SetProfileParamsSyncMessage to handle the passed profile
   * parameters
   *
   * @return Success if message is posted succesfully, Failure otherwise
   **/
  Message::Callback::Status setProfileParamsSync(
    vector<rildata::DataProfileInfo_t>& pInfo
  );

  bool getWDSProfileClatCapability( int32_t profileType, uint8_t profileIndex);

  /**
   * @brief Posts GetCallBringUpCapabilitySyncMessage to query the modem
   *
   * @return Success if message is posted succesfully, Failure otherwise
   **/
  Message::Callback::Status getCallBringUpCapabilitySync(
    std::shared_ptr<BringUpCapability>& callBringUpCapability
  );

  Message::Callback::Status registerForKeepAliveInd(bool toRegister);

  Message::Callback::Status setDefaultProfileNum(int index);

  bool getReportingStatus();

  bool getDataRegistrationState();

  void updateDataRegistrationState(bool registered);

  Message::Callback::Status registerDataRegistrationRejectCause();

  Message::Callback::Status registerDataRegistrationRejectCause(bool enable);
};

#endif