/******************************************************************************
#  Copyright (c) 2020-2022 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "DsiWrapper.h"
#include "legacy/qcril_data_utils.h"
#include <iomanip>
#include <sstream>

using namespace rildata;

int rilInstanceId = QCRIL_DEFAULT_INSTANCE_ID;
int subsId = QCRIL_DEFAULT_MODEM_STACK_ID;

/**
 * @brief [brief description]
 * @details [long description]
 *
 * @param dsiHandle [description]
 */
void dsiStopDataCall(dsi_hndl_t dsiHandle) {
  std::stringstream ss;
  ss << "[DsiWrapper] dsiStopDataCall hndl=" << (void *)dsiHandle;
  Log::getInstance().d(ss.str());
  dsi_stop_data_call(dsiHandle);
}

int dsiStartDataCall(dsi_hndl_t dsiHandle) {
  std::stringstream ss;
  ss << "[DsiWrapper] dsiStartDataCall hndl=" << (void *)dsiHandle;
  Log::getInstance().d(ss.str());
  return dsi_start_data_call(dsiHandle);
}

std::string convertToString(dsi_call_param_identifier_t key, dsi_call_param_value_t* value) {
  std::stringstream ss;
  switch (key) {
    case DSI_CALL_INFO_UMTS_PROFILE_IDX:
      ss << "UMTS_PROFILE_IDX";
      break;
    case DSI_CALL_INFO_APN_NAME:
      ss << "APN_NAME";
      break;
    case DSI_CALL_INFO_USERNAME:
      ss << "USERNAME";
      break;
    case DSI_CALL_INFO_PASSWORD:
      ss << "PASSWORD";
      break;
    case DSI_CALL_INFO_AUTH_PREF:
      ss << "AUTH_PREF";
      break;
    case DSI_CALL_INFO_CDMA_PROFILE_IDX:
      ss << "CDMA_PROFILE_IDX";
      break;
    case DSI_CALL_INFO_IP_ADDR:
      ss << "IP_ADDR";
      break;
    case DSI_CALL_INFO_DEVICE_NAME:
      ss << "DEVICE_NAME";
      break;
    case DSI_CALL_INFO_TECH_PREF:
      ss << "TECH_PREF";
      break;
    case DSI_CALL_INFO_CALL_TYPE:
      ss << "CALL_TYPE";
      break;
    case DSI_CALL_INFO_IP_VERSION:
      ss << "IP_VERSION";
      break;
    case DSI_CALL_INFO_EXT_TECH_PREF:
      ss << "EXT_TECH_PREF";
      break;
    case DSI_CALL_INFO_PARTIAL_RETRY:
      ss << "PARTIAL_RETRY";
      break;
    case DSI_CALL_INFO_APP_TYPE:
      ss << "APP_TYPE";
      break;
    case DSI_CALL_INFO_SUBS_ID:
      ss << "SUBS_ID";
      break;
    default:
      ss << std::to_string((int)key);
      break;
  }
  if (value != nullptr) {
    ss << " " << value->num_val;
    if (value->buf_val != nullptr) {
      ss << std::hex << std::setw(2) << std::setfill('0');
      for (int i = 0; i < value->num_val; i++) {
        ss << " 0x" << (int)value->buf_val[i];
      }
    }
  }
  return ss.str();
}

int dsiSetDataCallParam(
  dsi_hndl_t hndl,
  dsi_call_param_identifier_t identifier,
  dsi_call_param_value_t *info
) {
  Log::getInstance().d("[DsiWrapper] dsiSetDataCallParam " + convertToString(identifier, info));
  return dsi_set_data_call_param(hndl, identifier, info);
}

dsi_hndl_t dsiGetDataSrvcHndl(
  dsi_net_ev_cb cb_fn,
  void * user_data
) {
  dsi_hndl_t dsiHandle = dsi_get_data_srvc_hndl(cb_fn, user_data);
  std::stringstream ss;
  ss << "[DsiWrapper] dsiGetDataSrvcHndl " << (void *)dsiHandle;
  Log::getInstance().d(ss.str());
  return dsiHandle;
}

