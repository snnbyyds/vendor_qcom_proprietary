/******************************************************************************
#  Copyright (c) 2009-2013, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/******************************************************************************
  @file    qcril_arb.c
  @brief   qcril qmi - compatibility layer for Fusion

  DESCRIPTION

******************************************************************************/


/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#define TAG "QCRIL_ARB"

#include <framework/Log.h>
#include <errno.h>
#include <cutils/memory.h>
#include <pthread.h>
#include <cutils/properties.h>
#include <string.h>
#include "IxErrno.h"
#include "comdef.h"
#include "qcrili.h"
#include "qcril_cm_util.h"
#include "modules/nas/qcril_arb.h"
#include "modules/nas/qcril_qmi_nas.h"
#include "qcril_qmi_oem_events.h"
#include "modules/nas/NasImsHelper.h"
#include "modules/ims/ImsWfcSettingsStatusInd.h"
#include "modules/sms/qcril_qmi_sms.h"
#include "modules/nas/qcril_arb.h"
#include "qcril_cm_util.h"
#include "qcril_qmi_oem_events.h"
#include "modules/nas/Nas5gConnectionIndMessage.h"
#include "modules/nas/NasModule.h"

#define QCRIL_ARB_NETWORK_INFO_LEN 10 /* same defintion as QCRIL_NETWORK_INFO_LEN
                                         from qcril_data_netctrl.c
                                       */
/*===========================================================================

                    INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

/*! @brief Structure used to cache QCRIL instance data
*/
typedef struct
{
  qtimutex::QtiSharedMutex          mutex;               /* Mutex used to control simultaneous update/access to arb data */
  qcril_arb_pref_data_tech_e_type pref_data_tech;     /* Preferred data tech for Fusion architecture*/
  boolean                         is_current;
  dsd_system_status_ind_msg_v01   dsd_system_status;
  boolean                         is_dsd;
} qcril_arb_struct_type;


/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/

/* Cache QCRIL instances' data */
static qcril_arb_struct_type qcril_arb;

// static char *qcril_arb_pref_data_tech_name[] =
//   { "Unknown", "CDMA", "EVDO", "GSM", "UMTS", "EHRPD", "LTE", "TDSCDMA", "<invalid slot>" };

static qcril_arb_pref_data_snapshot_type qcril_arb_pref_data_snapshot;

/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/

static void qcril_qmi_arb_nas_control_evaluate_data_rte_on_pref_data_tech_change();
static int qcril_qmi_arb_nas_control_check_power_save_and_screen_off_status();
static int qcril_qmi_convert_rat_mask_to_technology(qcril_arb_pref_data_type *pref_data);
static void qcril_arb_current_data_technology_helper();

/*===========================================================================

                                FUNCTIONS

===========================================================================*/

/*=========================================================================
  FUNCTION:  qcril_arb_init

===========================================================================*/
/*!
    @brief
    Initialize the split modem architecture property.

    @return
    none
*/
/*=========================================================================*/
void qcril_arb_init
(
  void
)
{
  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_FUNC_ENTRY();

  /* Initialize internal data cache */
  memset( &qcril_arb, 0, sizeof( qcril_arb ) );

  memset(&qcril_arb_pref_data_snapshot, 0, sizeof(qcril_arb_pref_data_snapshot));
} /* qcril_arb_init */

//===========================================================================
// qcril_arb_current_data_technology_helper
//===========================================================================
void qcril_arb_current_data_technology_helper()
{
    qmi_ril_gen_operational_status_type cur_status;

    cur_status = qmi_ril_get_operational_status();

    QCRIL_LOG_INFO( "operational status %d", (int)cur_status);

    switch( cur_status )
    {
      case QMI_RIL_GEN_OPERATIONAL_STATUS_INIT_ONGOING:     // fallthrough
      case QMI_RIL_GEN_OPERATIONAL_STATUS_UNRESTRICTED:     // fallthrough
      case QMI_RIL_GEN_OPERATIONAL_STATUS_RESUMING:
          // Calculate Data rte and initiate voice radio tech calculation
          qcril_qmi_arb_nas_control_evaluate_data_rte_on_pref_data_tech_change();
          if( TRUE == qcril_qmi_arb_nas_control_check_power_save_and_screen_off_status() )
          {
              qmi_ril_nw_reg_data_pref_changed_action();
          }

          // TODO: add code for sending IND for data registration change.
          // Network information updated, send unsolicited network state changed indication
          qcril_qmi_nas_wave_voice_data_status();
          break;

      default:
          break;
    }
} //qcril_arb_current_data_technology_helper

