////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  camxentry.h
/// @brief CamX entry class implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CAMXENTRY_H
#define CAMXENTRY_H

#include "camxdefs.h"
#include "camxhal3.h"
#include "camxtypes.h"

CAMX_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Manages entry/exit point jumptables
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Dispatch
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Dispatch
    ///
    /// @brief  Constructor for initial jump table population
    ///
    /// @param  pJumpTable   Pointer to the initial jump table
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    explicit Dispatch(
        VOID* pJumpTable);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ~Dispatch
    ///
    /// @brief  Destructor
    ///
    /// @return n/a
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ~Dispatch() = default;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetJumpTable
    ///
    /// @brief  Returns the jump table for HAL3 entry points
    ///
    /// @return Pointer to the HAL3 jump table for this call
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID* GetJumpTable();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// SetJumpTable
    ///
    /// @brief  Sets the jump table for entry points
    ///
    /// @param  pJumpTable   Pointer to the new jump table
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    VOID SetJumpTable(
        VOID* pJumpTable);

private:
    VOID* m_pJumpTable;                             ///< Current jump table for any API

    Dispatch(const Dispatch&) = delete;             // Disable copy constructor
    Dispatch& operator=(const Dispatch&) = delete;  // Disable assignment
};

CAMX_NAMESPACE_END

#endif // CAMXENTRY_H