void dsiRelDataSrvcHndl(dsi_hndl_t hndl) {
  std::stringstream ss;
  ss << "[DsiWrapper] dsiRelDataSrvcHndl " << (void *)hndl;
  Log::getInstance().d(ss.str());
  dsi_rel_data_srvc_hndl(hndl);
}

/**
 * @brief [brief description]
 * @details [long description]
 *
 * @param DsiHandle [description]
 * @param deviceName [description]
 *
 * @return [description]
 */
int dsiGetDeviceName(dsi_hndl_t dsiHandle, std::string &deviceName) {
  Log::getInstance().d("[DsiWrapper] dsiGetDeviceName");

  int rc;
  char deviceNameBuf[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1];
  memset(deviceNameBuf, 0, sizeof(deviceNameBuf));

  rc = dsi_get_device_name(dsiHandle, deviceNameBuf, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1);
  if (rc!=0) {
    Log::getInstance().d("[DsiWrapper]: couldn't get device name");
  }
  deviceName = std::string(deviceNameBuf);

  Log::getInstance().d("[DsiWrapper]: device name = " + deviceName);

  return 0;
}

unsigned int dsiGetIPAddrCount(dsi_hndl_t dsiHandle) {
  unsigned int count = dsi_get_ip_addr_count(dsiHandle);
  Log::getInstance().d("[DsiWrapper]: dsiGetIPAddrCount = " + std::to_string(count));
  return count;
}

/**
 * @brief [brief description]
 * @details [long description]
 *
 * @param DsiHandle [description]
 * @param ipAddresses [description]
 * @param gatewayAddresses [description]
 * @param dnsAddresses [description]
 * @return [description]
 */