/*=========================================================================
  FUNCTION:  qcril_arb_set_dsd_sys_status

===========================================================================*/
/*!
    @brief
    Set the preferred data technology used for data call setup. This is
    called upon receipt of
    QMI_DSD_REPORT_SYSTEM_STATUS_IND_V01/QMI_DSD_GET_SYSTEM_STATUS_RESP_V01.

    @return
    None
*/
/*=========================================================================*/
void qcril_arb_set_dsd_sys_status(const dsd_system_status_ind_msg_v01 *const dsd_system_status)
{
    uint32_t i;
    uint8_t is_wwan_available = FALSE, is_need_unsol = FALSE;
    qcril_arb_pref_data_type old_pref_data;
    qcril_arb_pref_data_type new_pref_data;

    memset( &old_pref_data, 0, sizeof( old_pref_data ) );
    memset( &new_pref_data, 0, sizeof( new_pref_data ) );

    qcril_qmi_acquire_nas_cache_lock();
    qcril_qmi_get_pref_data_tech(&old_pref_data);

    if( dsd_system_status )
    {
        QCRIL_LOG_INFO( "available systems info valid %d", dsd_system_status->avail_sys_valid );

        if( TRUE == dsd_system_status->avail_sys_valid )
        {
            QCRIL_LOG_DEBUG("dsd_sys_status len=%d", dsd_system_status->avail_sys_len);

            QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

            memcpy(&qcril_arb.dsd_system_status,  dsd_system_status, sizeof(qcril_arb.dsd_system_status));
            qcril_arb.is_current = TRUE;
            qcril_arb.is_dsd = TRUE;

            QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

            qcril_qmi_get_pref_data_tech(&new_pref_data);

            QCRIL_LOG_ESSENTIAL("old_pref_data %d - new_data_pref %d", old_pref_data, new_pref_data);

            QCRIL_LOG_ESSENTIAL("preferred dsd_sys_status nw=0x%x, rat_value=0x%x, so_mask=0x%016llx",
                        dsd_system_status->avail_sys[0].technology,
                        dsd_system_status->avail_sys[0].rat_value,
                        dsd_system_status->avail_sys[0].so_mask);

            if( new_pref_data.dsd_sys_status_info.rat_value != DSD_SYS_RAT_EX_NULL_BEARER_V01 )
            {
                qcril_qmi_nas_reset_extrapolation_ban_expiry();
                is_need_unsol = TRUE;
            }

            if( 0 != memcmp( &old_pref_data, &new_pref_data, sizeof(qcril_arb_pref_data_type) ) )
            {
                for(i = 0; i < dsd_system_status->avail_sys_len; i++) //The first entry in the list will be the preferred system
                {
                    QCRIL_LOG_ESSENTIAL("preferred %d - dsd_sys_status nw=0x%x, rat_value=0x%x, so_mask=0x%016llx",
                                (QMI_RIL_ZERO == i),
                                dsd_system_status->avail_sys[i].technology,
                                dsd_system_status->avail_sys[i].rat_value,
                                dsd_system_status->avail_sys[i].so_mask);
                }

                if(dsd_system_status->avail_sys_len &&
                   DSD_SYS_RAT_EX_3GPP_WLAN_V01 == dsd_system_status->avail_sys[0].rat_value)
                {
                    for(i = 1; i < dsd_system_status->avail_sys_len; i++)
                    {
                        if((DSD_SYS_NETWORK_WLAN_V01 != dsd_system_status->avail_sys[i].technology) &&
                           (DSD_SYS_RAT_EX_NULL_BEARER_V01 != dsd_system_status->avail_sys[i].rat_value))
                        {
                            is_wwan_available = TRUE;
                            break;
                        }
                    }
                }

                QCRIL_LOG_ESSENTIAL("is_wwan_available %d",is_wwan_available);
                qcril_hook_unsol_response( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_EVT_HOOK_UNSOL_WWAN_AVAILABLE, (char*) &is_wwan_available, sizeof(    is_wwan_available));

                QCRIL_LOG_DEBUG("old_pref_data.five_g_data_tech: %d", old_pref_data.five_g_data_tech);
                QCRIL_LOG_DEBUG("new_pref_data.five_g_data_tech: %d", new_pref_data.five_g_data_tech);

                std::shared_ptr<Nas5gConnectionIndMessage> five_g_connected_ind;
                if (new_pref_data.five_g_data_tech == QCRIL_ARB_PREF_DATA_TECH_5G)
                {
                    if (dsd_system_status->avail_sys[0].so_mask & QMI_DSD_3GPP_SO_MASK_5G_MMWAVE_V01) {
                        five_g_connected_ind = std::make_shared<Nas5gConnectionIndMessage>(FIVE_G_BEARER_STATUS_MMW_ALLOCATED);
                    } else {
                        five_g_connected_ind = std::make_shared<Nas5gConnectionIndMessage>(FIVE_G_BEARER_STATUS_ALLOCATED);
                    }
                }
                else if (old_pref_data.five_g_data_tech == QCRIL_ARB_PREF_DATA_TECH_5G
                        && new_pref_data.five_g_data_tech != QCRIL_ARB_PREF_DATA_TECH_5G)
                {
                    five_g_connected_ind = std::make_shared<Nas5gConnectionIndMessage>(FIVE_G_BEARER_STATUS_NOT_ALLOCATED);
                }

                if (five_g_connected_ind != nullptr)
                {
                    five_g_connected_ind->broadcast();
                }

                // Drop sig info cache if not extrapolating in screen off state,
                if ( qcril_qmi_ril_domestic_service_is_screen_off() && !new_pref_data.is_extrapolation )
                {
                  qcril_qmi_drop_sig_info_cache();
                }

                qmi_ril_nw_reg_data_sys_update_pre_update_action();

                getNasModule().reportCurrentPhysChanConfig();

                qcril_arb_current_data_technology_helper();
            }
            else if(is_need_unsol)
            {
                qcril_qmi_nas_wave_voice_data_status();
            }
        }
    }
    else
    {
        QCRIL_LOG_FATAL("Null pointer passed for dsd_system_status");
    }

    qcril_qmi_release_nas_cache_lock();
}

