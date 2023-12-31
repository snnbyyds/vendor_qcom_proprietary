/*
* Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include <sstream>
#include <string>

#include "LogBufferReader.h"
#include "MessageDispatcher.h"
#include "Device.h"
#include "FwStateProvider.h"
#include "DebugLogger.h"
#include "DeviceManager.h"
#include "Host.h"
#include "LogCollectorDefinitions.h"
#include "LogCollectorModuleVerbosity.h"
#include "Utils.h" // for Timer

using namespace std;

LogBufferReader::LogBufferReader(const std::string& deviceName, CpuType tracerType, uint32_t logBufferInfoAddress, int ahbToLinkerDelta)
    : LogReaderBase(deviceName,
        deviceName + "_" + CpuTypeToString(tracerType) + " tracer: ",
        string(CpuTypeToString(tracerType)) + "_log_" + deviceName)
    , m_logBufferInfo(tracerType, logBufferInfoAddress, ahbToLinkerDelta)
    , m_messageDispatcher(new MessageDispatcher(m_deviceName, tracerType, false /*TSF not available*/))
    , m_extractedMessageParameters(MaxParams * 2) // maximum allowed DWs
{
    LOG_DEBUG << m_debugLogPrefix << "created with logBufferInfoAddress=" << Hex<8>(m_logBufferInfo.LogBufferInfoAddress)
        << ",  ahbToLinkerDelta=" << Hex<8>(m_logBufferInfo.AhbToLinkerDelta) << endl;
}

bool LogBufferReader::IsLogBufferAvailable(CpuType cpuType) const
{
    (void)cpuType;
    return m_logBufferInfo.LogBufferInfoAddress != Utils::REGISTER_DEFAULT_VALUE;
}

bool LogBufferReader::IsPollRequired() const
{
    return m_messageDispatcher->IsPollRequired();
}

OperationStatus LogBufferReader::UpdateDeviceInfo(const Device& device)
{
    OperationStatus os = ComputeLogBufferStartAddress(device, m_logBufferInfo);
    if (!os)
    {
        return os;
    }

    return GetModuleInfoFromDevice(device, m_logBufferInfo);
}

OperationStatus LogBufferReader::UpdateRingInfo(const Device& device)
{
    // not relevant for the legacy flow
    (void)device;
    return OperationStatus();
}

void LogBufferReader::ActivateConsumersIfRequired(const AtomicFwInfo& fwInfo)
{
    m_messageDispatcher->ActivateConsumersIfRequired(m_logBufferInfo.ModuleLevelInfo, fwInfo, m_logBufferInfo.LogAddressAhb, LogBufferContentDwords());
}

void LogBufferReader::ReportAndUpdateFwVersion(const AtomicFwInfo& fwInfo)
{
    m_messageDispatcher->ReportAndUpdateFwVersion(m_logBufferInfo.ModuleLevelInfo, fwInfo, m_logBufferInfo.LogAddressAhb, LogBufferContentDwords());
}

void LogBufferReader::ReportBufferFound()
{
    m_messageDispatcher->ReportBufferFound();
}

