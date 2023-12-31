/*
 * Copyright (c) 2017-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef VENDOR_QTI_GNSS_V1_0_LOCHIDLFLPSERVICE_H
#define VENDOR_QTI_GNSS_V1_0_LOCHIDLFLPSERVICE_H

#include <vendor/qti/gnss/1.0/ILocHidlFlpService.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <LocHidlFlpClient.h>
#include <location_interface.h>
#include <LocHidlUtils.h>

namespace vendor {
namespace qti {
namespace gnss {
namespace V1_0 {
namespace implementation {

using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::vendor::qti::gnss::V1_0::ILocHidlFlpService;
using ::vendor::qti::gnss::V1_0::ILocHidlFlpServiceCallback;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_death_recipient;
using ::android::sp;
using ::android::wp;

struct LocHidlDeathRecipient;
struct LocHidlFlpService : public ILocHidlFlpService {

    LocHidlFlpService();
    ~LocHidlFlpService();

    // Methods from ::vendor::qti::gnss::V1_0::ILocHidlFlpService follow.
    Return<bool> init(const sp<ILocHidlFlpServiceCallback>& callback) override;
    Return<uint32_t> getAllSupportedFeatures() override;
    Return<int32_t> getAllBatchedLocations(int32_t sessionId) override;
    Return<int32_t> stopFlpSession(int32_t sessionId) override;
    Return<int32_t> startFlpSession(int32_t id, uint32_t flags, int64_t minIntervalNanos,
            int32_t minDistanceMetres, uint32_t tripDistanceMeters) override;
    Return<int32_t> updateFlpSession(int32_t id, uint32_t flags, int64_t minIntervalNanos,
            int32_t minDistanceMetres, uint32_t tripDistanceMeters) override;
    Return<void> deleteAidingData(int64_t flags) override;
    Return<void> getMaxPowerAllocated() override;

private:
    LocHidlFlpClient* mIzatFlpApiClient = nullptr;
    sp<ILocHidlFlpServiceCallback> mCallbackIface = nullptr;

private:
    typedef const GnssInterface * (getLocationInterface)();
    GnssInterface* mGnssInterface = nullptr;
    bool mGetGnssInterfaceFailed = false;
    const GnssInterface* getGnssInterface();
    static sp<LocHidlDeathRecipient> mDeathRecipient;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace gnss
}  // namespace qti
}  // namespace vendor

#endif  // VENDOR_QTI_GNSS_V1_0_LOCHIDLFLPSERVICE_H