bool qcril_qmi_is_5g_data_connected()
{
    bool is_5g_data_connected;

    QCRIL_MUTEX_LOCK(&qcril_arb.mutex, "qcril_arb.mutex");
    is_5g_data_connected = \
        qcril_arb.dsd_system_status.avail_sys[0].rat_value == DSD_SYS_RAT_EX_3GPP_5G_V01 &&
        ((qcril_arb.dsd_system_status.avail_sys[0].so_mask & QMI_DSD_3GPP_SO_MASK_5G_TDD_V01) ||
        (qcril_arb.dsd_system_status.avail_sys[0].so_mask & QMI_DSD_3GPP_SO_MASK_5G_NSA_V01) ||
        (qcril_arb.dsd_system_status.avail_sys[0].so_mask & QMI_DSD_3GPP_SO_MASK_5G_SA_V01) ||
        (qcril_arb.dsd_system_status.avail_sys[0].so_mask & QMI_DSD_3GPP_SO_MASK_5G_MMWAVE_V01));
    QCRIL_MUTEX_UNLOCK(&qcril_arb.mutex, "qcril_arb.mutex");

    return is_5g_data_connected;
}

five_g_bearer_status qcril_qmi_query_bearer_status()
{
    five_g_bearer_status status = five_g_bearer_status::FIVE_G_BEARER_STATUS_NOT_ALLOCATED;

    QCRIL_MUTEX_LOCK(&qcril_arb.mutex, "qcril_arb.mutex");

    if (qcril_arb.dsd_system_status.avail_sys[0].rat_value == DSD_SYS_RAT_EX_3GPP_5G_V01)
    {
         if (qcril_arb.dsd_system_status.avail_sys[0].so_mask & QMI_DSD_3GPP_SO_MASK_5G_MMWAVE_V01)
         {
             status = five_g_bearer_status::FIVE_G_BEARER_STATUS_MMW_ALLOCATED;
         }
         else
         {
             status = five_g_bearer_status::FIVE_G_BEARER_STATUS_ALLOCATED;
         }
    }
    else
    {
          status = five_g_bearer_status::FIVE_G_BEARER_STATUS_NOT_ALLOCATED;
    }

    QCRIL_MUTEX_UNLOCK(&qcril_arb.mutex, "qcril_arb.mutex");

    QCRIL_LOG_FUNC_RETURN_WITH_RET(status);
    return status;
}

/*=========================================================================
  FUNCTION:  qcril_arb_set_pref_data_tech

===========================================================================*/
/*!
    @brief
    Set the preferred data technology used for data call setup.

    @return
    None
*/
/*=========================================================================*/
void qcril_arb_set_pref_data_tech
(
  qcril_arb_pref_data_tech_e_type pref_data_tech
)
{
  QCRIL_LOG_ESSENTIAL("qcril_arb_set_pref_data_tech action new tech %d", (int) pref_data_tech);

  qcril_qmi_acquire_nas_cache_lock();
  qmi_ril_nw_reg_data_sys_update_pre_update_action();

  QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );
  qcril_arb.pref_data_tech = pref_data_tech;
  qcril_arb.is_current = FALSE;
  QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );
  qcril_arb_current_data_technology_helper();
  qcril_qmi_release_nas_cache_lock();
} // qcril_arb_set_pref_data_tech