OperationStatus LogBufferReader::ReadAndConsumeBuffer(const Device& device)
{
    uint32_t wrPtr = 0;
    auto os = device.Read(m_logBufferInfo.LogAddressAhb, wrPtr);
    if (!os)
    {
        os.AddPrefix("Cannot read WrPtr from the log buffer");
        return os;
    }

    // Determine first iteration for correct miss rate calculation
    bool firstIteration = (m_rdPtr == 0);

    LOG_VERBOSE << m_debugLogPrefix << "Start iteration."
        << " First: " << BoolStr(firstIteration)
        << " WrPtr: " << wrPtr
        << " RdPtr: " << m_rdPtr
        << endl;

    if (m_minRdPtr > m_rdPtr)
    {
        // If log recording has been activated with IgnoreLogHistory flag,
        // write pointer has been captured during activation and saved in m_minRdPtr.
        LOG_DEBUG << m_debugLogPrefix << "Ignoring log history. Align rdPtr from " << m_rdPtr << " to " << m_minRdPtr << endl;
        m_rdPtr = m_minRdPtr;
        m_minRdPtr = 0;         // One-time action, we do not want this flow to be applied after device restart
    }

    if (wrPtr == m_rdPtr)
    {
        // No new messages since last iteration Nothing to read.
        LOG_VERBOSE << "Skip iteration, no new messages. rdPtr = wrPtr = " << wrPtr << endl;
        return OperationStatus();
    }

    // detect device restart
    bool deviceRestartDetected = false;
    // TODO: consider w_ptr wrap around as sub-condition
    if (wrPtr < m_rdPtr) // device was restarted since the last iteration - write pointer went back
    {
        LOG_INFO << m_debugLogPrefix << "Resetting RdPtr - device restart detected. WrPtr: " << wrPtr << " RdPtr: " << m_rdPtr << endl;

        // note: we do not limit here m_rdPtr not to read more than one buffer in order to catch and report buffer overrun during boot
        // if happens, it will be handled and reported by the regular overrun test
        m_rdPtr = 0U;
        firstIteration = true;
        deviceRestartDetected = true;
    }

    const uint32_t virtualChunkSizeDwords = wrPtr - m_rdPtr;        // Logs (DW) generated by FW since the previous iteration
    const auto logBufferContentSizeInDwords = LogBufferContentDwords();

    LOG_VERBOSE << m_debugLogPrefix << "Log buffer content in DW: " << logBufferContentSizeInDwords << endl;

    // May be updated below
    unsigned missedDwords = 0;
    bool corruptedEntryExpected = false;
    if (wrPtr - m_rdPtr > logBufferContentSizeInDwords) // no danger of underflow since w_ptr > m_rptr
    {
        // Log buffer overrun
        corruptedEntryExpected = true;  // We start parsing in the middle of a message, expect corrupted entry

        if (!firstIteration)
        {
            missedDwords = virtualChunkSizeDwords - logBufferContentSizeInDwords;
            m_totalLogDwords += virtualChunkSizeDwords;
            m_totalMissedDwords += missedDwords;
        }
        else
        {
            // First iteration - miss rate is irrelevant, we may start from a large rdPtr value
            m_totalLogDwords += std::min(virtualChunkSizeDwords, logBufferContentSizeInDwords);
        }

        // Advance read pointer to skip the missed log portion
        m_rdPtr = wrPtr - logBufferContentSizeInDwords;
    }
    else
    {
        // No overrun => no miss
        m_totalLogDwords += virtualChunkSizeDwords;
    }

    m_logChunkContent.clear();
    m_logChunkContent.reserve(m_logBufferContent.size()); // capacity unchanged

    os = ReadChunk(device, wrPtr);
    if (!os)
    {
        os.AddPrefix("Cannot read log buffer chunk");
        return os;
    }

    // The log buffer is being filled while we are reading it. So, the content may be corrupted in any part of the buffer.
    // The corruption may occur if final WrPtr reaches RdPtr that is deltaWrPtr > free space in the buffer.
    // the only way to prevent corruption is to skip possibly corrupted content that may increase miss rate.
    uint32_t postReadWrPtr = 0;
    os = device.Read(m_logBufferInfo.LogAddressAhb, postReadWrPtr);
    if (!os)
    {
        os.AddPrefix("Cannot read post read WrPtr from the log buffer");
        return os;
    }

    // At this point we are sure the diff is less than a buffer size
    const auto realChunkSizeDw = wrPtr - m_rdPtr;

    // This content may be corrupted by changes to log buffer while reading.
    // there is a small chance that wrPtr will be rolled-over during the delta period, so need to make sure the diff is positive.
    const uint32_t deltaWrPtrDw = (postReadWrPtr > wrPtr) ? postReadWrPtr - wrPtr : 0;
    const uint32_t freeSpaceDw = logBufferContentSizeInDwords - realChunkSizeDw;
    const uint32_t possiblyCorruptedDw = (deltaWrPtrDw <= freeSpaceDw)
        ? 0                                                         // Buffer change cannot overwrite the chunk
        : std::min(realChunkSizeDw, deltaWrPtrDw  - freeSpaceDw);   // Buffer change can overwrite the chunk, but up to its real size

    m_messageDispatcher->StartNewBuffer(
        m_logBufferInfo.ModuleLevelInfo,
        m_rdPtr, wrPtr, postReadWrPtr, possiblyCorruptedDw, firstIteration, deviceRestartDetected,
        reinterpret_cast<const uint32_t*>(m_logChunkContent.data()), realChunkSizeDw, virtualChunkSizeDwords,
        missedDwords, m_totalLogDwords, m_totalMissedDwords);

    uint32_t conversionErrCnt = 0;
    Timer chunkHandlingTimer;
    const uint32_t msgCounter = ConsumeRange(wrPtr, m_rdPtr, possiblyCorruptedDw, corruptedEntryExpected, conversionErrCnt);
    chunkHandlingTimer.Stop();

    // also collects statistics of processed chunk
    m_messageDispatcher->EndBuffer(msgCounter, chunkHandlingTimer.GetTotalMsec(), conversionErrCnt);
    return OperationStatus();
}

