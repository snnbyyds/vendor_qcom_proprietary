/**
 * Copyright (c) 2017, 2019-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "EsePowerManager.h"
#include "QSEEComAPI.h"
#include <utils/Log.h>
#include <vendor/qti/esepowermanager/1.1/IEsePowerManager.h>
#include <hwbinder/ProcessState.h>
#include <hidl/LegacySupport.h>

#define LOG_TAG "vendor.qti.esepowermanager@1.1-service"

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using vendor::qti::esepowermanager::V1_1::IEsePowerManager;
using android::hardware::registerPassthroughServiceImplementation;
using android::OK;

static char const app_name[] = "eseservice";
static char const app_path[] = "/vendor/firmware_mnt/image";
static struct QSEECom_handle* qseeComHandleeSEService = NULL;

int main()
{
    ALOGD("loading eseservice");
    int32_t ret = QSEECom_start_app(&qseeComHandleeSEService, app_path, app_name, 1024);
    if (ret) {
        ALOGD("loading eseservice failed");
        //Some customers may want to use the esepowermanager without the eseservice (SPI through HLOS)
    }
    ALOGD("eSEPowerManager V1.1 is starting");

    /**
     * esepowermanager is only calling couple of ioctls(get/set) to enable/disable ese power,
     * heance modified process max buffer size to 8k
     */
    android::hardware::ProcessState::initWithMmapSize((size_t)(8192));
    configureRpcThreadpool(1, true /*callerWillJoin*/);
    android::status_t status;
    status = registerPassthroughServiceImplementation<IEsePowerManager>();
    LOG_ALWAYS_FATAL_IF(status != OK, "Error while registering eSE power manager service: %d", status);
    joinRpcThreadpool();
    return status;
}