unsigned int dsiGetAddresses(dsi_hndl_t dsiHandle, bool &v4Connected, bool &v6Connected,
  std::string &ipAddresses, std::string &gatewayAddresses, std::string &dnsAddresses, const IpFamilyType_t &ipType) {
  Log::getInstance().d("[DsiWrapper] dsiGetAddresses");

  int rc;
  char IpAddr[DS_CALL_INFO_ADDR_IPV4V6_MAX_LEN+1];
  char GwAddr[DS_CALL_INFO_ADDR_IPV4V6_MAX_LEN+1];
  char DnsAddr[DS_CALL_INFO_ADDR_IPV4V6_MAX_LEN*2+1];
  char tempStr[DS_CALL_INFO_ADDR_IPV4V6_MAX_LEN+1];

  memset(IpAddr, 0, sizeof(IpAddr));
  memset(GwAddr, 0, sizeof(GwAddr));
  memset(DnsAddr, 0, sizeof(DnsAddr));
  memset(tempStr, 0, sizeof(tempStr));

  unsigned int count = dsi_get_ip_addr_count(dsiHandle);
  Log::getInstance().d("[DsiWrapper]: dsi_get_ip_addr_count = " + std::to_string(count));

  if (count == 0) {
    return 0;
  }

  dsi_addr_info_t *addr_info = nullptr;
  addr_info = (dsi_addr_info_t *)calloc(count, sizeof(dsi_addr_info_t));
  if (addr_info != nullptr) {
    rc = dsi_get_ip_addr(dsiHandle, addr_info, count);
    if (rc!=0) {
      Log::getInstance().d("[DsiWrapper]: couldn't get address info");
    }


    for (unsigned int i=0 ; i<count ; i++) {
      memset(tempStr, 0, sizeof(tempStr));
      if (addr_info[i].iface_addr_s.valid_addr) {
        if ((addr_info[i].iface_addr_s.addr.ss_family == AF_INET) && ((int32_t)ipType & ( (int32_t)IpFamilyType_t::IPv4v6 | (int32_t)IpFamilyType_t::IPv4 ))) {
          AddressFormatIPv4WithSubnetMask(SASTORAGE_DATA(addr_info[i].iface_addr_s.addr),
            addr_info[i].iface_mask, tempStr, sizeof(tempStr));
          v4Connected = true;
        }
        else if ((addr_info[i].iface_addr_s.addr.ss_family == AF_INET6) && ((int32_t)ipType & ((int32_t)IpFamilyType_t::IPv4v6 | (int32_t)IpFamilyType_t::IPv6 ))) {
          AddressFormatIPv6WithPrefixLength(SASTORAGE_DATA(addr_info[i].iface_addr_s.addr),
            addr_info[i].iface_mask, tempStr, sizeof(tempStr));
          v6Connected = true;
        }
        else {
          Log::getInstance().d("[DsiWrapper]: Invalid IP address family");
        }
        if(i>0) {strlcat(IpAddr, " ", sizeof(IpAddr));}
        strlcat(IpAddr, tempStr, sizeof(IpAddr));
      }

      memset(tempStr, 0, sizeof(tempStr));
      if (addr_info[i].gtwy_addr_s.valid_addr) {
        if ((addr_info[i].gtwy_addr_s.addr.ss_family == AF_INET) && ((int32_t)ipType & ((int32_t)IpFamilyType_t::IPv4v6 | (int32_t)IpFamilyType_t::IPv4 ))) {
          AddressFormatIPv4(SASTORAGE_DATA(addr_info[i].gtwy_addr_s.addr), tempStr, sizeof(tempStr));
        }
        else if (addr_info[i].gtwy_addr_s.addr.ss_family == AF_INET6 && ((int32_t)ipType & ((int32_t)IpFamilyType_t::IPv4v6 | (int32_t)IpFamilyType_t::IPv6 ))) {
          AddressFormatIPv6(SASTORAGE_DATA(addr_info[i].gtwy_addr_s.addr), tempStr, sizeof(tempStr));
        }
        else {
          Log::getInstance().d("[DsiWrapper]: Invalid gateway address family");
        }
        if(i>0 && strlen(tempStr)) {strlcat(GwAddr, " ", sizeof(GwAddr));}
        strlcat(GwAddr, tempStr, sizeof(GwAddr));
      }

      memset(tempStr, 0, sizeof(tempStr));
      if (addr_info[i].dnsp_addr_s.valid_addr) {
        if (addr_info[i].dnsp_addr_s.addr.ss_family == AF_INET && ((int32_t)ipType & ( (int32_t)IpFamilyType_t::IPv4v6 | (int32_t)IpFamilyType_t::IPv4 ))) {
          AddressFormatIPv4(SASTORAGE_DATA(addr_info[i].dnsp_addr_s.addr), tempStr, sizeof(tempStr));
        }
        else if (addr_info[i].dnsp_addr_s.addr.ss_family == AF_INET6 && ((int32_t)ipType & ( (int32_t)IpFamilyType_t::IPv4v6 | (int32_t)IpFamilyType_t::IPv6 ))) {
          AddressFormatIPv6(SASTORAGE_DATA(addr_info[i].dnsp_addr_s.addr), tempStr, sizeof(tempStr));
        }
        else {
          Log::getInstance().d("[DsiWrapper]: Invalid dnsp address family");
        }
        if(i>0 && strlen(tempStr)) {strlcat(DnsAddr, " ", sizeof(DnsAddr));}
        strlcat(DnsAddr, tempStr, sizeof(DnsAddr));

        if (addr_info[i].dnss_addr_s.valid_addr) {
          memset(tempStr, 0, sizeof(tempStr));
          if (addr_info[i].dnss_addr_s.addr.ss_family == AF_INET && ((int32_t)ipType & ( (int32_t)IpFamilyType_t::IPv4v6 | (int32_t)IpFamilyType_t::IPv4 ))) {
            AddressFormatIPv4(SASTORAGE_DATA(addr_info[i].dnss_addr_s.addr), tempStr, sizeof(tempStr));
          }
          else if (addr_info[i].dnss_addr_s.addr.ss_family == AF_INET6 && ((int32_t)ipType & ( (int32_t)IpFamilyType_t::IPv4v6 | (int32_t)IpFamilyType_t::IPv6 ))) {
            AddressFormatIPv6(SASTORAGE_DATA(addr_info[i].dnss_addr_s.addr), tempStr, sizeof(tempStr));
          }
          else {
            Log::getInstance().d("[DsiWrapper]: Invalid dnss address family");
          }
          if(strlen(tempStr)) {strlcat(DnsAddr, " ", sizeof(DnsAddr));}
          strlcat(DnsAddr, tempStr, sizeof(DnsAddr));
        }
      }
    }
  }
  else {
    Log::getInstance().d("[DsiWrapper]: Memory calloc failure");
  }

  ipAddresses = std::string(IpAddr);
  gatewayAddresses = std::string(GwAddr);
  dnsAddresses = std::string(DnsAddr);

  Log::getInstance().d("[DsiWrapper]: ip address = " + ipAddresses);
  Log::getInstance().d("[DsiWrapper]: gateway address = " + gatewayAddresses);
  Log::getInstance().d("[DsiWrapper]: dns address = " + dnsAddresses);

  if (addr_info != nullptr) {
    free(addr_info);
  }

  return count;
}


