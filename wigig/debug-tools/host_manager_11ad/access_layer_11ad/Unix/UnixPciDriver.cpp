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

#include "DebugLogger.h"
#include "UnixPciDriver.h"
#include "DriverContracts.h"
#include "PmcCfg.h"
#include "PmcData.h"
#include "ShellCommandExecutor.h"
#include "OperationStatus.h"
#include "Utils.h"

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <chrono>
#include <thread>

#include <net/if.h> // for struct ifreq
#include <sys/socket.h>
#include <sys/ioctl.h> // for struct ifreq
#include <sys/types.h> // for opendir
#include <dirent.h> // for opendir
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define EP_OPERATION_READ 0
#define EP_OPERATION_WRITE 1
#define WIL_IOCTL_MEMIO (SIOCDEVPRIVATE + 2)
#define WIL_IOCTL_MEMIO_BLOCK (SIOCDEVPRIVATE + 3)
#define WIL_IOCTL_MEMIO_RDR_GET_DATA (SIOCDEVPRIVATE + 4)

using namespace std;

UnixPciDriver::UnixPciDriver(const std::string& interfaceName)
    : DriverAPI(DeviceType::PCI, interfaceName, true /* monitored interface */),
    m_fileDescriptor(-1),
    m_driverEventsHandler(m_interfaceName, false)
{
    m_fileDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_fileDescriptor < 0)
    {
        // do not report error here, legitimate scenario during enumeration
        return;
    }

    // debug FS path is lazy initialized
}

UnixPciDriver::~UnixPciDriver()
{
    if (IsReady())
    {
        close(m_fileDescriptor);
        m_fileDescriptor = -1;
    }
}

bool UnixPciDriver::IsReady() const
{
    return m_fileDescriptor >= 0;
}


// Base access functions (to be implemented by specific device)
OperationStatus UnixPciDriver::Read(uint32_t address, uint32_t& value)
{
    if (!IsReady())
    {
        ostringstream oss;
        oss << "driver for device " << m_interfaceName << " is not ready";
        return OperationStatus(false, oss.str());
    }

    IoctlIO io;

    io.addr = address;
    io.val = 0;
    io.op = EP_OPERATION_READ;
    OperationStatus os = SendRWIoctl(io, m_fileDescriptor, m_interfaceName.c_str());
    if (!os)
    {
        return os;
    }

    value = io.val;
    LOG_VERBOSE << "in UnixPciDriver::Read(" << Address(address) << ") = " << Hex<8>(value) << std::endl;
    return OperationStatus(true);
}

OperationStatus UnixPciDriver::ReadBlock(uint32_t address, uint32_t blockSizeInDwords, vector<uint32_t>& values)
{
    values.clear();
    values.resize(blockSizeInDwords);

    char* buffer = reinterpret_cast<char*>(values.data());
    const uint32_t blockSizeInBytes = values.size() * sizeof(uint32_t);

    return ReadBlock(address, blockSizeInBytes, buffer);
}

OperationStatus UnixPciDriver::ReadBlock(uint32_t address, uint32_t blockSizeInBytes, char *arrBlock)
{
    if (!IsReady())
    {
        ostringstream oss;
        oss << "driver for device " << m_interfaceName << " is not ready";
        return OperationStatus(false, oss.str());
    }

    IoctlIOBlock io;
    io.op = EP_OPERATION_READ;
    io.addr = address;
    io.size = blockSizeInBytes;
    io.buf = arrBlock;

    return SendRWBIoctl(io, m_fileDescriptor, m_interfaceName.c_str());
}

OperationStatus UnixPciDriver::ReadRadarData(uint32_t maxBlockSizeInDwords, vector<uint32_t>& values)
{
    values.clear();
    values.resize(maxBlockSizeInDwords);

    char* buffer = reinterpret_cast<char*>(values.data());
    const uint32_t blockSizeInBytes = values.size() * sizeof(uint32_t);
    IoctlRadarBlock rd{ blockSizeInBytes, buffer };

    struct ifreq ifr;
    ifr.ifr_data = reinterpret_cast<char*>(&rd);
    snprintf(ifr.ifr_name, IFNAMSIZ, "%s", m_interfaceName.c_str());
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    int ret = ioctl(m_fileDescriptor, WIL_IOCTL_MEMIO_RDR_GET_DATA, &ifr);
    if (ret < 0)
    {
        ostringstream oss;
        oss << "Failed to send RDR_GET_DATA IOCTL. Device: " << m_interfaceName
            << ", Max block size (Bytes): " << blockSizeInBytes
            << ", Error: " << strerror(errno);
        return OperationStatus(false, oss.str());
    }

    // Convert to number of DWORDS
    if (ret % 4 != 0)
    {
        ret += 4;
    }
    ret /= 4;

    // it is important to shrink the vector to the actual size in use
    values.resize(ret);

    return OperationStatus(true);
}