OperationStatus LogBufferReader::ReadChunk(const Device& device, uint32_t wrPtr)
{
    // Case 1: Read [RdPtr, WrPtr)
    // Case 2: Read [RdPtr, BufferEnd), [BufferBegin, WrPtr)

    const uint32_t rdPtrBufferIndex = LogPtrToByteIndex(m_rdPtr);
    const uint32_t wrPtrBufferIndex = LogPtrToByteIndex(wrPtr);

    LOG_VERBOSE << m_debugLogPrefix << "Read Chunk."
        << " RdPtr: " << m_rdPtr
        << " WrPtr: " << wrPtr
        << " RdPtr buffer index: " << rdPtrBufferIndex
        << " WrPtr buffer index: " << wrPtrBufferIndex
        << endl;

    if (rdPtrBufferIndex < wrPtrBufferIndex)
    {
        return ReadContinuousChunk(device, rdPtrBufferIndex, wrPtrBufferIndex);
    }

    // This operation always succeeds, even when rdPtrBufferIndex points to the last buffer element
    OperationStatus os = ReadContinuousChunk(device, rdPtrBufferIndex, static_cast<uint32_t>(m_logBufferContent.size()));
    if (!os)
    {
        return os;
    }

    if (wrPtrBufferIndex == 0)
    {
        // The next step is cancelled in the edge case when rdPtrBufferIndex == wrPtrBufferIndex == 0
        return os;
    }

    os = ReadContinuousChunk(device, 0, wrPtrBufferIndex);
    //LOG_VERBOSE << ">>>> Log buffer: " << MemoryDump(reinterpret_cast<uint32_t *>(m_logBufferContent.data()), m_logBufferContent.size() / sizeof(uint32_t)) << endl;

    return os;
}

OperationStatus LogBufferReader::ReadContinuousChunk(const Device& device, uint32_t beginIndex, uint32_t endIndex)
{
    const uint32_t srcAddress = m_logBufferInfo.LogAddressAhb + sizeof(LogBufferHeader) + beginIndex;
    const uint32_t length = endIndex - beginIndex;
    char * const pDstBuffer = reinterpret_cast<char*>(m_logBufferContent.data()) + beginIndex;

    LOG_VERBOSE << m_debugLogPrefix << "Read a continuous chunk."
        << " AHB address: " << Address(srcAddress)
        << " Length: " << length << " (B)"
        << " begin index: " << beginIndex
        << " end index: " << endIndex
        << " content buffer size: " << m_logBufferContent.size() << " (B)"
        << endl;

    if (endIndex <= beginIndex || beginIndex >= m_logBufferContent.size() || endIndex > m_logBufferContent.size())
    {
        LOG_ERROR << "Inconsistent log buffer indexes: begin: " << beginIndex << " end: " << endIndex << endl;
        return OperationStatus(false, "Invalid log buffer indexes");
    }

    auto os =  device.ReadBlock(srcAddress, length, pDstBuffer);
    if (!os)
    {
        return os;
    }

    // copy continuous log chunk content if needed
    if (m_messageDispatcher->IsDebugMode())
    {
        copy(m_logBufferContent.begin() + beginIndex, m_logBufferContent.begin() + endIndex, std::back_inserter(m_logChunkContent));
    }

    return OperationStatus();
}