/**
 * @brief [brief description]
 * @details [long description]
 *
 * @param DsiHandle [description]
 * @return [description]
 */
unsigned int dsiGetMtu(dsi_hndl_t dsiHandle) {
  Log::getInstance().d("[DsiWrapper] dsiGetMtu");

  int rc;
  unsigned int mtu;
  if (qcril_data_mtu > 0) {
    mtu = qcril_data_mtu;
  }
  else {
    rc = dsi_get_link_mtu(dsiHandle, &mtu);
    if (rc!=0) {
      Log::getInstance().d("[DsiWrapper]: couldn't get mtu");
    }
  }

  Log::getInstance().d("[DsiWrapper]: mtu = " + std::to_string((int)mtu));
  return mtu;
}

/**
 * @brief [brief description]
 * @details [long description]
 *
 * @param DsiHandle [description]
 * @return [description]
 */
void dsiGetMtuByFamily(dsi_hndl_t dsiHandle, dsi_v4_v6_mtu_t &v4_v6_mtu) {
  Log::getInstance().d("[DsiWrapper] dsiGetMtu");

  int rc;
  if (qcril_data_mtu > 0) {
    v4_v6_mtu.v4_mtu = qcril_data_mtu;
    v4_v6_mtu.v6_mtu = qcril_data_mtu;
  }
  else {
    v4_v6_mtu.v4_mtu = 0;
    v4_v6_mtu.v6_mtu = 0;
  }

  Log::getInstance().d("[DsiWrapper]: v4 mtu = " + std::to_string((int)v4_v6_mtu.v4_mtu) + ", v6 mtu = " + std::to_string((int)v4_v6_mtu.v6_mtu));
  return;
}

/**
 * @brief [brief description]
 * @details [long description]
 *
 * @param DsiHandle [description]
 * @return [description]
 */