OperationStatus UnixPciDriver::Write(uint32_t address, uint32_t value)
{
    if (!IsReady())
    {
        ostringstream oss;
        oss << "driver for device " << m_interfaceName << " is not ready";
        return OperationStatus(false, oss.str());
    }

    if (!IsCapabilitySet(wil_nl_60g_driver_capa::NL_60G_DRIVER_CAPA_IOCTL_WRITE))
    {
        return OperationStatus(false, "write access is not supported by the network driver");
    }

    IoctlIO io = { EP_OPERATION_WRITE, address, value };
    return SendRWIoctl(io, m_fileDescriptor, m_interfaceName.c_str());
}

OperationStatus UnixPciDriver::WriteBlock(uint32_t address, uint32_t blockSizeInBytes, const char *valuesToWrite)
{
    if (!IsReady())
    {
        ostringstream oss;
        oss << "driver for device " << m_interfaceName << " is not ready";
        return OperationStatus(false, oss.str());
    }

    if (!IsCapabilitySet(wil_nl_60g_driver_capa::NL_60G_DRIVER_CAPA_IOCTL_WRITE))
    {
        return OperationStatus(false, "write access is not supported by the network driver");
    }

    IoctlIOBlock io;
    io.op = EP_OPERATION_WRITE;
    io.addr = address;
    io.size = blockSizeInBytes;
    io.buf = (void*)valuesToWrite;

    return SendRWBIoctl(io, m_fileDescriptor, m_interfaceName.c_str());
}

OperationStatus UnixPciDriver::RetryWhenBusyRead(uint32_t address, uint32_t& value)
{
    static const unsigned maxNumRetries = 41U; // 40 intervals of 5 mses
    static const unsigned sleepIntervalMsec = 5U;

    OperationStatus os = Read(address, value);
    unsigned numRetries = 1U;

    while (!os && errno == EBUSY && numRetries < maxNumRetries) // errno is thread local
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepIntervalMsec));
        os = Read(address, value);
        ++numRetries;
    }

    if (!os)
    {
        ostringstream oss;
        oss << "[num iterations is " << numRetries << "]";
        os.AddPrefix(oss.str());
    }

    return os;
}

OperationStatus UnixPciDriver::WriteBlock(uint32_t addr, vector<uint32_t> values)
{
    const char* valuesToWrite = reinterpret_cast<const char*>(values.data());
    uint32_t sizeToWriteInBytes = values.size() * sizeof(uint32_t);

    return WriteBlock(addr, sizeToWriteInBytes, valuesToWrite);
}

// lazy initialization of debug FS path
const string& UnixPciDriver::GetDebugFsPath()
{
    if (m_debugFsPath.empty())
    {
        SetDebugFsPath();
    }

    return m_debugFsPath;
}

bool UnixPciDriver::SetDebugFsPath() // assuming m_interfaceName is set
{
    // const definitions
    static const char* PHY = "phy";

    // find phy number
    ostringstream phyContaingFolder; // path of a folder which contains the phy of the specific interface
    phyContaingFolder << "/sys/class/net/" << m_interfaceName << "/device/ieee80211";
    DIR* dp = opendir(phyContaingFolder.str().c_str());
    if (!dp) // ieee80211 doesn't exist, meaning this isn't an 11ad interface
    {
        LOG_WARNING << "Failed to initialize Debug FS path, '" << phyContaingFolder.str() << "' doesn't exist" << endl;
        return false;
    }

    dirent* de; // read phy name (phy folder is named phyX where X is a digit). We assume there is only one folder, if not we take the first one
    do
    {
        de = readdir(dp);
    } while ((de != NULL) && (strncmp(PHY, de->d_name, strlen(PHY)) != 0));

    if (NULL == de)
    {
        LOG_WARNING << "Failed to initialize Debug FS path, cannot find related PHY name" << endl;
        closedir(dp);
        return false;
    }

    // find debug FS (using phy name)
    ostringstream debugFsPath;
    debugFsPath << "/sys/kernel/debug/ieee80211/" << de->d_name << "/wil6210";
    if(-1 == access(debugFsPath.str().c_str(), F_OK)) // didn't find debug FS
    {
        LOG_WARNING << "Failed to initialize Debug FS path, '" << debugFsPath.str() << "' is not accessible" << endl;
        closedir(dp);
        return false;
    }
    closedir(dp);

    // update debug FS path
    m_debugFsPath = debugFsPath.str();

    LOG_DEBUG << "Initialized Debug FS path: " << m_debugFsPath << endl;
    return true;
}

