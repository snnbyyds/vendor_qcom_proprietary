/*===========================================================================

  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#ifndef SETUPDATACALLREQUESTMESSAGE
#define SETUPDATACALLREQUESTMESSAGE
#include "MessageCommon.h"

#define REQUEST_SOURCE_INCLUDED

using namespace std;
namespace rildata {

class SetupDataCallRadioResponseIndMessage : public UnSolicitedMessage {
protected:
  const SetupDataCallResponse_t response;
  const int32_t serial;
  const Message::Callback::Status status;

public:
  SetupDataCallRadioResponseIndMessage() = delete;
  ~SetupDataCallRadioResponseIndMessage() = default;
  SetupDataCallRadioResponseIndMessage(message_id_t id, const SetupDataCallResponse_t& setResponse,
      int32_t setSerial, Message::Callback::Status setStatus): UnSolicitedMessage(id),
      response(setResponse), serial(setSerial), status(setStatus) {}

  string dump() {
    stringstream ss;
    ss << mName << " Serial=[" << (int)getSerial() << "] ";
    response.dump("", ss);
    return ss.str();
  }

  SetupDataCallResponse_t getResponse() { return response; }
  int32_t getSerial() { return serial; }
  Message::Callback::Status getStatus() { return status; }
};

class SetupDataCallRadioResponseIndMessage_1_4 : public SetupDataCallRadioResponseIndMessage,
                          public add_message_id<SetupDataCallRadioResponseIndMessage_1_4> {
public:
  static constexpr const char *MESSAGE_NAME = "SetupDataCallRadioResponseIndMessage_1_4";
  SetupDataCallRadioResponseIndMessage_1_4(const SetupDataCallResponse_t& setResponse,
      int32_t setSerial, Message::Callback::Status setStatus):
      SetupDataCallRadioResponseIndMessage(get_class_message_id(), setResponse, setSerial, setStatus) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<SetupDataCallRadioResponseIndMessage_1_4>(response, serial, status);
  }
};

class SetupDataCallRadioResponseIndMessage_1_5 : public SetupDataCallRadioResponseIndMessage,
                          public add_message_id<SetupDataCallRadioResponseIndMessage_1_5> {
public:
  static constexpr const char *MESSAGE_NAME = "SetupDataCallRadioResponseIndMessage_1_5";
  SetupDataCallRadioResponseIndMessage_1_5(const SetupDataCallResponse_t& setResponse,
      int32_t setSerial, Message::Callback::Status setStatus):
      SetupDataCallRadioResponseIndMessage(get_class_message_id(), setResponse, setSerial, setStatus) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<SetupDataCallRadioResponseIndMessage_1_5>(response, serial, status);
  }
};
#ifdef RIL_FOR_LOW_RAM
class SetupDataCallRadioResponseIndMessage_1_0 : public SetupDataCallRadioResponseIndMessage,
                          public add_message_id<SetupDataCallRadioResponseIndMessage_1_0> {
public:
  static constexpr const char *MESSAGE_NAME = "SetupDataCallRadioResponseIndMessage_1_0";
  SetupDataCallRadioResponseIndMessage_1_0(const SetupDataCallResponse_t& setResponse,
      int32_t setSerial, Message::Callback::Status setStatus):
      SetupDataCallRadioResponseIndMessage(get_class_message_id(), setResponse, setSerial, setStatus) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<SetupDataCallRadioResponseIndMessage_1_0>(response, serial, status);
  }
};

#endif

class SetupDataCallIWlanResponseIndMessage : public UnSolicitedMessage,
                          public add_message_id<SetupDataCallIWlanResponseIndMessage> {
private:
  SetupDataCallResponse_t response;
  int32_t serial;
  Message::Callback::Status status;

public:
  static constexpr const char *MESSAGE_NAME = "SetupDataCallIWlanResponseIndMessage";

  SetupDataCallIWlanResponseIndMessage() = delete;
  ~SetupDataCallIWlanResponseIndMessage() = default;
  SetupDataCallIWlanResponseIndMessage(const SetupDataCallResponse_t& setResponse, int32_t setSerial, Message::Callback::Status setStatus):
    UnSolicitedMessage(get_class_message_id()), response(setResponse), serial(setSerial), status(setStatus) {}

  string dump(){return mName;}

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<SetupDataCallIWlanResponseIndMessage>(response, serial, status);
  }

  SetupDataCallResponse_t getResponse() { return response; }
  int32_t getSerial() { return serial; }
  Message::Callback::Status getStatus() { return status; }
};

class SetupDataCallRequestMessage : public SolicitedMessage<SetupDataCallResponse_t>,
                          public add_message_id<SetupDataCallRequestMessage> {
private:
  int32_t mSerial;
  RequestSource_t mRequestSource;
  AccessNetwork_t mAccessNetwork;
  DataProfileId_t mProfileId;
  string mApn;
  string mProtocol;
  string mRoamingProtocol;
  ApnAuthType_t mAuthType;
  string mUsername;
  string mPassword;
  DataProfileInfoType_t mDataProfileInfoType;
  int32_t mMaxConnsTime;
  int32_t mMaxConns;
  int32_t mWaitTime;
  bool mEnableProfile;
  ApnTypes_t mSupportedApnTypesBitmap;
  RadioAccessFamily_t mBearerBitmap;
  int32_t mMtu;
  bool mPreferred;
  bool mPersistent;
  bool mRoamingAllowed;
  DataRequestReason_t mReason;
  vector<string> mAddresses;
  vector<string> mDnses;
  shared_ptr<function<void(int32_t)>> mAcknowlegeRequestCb;

public:
  static constexpr const char *MESSAGE_NAME = "SetupDataCallRequestMessage";
  SetupDataCallRequestMessage() = delete;
  SetupDataCallRequestMessage(
    const int32_t serial,
    const RequestSource_t requestSource,
    const AccessNetwork_t accessNetwork,
    const DataProfileId_t profileId,
    const string apn,
    const string protocol,
    const string roamingProtocol,
    const ApnAuthType_t authType,
    const string username,
    const string password,
    const DataProfileInfoType_t dataProfileInfoType,
    const int32_t maxConnsTime,
    const int32_t maxConns,
    const int32_t waitTime,
    const bool enableProfile,
    const ApnTypes_t supportedApnTypesBitmap,
    const RadioAccessFamily_t bearerBitmap,
    const int32_t mtu,
    const bool preferred,
    const bool persistent,
    const bool roamingAllowed,
    const DataRequestReason_t reason,
    const vector<string> addresses,
    const vector<string> dnses,
    const shared_ptr<function<void(int32_t)>> ackCb
    ):SolicitedMessage<SetupDataCallResponse_t>(get_class_message_id()) {
    mName = MESSAGE_NAME;
    mSerial = serial;
    mRequestSource = requestSource;
    mAccessNetwork = accessNetwork;
    mProfileId = profileId;
    mApn = apn;
    mProtocol = protocol;
    mRoamingProtocol = roamingProtocol;
    mAuthType = authType;
    mUsername = username;
    mPassword = password;
    mDataProfileInfoType = dataProfileInfoType;
    mMaxConnsTime = maxConnsTime;
    mMaxConns = maxConns;
    mWaitTime = waitTime;
    mEnableProfile = enableProfile;
    mSupportedApnTypesBitmap = supportedApnTypesBitmap;
    mBearerBitmap = bearerBitmap;
    mMtu = mtu;
    mPreferred = preferred;
    mPersistent = persistent;
    mRoamingAllowed = roamingAllowed;
    mReason = reason;
    mAddresses = addresses;
    mDnses = dnses;
    mAcknowlegeRequestCb = ackCb;
  }
  ~SetupDataCallRequestMessage() = default;

  string dump(){
    std::stringstream ss;
    ss << mName << " Serial=[" << (int)getSerial() << "] Params=[";
    ss << (int)getAccessNetwork() << ",";
    ss << (int)getProfileId() << ",";
    ss << getApn()+",";
    ss << getProtocol()+",";
    ss << getRoamingProtocol()+",";
    ss << (int)getAuthType() << ",";
    ss << getUsername()+",";
    ss << getPassword()+",";
    ss << (int)getDataProfileInfoType() << ",";
    ss << (int)getMaxConnsTime() << ",";
    ss << (int)getMaxConns() << ",";
    ss << (int)getWaitTime() << ",";
    ss << (int)getEnableProfile() << ",";
    ss << (int)getSupportedApnTypesBitmap() << ",";
    ss << (int)getBearerBitmap() << ",";
    ss << (int)getMtu() << ",";
    ss << (int)getPreferred() << ",";
    ss << (int)getPersistent() << ",";
    ss << (int)getRoamingAllowed() << ",";
    ss << (int)getDataRequestReason() << ",";
    ss << "Addrs=<";
    std::vector<std::string> addr = getAddresses();
    for (unsigned long i=0 ; i<addr.size(); i++) {
      ss << addr[i];
    }
    ss << ">,Dnses=<";
    std::vector<std::string> dns = getDnses();
    for (unsigned long i=0 ; i<dns.size(); i++) {
      ss << dns[i];
    }
    ss << ">]";
    return ss.str();
  }
  int32_t getSerial() {return mSerial;}
  RequestSource_t getRequestSource() {return mRequestSource;}
  AccessNetwork_t getAccessNetwork() {return mAccessNetwork;}
  DataProfileId_t getProfileId() {return mProfileId;}
  string getApn() {return mApn;}
  string getProtocol() {return mProtocol;}
  string getRoamingProtocol() {return mRoamingProtocol;}
  ApnAuthType_t getAuthType() {return mAuthType;}
  string getUsername() {return mUsername;}
  string getPassword() {return mPassword;}
  DataProfileInfoType_t getDataProfileInfoType() {return mDataProfileInfoType;}
  int32_t getMaxConnsTime() {return mMaxConnsTime;}
  int32_t getMaxConns() {return mMaxConns;}
  int32_t getWaitTime() {return mWaitTime;}
  bool getEnableProfile() {return mEnableProfile;}
  ApnTypes_t getSupportedApnTypesBitmap() {return mSupportedApnTypesBitmap;}
  RadioAccessFamily_t getBearerBitmap() {return mBearerBitmap;}
  int32_t getMtu() {return mMtu;}
  bool getPreferred() {return mPreferred;}
  bool getPersistent() {return mPersistent;}
  bool getRoamingAllowed() {return mRoamingAllowed;}
  DataRequestReason_t getDataRequestReason() {return mReason;}
  vector<string> getAddresses() {return mAddresses;}
  vector<string> getDnses() {return mDnses;}
  shared_ptr<function<void(int32_t)>> getAcknowlegeRequestCb() {return mAcknowlegeRequestCb;}
};

}

#endif