int dsiGetPcscfAddresses(dsi_hndl_t dsiHandle, std::string &pcscfAddr, const IpFamilyType_t &ipType) {
  Log::getInstance().d("[DsiWrapper] dsiGetPcscfAddresses");

  int rc;
  unsigned int k = 0;
  int err_code = DSI_ERROR;
  struct sockaddr_in *sin = NULL;
  struct sockaddr_in6 *sin6 = NULL;
  char addr_buf[50];
  const char *addr_ptr = NULL;
  char fmtstr[DS_CALL_INFO_ADDR_PCSCF_MAX_LEN];
  size_t fmtstr_size = sizeof(fmtstr);

  memset(fmtstr, 0, fmtstr_size);
  dsi_pcscf_addr_info_t *pcscf_addr_list = (dsi_pcscf_addr_info_t *)calloc(1, sizeof(dsi_pcscf_addr_info_t));
  if(pcscf_addr_list != nullptr) {
    rc = dsi_iface_ioctl(dsiHandle,
                         DSI_IFACE_IOCTL_GET_PCSCF_SERV_ADDRESS,
                         (void*) pcscf_addr_list,
                         &err_code);
    for (unsigned int i = 0; i < pcscf_addr_list->addr_count; i++) {
      if (pcscf_addr_list->pcscf_address[i].valid_addr != true ||
            (pcscf_addr_list->pcscf_address[i].addr.ss_family != AF_INET &&
            pcscf_addr_list->pcscf_address[i].addr.ss_family != AF_INET6))
      {
        Log::getInstance().d("Invalid pcscf address");
        continue;
      }
      if ((pcscf_addr_list->pcscf_address[i].addr.ss_family == AF_INET) && ((int32_t)ipType & ( (int32_t)IpFamilyType_t::IPv4v6 | (int32_t)IpFamilyType_t::IPv4 )))
      {
        sin = (struct sockaddr_in *)&pcscf_addr_list->pcscf_address[i].addr;
        addr_ptr = inet_ntop(AF_INET, &sin->sin_addr, addr_buf, sizeof(addr_buf));
      }
      else if ( (int32_t)ipType & ( (int32_t)IpFamilyType_t::IPv4v6 | (int32_t)IpFamilyType_t::IPv6 ))
      {
        sin6 = (struct sockaddr_in6 *)&pcscf_addr_list->pcscf_address[i].addr;
        addr_ptr = inet_ntop(AF_INET6, &sin6->sin6_addr, addr_buf, sizeof(addr_buf));
      }

      if (NULL != addr_ptr)
      {
        if ((fmtstr_size - strlen(fmtstr)) <= (strlen(addr_ptr) + 1))
        {
          break;
        }
        if (k++ > 0)
        {
          strlcat(fmtstr, " ", fmtstr_size);
        }
        strlcat(fmtstr, addr_ptr, fmtstr_size);
      }
    }
  }
  else {
    Log::getInstance().d("[DsiWrapper]: Memory calloc failure");
  }

  pcscfAddr = std::string(fmtstr);
  Log::getInstance().d("[DsiWrapper]: pcscf address = " + pcscfAddr);

  if (pcscf_addr_list != nullptr) {
    free(pcscf_addr_list);
  }

  return 0;
}

dsi_ce_reason_t dsiGetVerboseCallEndReason(dsi_hndl_t DsiHandle, std::string ipType) {
  dsi_ip_family_t ipf = DSI_IP_FAMILY_V4;
  if (ipType == "IPV6") {
    ipf = DSI_IP_FAMILY_V6;
  }
  dsi_ce_reason_t dsiEndReason;
  dsi_get_call_end_reason(DsiHandle, &dsiEndReason, ipf);
  Log::getInstance().d("[DsiWrapper] dsiGetVerboseCallEndReason type=" + std::to_string(dsiEndReason.reason_type) +
                       " code=" + std::to_string(dsiEndReason.reason_code));
  return dsiEndReason;
}

/**
 * @brief Routine to obtain the call status of a Dual-IP call. This function must
 *        only be called for a Dual-IP call
 *
 * @param DsiHandle
 * @return Success if end reason is valid
 */
bool dsiGetDualIpCallEndReason(dsi_hndl_t DsiHandle, dsi_ce_reason_t &v4Reason, dsi_ce_reason_t &v6Reason) {
  Log::getInstance().d("[DsiWrapper] dsiGetDualIpCallEndReason");

  if (DSI_SUCCESS != dsi_get_call_end_reason(DsiHandle, &v4Reason, DSI_IP_FAMILY_V4)) {
    Log::getInstance().d("[DsiWrapper] failed to get dsi end reason V4");
    return false;
  }
  if (DSI_SUCCESS != dsi_get_call_end_reason(DsiHandle, &v6Reason, DSI_IP_FAMILY_V6)) {
    Log::getInstance().d("[DsiWrapper] failed to get dsi end reason V6");
    return false;
  }
  if (DSI_CE_TYPE_UNINIT == v4Reason.reason_type ||
      DSI_CE_TYPE_UNINIT == v6Reason.reason_type) {
    Log::getInstance().d("[DsiWrapper] Unitialized end reason v4=" + std::to_string(v4Reason.reason_type) +
                         " v6=" + std::to_string(v6Reason.reason_type));
    return false;
  }

  Log::getInstance().d("[DsiWrapper] v4type=" + std::to_string(v4Reason.reason_type) +
                       " v4code=" + std::to_string(v4Reason.reason_code) + " v6type=" +
                       std::to_string(v6Reason.reason_type) + " v6code=" + std::to_string(v6Reason.reason_code));
  return true;
}