OperationStatus UnixPciDriver::AllocPmc(unsigned descSize, unsigned descNum)
{
    PmcCfg pmcCfg(GetDebugFsPath().c_str());
    return pmcCfg.AllocateDmaDescriptors(descSize, descNum);
}

OperationStatus UnixPciDriver::DeallocPmc(bool bSuppressError)
{
    PmcCfg pmcCfg(GetDebugFsPath().c_str());
    return pmcCfg.FreeDmaDescriptors(bSuppressError);
}

OperationStatus UnixPciDriver::CreatePmcFiles(unsigned refNumber)
{
    LOG_DEBUG << "Creating PMC files #" << refNumber << std::endl;

    PmcFileWriter pmcFileWriter(refNumber, GetDebugFsPath());

    OperationStatus st = pmcFileWriter.WriteFiles();
    if (!st.IsSuccess())
    {
        LOG_ERROR << "Error creating PMC files for #" << refNumber << std::endl;
        return st;
    }

    LOG_DEBUG << "PMC files created. Reported size: " << st.GetStatusMessage() << std::endl;
    return st;
}

OperationStatus UnixPciDriver::FindPmcDataFile(unsigned refNumber, std::string& foundPmcFilePath)
{
    LOG_DEBUG << "Looking for the PMC data file #" << refNumber << endl;

    PmcFileLocator pmcFileLocator(refNumber);

    if (!pmcFileLocator.DataFileExists())
    {
        std::ostringstream errorMsgBuilder;
        errorMsgBuilder << "Cannot find PMC data file " << pmcFileLocator.GetDataFileName();
        return OperationStatus(false, errorMsgBuilder.str());
    }

    foundPmcFilePath = pmcFileLocator.GetDataFileName();
    return OperationStatus(true);
}

OperationStatus UnixPciDriver::FindPmcRingFile(unsigned refNumber, std::string& foundPmcFilePath)
{
    LOG_DEBUG << "Looking for the PMC ring file #" << refNumber << endl;

    PmcFileLocator pmcFileLocator(refNumber);

    if (!pmcFileLocator.RingFileExists())
    {
        std::ostringstream errorMsgBuilder;
        errorMsgBuilder << "Cannot find PMC ring file " << pmcFileLocator.GetRingFileName();
        return OperationStatus(false, errorMsgBuilder.str());
    }

    foundPmcFilePath = pmcFileLocator.GetRingFileName();
    return OperationStatus(true);
}

OperationStatus UnixPciDriver::PmcGetData(uint32_t& bufLenBytesInOut, uint8_t* pBuffer, bool& hasMoreData)
{
    return m_driverEventsHandler.PmcGetData(PmcDataType::PMC_RING_PAYLOAD, bufLenBytesInOut, pBuffer, hasMoreData);
}

OperationStatus UnixPciDriver::InterfaceReset()
{
    if (!IsReady())
    {
        ostringstream oss;
        oss << "driver for device " << m_interfaceName << " is not ready";
        return OperationStatus(false, oss.str());
    }

    // For doing an interface reset, ifconfig down and up is done
    // When this method does not work, try reseting FW through a Driver command

    ostringstream ss;

    // Interface down:
    ss << "ifconfig " << m_interfaceName << " down";
    ShellCommandExecutor sce(ss.str().c_str());
    if (sce.ExitStatus() != 0)
    {
        ostringstream errStr;
        errStr << "Couldn't make '" << ss.str() << "': " << (sce.Output().size() > 0 ? sce.Output().front() : "");
        return OperationStatus(false, errStr.str());
    }

    ss.str(string());

    // Interface up:
    ss << "ifconfig " << m_interfaceName << " up";
    ShellCommandExecutor sce1(ss.str().c_str());
    if (sce1.ExitStatus() == 0)
    {
        LOG_INFO << "Interface reset for " << m_interfaceName << " completed through ifconfig" << endl;
        return OperationStatus(true);
    }
    else
    {
        // print this message only for debug
        LOG_DEBUG << "Couldn't make '" << ss.str() << "': " << (sce1.Output().size() > 0 ? sce1.Output().front() : "") << endl;
    }

    // If we got here, it may be a case of WMI_ONLY FW. Try to reset FW through a Driver command.
    // Note: This will not work when using Debug mailbox or using older driver before adding support for this command.
    return m_driverEventsHandler.TryFwReset();
}

