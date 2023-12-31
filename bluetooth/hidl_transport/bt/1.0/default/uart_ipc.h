/*==========================================================================
Description
  It has implementation for IPC logging mechanism.

# Copyright (c) 2017 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#pragma once

#include <hidl/HidlSupport.h>
#define UART_IPC_TX_LOG_PREFIX  "ramdump_bt_uart_ipc_tx_"
#define UART_IPC_RX_LOG_PREFIX  "ramdump_bt_uart_ipc_rx_"
#define UART_IPC_STATE_LOG_PREFIX "ramdump_bt_uart_ipc_state_"
#define UART_IPC_PWR_LOG_PREFIX "ramdump_bt_uart_ipc_pwr_"

#define UART_IPC_PATH_BUF_SIZE 255

namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

#ifdef DUMP_IPC_LOG

class IpcLogs
{
 private:
  bool DumpIpcLogs(const char *, const char *, long);
  void *logger_;

 public:
  // IPC log buffer sizes are based on the page sizes/number of pages used by UART driver
  // for different logs
  const unsigned long UART_IPC_LOG_PWR_MAX_SIZE = 2 * 10 * 4 * 1024;    // 2 times 10 pages of 4K size
  const unsigned long UART_IPC_LOG_MISC_MAX_SIZE = 2 * 30 * 4 * 1024; // 2 times 30 pages of 4K size
  const unsigned long UART_IPC_LOG_TX_RX_MAX_SIZE = 2 * 30 * 4 * 1024;  // 2 times 30 pages of 4K size
  //Kernel cannot guarantee contiguous allocation more than 16K
  const unsigned long UART_IPC_LOG_MAX_READ_PER_ITERATION = 16 * 1024;

  enum uart_crash_type{
    TX_LOG,
    RX_LOG,
    STATE_LOG,
    POWER_LOG,
  };
  void DumpUartLogs();
  IpcLogs() {};
};

#endif

} // namespace implementation
} // namespace V1_0
} // namespace bluetooth
} // namespace hardware
} // namespace android
