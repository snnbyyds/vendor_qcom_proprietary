/******************************************************************************
Copyright (c) 2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#include <vendor/qti/hardware/servicetracker/1.2/IServicetracker.h>
#include <Servicetracker.h>
#include <hidl/LegacySupport.h>

using vendor::qti::hardware::servicetracker::V1_2::IServicetracker;
using android::hardware::defaultPassthroughServiceImplementation;

int main() {
    return defaultPassthroughServiceImplementation<IServicetracker>(16);
}
