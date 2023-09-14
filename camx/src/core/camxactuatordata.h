////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file camxactuatordata.h
/// @brief Actuator driver data
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CAMXACTUATORDATA_H
#define CAMXACTUATORDATA_H

#include "camxactuatordriver.h"
#include "camxcslsensordefs.h"
#include "camxdefs.h"
#include "camxhal3defs.h"
#include "camxsensorproperty.h"
#include "camxtypes.h"

CAMX_NAMESPACE_BEGIN

/// @todo (CAMX-831) Support table config with Settings camxsettings.xml to support custom AF algorithm
static const UINT  MoveToMacro                = 0;                     ///< Indicate move direction
static const UINT  MoveToInfinity             = 1;                     ///< Indicate move direction
static const UINT  MaxSteps                   = 400;                   ///< Max number of steps
static const UINT  MacroIndex                 = 0;                     ///< Step table index which stores macro DAC
static const UINT  InfinityIndex              = MaxSteps - 1;          ///< Step table index which stores infinity DAC
static const UINT  ParkLensDelayMicroseconds  = 10000;                 ///< Delay in microseconds

static const UINT  InvalidLensData            = 0x7FFFFFFF;            ///< Invalid lens data
static const UINT  ParkLensPaceValue          = 50;                    ///< Distance the lens is moved
static const UINT  PaceThroughDistance        = 150;                   ///< Distance to gradually move the lens through

/// @brief PositionUnit
enum class PositionUnit
{
    Step,        ///< Logical step
    DAC,         ///< HW DAC code
    NoOperation, ///< No change of position needed
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Class containing actuator APIs
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ActuatorData
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ActuatorData
    ///
    /// @brief  Constructor
    ///
    /// @param  pActuatorDriverData    Pointer to actuator driver data
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    explicit ActuatorData(
        ActuatorDriverData* pActuatorDriverData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ~ActuatorData
    ///
    /// @brief  Destructor
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ~ActuatorData();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// InitializeStepTable
    ///
    /// @brief  Initialize step table
    ///
    /// @param  pAFData Actuator calibration data from OTP
    ///
    /// @return Success or failure
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult InitializeStepTable(
        AFCalibrationData* pAFData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// CreateI2CInfoCmd
    ///
    /// @brief  Create I2C Info command
    ///
    /// @param  pI2CInfoCmd I2CInfo command
    ///
    /// @return CamxResultSuccess, if SUCCESS
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult CreateI2CInfoCmd(
        CSLSensorI2CInfo* pI2CInfoCmd);

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
    /// GetInitializeCmdSize
    ///
    /// @brief  Get size for init command buffer
    ///
    /// @return Size of the command buffer
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT GetInitializeCmdSize();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// CreateInitializeCmd
    ///
    /// @brief  Create initialize commands to load init register settings. It is one-time execution at node initialization.
    ///
    /// @param  pCmdBuffer  init I2C command
    ///
    /// @return CamxResultSuccess, if SUCCESS
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult CreateInitializeCmd(
        VOID*   pCmdBuffer);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetMaxMoveFocusCmdSize
    ///
    /// @brief  Get Max size for move focus command buffer
    ///
    /// @return Size of the command buffer
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT GetMaxMoveFocusCmdSize();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// CreateMoveFocusCmd
    ///
    /// @brief  Create move focus command
    ///
    /// @param  targetPosition     Target position
    /// @param  unit               Unit to specify position in DAC or step
    /// @param  pCmdBuffer         Move focus I2C command
    /// @param  additionalDelay    Additional delay besides those specified in driver
    ///
    /// @return CamxResultSuccess, if SUCCESS
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult CreateMoveFocusCmd(
    INT          targetPosition,
    PositionUnit unit,
    VOID*        pCmdBuffer,
    UINT16       additionalDelay);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetMoveFocusCmdSize
    ///
    /// @brief  Get the command buffer size dependening on whether additional delay is being provided or not and operation.
    ///
    /// @param  additionalDelay  Additional Delay for each actuator operation.
    ///
    /// @return command size
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    INT GetMoveFocusCmdSize(
        UINT16 additionalDelay);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// StepToDAC
    ///
    /// @brief  convert step index to DAC value
    ///
    /// @param  targetPositionStep  Target position in step index
    ///
    /// @return DAC value
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    INT16 StepToDAC(
        const UINT targetPositionStep);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetCurrentPosition
    ///
    /// @brief  Get current position
    ///
    /// @param  unit Specify position in DAC or Step
    ///
    /// @return Lens Postion
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    INT GetCurrentPosition(
        PositionUnit unit);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetSensitivity
    ///
    /// @brief  Get actuator sensitivity
    ///
    /// @return Actuator sensitivity
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CAMX_INLINE FLOAT GetSensitivity()
    {
        return (static_cast<FLOAT>(m_stepTableSize) / (m_macroDAC - m_infinityDAC));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetActuatorLibrary
    ///
    /// @brief  Retrieve the actuator library
    ///
    /// @return Actuator data object at index or NULL
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CAMX_INLINE const ActuatorDriverData* GetActuatorLibrary()
    {
        return m_pActuatorDriverData;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetActuatorDataBitwidth
    ///
    /// @brief  Get actuator data bitwidth
    ///
    /// @return Actuator data bitwidth
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CAMX_INLINE UINT16 GetActuatorDataBitwidth()
    {
        return  m_pActuatorDriverData->slaveInfo.dataBitWidth;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetInfinityPosition
    ///
    /// @brief  Retrieve the Infinity DAC position
    ///
    /// @return Infinity DAC value
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CAMX_INLINE INT16 GetInfinityPosition()
    {
        return m_infinityDAC;
    }


private:

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// CalibrateActuatorDriverData
    ///
    /// @brief  Calibrates the actuator driver data with the OTP data obtained from EEPROM
    ///
    /// @param  pAFData Actuator calibration data from OTP
    ///
    /// @return Success or failure
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    CamxResult CalibrateActuatorDriverData(
        const AFCalibrationData* pAFData);

    ActuatorData(const ActuatorData&) = delete;               ///< Disallow the copy constructor
    ActuatorData& operator=(const ActuatorData&) = delete;    ///< Disallow assignment operator

    INT16               m_stepTable[MaxSteps]; ///< Step position table mapping step number to DAC code
    UINT                m_currentPositionStep; ///< Current lens position in step index
    UINT                m_currentPositionDAC;  ///< Current lens position in DAC
    INT16               m_infinityDAC;         ///< Infinity position in DAC
    INT16               m_macroDAC;            ///< Macro position in DAC
    UINT                m_stepTableSize;       ///< Step table size
    ActuatorDriverData* m_pActuatorDriverData; ///< Actuator driver data pointer
};

CAMX_NAMESPACE_END

#endif // CAMXACTUATORDATA_H