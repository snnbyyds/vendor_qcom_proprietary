/*
* Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "PmcRegistersAccessor.h"
#include "DeviceManager.h"
#include "DebugLogger.h"
#include "Device.h"

#include <sstream>

const std::map<std::string, std::map<BasebandRevision, PmcRegisterInfo>> PmcRegistersAccessor::s_pmcRegisterMap
{
     {"MAC_RGF.MAC_SXD.TIMING_INDIRECT.TIMING_INDIRECT_REG_5.msrb_capture_ts_low", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x886eb8 ,0 , 31}}}},
     {"MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_0", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x88607c ,0 , 31}}}},
     {"MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_0.delimiter", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x88607c ,0 , 15}}}},
     {"MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_0.rec_en_set",{{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x88607c ,16 , 16}}}},
     {"MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_0.rec_en_clr", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x88607c ,17 , 17}}}},
     {"MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_0.rec_active", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x88607c ,18 , 18}}}},
     {"MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_0.rx_en", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x88607c ,24 , 24}}}},
     {"MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_0.tx_en", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x88607c ,25 , 25}}}},
     {"MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_0.fw_udef_en", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x88607c ,26 , 26}}}},
     {"MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_0.ucpu_udef_en", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x88607c ,27, 27}}}},
     {"MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_0.ucode_event_en", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x88607c ,28 , 28}}}},
     {"MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_0.idle_sm_en", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x88607c ,29 , 29}}}},
     {"MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_1", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x886080 ,0, 31}}}},
     {"MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_1.intf_type", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x886080 ,0, 0}}}},
     {"MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_1.dma_if_en", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x886080 ,2, 2}}}},
     {"MAC_RGF.PMC.GENERAL.MAC_MAC_RGF_PMC_GENERAL_1.pkt_treshhold", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x886080 ,16, 27}}}},
     {"MAC_RGF.PMC.IDLE_SM.MAC_MAC_RGF_PMC_IDLE_SM_0.slot_en", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8860d0 ,0, 0}}}},
     {"MAC_RGF.PMC.RX_TX.MAC_MAC_RGF_PMC_RX_TX_0", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x88608c ,0, 31}}}},
     {"MAC_RGF.PMC.RX_TX.RX.MAC_MAC_RGF_PMC_RX_0.qid_mask", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x886094 ,0, 31}}}},
     {"MAC_RGF.PMC.RX_TX.TX.MAC_MAC_RGF_PMC_RX_TX_TX_0.qid_mask", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8860ac ,0, 31}}}},
     {"MAC_RGF.PRS.CTRL.MAC_PRS_CTRL_0.pmc_post_dec_mode", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x886200 ,16, 16}}}},
     {"MAC_RGF.PMC.RX_TX.RX.MAC_MAC_RGF_PMC_RX_1.qos_data_en", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x886098 ,0, 0}}}},
     {"MAC_RGF.PMC.RX_TX.RX.MAC_MAC_RGF_PMC_RX_1.management_en", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x886098 ,1, 1}}}},
     {"MAC_RGF.PMC.RX_TX.RX.MAC_MAC_RGF_PMC_RX_1.control_en", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x886098 ,2, 2}}}},
     {"MAC_RGF.PMC.RX_TX.RX.MAC_MAC_RGF_PMC_RX_1.beacon_en", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x886098 ,3, 3}}}},
     {"MAC_RGF.PMC.RX_TX.RX.MAC_MAC_RGF_PMC_RX_1.error_en", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x886098 ,4, 4}}}},
     {"MAC_RGF.PMC.RX_TX.RX.MAC_MAC_RGF_PMC_RX_1.phy_info_en", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x886098 ,5, 5}}}},
     {"MAC_RGF.PMC.RX_TX.RX.MAC_MAC_RGF_PMC_RX_2.qos_data_max_data", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x88609c ,0, 7}}}},
     {"MAC_RGF.PMC.RX_TX.RX.MAC_MAC_RGF_PMC_RX_2.management_max_data", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x88609c ,8, 15}}}},
     {"MAC_RGF.PMC.RX_TX.RX.MAC_MAC_RGF_PMC_RX_2.control_max_data", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x88609c ,16, 23}}}},
     {"MAC_RGF.PMC.RX_TX.RX.MAC_MAC_RGF_PMC_RX_2.beacon_max_data", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x88609c ,24, 31}}}},
     {"MAC_RGF.PMC.RX_TX.RX.MAC_MAC_RGF_PMC_RX_3.error_max_data", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8860a0 ,0, 7}}}},
     {"MAC_RGF.PMC.RX_TX.RX.MAC_MAC_RGF_PMC_RX_3.phy_info_max_data", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8860a0 ,8, 15}}}},
     {"MAC_RGF.PMC.RX_TX.TX.MAC_MAC_RGF_PMC_RX_TX_TX_1.qos_data_en", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8860b0 ,0, 0}}}},
     {"MAC_RGF.PMC.RX_TX.TX.MAC_MAC_RGF_PMC_RX_TX_TX_1.management_en", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8860b0 ,1, 1}}}},
     {"MAC_RGF.PMC.RX_TX.TX.MAC_MAC_RGF_PMC_RX_TX_TX_1.control_en", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8860b0 ,2, 2}}}},
     {"MAC_RGF.PMC.RX_TX.TX.MAC_MAC_RGF_PMC_RX_TX_TX_1.beacon_en", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8860b0 ,3, 3}}}},
     {"MAC_RGF.PMC.RX_TX.TX.MAC_MAC_RGF_PMC_RX_TX_TX_1.direct_pkt_en", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8860b0 ,4, 4}}}},
     {"MAC_RGF.PMC.RX_TX.TX.MAC_MAC_RGF_PMC_RX_TX_TX_2.qos_data_max_data", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8860b4 ,0, 7}}}},
     {"MAC_RGF.PMC.RX_TX.TX.MAC_MAC_RGF_PMC_RX_TX_TX_2.management_max_data", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8860b4 ,8, 15}}}},
     {"MAC_RGF.PMC.RX_TX.TX.MAC_MAC_RGF_PMC_RX_TX_TX_2.control_max_data", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8860b4 ,16, 23}}}},
     {"MAC_RGF.PMC.RX_TX.TX.MAC_MAC_RGF_PMC_RX_TX_TX_2.beacon_max_data", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8860b4 ,24, 31}}}},
     {"MAC_RGF.PMC.RX_TX.TX.MAC_MAC_RGF_PMC_RX_TX_TX_3.direct_pkt_max_data", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8860b8 ,0, 7}}}},
     {"MAC_RGF.MAC_SXD.LOCAL_REGISTER_IF.LOCAL_WR_REG.sxd_local_wr_data", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x886dc8 ,0, 31}}}},
     {"USER.CLK_CTL_EXTENTION.USER_USER_CLKS_CTL_EXT_SW_BYPASS_HW_CG_0.mac_pmc_clk_sw_bypass_hw", {{BasebandRevision::SPR_B0, PmcRegisterInfo{0x880c20 ,9, 9}}, {BasebandRevision::SPR_D0, PmcRegisterInfo{0x880c20 ,9, 9}}}},
     {"USER.CLKS_CTL.SW.RST.USER_USER_CLKS_CTL_SW_RST_VEC_0", { {BasebandRevision::TLN_M_B0, PmcRegisterInfo{0x880b04 ,0, 31}}, {BasebandRevision::TLN_M_C0, PmcRegisterInfo{ 0x880b04 ,0, 31}} }},
     {"USER.EXTENTION.USER_USER_EXTENTION_3.mac_pmc_clk_sw_bypass_hw", { {BasebandRevision::TLN_M_B0, PmcRegisterInfo{0x880C20 ,9, 9}}, {BasebandRevision::TLN_M_C0, PmcRegisterInfo{ 0x880C20 ,9, 9}} }},
     {"USER.EXTENTION.USER_USER_EXTENTION_3.mac_prp_ahb_rgf_hclk_sw_bypass_hw", {{BasebandRevision::TLN_M_B0, PmcRegisterInfo{0x880C20 ,16, 16}}, {BasebandRevision::TLN_M_C0, PmcRegisterInfo{0x880C20 ,16, 16}} }},
     {"MAC_RGF.PRP.TOP.MAC_MAC_RGF_PRP_TOP_QID_CTRL_3.pmc_pkt_qring", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x00886838 ,8, 11}}}},
     {"DMA_RGF.DESCQ.<0>.SW_BASE.BASE_L", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881328 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<1>.SW_BASE.BASE_L", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881398 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<2>.SW_BASE.BASE_L", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881408 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<3>.SW_BASE.BASE_L", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881478 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<4>.SW_BASE.BASE_L", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8814E8 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<5>.SW_BASE.BASE_L", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881558 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<6>.SW_BASE.BASE_L", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8815C8 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<7>.SW_BASE.BASE_L", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881638 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<8>.SW_BASE.BASE_L", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8816A8 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<9>.SW_BASE.BASE_L", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881718 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<10>.SW_BASE.BASE_L", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881788 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<11>.SW_BASE.BASE_L", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8817F8 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<0>.SW_SIZE", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8812E4 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<1>.SW_SIZE", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881354 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<2>.SW_SIZE", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8813C4 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<3>.SW_SIZE", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881434 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<4>.SW_SIZE", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8814A4 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<5>.SW_SIZE", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881514 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<6>.SW_SIZE", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881584 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<7>.SW_SIZE", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8815F4 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<8>.SW_SIZE", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881664 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<9>.SW_SIZE", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8816D4 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<10>.SW_SIZE", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881744 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<11>.SW_SIZE", {{ BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8817B4 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<0>.RX_MAX_SIZE.MAX_RX_PL_PER_DESCRIPTOR.val", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881344 ,0, 15}}}},
     {"DMA_RGF.DESCQ.<1>.RX_MAX_SIZE.MAX_RX_PL_PER_DESCRIPTOR.val", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8813b4 ,0, 15}}}},
     {"DMA_RGF.DESCQ.<2>.RX_MAX_SIZE.MAX_RX_PL_PER_DESCRIPTOR.val", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881424 ,0, 15}}}},
     {"DMA_RGF.DESCQ.<3>.RX_MAX_SIZE.MAX_RX_PL_PER_DESCRIPTOR.val", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881494 ,0, 15}}}},
     {"DMA_RGF.DESCQ.<4>.RX_MAX_SIZE.MAX_RX_PL_PER_DESCRIPTOR.val", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881504 ,0, 15}}}},
     {"DMA_RGF.DESCQ.<5>.RX_MAX_SIZE.MAX_RX_PL_PER_DESCRIPTOR.val", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881574 ,0, 15}}}},
     {"DMA_RGF.DESCQ.<6>.RX_MAX_SIZE.MAX_RX_PL_PER_DESCRIPTOR.val", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8815e4 ,0, 15}}}},
     {"DMA_RGF.DESCQ.<7>.RX_MAX_SIZE.MAX_RX_PL_PER_DESCRIPTOR.val", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881654 ,0, 15}}}},
     {"DMA_RGF.DESCQ.<8>.RX_MAX_SIZE.MAX_RX_PL_PER_DESCRIPTOR.val", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8816c4 ,0, 15}}}},
     {"DMA_RGF.DESCQ.<9>.RX_MAX_SIZE.MAX_RX_PL_PER_DESCRIPTOR.val", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881734 ,0, 15}}}},
     {"DMA_RGF.DESCQ.<10>.RX_MAX_SIZE.MAX_RX_PL_PER_DESCRIPTOR.val", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8817a4 ,0, 15}}}},
     {"DMA_RGF.DESCQ.<11>.RX_MAX_SIZE.MAX_RX_PL_PER_DESCRIPTOR.val", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881814 ,0, 15}}}},
     {"DMA_RGF.DESCQ.<0>.SW_HEAD.CRNT", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881310 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<1>.SW_HEAD.CRNT", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881380 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<2>.SW_HEAD.CRNT", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8813f0 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<3>.SW_HEAD.CRNT", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881460 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<4>.SW_HEAD.CRNT", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8814d0 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<5>.SW_HEAD.CRNT", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881540 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<6>.SW_HEAD.CRNT", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8815b0 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<7>.SW_HEAD.CRNT", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881620 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<8>.SW_HEAD.CRNT", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881690 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<9>.SW_HEAD.CRNT", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881700 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<10>.SW_HEAD.CRNT", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x881770 ,0, 31}}}},
     {"DMA_RGF.DESCQ.<11>.SW_HEAD.CRNT", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x8817e0 ,0, 31}}}},
     {"MAC_EXT_RGF.MAC_SXD_EXT.PMC_UCODE_WR_RANGE.MAC_EXT_MAC_EXT_RGF_MAC_SXD_EXT_PMC_UCODE_WR_RANGE_0.sxd_pmc_start_1", { {BasebandRevision::TLN_M_B0, PmcRegisterInfo{0x88C1BC ,0, 7}}, {BasebandRevision::TLN_M_C0, PmcRegisterInfo{0x88C1BC ,0, 7}} }},
     {"MAC_EXT_RGF.MAC_SXD_EXT.PMC_UCODE_WR_RANGE.MAC_EXT_MAC_EXT_RGF_MAC_SXD_EXT_PMC_UCODE_WR_RANGE_0.sxd_pmc_end_1", { {BasebandRevision::TLN_M_B0, PmcRegisterInfo{0x88C1BC ,8, 15}}, {BasebandRevision::TLN_M_C0, PmcRegisterInfo{0x88C1BC ,8, 15}} }},
     {"MAC_EXT_RGF.MAC_SXD_EXT.PMC_UCODE_WR_RANGE.MAC_EXT_MAC_EXT_RGF_MAC_SXD_EXT_PMC_UCODE_WR_RANGE_0.sxd_pmc_start_2", { {BasebandRevision::TLN_M_B0, PmcRegisterInfo{0x88C1BC ,16, 23}}, {BasebandRevision::TLN_M_C0, PmcRegisterInfo{0x88C1BC ,16, 23}} }},
     {"MAC_EXT_RGF.MAC_SXD_EXT.PMC_UCODE_WR_RANGE.MAC_EXT_MAC_EXT_RGF_MAC_SXD_EXT_PMC_UCODE_WR_RANGE_0.sxd_pmc_end_2", { {BasebandRevision::TLN_M_B0, PmcRegisterInfo{0x88C1BC ,24, 31}}, {BasebandRevision::TLN_M_C0, PmcRegisterInfo{ 0x88C1BC ,24, 31}} }},
     {"MSXD_CMD.EXT_CONTROL.NID_UCODE_ACTIVE.MSXD_CMD_MSXD_CMD_EXT_CONTROL_NID_UCODE_ACTIVE_0.msrl_ucode_active_nid", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x180 ,0, 1}}}},
     {"MSXD_CMD.BI_TSF_CONTROL.BI_CONTROL.msrl_en_tsf_timer", {{BasebandRevision::BB_REV_UNKNOWN, PmcRegisterInfo{0x34 ,1, 1}}}},
 };

std::ostream& operator<<(std::ostream& os, const PmcRegisterInfo& pmcRegInfo)
{
    return os << "{" << Address(pmcRegInfo.GetRegisterAddress()) << '|' << pmcRegInfo.GetBitStart() << '|' << pmcRegInfo.GetBitEnd() << '}';
}

PmcRegistersAccessor::PmcRegistersAccessor(const Device& device): m_device(device)
{
}

//registerName => { revision => register info }
PmcRegisterInfo PmcRegistersAccessor::GetPmcRegisterInfo(const std::string& registerName, const BasebandRevision deviceType)
{
    const auto registerNameIterator = s_pmcRegisterMap.find(registerName);
    if(registerNameIterator == s_pmcRegisterMap.end())
    {
        return {};
    }

    auto revisionToRegInfoMap = registerNameIterator->second;
    const auto revisionIterator = revisionToRegInfoMap.find(deviceType);
    if(revisionIterator != revisionToRegInfoMap.end())
    {
        return revisionIterator->second;
    }

    const auto unknownRevisionIterator = revisionToRegInfoMap.find(BasebandRevision::BB_REV_UNKNOWN);
    if(unknownRevisionIterator != revisionToRegInfoMap.end())
    {
        return unknownRevisionIterator->second;
    }

    return {};
}

OperationStatus PmcRegistersAccessor::WritePmcRegister(const std::string& registerName, uint32_t value) const
{
    PmcRegisterInfo registerInfo = GetPmcRegisterInfo(registerName, m_device.GetBasebandRevision());

    LOG_VERBOSE << "Writing PMC register."
        << " Device: " << m_device.GetDeviceName()
        << " Name: " << registerName
        << " Register: " << registerInfo
        << " Value: " << value
        << std::endl;

    if(!registerInfo.IsValid())
    {
        std::ostringstream oss;
        oss << "Failed retrieving register info for " << registerName;
        return OperationStatus(false, oss.str());
    }

    uint32_t regValue = 0;
    OperationStatus statusRead = m_device.Read(registerInfo.GetRegisterAddress(), regValue);
    if(!statusRead.IsSuccess())
    {
        LOG_ERROR << statusRead;
        std::ostringstream oss;
        oss << "Failed to read mnemonic " << registerName;
        statusRead.AddPrefix(oss.str());
        return statusRead;
    }

    uint32_t dataToWrite = WriteToBitMask(regValue, registerInfo.GetBitStart(), registerInfo.GetBitEnd() - registerInfo.GetBitStart() + 1, value);
    LOG_VERBOSE << " >>>> Writing " << Hex<8>(dataToWrite) << " to " << registerInfo << std::endl;
    OperationStatus statusWrite = m_device.Write(registerInfo.GetRegisterAddress(), dataToWrite);
    if(!statusWrite.IsSuccess())
    {
        LOG_ERROR << statusWrite;
        std::ostringstream oss;
        oss << "Failed to write mnemonic " << registerName;
        statusWrite.AddPrefix(oss.str());
        return statusWrite;
    }

    return OperationStatus(true);
}

OperationStatus PmcRegistersAccessor::ReadPmcRegister(const std::string& registerName, uint32_t& registerValue) const
{
    const PmcRegisterInfo registerInfo = GetPmcRegisterInfo(registerName, m_device.GetBasebandRevision());

    if(!registerInfo.IsValid())
    {
        return OperationStatus(false, "Failed retrieving register info for register " + registerName);
    }

    uint32_t temp = 0U;
    OperationStatus res = m_device.Read(registerInfo.GetRegisterAddress(), temp);
    if(!res.IsSuccess())
    {
        LOG_ERROR << res;
        std::ostringstream oss;
        oss << "Failed to read mnemonic " << registerName;
        res.AddPrefix(oss.str());
        return res;
    }

    registerValue = ReadFromBitMask(temp, registerInfo.GetBitStart(), registerInfo.GetBitEnd() - registerInfo.GetBitStart() + 1);

    LOG_VERBOSE << "Reading PMC register."
        << " Device: " << m_device.GetDeviceName()
        << " Name: " << registerName
        << " Register: " << registerInfo
        << " Value: " << registerValue
        << std::endl;

    return OperationStatus(true);
}

uint32_t PmcRegistersAccessor::WriteToBitMask(uint32_t dataBufferToWriteTo, uint32_t index, uint32_t size, uint32_t valueToWriteInData)
{
    return (dataBufferToWriteTo & (~GetBitMask(index, size))) | (valueToWriteInData << index);
}

uint32_t PmcRegistersAccessor::GetBitMask(uint32_t index, uint32_t size)
{
    if(size == 32)
    {
        return 0xFFFFFFFF << index;
    }
    return ((1 << (size)) - 1) << index;
}

uint32_t PmcRegistersAccessor::ReadFromBitMask(uint32_t data, uint32_t index, uint32_t size)
{
    return (data & GetBitMask(index, size)) >> index;
}