//===========================================================================
// qcril_qmi_get_pref_data_tech
//===========================================================================
void qcril_qmi_get_pref_data_tech(qcril_arb_pref_data_type *pref_data)
{
    uint32_t index_tech_report = QMI_RIL_ZERO;
    uint32_t wlan_index = QMI_RIL_ZERO;
    uint8_t wlan_status = FALSE;
    uint8_t lte_status = FALSE;
    uint8_t report_dsd_status = TRUE;
    uint8_t only_5g_reported = FALSE;

    if( pref_data )
    {
        pref_data->is_extrapolation = FALSE;

        QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

        QCRIL_LOG_DEBUG( "is_current %d", qcril_arb.is_current);

        if( TRUE == qcril_arb.is_current )
        {
            pref_data->is_current = TRUE;
            pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_UNKNOWN;
            QCRIL_LOG_DEBUG( "is_dsd %d", qcril_arb.is_dsd);
            if( TRUE == qcril_arb.is_dsd )
            {
                pref_data->is_dsd = TRUE;

                if(qmi_ril_is_feature_supported(QMI_RIL_FEATURE_RIL_DATA_REG_STATE_CONDITIONAL_REPORT))
                {
                    wlan_status = qcril_arb_check_wlan_rat_dsd_reported(&wlan_index);
                    QCRIL_LOG_DEBUG( "DSD: WLAN RAT STATUS %d, WLAN Index %d", wlan_status, wlan_index);
                    lte_status = qcril_arb_check_lte_rat_dsd_reported();

                    if(wlan_status == TRUE && lte_status == TRUE)
                    {
                        /* IMSA REFACTOR: TEST */
                        wlan_status = qcril_qmi_nas_ims_registered_wlan_status();
                        QCRIL_LOG_DEBUG( "IMSA: WLAN STATUS %d", wlan_status);
                        if(wlan_status)
                        {
                            index_tech_report = wlan_index;
                            QCRIL_LOG_DEBUG( "Report WLAN RAT as reported through DSD as well as from IMS and WFC is ON");
                        }
                        else
                        {
                            wlan_status = qcril_qmi_nas_ims_wlan_status();
                            QCRIL_LOG_DEBUG( "IMSS: WLAN STATUS %d", wlan_status);
                            if(wlan_status)
                            {
                                index_tech_report = wlan_index;
                                QCRIL_LOG_DEBUG( "Report WLAN RAT as reported through DSD as well as WFC is ON and WFC pref is WLAN_PREF/WLAN_ONLY");
                            }
                            else
                            {
                                if(QMI_RIL_ZERO == wlan_index)
                                {
                                    report_dsd_status = qcril_arb_decide_data_rat_to_report(&index_tech_report);
                                    if(!report_dsd_status)
                                    {
                                        QCRIL_LOG_DEBUG( "Report OOS to Telephony");
                                        pref_data->dsd_sys_status_info.rat_value = DSD_SYS_RAT_EX_NULL_BEARER_V01;
                                        pref_data->radio_technology = RADIO_TECH_UNKNOWN;
                                        pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_UNKNOWN;
                                    }
                                }
                                else
                                {
                                    index_tech_report = QMI_RIL_ZERO;
                                    QCRIL_LOG_DEBUG( "Report default tech i.e. Preffered one");
                                }
                            }
                        }
                    }
                    else
                    {
                        index_tech_report = QMI_RIL_ZERO;
                        if(lte_status == FALSE)
                        {
                            /* In this case, where LTE is absent, we will report the global preferred rat reported by DSD.
                               And If IWLAN is present, DSD will report IWLAN as global preferred rat */
                            QCRIL_LOG_DEBUG( "LTE RAT is not reported in DSD, report the preferred rat");
                        }
                        if(wlan_status == FALSE)
                        {
                            /* In this case, where IWLAN is absent, we will report the global preferred rat reported by DSD. */
                            QCRIL_LOG_DEBUG( "WLAN RAT is not reported in DSD, report the preferred rat");
                        }
                    }
                }

                if (!(qmi_ril_is_feature_supported(QMI_RIL_FEATURE_RIL_DATA_REG_STATE_CONDITIONAL_REPORT)
                            && lte_status == TRUE && wlan_status == TRUE)
                        && qcril_arb.dsd_system_status.avail_sys_len >= 1)
                {
                    if (qcril_arb.dsd_system_status.avail_sys[QMI_RIL_ZERO].rat_value == DSD_SYS_RAT_EX_3GPP_5G_V01)
                    {
                        QCRIL_LOG_DEBUG("First preferred RAT is 5G.");
                        pref_data->five_g_data_tech = QCRIL_ARB_PREF_DATA_TECH_5G;
                        if (qcril_arb.dsd_system_status.avail_sys[QMI_RIL_ZERO].so_mask & QMI_DSD_3GPP_SO_MASK_5G_NSA_V01)
                        {
                            if (qcril_arb.dsd_system_status.avail_sys_len >= 2)
                            {
                                index_tech_report = 1;
                            }
                            else
                            {
                                 only_5g_reported = TRUE;
                                 pref_data->dsd_sys_status_info.rat_value = DSD_SYS_RAT_EX_NULL_BEARER_V01;
                                 pref_data->radio_technology = RADIO_TECH_UNKNOWN;
                                 pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_UNKNOWN;
                            }
                        }
                    }
                    else
                    {
                        pref_data->five_g_data_tech = QCRIL_ARB_PREF_DATA_TECH_UNKNOWN;
                    }
                }

                if(report_dsd_status && !only_5g_reported)
                {
                    QCRIL_LOG_DEBUG( "Report DSD technology Index %d", index_tech_report);
                    pref_data->dsd_sys_status_info.technology = qcril_arb.dsd_system_status.avail_sys[index_tech_report].technology; //The first entry in the list will be the preferred system
                    pref_data->dsd_sys_status_info.rat_value = qcril_arb.dsd_system_status.avail_sys[index_tech_report].rat_value;
                    pref_data->dsd_sys_status_info.so_mask = qcril_arb.dsd_system_status.avail_sys[index_tech_report].so_mask;

                    QCRIL_LOG_DEBUG( "technology %d rat_value %x so_mask 0x%016llx",
                                  pref_data->dsd_sys_status_info.technology,
                                  pref_data->dsd_sys_status_info.rat_value,
                                  pref_data->dsd_sys_status_info.so_mask);
                }
            }
        }
        else
        {
            pref_data->is_current = FALSE;
            pref_data->pref_data_tech = qcril_arb.pref_data_tech;
        }


        QCRIL_LOG_DEBUG( "before translation : pref_data_tech %s",qcril_qmi_util_retrieve_pref_data_tech_name(pref_data->pref_data_tech));

        if( ( TRUE == pref_data->is_current ) && report_dsd_status && !only_5g_reported)
        {
            pref_data->radio_technology = qcril_qmi_convert_rat_mask_to_technology(pref_data);
            qcril_arb_pref_data_snapshot.snapshot_radio_technology = pref_data->radio_technology;
            switch( pref_data->radio_technology )
            {
                case RADIO_TECH_HSDPA: //HSDPA, HSUPA, HSPA and HSPAP can belong to either TDSCDMA or WCDMA
                case RADIO_TECH_HSUPA: //DsD layer ORs the rat_mask with TDSCDMA or WCDMA (mutually exclusive) to discern between the two
                case RADIO_TECH_HSPA:
                case RADIO_TECH_HSPAP:
                    if ( pref_data->is_dsd ) // dsd
                    {
                        switch(pref_data->dsd_sys_status_info.rat_value)
                        {
                            case DSD_SYS_RAT_EX_3GPP_TDSCDMA_V01:
                                pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_TDSCDMA;
                                break;
                            case DSD_SYS_RAT_EX_3GPP_WCDMA_V01:
                                pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_UMTS;
                                break;
                            default:
                                // ideally should not come here when radio tech is HS*,
                                // just in case set it to umts
                                QCRIL_LOG_DEBUG( "rat_value is not tdscdma/umts, set pref_data_tech to umts" );
                                pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_UMTS;
                                break;
                        }
                    }
                    break;

                case RADIO_TECH_UMTS:
                    pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_UMTS;
                    break;

#ifndef QMI_RIL_UTF
                case RADIO_TECH_TD_SCDMA:
                    pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_TDSCDMA;
                    break;
#endif

                case RADIO_TECH_GPRS:
                case RADIO_TECH_EDGE:
                case RADIO_TECH_GSM:
                    pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_GSM;
                    break;

                case RADIO_TECH_LTE:
                case RADIO_TECH_IWLAN:
#ifndef NOT_SUPPORTED_LTE_CA
              case RADIO_TECH_LTE_CA:
#endif
                    pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_LTE;
                    break;

                case RADIO_TECH_IS95A:
                case RADIO_TECH_IS95B:
                case RADIO_TECH_1xRTT:
                    pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_CDMA;
                    break;

                case RADIO_TECH_EVDO_0:
                case RADIO_TECH_EVDO_A:
                case RADIO_TECH_EVDO_B:
                    pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_EVDO;
                    break;

                case RADIO_TECH_EHRPD:
                    pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_EHRPD;
                    break;

                case RADIO_TECH_5G:
                    pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_5G;
                    break;

                case RADIO_TECH_UNKNOWN:
                default:
                    pref_data->pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_UNKNOWN;
                    break;
            }
        }

        QCRIL_LOG_DEBUG( "after translation : pref_data_tech %s technology %d", qcril_qmi_util_retrieve_pref_data_tech_name(pref_data->pref_data_tech),
                                                                                    pref_data->radio_technology);

        QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );
    }

    if(!report_dsd_status)
    {
        qcril_qmi_nas_reset_data_snapshot_cache_and_timer();
        qcril_qmi_arb_reset_pref_data_snapshot();
    }
} // qcril_qmi_get_pref_data_tech