uint32_t LogBufferReader::ConsumeRange(uint32_t wrPtr, uint32_t chunkPtr, uint32_t possiblyCorruptedDw, bool corruptedEntryExpected, uint32_t& conversionErrCnt)
{
    LOG_VERBOSE << m_debugLogPrefix << "Consume range."
        << " RdPtr = " << m_rdPtr
        << " WrPtr = " << wrPtr
        << " Possibly corrupted DWs: " << possiblyCorruptedDw
        << " Log buffer Size: " << LogBufferContentDwords() << " (DW)"
        << " Corrupted entry expected: " << BoolStr(corruptedEntryExpected)
        << endl;

    unsigned msgCounter = 0;

    while (m_rdPtr < wrPtr)
    {
        const uint32_t rdPtrBufferIndex = LogPtrToByteIndex(m_rdPtr);
        const log_trace_header *pLogEventHeader = reinterpret_cast<const log_trace_header *>(&m_logBufferContent[rdPtrBufferIndex]);
        if (pLogEventHeader->signature != LogLineHeaderSignature)
        {
            if (!corruptedEntryExpected)
            {
                if (!Tracer3ppLogsFilterOn)
                {
                    m_messageDispatcher->ReportCorruptedEntry(pLogEventHeader->signature);
                }
            }
            LOG_VERBOSE << "Skipping a non-signature dword."
                << " RdPtr = " << m_rdPtr
                << " Buffer index: " << rdPtrBufferIndex
                << " Value: " << Hex<>(pLogEventHeader->signature)
                << " Header: " << Hex<8>(reinterpret_cast<const uint32_t &>(pLogEventHeader))
                << endl;
            m_rdPtr++;
            continue;
        }
        corruptedEntryExpected = false; // at this point corrupted entries are not expected, next invalid signature should be reported.

        if (!Tracer3ppLogsFilterOn || m_logBufferInfo.ModuleLevelInfo[pLogEventHeader->module].third_party_mode == 1)
        {
            ConsumeMessage(wrPtr, chunkPtr, possiblyCorruptedDw, conversionErrCnt);
            ++msgCounter;
        }
    }

    return msgCounter;
}

void LogBufferReader::ConsumeMessage(uint32_t wrPtr, uint32_t chunkPtr, uint32_t possiblyCorruptedDw, uint32_t& conversionErrCnt)
{
    const uint32_t* pLogBufferBase = reinterpret_cast<const uint32_t*>(m_logBufferContent.data());
    const uint32_t logBufferContentSizeInDwords = LogBufferContentDwords();
    const uint32_t messageHeader = pLogBufferBase[m_rdPtr % logBufferContentSizeInDwords];
    const auto& logLineHeader = reinterpret_cast<const log_trace_header&>(messageHeader);
    const unsigned numOfDwords = 4 * logLineHeader.dword_num_msb + logLineHeader.dword_num_lsb;
    const auto msgLength = numOfDwords + 1U; // including header

    LOG_VERBOSE << "Extracted message."
        << " RdPtr = " << m_rdPtr
        << " WrPtr = " << wrPtr
        << " Dwords = " << numOfDwords
        << " Max params: " << MaxParams
        << endl;

    if (numOfDwords > (MaxParams * 2))
    {
        m_messageDispatcher->ReportTooManyParameters(numOfDwords, (MaxParams * 2));
        m_rdPtr += msgLength;
        return;
    }

    // increase m_rdPtr only at the end of iteration
    const auto paramsPtr = m_rdPtr + 1;
    if (wrPtr - paramsPtr < numOfDwords)
    {
        const uint32_t diffDW = numOfDwords - (wrPtr - paramsPtr);
        LOG_ERROR << m_debugLogPrefix << "Inconsistent wptr position in the middle of a message detected, diff (DWORDS) is " << diffDW << endl;
        m_messageDispatcher->ReportInconsistentWptr(diffDW);
        m_rdPtr = wrPtr;
        return;
    }

    // extract parameters
    m_extractedMessageParameters.clear(); // capacity unchanged
    for (unsigned i = 0; i < numOfDwords; ++i)
    {
        m_extractedMessageParameters.push_back(pLogBufferBase[(paramsPtr + i) % logBufferContentSizeInDwords]);
    }

    const auto msgOffset = m_rdPtr - chunkPtr;
    bool possiblyCorrupted = (msgOffset < possiblyCorruptedDw);

    bool invalidStringOffsetFound = false;
    m_messageDispatcher->DispatchMessage(
        possiblyCorrupted, messageHeader, m_extractedMessageParameters.data(), static_cast<uint32_t>(m_extractedMessageParameters.size()),
        reinterpret_cast<uint32_t*>(m_logChunkContent.data()) + msgOffset, msgOffset, m_rdPtr,
        conversionErrCnt, invalidStringOffsetFound);

    // In case of invalid string offset, try to prefetch the next message signature
    // Skip 1 dword instead of a message
    m_rdPtr += (invalidStringOffsetFound ? 1U : msgLength);
}

