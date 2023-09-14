////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file camxeepromdata.h
/// @brief API to interact with the underlying EEPROM driver
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CAMXEEPROMDATA_H
#define CAMXEEPROMDATA_H

#include "camxcmdbuffermanager.h"
#include "camxcsl.h"
#include "camxcslsensordefs.h"
#include "camxdefs.h"
#include "camxeepromdriver.h"
#include "camxhal3defs.h"
#include "camxhwcontext.h"
#include "camxpacket.h"
#include "camxsensorproperty.h"
#include "camxtypes.h"
#include "camxeepromdriverapi.h"

CAMX_NAMESPACE_BEGIN

static const UINT MaximumNumberOfMemoryBlocks         = 100;
static const UINT MaximumNumberOfSettingsPerBlock     = 80;
static const UINT BitsPerByte                         = 8;
static const UINT BitsPerInteger                      = 32;
static const UINT GainMapPrecision                    = 256;

static const UINT32 HWRollOffTableRowSize             = 13;
static const UINT32 HWRollOffTableColSize             = 17;
static const UINT32 RelativeRotationMatrixMax         = 9;
static const UINT32 RelativeGeometricSurfaceParamsMax = 32;
static const FLOAT  QValueDefault                     = 1.0;
static const FLOAT  DualAecDefaultValue               = -1.0;
static const UINT32 DualDefaultColorTemp              = 0;