//===========================================================================
// qcril_qmi_convert_rat_mask_to_technology
//===========================================================================
int qcril_qmi_convert_rat_mask_to_technology(qcril_arb_pref_data_type *pref_data)
{
    int technology = RADIO_TECH_UNKNOWN;

    QCRIL_LOG_FUNC_ENTRY();
    if( pref_data )
    {
        pref_data->is_extrapolation = FALSE;
        if( TRUE == pref_data->is_current )
        {
            if( TRUE == pref_data->is_dsd )
            {
                switch( pref_data->dsd_sys_status_info.rat_value )
                {
                    case DSD_SYS_RAT_EX_NULL_BEARER_V01:
                        technology = qcril_arb_pref_data_snapshot.snapshot_radio_technology;
                        pref_data->is_extrapolation = TRUE;
                        break;

                    case DSD_SYS_RAT_EX_3GPP_WCDMA_V01:
                    case DSD_SYS_RAT_EX_3GPP_TDSCDMA_V01:
                        if( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_HSDPAPLUS_V01)
                            || (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_DC_HSDPAPLUS_V01)
                            || (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_64_QAM_V01) )
                        {
                            technology = RADIO_TECH_HSPAP;
                        }
                        else if( ((pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_HSDPA_V01)
                                 && (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_HSUPA_V01))
                                 || (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_HSPA_V01) )
                        {
                            technology = RADIO_TECH_HSPA;
                        }
                        else if( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_HSDPA_V01) )
                        {
                            technology = RADIO_TECH_HSDPA;
                        }
                        else if( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_HSUPA_V01) )
                        {
                            technology = RADIO_TECH_HSUPA;
                        }
                        else if( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_WCDMA_V01) )
                        {
                            technology = RADIO_TECH_UMTS;
                        }
                        else if ( pref_data->dsd_sys_status_info.rat_value == DSD_SYS_RAT_EX_3GPP_TDSCDMA_V01 )
                        {
#ifndef QMI_RIL_UTF
                            technology = RADIO_TECH_TD_SCDMA;
#endif
                        }
                        break;

                    case DSD_SYS_RAT_EX_3GPP_GERAN_V01:
                        if( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_EDGE_V01) )
                        {
                            technology = RADIO_TECH_EDGE;
                        }
                        else if( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_GPRS_V01)
                                 || (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_GSM_V01) )
                        {
                            technology = RADIO_TECH_GPRS;
                        }
                        break;

                    case DSD_SYS_RAT_EX_3GPP_LTE_V01:
                        if( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_LTE_CA_DL_V01) )
                        {
                            technology = RADIO_TECH_LTE_CA;
                        }
                        else if( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_LTE_CA_UL_V01) )
                        {
                            technology = RADIO_TECH_LTE_CA;
                        }
                        else
                        {
                            technology = RADIO_TECH_LTE;
                        }
                        break;

                    case DSD_SYS_RAT_EX_3GPP_WLAN_V01:
                        technology = RADIO_TECH_IWLAN;
                        break;

                    case DSD_SYS_RAT_EX_3GPP2_1X_V01:
                        if( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP2_SO_MASK_1X_IS2000_REL_A_V01)
                            || (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP2_SO_MASK_1X_IS2000_V01) )
                        {
                            technology = RADIO_TECH_1xRTT;
                        }
                        else if( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP2_SO_MASK_1X_IS95_V01) )
                        {
                            technology = RADIO_TECH_IS95A;
                        }
                        break;

                    case DSD_SYS_RAT_EX_3GPP2_HRPD_V01:
                        if ( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVB_DPA_V01)
                                 || (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVB_MPA_V01)
                                 || (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVB_MMPA_V01)
                                 || (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVB_EMPA_V01) )
                        {
                            technology = RADIO_TECH_EVDO_B;
                        }
                        else if( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVA_DPA_V01)
                                 || (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVA_MPA_V01)
                                 || (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVA_EMPA_V01) )
                        {
                            technology = RADIO_TECH_EVDO_A;
                        }
                        else if( (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REV0_DPA_V01) )
                        {
                            technology = RADIO_TECH_EVDO_0;
                        }
                        break;

                    case DSD_SYS_RAT_EX_3GPP2_EHRPD_V01:
                            technology = RADIO_TECH_EHRPD;
                        break;

                    case DSD_SYS_RAT_EX_3GPP_5G_V01:
                        if ((pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_5G_TDD_V01) ||
                            (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_5G_NSA_V01) ||
                            (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_5G_SA_V01)  ||
                            (pref_data->dsd_sys_status_info.so_mask & QMI_DSD_3GPP_SO_MASK_5G_MMWAVE_V01))
                        {
                            technology = RADIO_TECH_5G;
                        }
                        break;

                    default: //no action
                        break;
                }
            }

        }
    }
    QCRIL_LOG_INFO("technology %s", qcril_qmi_util_retrieve_technology_name(technology));


    QCRIL_LOG_FUNC_RETURN_WITH_RET(technology);
    return technology;
} // qcril_qmi_convert_rat_mask_to_technology