void LogBufferReader::ReportDeviceRemoved()
{
    if (m_valid)
    {
        LOG_DEBUG << m_debugLogPrefix << "device removal reported, going to reset Log reader and close recording files" << endl;
        m_messageDispatcher->ReportDeviceRemoved();
        ResetState();
    }
}

OperationStatus LogBufferReader::EnforceModuleVerbosity(CpuType cpuType)
{
    (void)cpuType;
    return EnforceModuleVerbosityImpl(m_enforcedVerbosity, m_logBufferInfo);
}

OperationStatus LogBufferReader::SetModuleVerbosity(CpuType cpuType, const std::vector<LogCollectorModuleVerbosity>& enforcedVerbosity)
{
    m_enforcedVerbosity = enforcedVerbosity;
    LOG_INFO << "Set enforced verbosity for " << cpuType << ": " << m_enforcedVerbosity << endl;

    return EnforceModuleVerbosity(cpuType);
}

OperationStatus LogBufferReader::GetModuleVerbosity(CpuType cpuType, std::vector<LogCollectorModuleVerbosity>& moduleVerbosity)
{
    (void)cpuType;
    return GetModuleVerbosityImpl(m_logBufferInfo, moduleVerbosity);
}

OperationStatus LogBufferReader::GetEnforcedModuleVerbosity(CpuType cpuType, std::vector<LogCollectorModuleVerbosity> &moduleVerbosity) const
{
    (void)cpuType;
    moduleVerbosity = m_enforcedVerbosity;
    return OperationStatus();
}

OperationStatus LogBufferReader::ActivateLogging(
    CpuType cpuType, LoggingType loggingType, bool ignoreLogHistory, bool compress, bool upload,
    const std::vector<LogCollectorModuleVerbosity>& enforcedVerbosity,
    RecordingTrigger recordingTrigger, bool debugMode)
{
    if (loggingType == PmcDataRecorder)
    {
        return OperationStatus(false, "PMC data collection activation supported in continuous PMC mode only");
    }

    return ActivateLoggingImpl(*m_messageDispatcher, cpuType, loggingType, ignoreLogHistory, compress, upload, enforcedVerbosity, recordingTrigger, debugMode);
}

OperationStatus LogBufferReader::RestoreLogging()
{
    // should be used only on restoring session - device was already running

    LOG_DEBUG << m_debugLogPrefix << "Restore logging state" << endl;
    if (m_messageDispatcher->IsActivationRequired()) // at least one consumer is enabled
    {
        if (!IsLogBufferAvailable(m_logBufferInfo.LogCpuType)) // unable to read log buffer start
        {
            const string errMsg = "Log buffer address unavailable";
            m_messageDispatcher->HandleErrorForEnabledConsumers(errMsg);
            return OperationStatus(false, errMsg);
        }
        LOG_INFO << m_debugLogPrefix << "activation required, resuming " << m_logBufferInfo.LogCpuType << " log poller" << endl;
        EnforceModuleVerbosity(m_logBufferInfo.LogCpuType);
        m_messageDispatcher->ReportPersistentRecordingTriggered();
        return RegisterPoller();
    }

    return OperationStatus();
}