/**
 * @brief [brief description]
 * @details [long description]
 *
 * @param DsiHandle [description]
 * @return [description]
 */
int dsiGetCallEndReason(dsi_hndl_t DsiHandle) {
  Log::getInstance().d("[DsiWrapper] dsiGetCallEndReason");

  int endReason = PDP_FAIL_NONE;
  dsi_ip_family_t ipf = DSI_IP_FAMILY_V4;
  dsi_ce_reason_t dsiEndReason;

  if (dsi_get_call_end_reason(DsiHandle, &dsiEndReason, ipf) == DSI_SUCCESS) {
    if (0!=qcril_data_get_ril_ce_code(&dsiEndReason, &endReason)) {
      endReason = PDP_FAIL_OEM_DCFAILCAUSE_1;
    }
  }

  Log::getInstance().d("[DsiWrapper] endReason = "+std::to_string(endReason));
  return endReason;
}

dsi_call_tech_type dsiGetCallTech(dsi_hndl_t DsiHandle) {
  Log::getInstance().d("[DsiWrapper] dsiGetCallTech");

  dsi_call_tech_type callTech = DSI_EXT_TECH_INVALID;
  dsi_get_call_tech(DsiHandle, &callTech);
  return callTech;
}

int dsiTmgiActivate(
  dsi_hndl_t                          hndl,
  char                               *tmgi,
  unsigned int                       *earfcnlist,
  unsigned char                       earfcnlist_len,
  unsigned long                       preemption_priority,
  unsigned int                       *sailist,
  unsigned char                       sailist_len,
  int                                 dbg_trace_id
)
{
  Log::getInstance().d("[DsiWrapper] dsiTmgiActivate");
  return dsi_embms_tmgi_activate(hndl, tmgi, earfcnlist, earfcnlist_len, preemption_priority, sailist, sailist_len, dbg_trace_id);
}

int dsiGetTmgiDeactivateReason (
  dsi_hndl_t                               hndl,
  dsi_embms_tmgi_status_field_type         status_field,
  int                                     *dsi_tmgi_deactivate_reason
)
{
  Log::getInstance().d("[DsiWrapper] dsiGetTmgiDeactivateReason");
  return dsi_embms_get_tmgi_deactivate_reason(hndl, status_field, dsi_tmgi_deactivate_reason);
}

int dsiTmgiDeactivate (
  dsi_hndl_t hndl,
  char      *tmgi,
  int       dbg_trace_id
)
{
  Log::getInstance().d("[DsiWrapper] dsiTmgiDeactivate");
  return dsi_embms_tmgi_deactivate(hndl, tmgi, dbg_trace_id);
}

int dsiTmgiActivateDeactivate (
  dsi_hndl_t                         hndl,
  char                              *activate_tmgi,
  unsigned int                      *earfcnlist,
  unsigned char                      earfcnlist_len,
  unsigned long                      preemption_priority,
  char                              *deactivate_tmgi,
  unsigned int                      *sailist,
  unsigned char                      sailist_len,
  int                                dbg_trace_id
)
{
  Log::getInstance().d("[DsiWrapper] dsiTmgiActivateDeactivate");
  return dsi_embms_tmgi_activate_deactivate(hndl, activate_tmgi, earfcnlist, earfcnlist_len, preemption_priority, deactivate_tmgi, sailist, sailist_len, dbg_trace_id);
}