struct MemoryMapParsedData
{
    UINT32 memoryBlockStartingIndex;    ///< Indicates the starting address of the each memory block in the EEPROM memory map
    UINT32 numberOfMemorySettings;      ///< Indicates number of memory settings available in a particular block
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class containing APIs to interact with the underlying EEPROM hardware
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class EEPROMData
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// EEPROMData
    ///
    /// @brief  constructor for the EEPROMData class.
    ///
    /// @param  pEEPROMDriverData   Pointer to EEPROM driver data.
    /// @param  pSensorInfoTable    slot of the EEPROM obtained through enumurate devices.
    /// @param  pDeviceInfo         device info containing device indicies of CSLDeviceTypeEEPROM.
    /// @param  hCSL                Handle to the CSL session
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    explicit EEPROMData(
        EEPROMDriverData*       pEEPROMDriverData,
        HwSensorInfo*           pSensorInfoTable,
        const HwDeviceTypeInfo* pDeviceInfo,
        CSLHandle               hCSL);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ~EEPROMData
    ///
    /// @brief  Default destructor for the ~EEPROMData class.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ~EEPROMData();

private:

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ParseMemoryMapData
    ///
    /// @brief  Helper method to obtain the memory map data from the EEPROM driver and generates MemoryMapParsedData.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID ParseMemoryMapData();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetEEPROMCSLDeviceIndex
    ///
    /// @brief  Helper method to get device index of the EEPROM device corresponding the provided EEPROM slot ID.
    ///
    /// @param  EEPROMSlotID    slot of the EEPROM obtained through enumurate devices.
    /// @param  pDeviceInfo     device info containing device indicies of CSLDeviceTypeEEPROM.
    ///
    /// @return CamxResultSuccess
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult GetEEPROMCSLDeviceIndex(
        UINT32                  EEPROMSlotID,
        const HwDeviceTypeInfo* pDeviceInfo);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// InitializeCSL
    ///
    /// @brief  Helper method to initialize and acquire the CSL session to communicate with EEPROM.
    ///
    /// @param  EEPROMSlotID    slot of the EEPROM obtained through enumurate devices.
    /// @param  pDeviceInfo     device info containing device indicies of CSLDeviceTypeEEPROM.
    ///
    /// @return CamxResultSuccess in case if success or failure code.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult InitializeCSL(
        UINT32                  EEPROMSlotID,
        const HwDeviceTypeInfo* pDeviceInfo);


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// UnInititialize
    ///
    /// @brief  Helper method to release and close the CSL session.
    ///
    /// @return CamxResultSuccess
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult UnInititialize();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetPowerSequenceCmdSize
    ///
    /// @brief  Get size for power up/down command buffer
    ///
    /// @param  isPowerUp   Is it power up or power down
    ///
    /// @return Size of the command buffer
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT GetPowerSequenceCmdSize(
        BOOL isPowerUp);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// CreatePowerSequenceCmd
    ///
    /// @brief  Create power sequence commands
    ///
    /// @param  isPowerUp   Is it power up or power down
    /// @param  pCmdBuffer  Power up/down command
    ///
    /// @return CamxResultSuccess, if SUCCESS
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult CreatePowerSequenceCmd(
        BOOL    isPowerUp,
        VOID*   pCmdBuffer);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// CreateI2CInfoCmd
    ///
    /// @brief  Create I2C Info commands
    ///
    /// @param  pI2CInfoCmd     Pointer to I2CInfo command
    /// @param  memoryMapIndex  Memory Map Index
    ///
    /// @return CamxResultSuccess, if SUCCESS
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult CreateI2CInfoCmd(
    CSLSensorI2CInfo*   pI2CInfoCmd,
    UINT16              memoryMapIndex);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetMemoryMapCmdSize
    ///
    /// @brief  Get the command size of the Memory Block
    ///
    /// @param  memMapIndex  Memory Map Index
    ///
    /// @return Size of the Memorymap commands in a Block
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT GetMemoryMapCmdSize(
        UINT memMapIndex);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetTotalMemoryMapCmdSize
    ///
    /// @brief  Get the total command size of the Memory map
    ///
    /// @return Size of the complete Memorymap
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT GetTotalMemoryMapCmdSize();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// CreateMemoryMapCmd
    ///
    /// @brief  Create the command for the Memory Map
    ///
    /// @param  pCmdBuffer   Pointer to the command Buffer
    /// @param  memMapIndex  Memory Map Index
    ///
    /// @return success or failure
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult CreateMemoryMapCmd(
        VOID* pCmdBuffer,
        UINT  memMapIndex);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetMemorySizeBytes
    ///
    /// @brief  Get the total memory size to be read in bytes.
    ///
    /// @return Size of the memory map in bytes
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    SIZE_T GetMemorySizeBytes();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetCommandBuffer
    ///
    /// @brief  Helper method to obtain command buffer
    ///
    /// @param  pCmdManager         pointer to the command buffer manager
    /// @param  ppPacketResource    Pointer to packet resource containing buffer info
    ///
    /// @return success or failure
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult GetCommandBuffer(
        CmdBufferManager*   pCmdManager,
        PacketResource**    ppPacketResource);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ReadEEPROMDevice
    ///
    /// @brief  Helper method to read the EEPROM device
    ///
    /// @return CamxResultSuccess or Failure code.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult ReadEEPROMDevice();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// MaskLengthInBytes
    ///
    /// @brief  Get the total number of valid bytes in the mask.
    ///
    /// @param  mask   input mask in which bytes needs to be calculated
    ///
    /// @return number of the valid bytes in the mask
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT32 MaskLengthInBytes(
        UINT mask);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// MaskLengthInBits
    ///
    /// @brief  Get the total number of bits set to 1 in the mask.
    ///
    /// @param  mask   input mask in which bits set to 1 needs to be calculated.
    ///
    /// @return number of the bits set to 1 in the mask
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT32 MaskLengthInBits(
        UINT mask);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FormatDataTypeByte
    ///
    /// @brief  Read the byte data from the OTP and format it to exact bits to be copied
    ///
    /// @param  pMemoryInfo     Contains memory address to read and mask having the number bytes to read and exact bits to copy.
    ///
    /// @return number of the bits set to 1 in the mask
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT8 FormatDataTypeByte(
        MemoryInfo* pMemoryInfo);


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FormatDataTypeInteger
    ///
    /// @brief  Read the integer data from the OTP buffer read from EEPROM.
    ///
    /// @param  pMemoryInfo     Contains memory address to read and mask having the number bytes to read and exact bits to copy.
    /// @param  endian          Enum value indicating whether the data is in big endian or little endian format
    ///
    /// @return integer value from the OTP buffer preserving the sign bit information.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    INT32 FormatDataTypeInteger(
        MemoryInfo* pMemoryInfo,
        EndianType endian);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FormatDataTypeFloat
    ///
    /// @brief  Read the float data from the OTP buffer read from EEPROM.
    ///
    /// @param  pMemoryInfo     Contains memory address to read and mask having the number bytes to read and exact bits to copy.
    ///
    /// @return float value from the OTP buffer.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    FLOAT FormatDataTypeFloat(
        MemoryInfo* pMemoryInfo);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FormatAFData
    ///
    /// @brief  Reads the AF data from the OTP buffer and copies to AF data calibration part of the sensor module
    ///         static capabilities.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID FormatAFData();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FormatWBData
    ///
    /// @brief  Reads the WB data from the OTP buffer and copies to WB data calibration part of the sensor module static
    ///         capabilities
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID FormatWBData();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FormatLSCData
    ///
    /// @brief  Reads the LSC data from the OTP buffer and copies to LSC data calibration part of the sensor module static
    ///         Capabilities.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID FormatLSCData();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FormatDualCameraData
    ///
    /// @brief  Reads the dual camera data from the OTP buffer and copies to dual camera data calibration part of the sensor
    ///         module static capabilities
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID FormatDualCameraData();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FormatSPCData
    ///
    /// @brief  Reads the SPC data from the OTP buffer and copies to SPC data calibration part of the sensor module static
    ///         capabilities.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID FormatSPCData();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FormatQSCData
    ///
    /// @brief  Reads the QCFA sensitivity correction data from the OTP buffer and copies to QSC data calibration part of the
    ///         sensor module static capabilities.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID FormatQSCData();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FormatOISData
    ///
    /// @brief  Reads the OIS data from the OTP buffer and copies to OIS data calibration part of the sensor module static
    ///         capabilities.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID FormatOISData();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FormatPDAFDCCData
    ///
    /// @brief  Reads the PDAF DCC data from the OTP buffer and copies to PDAF DCC data calibration part of the sensor module
    ///         static capabilities.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID FormatPDAFDCCData();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FormatPDAF2DData
    ///
    /// @brief  Reads the PDAF 2D data from the OTP buffer and copies to PDAF 2D data calibration part of the sensor module
    ///         static capabilities.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID FormatPDAF2DData();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FormatLensData
    ///
    /// @brief  Reads the Lens data from the OTP buffer and copies to lens part of the sensor module
    ///         static capabilities.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID FormatLensData();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// LoadEEPROMLibrary
    ///
    /// @brief  Loads eeprom library having API implementations.
    ///
    /// @return CamxResultSuccess, if SUCCESS
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult LoadEEPROMLibrary();

    EEPROMData(const EEPROMData&)            = delete;    ///< Disallow the copy constructor
    EEPROMData& operator=(const EEPROMData&) = delete;    ///< Disallow assignment operator

    EEPROMDriverData*       m_pEEPROMDriverData;                                ///< Pointer to the EEPROM driver data
    HwSensorInfo*           m_pSensorInfoTable;                                 ///< Pointer to the sensor information
    INT32                   m_EEPROMDeviceIndex;                                ///< Device index of this EEPROM node
    CSLHandle               m_hEEPROMSessionHandle;                             ///< CSL session handle
    BOOL                    m_isCSLOpenByEEPROM;                                ///< Indicates whether CSL session is opened
                                                                                ///   by EEPROM or not
    CSLDeviceHandle         m_hEEPROMDevice;                                    ///< EEPROM device handle
    UINT32                  m_numberOfMemoryBlocks;                             ///< number of memory blocks in the memory map
    MemoryMapParsedData     m_memoryMapParsedData[MaximumNumberOfMemoryBlocks]; ///< maximum number of memory maps allowed
    Packet*                 m_pEEPROMInitReadPacket;                            ///< Handle of EEPROM packet
    CmdBufferManager*       m_pPacketManager;                                   ///< Command Buffer Manager for Packet
    ImageBufferManager*     m_pImageBufferManager;                              ///< Image Buffer Manager object
    ImageBuffer*            m_pImage;                                           ///< Image buffer to hold the OTP data read
    UINT8*                  m_pOTPData;                                         ///< pointer to the OTP data read from EEPROM
    UINT32                  m_OTPDataSize;                                      ///< size of the OTP data to read
    BOOL                    m_deviceAcquired;                                   ///< device acquire state
    EEPROMLibraryAPI        m_EEPROMLibraryAPI;                                 ///< pointers to APIs in EEPROM library.
    VOID*                   m_phEEPROMLibHandle;                                ///< Handle of EEPROM library
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class containing APIs to dump the EEPROM data
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class EEPROMDataDump
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// EEPROMDataDump
    ///
    /// @brief  constructor for the EEPROMDataDump class.
    ///
    /// @param  pEEPROMDriverData   Pointer to EEPROM driver data.
    /// @param  pSensorInfoTable    slot of the EEPROM obtained through enumurate devices.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    explicit EEPROMDataDump(
        EEPROMDriverData*       pEEPROMDriverData,
        HwSensorInfo*           pSensorInfoTable);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ~EEPROMDataDump
    ///
    /// @brief  Default destructor for the ~EEPROMDataDump class.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ~EEPROMDataDump();

private:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// PrintLightType
    ///
    /// @brief  prints the light type name based on the illuminant
    ///
    /// @param  pDumpFile   Pointer to the file to print the illuminant
    /// @param  illuminant  Illuminant value
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID PrintLightType(
        FILE*                pDumpFile,
        EEPROMIlluminantType illuminant);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DumpAFData
    ///
    /// @brief  Dumps the AF data from AF calibration part of the sensor module static capabilities.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID DumpAFData();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DumpWBData
    ///
    /// @brief  Dumps the WB data from WB calibration part of the sensor module static capabilities.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID DumpWBData();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DumpLSCData
    ///
    /// @brief  Dumps the LSC data from LSC calibration part of the sensor module static capabilities.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID DumpLSCData();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DumpDualCameraData
    ///
    /// @brief  Dumps the dual camera data from dual camera calibration part of the sensor module static capabilities.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID DumpDualCameraData();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DumpPDAFData
    ///
    /// @brief  Dumps the PDAF data from PDAF calibration part of the sensor module static capabilities.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID DumpPDAFData();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DumpKernelBuffer
    ///
    /// @brief  Dumps the buffer returned from kernel, byte by byte, into a text file.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID DumpKernelBuffer();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// DumpEEPROMData
    ///
    /// @brief  Dump the EEPROM data to a file.
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID DumpEEPROMData();

    EEPROMDataDump(const EEPROMDataDump&)            = delete;    ///< Disallow the copy constructor
    EEPROMDataDump& operator=(const EEPROMDataDump&) = delete;    ///< Disallow assignment operator

    EEPROMDriverData*       m_pEEPROMDriverData;                  ///< Pointer to the EEPROM driver data
    HwSensorInfo*           m_pSensorInfoTable;                   ///< Pointer to the sensor information
};

CAMX_NAMESPACE_END

#endif // CAMXEPROMDATA_H