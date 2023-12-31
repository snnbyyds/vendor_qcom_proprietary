/*
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright 2012 The Android Open Source Project
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "hci_transport.h"

#define PWR_SRC_NOT_AVAILABLE -2
#define DEFAULT_INVALID_VALUE -1
#define PWR_SRC_INIT_STATE_IDX 0
#define MAX_TS_SIZE 64

namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

/*Constants*/
typedef enum {
  POWER_MANAGER_OFF = 0,
  POWER_MANAGER_TURNING_ON,
  POWER_MANAGER_ON,
  POWER_MANAGER_TURNING_OFF,
} PowerManagerState;

enum power_src_pos {
  BT_RESET_GPIO = PWR_SRC_INIT_STATE_IDX,
  BT_SW_CTRL_GPIO,
  BT_VDD_AON_LDO,
  BT_VDD_DIG_LDO,
  BT_VDD_RFA1_LDO,
  BT_VDD_RFA2_LDO,
  BT_VDD_ASD_LDO,
  BT_VDD_XTAL_LDO,
  BT_VDD_PA_LDO,
  BT_VDD_CORE_LDO,
  BT_VDD_IO_LDO,
  BT_VDD_LDO,
  BT_VDD_RFA_0p8,
  BT_VDD_RFACMN,
  // these indexes GPIOs/regs value are fetched during crash.
  BT_RESET_GPIO_CURRENT,
  BT_SW_CTRL_GPIO_CURRENT,
  BT_VDD_AON_LDO_CURRENT,
  BT_VDD_DIG_LDO_CURRENT,
  BT_VDD_RFA1_LDO_CURRENT,
  BT_VDD_RFA2_LDO_CURRENT,
  BT_VDD_ASD_LDO_CURRENT,
  BT_VDD_XTAL_LDO_CURRENT,
  BT_VDD_PA_LDO_CURRENT,
  BT_VDD_CORE_LDO_CURRENT,
  BT_VDD_IO_LDO_CURRENT,
  BT_VDD_LDO_CURRENT,
  BT_VDD_RFA_0p8_CURRENT,
  BT_VDD_RFACMN_CURRENT,
  BT_VDD_IPA_2p2,
  BT_VDD_IPA_2p2_CURRENT,
  BT_VDD_SMPS,
  BT_VDD_SMPS_CURRENT,
  /* New entries need to be added before PWR_SRC_SIZE.
   * Its hold the max size of power sources states.
   */
  PWR_SRC_SIZE,
};

class PowerManager {
 private:
  /*Variables*/
  int rfkill_id_;
  BluetoothSocType bt_soc_type_;
  PowerManagerState pm_state_;
  bool ext_ldo_;
  char ts_init[MAX_TS_SIZE];
  char ts_sw_ctrl[MAX_TS_SIZE];

  /*Constants*/
  static const int power_ctrl_cmd_ = 0xbfad;
  static const int soc_version_ctrl_cmd_ = 0xbfae;
  static const int get_bt_sw_ctrl_gpio_cmd_ = 0xbfb0;
  static const int get_bt_pwr_srcs_state_cmd_ = 0xbfb1;
  static const int set_radio_config_in_tcs_cmd_ = 0xbfc0;

  const char *pwr_control_dev_;

  /*Methods*/
  bool PowerUpSmd(void);
  int GetRfkillFd(void);
  bool ControlRfkill(int rfkill_fd, bool enable);
  bool PowerUpChip(bool enable, bool retentionMode = false);
  int InitializeRfkill(void);
  bool SetExtLdoStatus(void);
  int Ioctl(int, int);
  void Cleanup(int);

 public:
  /*Methods*/
  void Init(BluetoothSocType soc_type);
  bool SetPower(bool enable, bool retentionMode = false);
  PowerManagerState GetState(void);
  void Cleanup(void);
  PowerManager(void);
  bool SendSocVersion(unsigned int soc_id);
  bool ExternalldoStatus();
  bool SendIoctlToConfigTCS();
  int GetAndLogPwrRsrcStates(char * power_state_buff);
  void CheckSwCtrl();
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace bluetooth
}  // namespace hardware
}  // namespace android
