/*
* Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*
* Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*       copyright notice, this list of conditions and the following
*       disclaimer in the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of The Linux Foundation nor the names of its
*       contributors may be used to endorse or promote products derived
*       from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <set>
#include <unordered_map>
#include <mutex>

#include "OperationStatus.h"
#include "DriversFactory.h"

class Device;
class DriverAPI;
struct DeviceData;

class DeviceManager
{
public:
    DeviceManager();
    void Init();
    ~DeviceManager();

    std::vector<std::string> GetDeviceNames() const;
    std::vector<std::shared_ptr<Device>> GetDevices() const;

    OperationStatus GetDeviceByName(const std::string& deviceName, std::shared_ptr<Device>& spDevice);

private:
    void UpdateNonMonitoredDevices(); // periodic update of non-monitored devices

    // Callbacks for change in connected devices
    void OnDeviceAdded(std::vector<std::unique_ptr<DriverAPI>>& deviceDriversAdded);
    void OnDeviceRemoved(const std::set<std::string>& devicesRemoved);

    void CreateDevice(std::unique_ptr<DriverAPI>&& deviceDriver);
    void DeleteDevice(const std::string& deviceName);

    std::unordered_map<std::string, std::shared_ptr<Device>> m_devices; // map from unique string (unique inside a host) to a connected device
    unsigned m_enumerationPollingIntervalMs;
    const std::string m_enumerationTaskName;
    mutable std::mutex m_connectedDevicesMutex;
    DriversFactory m_driversFactory;
};