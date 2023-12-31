/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/
#ifndef GNSS_MULTI_CLIENT_HIDL_IGNSS_CB_H
#define GNSS_MULTI_CLIENT_HIDL_IGNSS_CB_H
#include "GnssMultiClientCase.h"
#include "GnssCbBase.h"

#include <android/hardware/gnss/1.0/IGnss.h>
#include <android/hardware/gnss/1.1/IGnss.h>
#include <android/hardware/gnss/1.0/IGnssMeasurement.h>
#include <android/hardware/gnss/1.1/IGnssMeasurement.h>

#include <android/hardware/gnss/2.0/IGnss.h>
#include <android/hardware/gnss/2.1/IGnss.h>
#include <android/hardware/gnss/visibility_control/1.0/IGnssVisibilityControlCallback.h>
#include <android/hardware/gnss/measurement_corrections/1.0/IMeasurementCorrectionsCallback.h>


using android::hardware::Return;
using android::hardware::Void;
using android::hardware::hidl_vec;

using IGnss_V1_0 = android::hardware::gnss::V1_0::IGnss;
using IGnss_V1_1 = android::hardware::gnss::V1_1::IGnss;

using GnssLocation = android::hardware::gnss::V2_0::GnssLocation;
using GnssSvInfo_2_0 = android::hardware::gnss::V2_0::IGnssCallback::GnssSvInfo;
using GnssSvInfo_2_1 = android::hardware::gnss::V2_1::IGnssCallback::GnssSvInfo;
using GnssData_V2_0 = android::hardware::gnss::V2_0::IGnssMeasurementCallback::GnssData;
using GnssData_V2_1 = android::hardware::gnss::V2_1::IGnssMeasurementCallback::GnssData;
using Capabilities = android::hardware::gnss::V2_0::IGnssCallback::Capabilities;
using IGnssCallback_V1_1 = android::hardware::gnss::V1_1::IGnssCallback;
using IGnssCallback_V2_0 = android::hardware::gnss::V2_0::IGnssCallback;
using IGnssCallback_V2_1 = android::hardware::gnss::V2_1::IGnssCallback;

using IGnssMeasurement_V1_0 = android::hardware::gnss::V1_0::IGnssMeasurement;
using IGnssMeasurement_V1_1 = android::hardware::gnss::V1_1::IGnssMeasurement;
using IGnssMeasurementCallback_V1_0 =
        android::hardware::gnss::V1_0::IGnssMeasurementCallback;
using IGnssMeasurementCallback_V1_1 =
        android::hardware::gnss::V1_1::IGnssMeasurementCallback;
using IGnssMeasurementCallback_V2_0 =
        android::hardware::gnss::V2_0::IGnssMeasurementCallback;
using IGnssMeasurementCallback_V2_1 =
        android::hardware::gnss::V2_1::IGnssMeasurementCallback;
using IGnssBatching_V2_0 = android::hardware::gnss::V2_0::IGnssBatching;
using IGnssBatchingCallback_V2_0 = android::hardware::gnss::V2_0::IGnssBatchingCallback;
using IAGnssCallback_V2_0 = android::hardware::gnss::V2_0::IAGnssCallback;
using IGnssVisibilityControlCallback =
        android::hardware::gnss::visibility_control::V1_0::IGnssVisibilityControlCallback;
using IMeasurementCorrectionsCallback =
        android::hardware::gnss::measurement_corrections::V1_0::IMeasurementCorrectionsCallback;
using gnssCapabilities_V2_0 = IGnssCallback_V2_0::Capabilities;
using gnssCapabilities_V2_1 = IGnssCallback_V2_1::Capabilities;
using mcCapabilities = IMeasurementCorrectionsCallback::Capabilities;
using android::hardware::gnss::V2_1::IGnssAntennaInfo;
using android::hardware::gnss::V2_1::IGnssAntennaInfoCallback;