//===========================================================================
// qcril_qmi_arb_reset_pref_data_snapshot
//===========================================================================
void qcril_qmi_arb_reset_pref_data_snapshot()
{
    QCRIL_LOG_FUNC_ENTRY();

    QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );
    memset(&qcril_arb_pref_data_snapshot, 0, sizeof(qcril_arb_pref_data_snapshot));

    if ( !qcril_arb.is_current && QCRIL_ARB_PREF_DATA_TECH_UNKNOWN == qcril_arb.pref_data_tech )
    {
        qcril_arb.pref_data_tech = QCRIL_ARB_PREF_DATA_TECH_INVALID;
    }

    QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

    QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_arb_reset_pref_data_snapshot

//===========================================================================
// qcril_qmi_arb_nas_control_evaluate_data_rte_on_pref_data_tech_change
//===========================================================================
void qcril_qmi_arb_nas_control_evaluate_data_rte_on_pref_data_tech_change()
{
    QCRIL_LOG_FUNC_ENTRY();
    qcril_qmi_nas_evaluate_data_rte_on_pref_data_tech_change();
    QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_arb_nas_control_evaluate_data_rte_on_pref_data_tech_change

//===========================================================================
// qcril_qmi_arb_nas_control_check_power_save_and_screen_off_status
//===========================================================================
int qcril_qmi_arb_nas_control_check_power_save_and_screen_off_status()
{
    int ret;

    QCRIL_LOG_FUNC_ENTRY();
    ret = qcril_qmi_nas_check_power_save_and_screen_off_status();
    QCRIL_LOG_FUNC_RETURN_WITH_RET(ret);
    return ret;
} //qcril_qmi_arb_nas_control_check_power_save_and_screen_off_status

/*=========================================================================
  FUNCTION:  qcril_arb_query_ph_srv_modem_id

===========================================================================*/
/*!
    @brief
    Find the modem that should be requested for Phone Service.

    Note: Modem or Telephony is responsible to check the validity of the
    type of the requested service.

    @return
    E_SUCCESS if the request should be served by QC modem
    E_NOT_SUPPORTED if the request should be served by Third Party modem
    E_NOT_ALLOWED if the request is not allowed on the device
*/
/*=========================================================================*/
IxErrnoType qcril_arb_query_ph_srv_modem_id
(
  qcril_arb_ph_srv_cat_e_type ph_srv_cat,
  qcril_instance_id_e_type instance_id,
  qcril_modem_ids_list_type *modem_ids_list_ptr
)
{
  IxErrnoType status = E_SUCCESS;

  if( instance_id < QCRIL_MAX_INSTANCE_ID &&  modem_ids_list_ptr != NULL)
  {
  if ( ph_srv_cat > QCRIL_ARB_PH_SRV_CAT_COMMON )
  {
    QCRIL_LOG_ERROR( "ph srv category %d not supported", ph_srv_cat );
    status = E_NOT_SUPPORTED;
    QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );
  }
  else
  {
  modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
  modem_ids_list_ptr->num_of_modems = 1;
  }
  }
  else
  {
      status = E_NOT_ALLOWED;
  }

  return status;

} /* qcril_arb_query_ph_srv_modem_id */


/*=========================================================================
  FUNCTION:  qcril_arb_query_sms_srv_modem_id

===========================================================================*/
/*!
    @brief
    Find the modem that should be requested for SMS Service

    Note: Modem or Telephony is responsible to check the validity of the
    type of the requested service.

    @return
    E_SUCCESS if the request should be served by QC modem
    E_NOT_SUPPORTED if the request should be served by Third Party modem
    E_NOT_ALLOWED if the request is not allowed on the device
*/
/*=========================================================================*/
IxErrnoType qcril_arb_query_sms_srv_modem_id
(
  qcril_arb_sms_srv_cat_e_type sms_srv_cat,
  qcril_instance_id_e_type instance_id,
  qcril_modem_ids_list_type *modem_ids_list_ptr
)
{

  IxErrnoType status = E_SUCCESS;
  QCRIL_NOTUSED(sms_srv_cat);
  QCRIL_NOTUSED(instance_id);

  modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
  modem_ids_list_ptr->num_of_modems = 1;

  return status;

} /* qcril_arb_query_sms_srv_modem_id */