OperationStatus UnixPciDriver::GetFwState(wil_fw_state& fwState)
{
    if (!IsReady()) // shouldn't happen
    {
        ostringstream oss;
        oss << "driver for device " << m_interfaceName << " is not ready";
        return OperationStatus(false, oss.str());
    }

    if (!IsCapabilitySet(wil_nl_60g_driver_capa::NL_60G_DRIVER_CAPA_FW_STATE))
    {
        ostringstream oss;
        oss << "Unsupported driver command (NL_60G_GEN_GET_FW_STATE) for device " << m_interfaceName;
        return OperationStatus(false, oss.str());
    }

    return m_driverEventsHandler.GetFwState(fwState);
}

bool UnixPciDriver::InitializeTransports()
{
    if (!IsReady())
    {   // shouldn't happen
        LOG_ERROR << "Failed to initialize driver transports for " << m_interfaceName << ", driver is not ready\n";
        return false;
    }

    m_driverEventsHandler.InitializeTransports(m_capabilityMask);
    LOG_INFO << "wil6210 driver capabilities are 32b'" << m_capabilityMask << endl;
    return true;
}

void UnixPciDriver::RegisterDriverControlEvents(FwStateProvider* fwStateProvider)
{
    if (!IsReady())
    {
        return;
    }

    if (IsCapabilitySet(wil_nl_60g_driver_capa::NL_60G_DRIVER_CAPA_WMI))
    {
        m_driverEventsHandler.RegisterForDriverEvents(fwStateProvider);
    }
}

OperationStatus UnixPciDriver::DriverControl(uint32_t id,
                                             const void *inBuf, uint32_t inBufSize,
                                             void *outBuf, uint32_t outBufSize)
{
    if (!IsReady()) // shouldn't happen
    {
        ostringstream oss;
        oss << "driver for device " << m_interfaceName << " is not ready (driver command with id " << Hex<>(id) << ")";
        return OperationStatus(false, oss.str());
    }

    return m_driverEventsHandler.SendDriverCommand(id, inBuf, inBufSize, outBuf, outBufSize);
}

OperationStatus UnixPciDriver::SendWmiWithEvent(const void *inBuf, uint32_t inBufSize, uint32_t commandId, uint32_t moduleId, uint32_t subtypeId,
        uint32_t eventId, unsigned timeoutSec, std::vector<uint8_t>& eventBinaryData)
{
    if (!IsReady()) // shouldn't happen
    {
        ostringstream oss;
        oss << "driver for device " << m_interfaceName << " is not ready (WMI command)" << ")";
        return OperationStatus(false, oss.str());
    }

    return m_driverEventsHandler.SendWmiWithEvent(inBuf, inBufSize, commandId, moduleId, subtypeId, eventId, timeoutSec, eventBinaryData);
}

void UnixPciDriver::StopDriverControlEvents()
{
    m_driverEventsHandler.StopDriverControlEventsBlocking();
}

OperationStatus UnixPciDriver::SendRWIoctl(IoctlIO & io, int fd, const char* interfaceName)
{
    int ret;
    struct ifreq ifr;
    ifr.ifr_data = (char*)&io;

    snprintf(ifr.ifr_name, IFNAMSIZ, "%s", interfaceName);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    ret = ioctl(fd, WIL_IOCTL_MEMIO, &ifr); // read/write DWORD
    if (ret < 0)
    {
        ostringstream oss;
        oss << "Failed to send RW (dword) IOCTL. "
            << DeviceAddressBlock(m_interfaceName, io.addr, sizeof(uint32_t))
            << ", Op: " << io.op
            << ", Error: " << strerror(errno);
        return OperationStatus(false, oss.str());
    }

    return OperationStatus(true);
}

OperationStatus UnixPciDriver::SendRWBIoctl(IoctlIOBlock & io, int fd, const char* interfaceName)
{
    struct ifreq ifr;
    ifr.ifr_data = (char*)&io;

    snprintf(ifr.ifr_name, IFNAMSIZ, "%s", interfaceName);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    int ret = ioctl(fd, WIL_IOCTL_MEMIO_BLOCK, &ifr);  // read/write BYTES. number of bytes must be multiple of 4
    if (ret < 0)
    {
        ostringstream oss;
        oss << "Failed to send RW (Bytes) IOCTL. "
            << DeviceAddressBlock(m_interfaceName, io.addr, io.size)
            << ", Op: " << io.op
            << ", Error: " << strerror(errno);
        return OperationStatus(false, oss.str());
    }

    return OperationStatus(true);
}