OperationStatus LogBufferReader::DeactivateLogging(CpuType cpuType, LoggingType loggingType)
{
    (void)cpuType;
    LOG_INFO << m_debugLogPrefix << "Logging deactivation requested for " << loggingType << endl;

    if (loggingType == PmcDataRecorder)
    {
        return OperationStatus(false, "PMC data collection deactivation supported in continuous PMC mode only");
    }

    // closes the file for recording consumers, returns target directory
    auto os = m_messageDispatcher->StopConsumer(loggingType);

    if (!m_messageDispatcher->IsPollRequired())
    {
        LOG_DEBUG << m_debugLogPrefix << "no more enabled logging consumers, going to stop polling and reset Log reader" << endl;
        m_enforcedVerbosity.clear();
        UnRegisterPoller();
        ResetState();
    }

    return os;
}

OperationStatus LogBufferReader::SplitRecordings()
{
    LOG_INFO << m_debugLogPrefix << "Split recording file" << endl;
    return m_messageDispatcher->SplitRecordings();
}

LoggingState LogBufferReader::GetLoggingState(CpuType cpuType, LoggingType loggingType) const
{
    (void)cpuType;
    return m_messageDispatcher->GetConsumerState(loggingType);
}

const string& LogBufferReader::GetLoggingStateMsg(CpuType cpuType, LoggingType loggingType) const
{
    (void)cpuType;
    return m_messageDispatcher->GetConsumerStateMessage(loggingType);
}

ChunkHealthReport LogBufferReader::GetLoggingHealth(CpuType cpuType, LoggingType loggingType) const
{
    (void)cpuType;
    return m_messageDispatcher->GetConsumerHealth(loggingType);
}

LogFmtStringsContainer LogBufferReader::GetFmtStrings(CpuType cpuType, LoggingType loggingType) const
{
    (void)cpuType;
    return m_messageDispatcher->GetFmtStrings(loggingType);
}

uint32_t LogBufferReader::GetBufferSizeInBytesById(uint32_t bufferSizeId) const
{
    /*
    * The 3 MSBs of the value in REG_FW_USAGE_1 (for fw) and REG_FW_USAGE_2 (for uCode) are used to determine the size
    * of the their log buffer (respectively). The number created with these 3 MSBs is an index in the following sizes table:
    * 0 - default (4KB for FW, 1KB for uCode) - for backward compatibility
    * 1 - 1K
    * 2 - 2K
    * 3 - 4K
    * 4 - 8K
    * 5 - 16K
    * 6 - 32K
    * 7 - 64K
    */
    const int bufferSizeMultiplier[] = { 0, 1, 2, 4, 8, 16, 32, 64 };
    int result;
    const int KB = 1024;

    if (bufferSizeId > 7)
    {
        LOG_ERROR << "Could not get buffer size from logs header roll back to default size (CPU_TYPE_FW = 1024, CPU_TYPE_UCODE = 256)" << endl;
        bufferSizeId = 0;
    }

    if (bufferSizeId == 0)
    { // default values, for backward compatibility
        result = (m_logBufferInfo.LogCpuType == CPU_TYPE_FW) ? 4 * KB : 1 * KB;
    }
    else
    {
        result = bufferSizeMultiplier[bufferSizeId] * KB; // buffer size id is calculated by 3 bits only, so its range is 0-7. Therefore can't be out of range.
    }

    return result;
}

uint32_t LogBufferReader::GetCurrentWrPtr()
{
    shared_ptr<Device> spDevice;
    OperationStatus os = Host::GetHost().GetDeviceManager().GetDeviceByName(m_deviceName, spDevice);
    if (!os)
    {
        LOG_DEBUG << m_debugLogPrefix << "Cannot compute current wr_ptr, device is not available: " << m_deviceName << endl;
        return 0;
    }

    os = ComputeLogBufferStartAddress(*spDevice, m_logBufferInfo);
    if (!os)
    {
        LOG_DEBUG << m_debugLogPrefix << "Cannot compute current wr_ptr, device is not ready: " << m_deviceName << endl;
        return 0;
    }

    uint32_t wrPtr = 0;
    os = spDevice->Read(m_logBufferInfo.LogAddressAhb,  wrPtr);
    if (!os)
    {
        LOG_DEBUG << m_debugLogPrefix << "Cannot compute current wr_ptr for: " << m_deviceName << endl;
        return 0;
    }

    return wrPtr;
}

uint32_t LogBufferReader::LogPtrToByteIndex(uint32_t logPtr) const
{
    return (logPtr % LogBufferContentDwords()) * sizeof(uint32_t);
}