/*=========================================================================
  FUNCTION:  qcril_arb_query_auth_srv_modem_id

===========================================================================*/
/*!
    @brief
    Find the modem that should be requested for AUTH Service.

    Note: Modem or Telephony is responsible to check the validity of the
    type of the requested service.

    @return
    E_SUCCESS if the request should be served by QC modem
    E_NOT_SUPPORTED if the request should be served by Third Party modem
    E_NOT_ALLOWED if the request is not allowed on the device
*/
/*=========================================================================*/
IxErrnoType qcril_arb_query_auth_srv_modem_id
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type *modem_id_ptr
)
{
  IxErrnoType status = E_SUCCESS;

  if(instance_id < QCRIL_MAX_INSTANCE_ID && modem_id_ptr != NULL)
  {
      *modem_id_ptr = QCRIL_DEFAULT_MODEM_ID;
  }
  else
  {
      status = E_NOT_ALLOWED;
  }


  return status;

} /* qcril_arb_query_auth_srv_modem_id */


/*=========================================================================
  FUNCTION:  qcril_arb_query_nv_srv_modem_id

===========================================================================*/
/*!
    @brief
    Find the modem that should be requested for NV Service.

    Note: Modem or Telephony is responsible to check the validity of the
    type of the requested service.

    @return
    E_SUCCESS if the request should be served by QC modem
    E_NOT_SUPPORTED if the request should be served by Third Party modem
    E_NOT_ALLOWED if the request is not allowed on the device
*/
/*=========================================================================*/
IxErrnoType qcril_arb_query_nv_srv_modem_id
(
  qcril_arb_nv_srv_cat_e_type nv_srv_cat,
  qcril_instance_id_e_type instance_id,
  qcril_modem_ids_list_type *modem_ids_list_ptr
)
{
  IxErrnoType status = E_SUCCESS;
  QCRIL_NOTUSED(nv_srv_cat);

  if( instance_id < QCRIL_MAX_INSTANCE_ID && modem_ids_list_ptr != NULL )
  {
  modem_ids_list_ptr->modem_id[ 0 ] = QCRIL_DEFAULT_MODEM_ID;
  modem_ids_list_ptr->num_of_modems = 1;
  }
  else
  {
      status = E_NOT_ALLOWED;
  }

  return status;

} /* qcril_arb_query_nv_srv_modem_id */


/*=========================================================================
  FUNCTION:  qcril_arb_ma_is_dsds

===========================================================================*/
/*!
    @brief
    Indicates whether the modem architecture is DSDS.

    @return
     True if DSDS is the current modem architecture,
     False otherwise.
*/
/*=========================================================================*/
boolean qcril_arb_ma_is_dsds
(
  void
)
{
  boolean ma_is_dsds = FALSE;

  /*-----------------------------------------------------------------------*/

  return ma_is_dsds;

} /* qcril_arb_ma_is_dsds */


/*=========================================================================
  FUNCTION:  qcril_arb_in_dsds_ssmm

===========================================================================*/
/*!
    @brief
    Indicates whether the DSDS preference is single standby multimode.

    @return
     True if Multimode is the current DSDS preference,
     False otherwise.
*/
/*=========================================================================*/
boolean qcril_arb_in_dsds_ssmm
(
  void
)
{
  boolean dsds_pref_is_mm = FALSE;

  /*-----------------------------------------------------------------------*/
  return dsds_pref_is_mm;

} /* qcril_arb_in_dsds_ssmm */




/*=========================================================================
  FUNCTION:  qcril_arb_jpn_band_is_supported

===========================================================================*/
/*!
    @brief
    Indicates whether JCDMA is supported.

    @return
     True if JPN band is supported,
     False otherwise.
*/
/*=========================================================================*/
boolean  qcril_arb_jpn_band_is_supported
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id
)
{
  boolean jpn_band_is_supported = FALSE;
  QCRIL_NOTUSED(instance_id);
  QCRIL_NOTUSED(modem_id);

  return jpn_band_is_supported;

} /* qcril_arb_jpn_band_is_supported */


/*=========================================================================
  FUNCTION:  qcril_arb_in_airplane_mode

===========================================================================*/
/*!
    @brief
    Query if the phone is in Airplane mode.

    @return
    TRUE if the phone is in airplane mode.
    FALSE otherwise
*/
/*=========================================================================*/
boolean qcril_arb_in_airplane_mode
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id
)
{
  boolean in_airplane_mode = FALSE;
  QCRIL_NOTUSED(instance_id);
  QCRIL_NOTUSED(modem_id);
  return in_airplane_mode;

} /* qcril_arb_in_airplane_mode */

/*=========================================================================
  FUNCTION:  qcril_arb_check_lte_rat_dsd_reported

===========================================================================*/
/*!
    @brief
    Query whether LTE RAT reported through DSD.
    If LTE is present it would be the Global preferred rat.

    @return
    If LTE present return TRUE
    Else return FALSE
*/
/*=========================================================================*/
uint8_t qcril_arb_check_lte_rat_dsd_reported()
{
    uint32_t i = 0;
    uint8_t status = FALSE;

    for(i = 0; i < qcril_arb.dsd_system_status.avail_sys_len; i++)
    {
        if(DSD_SYS_RAT_EX_3GPP_LTE_V01 == qcril_arb.dsd_system_status.avail_sys[i].rat_value)
        {
            status = TRUE;
            break;
        }
    }

    QCRIL_LOG_INFO("DSD LTE RAT STATUS: %d, LTE index %d", status, i);
    return status;
}

/*=========================================================================
  FUNCTION:  qcril_arb_check_wlan_rat_dsd_reported

===========================================================================*/
/*!
    @brief
    Query whether WLAN RAT reported through DSD.

    @return
    If WLAN present return TRUE and update "index"
    Else return FALSE
*/
/*=========================================================================*/
uint8_t qcril_arb_check_wlan_rat_dsd_reported(uint32_t *index)
{
    uint32_t i = 0;
    uint8_t status = FALSE;

    for(i = 0; i < qcril_arb.dsd_system_status.avail_sys_len; i++)
    {
        if(DSD_SYS_RAT_EX_3GPP_WLAN_V01 == qcril_arb.dsd_system_status.avail_sys[i].rat_value)
        {
            status = TRUE;
            *index = i;
            break;
        }
    }

    QCRIL_LOG_INFO("DSD WLAN RAT STATUS: %d, WLAN index %d", status, *index);
    return status;
}