int dsiContentDescUpdate (
  dsi_hndl_t                    hndl,
  char                         *content_desc_update_tmgi,
  unsigned char                 content_desc_valid,
  unsigned int                  content_desc_len,
  dsi_embms_content_desc_type  *content_desc,
  int                           dbg_trace_id
)
{
  Log::getInstance().d("[]DsiWrapper] dsiContentDescUpdate");
  return dsi_embms_content_desc_update(hndl, content_desc_update_tmgi, content_desc_valid, content_desc_len, content_desc, dbg_trace_id);
}

int dsiGetEmbmsTmgiListquery (
 dsi_hndl_t                               hndl,
 wds_embms_tmgi_list_type_enum_v01        embms_tmgi_list_query,
 wds_embms_tmgi_list_query_resp_msg_v01  *list_info,
 int                                      dbg_trace_id
)
{
  Log::getInstance().d("[DsiWrapper] dsiGetEmbmsTmgiListquery");
  return dsi_embms_tmgi_list_query(hndl, embms_tmgi_list_query, list_info, dbg_trace_id);
}

int dsiEmbmsSvcInterestList (
  dsi_hndl_t          hndl,
  char              **svc_interest_tmgi_list,
  unsigned int        svc_interest_tmgi_list_len,
  int                 dbg_trace_id
)
{
  Log::getInstance().d("[DsiWrapper] dsiEmbmsSvcInterestList");
  return dsi_embms_svc_interest_list(hndl, svc_interest_tmgi_list, svc_interest_tmgi_list_len, dbg_trace_id);
}

/**
 * @brief Routine to trigger DSI Initialization
 *
 * @param DsiHandle
 * @return DSI_SUCCESS if successfully trigerred DSI initialization
 * @return DSI_ERROR if DSI Release fails
 */
int dsiInit(bool fromSSR, void (* dsi_init_cb_func)( void * ))
{
  int result = DSI_ERROR;
  qcril_modem_stack_id_e_type stackId = QCRIL_INVALID_MODEM_STACK_ID;

  rilInstanceId = qmi_ril_get_process_instance_id();
  stackId = qmi_ril_get_stack_id(static_cast<qcril_instance_id_e_type>(rilInstanceId));
  switch(stackId)
  {
    case QCRIL_MODEM_PRIMARY_STACK_ID:
      subsId = WDS_PRIMARY_SUBS_V01;
      break;

    case QCRIL_MODEM_SECONDARY_STACK_ID:
      subsId = WDS_SECONDARY_SUBS_V01;
      break;

    case QCRIL_MODEM_TERTIARY_STACK_ID:
      subsId = WDS_TERTIARY_SUBS_V01;
      break;

    default:
      subsId = WDS_DEFAULT_SUBS_V01;
  }
  Log::getInstance().d("[DsiWrapper]:: stackId ="+std::to_string(stackId)+" subs_id ="+ std::to_string(subsId));
  dsi_set_modem_subs_id(subsId);

  if(fromSSR)
  {
    result = dsi_init_ex( DSI_MODE_SSR, dsi_init_cb_func, NULL );
    if(DSI_SUCCESS != result)
      Log::getInstance().d("dsi_init_ex trigger failed "+ std::to_string(result));
    else
      Log::getInstance().d("dsi_init_ex triggered Successfully"+ std::to_string(result));
  }
  else
  {
    result = dsi_init_ex( DSI_MODE_GENERAL, dsi_init_cb_func, NULL );
    if(DSI_SUCCESS != result)
      Log::getInstance().d("dsi_init_ex trigger failed"+ std::to_string(result));
    else
      Log::getInstance().d("dsi_init_ex triggered Successfully"+std::to_string(result));
  }
  return result;
}

/**
 * @brief Routine to trigger DSI release
 *
 * @param DsiHandle
 * @return DSI_SUCCESS if successfully DSI Release
 * @return DSI_ERROR if DSI Release fails
 */
int dsiRelease()
{
  int result = DSI_ERROR;

  result = dsi_release(DSI_MODE_GENERAL);

  if(DSI_SUCCESS != result)
    Log::getInstance().d("dsi_release failed");
  else
    Log::getInstance().d("dsi_release Successful");
  return result;
}