namespace garden {

class GnssMultiClientHidlIGnss;

class GnssMultiClientHidlIGnssCb :
        public IGnssCallback_V2_1,
        public IGnssMeasurementCallback_V2_1,
        public IGnssVisibilityControlCallback,
        public IGnssBatchingCallback_V2_0,
        public IAGnssCallback_V2_0,
        public IMeasurementCorrectionsCallback,
        public IGnssAntennaInfoCallback

{
public:
    GnssMultiClientHidlIGnssCb(GnssMultiClientHidlIGnss* hidlIGnss);
    ~GnssMultiClientHidlIGnssCb();

    Return<void> gnssMeasurementCb(
            const IGnssMeasurementCallback_V1_1::GnssData& data) override;
    Return<void> GnssMeasurementCb(
            const IGnssMeasurementCallback_V1_0::GnssData& data) override;
    Return<void> nfwNotifyCb(
            const IGnssVisibilityControlCallback::NfwNotification& notification) override;
    Return<bool> isInEmergencySession() override;
    Return<void> setCapabilitiesCb(
            android::hardware::hidl_bitfield<mcCapabilities> capabilities) override;

    Return<void> gnssLocationCb(const ::android::hardware::gnss::V1_0::GnssLocation& gnssLocation);
    Return<void> gnssStatusCb(::android::hardware::gnss::V1_0::IGnssCallback::GnssStatusValue status);
    Return<void> gnssSvStatusCb(const ::android::hardware::gnss::V1_0::IGnssCallback::GnssSvStatus& svInfo);
    Return<void> gnssNmeaCb(int64_t timestamp, const ::android::hardware::hidl_string& nmea);
    Return<void> gnssSetCapabilitesCb(uint32_t capabilities);
    Return<void> gnssAcquireWakelockCb();
    Return<void> gnssReleaseWakelockCb();
    Return<void> gnssRequestTimeCb();
    Return<void> gnssSetSystemInfoCb(const ::android::hardware::gnss::V1_0::IGnssCallback::GnssSystemInfo& info);
    Return<void> gnssNameCb(const ::android::hardware::hidl_string& name);
    Return<void> gnssRequestLocationCb(bool independentFromGnss);

    // Methods from ::android::hardware::gnss::V2_0::IGnssCallback follow.
    Return<void> gnssSetCapabilitiesCb_2_0(
            android::hardware::hidl_bitfield<gnssCapabilities_V2_0> capabilities) override;
    Return<void> gnssLocationCb_2_0(const GnssLocation& location) override;
    Return<void> gnssRequestLocationCb_2_0(bool independentFromGnss, bool isUserEmergency) override;
    Return<void> gnssSvStatusCb_2_0(const android::hardware::hidl_vec<GnssSvInfo_2_0>& svInfoList) override;
    Return<void> gnssSvStatusCb_2_1(const android::hardware::hidl_vec<GnssSvInfo_2_1>& svInfoList) override;
    // Methods from ::android::hardware::gnss::V2_0::IGnssBatchingCallback follow.
    Return<void> gnssLocationBatchCb(const android::hardware::hidl_vec<GnssLocation>& locations) override;
    // Methods from ::android::hardware::gnss::V2_0::IGnssMeasurementCallback follow.
    Return<void> gnssMeasurementCb_2_0(const GnssData_V2_0& data) override;
    // Methods from ::android::hardware::gnss::V2_1::IGnssMeasurementCallback follow.
    Return<void> gnssMeasurementCb_2_1(const GnssData_V2_1& data) override;
    // Methods from ::android::hardware::gnss::V2_0::IAGnssCallback follow.
    Return<void> agnssStatusCb(IAGnssCallback_V2_0::AGnssType type,
            IAGnssCallback_V2_0::AGnssStatusValue status) override;
    // Methods from ::android::hardware::gnss::V2_1::IGnssAntennaInfoCallback follow.
    Return<void> gnssAntennaInfoCb(const hidl_vec<GnssAntennaInfo>& antInfos) override;
    Return<void> gnssSetCapabilitiesCb_2_1(
            android::hardware::hidl_bitfield<gnssCapabilities_V2_1> capabilities) override;
private:
    GnssMultiClientHidlIGnss* mHidlGnss = nullptr;
};

} //namespace garden

#endif //GNSS_MULTI_CLIENT_HIDL_IGNSS_CB_H