uint8_t qcril_arb_check_wlan_rat_dsd_reported_helper(uint32_t *index)
{
    uint8_t status = FALSE;

    QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );
    status = qcril_arb_check_wlan_rat_dsd_reported(index);
    QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

    return status;
}

/*=========================================================================
  FUNCTION:  qcril_arb_find_index_rat_not_wlan_dsd_reported

===========================================================================*/
/*!
    @brief
    Query whether valid RAT reported through DSD other than WLAN.

    @return
    If present return TRUE and update "index"
    Else return FALSE
*/
/*=========================================================================*/
uint8_t qcril_arb_find_index_rat_not_wlan_dsd_reported(uint32_t *index)
{
    uint32_t i = 0;
    uint8_t status = FALSE;

    for(i = 0; i < qcril_arb.dsd_system_status.avail_sys_len; i++)
    {
        if( (DSD_SYS_RAT_EX_NULL_BEARER_V01 != qcril_arb.dsd_system_status.avail_sys[i].rat_value) &&
            (DSD_SYS_RAT_EX_3GPP_WLAN_V01 != qcril_arb.dsd_system_status.avail_sys[i].rat_value) &&
            (DSD_SYS_RAT_EX_WLAN_V01 != qcril_arb.dsd_system_status.avail_sys[i].rat_value)
          )
        {
            status = TRUE;
            *index = i;
            break;
        }
    }

    QCRIL_LOG_INFO("DSD valid RAT NOT WLAN STATUS: %d, RAT index %d", status, *index);
    return status;
}

uint8_t qcril_arb_find_index_rat_not_wlan_dsd_reported_helper(uint32_t *index)
{
    uint8_t status = FALSE;

    QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );
    status = qcril_arb_find_index_rat_not_wlan_dsd_reported(index);
    QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );

    return status;
}

/*=========================================================================
  FUNCTION:  qcril_arb_decide_data_rat_to_report

===========================================================================*/
/*!
    @brief
    Decide Data RAT to Telephony.

    @return
    If valid RAT return TRUE and update "index"
    Else return FALSE
*/
/*=========================================================================*/
uint8_t qcril_arb_decide_data_rat_to_report(uint32_t *index)
{
    uint8_t status = FALSE;
    uint8_t dsd_rat_status = FALSE;
    uint32_t local_dsd_rat_index = QMI_RIL_ZERO;
    ImsWfcSettingsStatusInd::Status wifi_call;
    ImsWfcSettingsStatusInd::Preference   wifi_call_preference;

    qcril_qmi_nas_ims_fetch_wifi_call_info(&wifi_call, &wifi_call_preference);
    uint8_t wifi_call_valid = (wifi_call != ImsWfcSettingsStatusInd::Status::INVALID);
    uint8_t wifi_call_preference_valid = (wifi_call_preference != ImsWfcSettingsStatusInd::Preference::INVALID);
    dsd_rat_status = qcril_arb_find_index_rat_not_wlan_dsd_reported(&local_dsd_rat_index);

    QCRIL_LOG_DEBUG( "WiFi Call valid %d, WiFi Call %d", wifi_call_valid, wifi_call);
    QCRIL_LOG_DEBUG( "WiFi Call Pref valid %d, WiFi Call Pref %d", wifi_call_preference_valid, wifi_call_preference);
    QCRIL_LOG_DEBUG( "DSD valid RAT status, other than WLAN %d, RAT index %d", dsd_rat_status, local_dsd_rat_index);

    if(!wifi_call_valid || (ImsWfcSettingsStatusInd::Status::WFC_ON != wifi_call))
    {
        QCRIL_LOG_DEBUG( "WFC is NOT ON");

        if(dsd_rat_status)
        {
            status = TRUE;
            *index = local_dsd_rat_index;
        }
    }
    else if(!dsd_rat_status)
    {
        QCRIL_LOG_DEBUG( "WFC is ON and DSD reported only WLAN");
        status = TRUE;
        *index = QMI_RIL_ZERO;
    }
    else
    {
        if(wifi_call_preference_valid && ((ImsWfcSettingsStatusInd::Preference::WLAN_PREFERRED == wifi_call_preference) ||
           (ImsWfcSettingsStatusInd::Preference::WLAN_ONLY == wifi_call_preference))
          )
        {
            QCRIL_LOG_DEBUG( "WFC is ON and DSD reported WLAN as preffered tech and wfc_pref is valid one");
            status = TRUE;
            *index = QMI_RIL_ZERO;
        }
        else
        {
            QCRIL_LOG_DEBUG( "WFC is ON and DSD reported WLAN as preffered tech+otherRAT, wfc_pref is cellular");
            status = TRUE;
            *index = local_dsd_rat_index;
        }
    }

    QCRIL_LOG_INFO("Data RAT report STATUS: %d, RAT index %d", status, *index);
    return status;
}

void qcril_qmi_arb_reset_dsd_system_status_info()
{
    QCRIL_MUTEX_LOCK( &qcril_arb.mutex, "qcril_arb.mutex" );
    memset(&qcril_arb.dsd_system_status,  0, sizeof(qcril_arb.dsd_system_status));
    QCRIL_MUTEX_UNLOCK( &qcril_arb.mutex, "qcril_arb.mutex" );
}
