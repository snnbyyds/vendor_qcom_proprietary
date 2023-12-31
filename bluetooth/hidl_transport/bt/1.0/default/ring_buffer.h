/*==========================================================================
Description
  It has implementation for ring buffer.

# Copyright (c) 2017 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#pragma once

#include <fcntl.h>
#include <errno.h>
#include <mutex>
#include "hci_internals.h"
#include <map>
#define BT_FILE_NAME_LEN 255
#define HCICMD_TIMESTAMP_LEN (64)
#define SYS_TIMESTAMP_LEN_MAX (32)
#define SNOOP_FILE_NAME_PREFIX    "ramdump_bt_snoop_"

/* Over come leap year ambiguity by adding the below to time stamp */
#define BTSNOOP_EPOCH_DELTA    0x00dcddb30f2f8000ULL

#define TIME_STR_SIZE     13
#define RX_TAG_STR_LEN    64
#define IBS_PKT_SIZE      0x05

namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

enum RxThreadEventType {
  RX_PRE_STACK_EVT_CALL_BACK = 0,
  RX_POST_STACK_EVT_CALL_BACK = 1,
  RX_PRE_STACK_ACL_CALL_BACK = 2,
  RX_POST_STACK_ACL_CALL_BACK = 3,
  RX_PRE_SELECT_CALL_BACK = 4,
  RX_POST_SELECT_CALL_BACK = 5
};

enum PrimaryReasonCode  {
  BT_HOST_REASON_DEFAULT_NONE  = 0x00,                         //INVALID REASON
  BT_HOST_REASON_SOC_CRASHED = 0x01,                           //SOC WAS CRASHED
  BT_HOST_REASON_SOC_CRASHED_DIAG_SSR = 0x02,                  //SOC CRASHED DIAG INITIATED SSR
  BT_HOST_REASON_INIT_FAILED = 0x03,                           //HOST INITIALIZATION FAILED
  BT_HOST_REASON_CLOSE_RCVD_DURING_INIT = 0x04,                //CLOSE RECEIVED FROM STACK DURING SOC INIT
  BT_HOST_REASON_ERROR_READING_DATA_FROM_UART = 0x05,          //ERROR READING DATA FROM UART
  BT_HOST_REASON_WRITE_FAIL_SPCL_BUFF_CRASH_SOC = 0x06,        //FAILED TO WRITE SPECIAL BYTES TO CRASH SOC
  BT_HOST_REASON_RX_THREAD_STUCK = 0x07,                       //RX THREAD STUCK
  BT_HOST_REASON_SSR_CMD_TIMEDOUT = 0x08,                      //SSR DUE TO CMD TIMED OUT
  BT_HOST_REASON_SSR_SPURIOUS_WAKEUP = 0x09,                   //SSR DUE TO SPURIOUS WAKE UP
  BT_HOST_REASON_SSR_INVALID_BYTES_RCVD = 0x0A,                //INVALID HCI CMD TYPE RECEIVED
  BT_HOST_REASON_SSR_RCVD_LARGE_PKT_FROM_SOC = 0x0B,           //SSR DUE TO LARGE PKT RECVIVED FROM SOC
  BT_HOST_REASON_SSR_UNABLE_TO_WAKEUP_SOC = 0x0C,              //UNABLE TO WAKE UP SOC
  BT_HOST_REASON_CMD_TIMEDOUT_SOC_WAIT_TIMEOUT = 0x0D,         //COMMAND TIMEOUT AND SOC CRASH WAIT TIMEOUT
  BT_HOST_REASON_SPURIOUS_WAKEUP_SOC_WAIT_TIMEOUT = 0x0E,      //SPURIOUS WAKE AND SOC CRASH WAIT TIMEOUT
  BT_HOST_REASON_INV_BYTES_SOC_WAIT_TIMEOUT = 0x0F,            //INVALID BYTES AND SOC CRASH WAIT TIMEOUT
  BT_HOST_REASON_SOC_WAKEUP_FAILED_SOC_WAIT_TIMEOUT = 0x10,    //SOC WAKEUP FAILURE AND SOC CRASH WAIT TIMEOUT
  BT_HOST_REASON_SOC_CRASHED_DIAG_SSR_SOC_WAIT_TIMEOUT = 0x11, //SOC CRASHED DIAG INITIATED SSR CRASH WAIT TIMEOUT
  BT_HOST_REASON_NONE_SOC_WAIT_TIMEOUT = 0x12,                 //INVALID FAILURE AND SOC CRASH WAIT TIMEOUT
  BT_HOST_REASON_SOC_DEINIT_STUCK = 0x13,                      //SOC DEINIT STUCK
  BT_HOST_REASON_SSR_INTERNAL_CMD_TIMEDOUT = 0x14,             //SSR DUE TO CMD INTERNAL TIMED OUT
  BT_HOST_REASON_FAILED_TO_SEND_INTERNAL_CMD = 0x15,           //FAILED TO SEND INTERNAL CMD
  BT_HOST_REASON_SSR_SLEEP_IND_NOT_RCVD = 0x16,                //SOC DID NOT RCVD SLEEP IND DURING CLOSE
  BT_HOST_REASON_DIAG_LOG_API_STUCK = 0x39,                    //DIAG log API stuck.
};

typedef struct {
  uint32_t length_original;
  uint32_t length_captured;
  uint32_t flags;
  uint32_t dropped_packets;
  uint64_t timestamp;
  uint8_t type;
} __attribute__((__packed__)) btsnoop_header_t;

class PacketBuff
{
 private:
  const static long int max_buff_size_ = 200 * 1024;

  unsigned int active_buff_ = 0;
  uint8_t buff_[2][max_buff_size_];
  long int buff_index_[2];
  void *logger_;
  std::map<int, char*> map_code_reason_;
  uint16_t snoop_hdr_len = sizeof(btsnoop_header_t);
  btsnoop_header_t header;
 public:
  PacketBuff(void);
  ~PacketBuff();
  void AddBuffNode(short int dir, HciPacketType, uint16_t, uint8_t *);
  void DumpData();
  void DumpStatInfo(int dst_fd);
  char *GetReason(PrimaryReasonCode reason);
};

class RingBuffer
{
 private:
  PacketBuff snoop_;
  std::mutex snoop_ring_buffer_mutex_;
  struct timeval time_pre_stack_event_call_back;
  struct timeval time_post_stack_event_call_back;
  struct timeval time_pre_stack_acl_call_back;
  struct timeval time_post_stack_acl_call_back;
  struct timeval time_pre_select_call_back;
  struct timeval time_post_select_call_back;
  void AddRxTagEvent(char* dest_tag_str, struct timeval& time_val);
  char last_hci_cmd[HCICMD_TIMESTAMP_LEN];
 public:
  enum {
    HOST_TO_SOC,
    SOC_TO_HOST,
    IBS_BYTE,
  } pkt_direction;

  RingBuffer(void) {};
  void AddLog(short int dir, HciPacketType, uint8_t *, uint16_t);
  void DumpData();
  void UpdateRxEventTag(RxThreadEventType rx_event);
  void SaveLastStackHciCommand(char*);
  void UnlockMutex();
  char *GetPrimaryReason(PrimaryReasonCode);
};

static inline uint64_t get_time_stamp(uint64_t ll);
static inline uint64_t time_gettimeofday_us(void);
void add_time_str(char *src, struct timeval* p_tval);
}  // namespace implementation
}  // namespace V1_0
}  // namespace bluetooth
}  // namespace hardware
}  // namespace android
