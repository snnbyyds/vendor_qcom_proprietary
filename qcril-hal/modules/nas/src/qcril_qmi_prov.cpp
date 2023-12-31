/******************************************************************************
#  Copyright (c) 2015, 2017 - 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/******************************************************************************
#  @file    qcril_qmi_prov.c
#  @brief   Manual Provisioning and MSIM feature
#
#******************************************************************************/

#include "telephony/ril.h"
#include "IxErrno.h"
#include "comdef.h"
#include "qcrili.h"
#include "qcril_log.h"
#include "qcril_reqlist.h"
#include "qcril_qmi_client.h"
#include "qmi_ril_platform_dep.h"
#include "qcril_qmi_oem_events.h"
#include <QtiMutex.h>
#include "modules/nas/qcril_db.h"
#include "modules/nas/NasModule.h"
#include "interfaces/nas/nas_types.h"
#include "modules/nas/qcril_qmi_nas.h"
#include "modules/nas/qcril_qmi_prov.h"
#include "modules/nas/qcril_nas_legacy.h"
#include "modules/uim/UimGetCardStatusRequestSyncMsg.h"
#include "modules/nas/NasDataCallbacks.h"
#include "interfaces/nas/RilRequestEnableUiccAppMessage.h"
#include "interfaces/nas/RilRequestGetUiccAppStatusMessage.h"
#include "interfaces/nas/RilUnsolUiccAppsStatusChangedMessage.h"

qcril_qmi_prov_common_type prov_common_cache;

/*=========================================================================
  FUNCTION:  qcril_qmi_prov_send_unsol_uicc_applications_enablement_change

===========================================================================*/
/*!
    @brief
    Sends QCRIL_EVT_HOOK_UNSOL_SUB_PROVISION_STATUS to telephony.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_prov_send_unsol_uicc_applications_enablement_change()
{
    bool status = false;
    int state = QCRIL_INVALID_PROV_PREF;

    QCRIL_LOG_FUNC_ENTRY();

    state = qcril_qmi_prov_get_curr_sub_prov_status();
    QCRIL_LOG_INFO( "uicc applications status - %d", state);

    status = (state == UIM_UICC_SUBSCRIPTION_ACTIVATE) ? true : false;
    auto msg = std::make_shared<RilUnsolUiccAppsStatusChangedMessage>(status);
    Dispatcher::getInstance().dispatchSync(msg);

    QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_prov_send_unsol_sub_pref_change

===========================================================================*/
/*!
    @brief
    Sends QCRIL_EVT_HOOK_UNSOL_SUB_PROVISION_STATUS to telephony.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_prov_send_unsol_sub_pref_change()
{
    RIL_SubProvStatus resp_payload;

    QCRIL_LOG_FUNC_ENTRY();

    qcril_qmi_prov_fill_prov_preference_info(&resp_payload);

    QCRIL_LOG_INFO( "User pref %d", resp_payload.user_preference);
    QCRIL_LOG_INFO( "Current sub pref %d", resp_payload.current_sub_preference);
    qcril_hook_unsol_response( QCRIL_DEFAULT_INSTANCE_ID,
                               QCRIL_EVT_HOOK_UNSOL_SUB_PROVISION_STATUS,
                               (char *)&resp_payload,
                               sizeof(resp_payload));

    QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_prov_get_sim_iccid_req_handler

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_HOOK_GET_SIM_ICCID

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_prov_get_sim_iccid_req_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
    char                               iccid[QMI_DMS_UIM_ID_MAX_V01 + 1];
    RIL_Errno                          ril_req_res = RIL_E_SUCCESS;
    qcril_request_resp_params_type     resp;
    QCRIL_NOTUSED(ret_ptr);

    QCRIL_LOG_FUNC_ENTRY();

    memset(iccid, 0, sizeof(iccid));
    qcril_qmi_prov_get_iccid_from_cache(iccid);

    QCRIL_LOG_INFO("iccid - %s",iccid);
    qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                       params_ptr->t,
                                       params_ptr->event_id,
                                       ril_req_res,
                                       &resp );

    resp.resp_pkt = iccid;
    resp.resp_len = strlen(iccid);
    qcril_send_request_response( &resp );

    QCRIL_LOG_FUNC_RETURN();
}


/*=========================================================================
  FUNCTION:  qcril_qmi_prov_get_sub_prov_pref_req_handler

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_HOOK_GET_SUB_PROVISION_PREFERENCE_REQ

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_prov_get_sub_prov_pref_req_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
    RIL_Errno                             ril_req_res = RIL_E_SUCCESS;
    RIL_SubProvStatus                     resp_payload;
    qcril_request_resp_params_type        resp;

    QCRIL_NOTUSED( ret_ptr );

    QCRIL_LOG_FUNC_ENTRY();

    memset(&resp_payload, 0, sizeof(resp_payload));

    qcril_qmi_prov_fill_prov_preference_info(&resp_payload);

    QCRIL_LOG_INFO( "User pref %d", resp_payload.user_preference);
    QCRIL_LOG_INFO( "Current sub pref %d", resp_payload.current_sub_preference);

    qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                       params_ptr->t,
                                       params_ptr->event_id,
                                       ril_req_res,
                                       &resp );
    resp.resp_pkt = &resp_payload;
    resp.resp_len = sizeof(resp_payload);
    qcril_send_request_response( &resp );

    QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_prov_handle_prov_state_change
===========================================================================*/
/*!
    @brief
    Post internal event to handle different states in provisioning module.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_prov_handle_prov_state_change(qcril_qmi_prov_state_type state)
{
    QCRIL_LOG_FUNC_ENTRY();

    qcril_event_queue(QCRIL_DEFAULT_INSTANCE_ID,
                      QCRIL_DEFAULT_MODEM_ID,
                      QCRIL_DATA_ON_STACK,
                      QCRIL_EVT_RIL_REQUEST_MANUAL_PROVISIONING,
                      &state,
                      sizeof(state),
                      (RIL_Token) QCRIL_SUB_PROVISION_INTERNAL_TOKEN);

    QCRIL_LOG_FUNC_RETURN();
}
/*=========================================================================
  FUNCTION:  qcril_qmi_prov_get_sub_prov_pref_req_handler

===========================================================================*/
void qcril_qmi_prov_handle_get_sim_enable_status(
    std::shared_ptr<RilRequestGetUiccAppStatusMessage> msg)
{
    bool status = false;
    int state = QCRIL_INVALID_PROV_PREF;
    RIL_Errno res = RIL_E_SUCCESS;

    QCRIL_LOG_FUNC_ENTRY();

    state = qcril_qmi_prov_get_curr_sub_prov_status();
    QCRIL_LOG_INFO( "Send response to telephony, res - %d, sim activation status - %d", (int)res, state);

    std::shared_ptr<qcril::interfaces::RilGetUiccAppStatusResult_t> payload = nullptr;
    if (state != QCRIL_INVALID_PROV_PREF) {
        status = (state == UIM_UICC_SUBSCRIPTION_ACTIVATE) ? true : false;
        payload = std::make_shared<qcril::interfaces::RilGetUiccAppStatusResult_t>(status);
    } else {
        res = RIL_E_GENERIC_FAILURE;
    }
    auto respData = std::make_shared<QcRilRequestMessageCallbackPayload>(
        res, payload);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respData);

    QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_prov_handle_sim_enable_request

===========================================================================*/
void qcril_qmi_prov_handle_sim_enable_request(
    std::shared_ptr<RilRequestEnableUiccAppMessage> msg)
{
    bool enable = false;
    qcril_qmi_prov_state_type prov_state;
    RIL_Errno res = RIL_E_SUCCESS;
    qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
    uint32 user_data;

    QCRIL_LOG_FUNC_ENTRY();

    do
    {
        // Insert to pending list only if request is successfully sent to modem
        auto& msgList = getNasModule().getPendingMessageList();
        auto pendingMsgStatus = msgList.insert(msg);
        uint16_t req_id = pendingMsgStatus.first;
        user_data = QCRIL_COMPOSE_USER_DATA(instance_id, QCRIL_DEFAULT_MODEM_ID, req_id);
        if (pendingMsgStatus.second != true) {
            QCRIL_LOG_INFO( "failed to create pending message list");
            res = RIL_E_GENERIC_FAILURE;
            break;
        }

        enable = msg->getEnableUiccApp();
        prov_state = enable ? QCRIL_QMI_PROV_STATE_USER_ACTIVATE :
            QCRIL_QMI_PROV_STATE_USER_DEACTIVATE;
        qcril_qmi_prov_backup_iccid_for_prov();
        if (qcril_qmi_nas_is_prov_in_progress() == false) {
            qcril_qmi_prov_handle_prov_state_change(prov_state);
        } else {
            QCRIL_LOG_DEBUG("Do not send request as provisioning is pending");
        }

    } while(FALSE);

    if (res != RIL_E_SUCCESS) {
        QCRIL_LOG_ERROR( "Send response to telephony, res - %d", (int)res);

        auto respData = std::make_shared<QcRilRequestMessageCallbackPayload>(
            res, nullptr);
        msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respData);
    }

    QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_prov_set_sub_prov_pref_req_handler

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_HOOK_SET_SUB_PROVISION_PREFERENCE_REQ

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_prov_set_sub_prov_pref_req_handler
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
    qcril_evt_e_type                      pending_event;
    RIL_Errno                             ril_req_res = RIL_E_SUCCESS;
    qcril_qmi_prov_state_type             prov_state;
    RIL_SetSubProvPreference              *prov_pref;
    qcril_reqlist_public_type             qcril_req_info;
    qcril_request_resp_params_type        resp;

    QCRIL_NOTUSED( ret_ptr );

    QCRIL_LOG_FUNC_ENTRY();

    if(NULL != params_ptr->data && params_ptr->datalen > QMI_RIL_ZERO )
    {
        do
        {
            memset(&resp, 0, sizeof(resp));
            memset(&qcril_req_info, 0, sizeof(qcril_req_info));
            prov_pref = (RIL_SetSubProvPreference *)params_ptr->data;
            QCRIL_LOG_INFO("USER %s", prov_pref->act_status ? "ACTIVATE":"DEACTIVATE");

            /* Set state to USER_ACTIVATE/USER_DEACTIVATE and wait for event
            ** QCRIL_EVT_QMI_PROV_ACTIVATE_SUB_STATUS/QCRIL_EVT_QMI_PROV_DEACTIVATE_SUB_STATUS.
            ** As part of above events handling, update user preference in database and send
            ** response to telephony.
            */
            if ( prov_pref->act_status == UIM_UICC_SUBSCRIPTION_ACTIVATE )
            {
                pending_event = QCRIL_EVT_QMI_PROV_ACTIVATE_SUB_STATUS;
                prov_state = QCRIL_QMI_PROV_STATE_USER_ACTIVATE;
            }
            else
            {
                pending_event = QCRIL_EVT_QMI_PROV_DEACTIVATE_SUB_STATUS;
                prov_state = QCRIL_QMI_PROV_STATE_USER_DEACTIVATE;
            }

            qcril_reqlist_default_entry( params_ptr->t,
                                         params_ptr->event_id,
                                         QCRIL_DEFAULT_MODEM_ID,
                                         QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                                         pending_event,
                                         NULL,
                                         &qcril_req_info );

            if ( qcril_reqlist_new( QCRIL_DEFAULT_INSTANCE_ID, &qcril_req_info ) == E_SUCCESS )
            {
                qcril_qmi_prov_backup_iccid_for_prov();
                if (qcril_qmi_nas_is_prov_in_progress() == false) {
                    qcril_qmi_prov_handle_prov_state_change(prov_state);
                } else {
                    QCRIL_LOG_DEBUG("Do not send request as provisioning is pending");
                }
            }
            else
            {
                QCRIL_LOG_ERROR("Failed to add entry to reqlist..");
                ril_req_res = RIL_E_GENERIC_FAILURE;
                break;
            }

        } while(FALSE);
    }

    if ((ril_req_res != RIL_E_SUCCESS))
    {
        QCRIL_LOG_ESSENTIAL("res %d", ril_req_res);
        qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                           params_ptr->t,
                                           params_ptr->event_id,
                                           ril_req_res,
                                           &resp );
        qcril_send_request_response( &resp );
    }

    QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_nas_prov_activate_sub_status_hndlr

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_QMI_PROV_ACTIVATE_SUB_STATUS

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_nas_prov_activate_sub_status_hndlr
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
)
{
    RIL_Errno                        res = RIL_E_GENERIC_FAILURE;
    int                              found_req_res = RIL_E_GENERIC_FAILURE;
    char                             iccid[QMI_DMS_UIM_ID_MAX_V01+1];
    qcril_reqlist_public_type        req_info;
    qcril_request_resp_params_type   resp;
    QCRIL_NOTUSED(ret_ptr);

    QCRIL_LOG_FUNC_ENTRY();

    memset(&resp, 0, sizeof(resp));
    memset(&req_info, 0, sizeof(req_info));
    memset(&iccid, 0, sizeof(iccid));

    if ( params_ptr->data != NULL && params_ptr->datalen > 0 )
    {
        res = (RIL_Errno)*(int*) params_ptr->data;

        qcril_qmi_prov_fetch_backup_iccid_and_reset(iccid);
        if ( res == RIL_E_SUCCESS )
        {
            /* Request is from user and response is success,
            ** so update data base with user preference.
            */
            qcril_qmi_prov_update_db_with_user_pref(UIM_UICC_SUBSCRIPTION_ACTIVATE,
                    iccid);
        }

        // check if this is user trigger request received from IRadio
        auto& msgList = getNasModule().getPendingMessageList();
        auto pendingMsg = msgList.find(RilRequestEnableUiccAppMessage::get_class_message_id());
        if (pendingMsg)
        {
            auto msg = std::static_pointer_cast<RilRequestEnableUiccAppMessage>(pendingMsg);
            //Activation triggerd through IRadio interface.
            auto respData = std::make_shared<QcRilRequestMessageCallbackPayload>(
                    res, nullptr);
            msgList.erase(pendingMsg);
            msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respData);
            if (res == RIL_E_SUCCESS)
            {
                qcril_qmi_prov_send_unsol_uicc_applications_enablement_change();
            }
        }
        else
        {
            found_req_res = qcril_reqlist_query_by_event( QCRIL_DEFAULT_INSTANCE_ID,
                                                          QCRIL_DEFAULT_MODEM_ID,
                                                          QCRIL_EVT_QMI_PROV_ACTIVATE_SUB_STATUS,
                                                          &req_info );

            QCRIL_LOG_INFO("found req - %d, res - %d", found_req_res, res );

            if ( found_req_res == RIL_E_SUCCESS )
            {
                qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                                   req_info.t,
                                                   req_info.request,
                                                   res,
                                                   &resp );
                qcril_send_request_response( &resp );
                qcril_qmi_prov_send_unsol_sub_pref_change();
            }
        }

    }

    QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_nas_prov_deactivate_sub_status_hndlr

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_QMI_PROV_DEACTIVATE_SUB_STATUS

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_nas_prov_deactivate_sub_status_hndlr
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
)
{
    RIL_Errno                        res = RIL_E_GENERIC_FAILURE;
    int                              found_req_res = RIL_E_GENERIC_FAILURE;
    char                             iccid[QMI_DMS_UIM_ID_MAX_V01+1];
    qcril_reqlist_public_type        req_info;
    qcril_request_resp_params_type   resp;
    QCRIL_NOTUSED(ret_ptr);

    QCRIL_LOG_FUNC_ENTRY();


    memset(&resp, 0, sizeof(resp));
    memset(&req_info, 0, sizeof(req_info));
    memset(&iccid, 0, sizeof(iccid));

    if ( params_ptr->data != NULL && params_ptr->datalen > 0 )
    {
        res = (RIL_Errno)*(int*) params_ptr->data;

        qcril_qmi_prov_fetch_backup_iccid_and_reset(iccid);
        if ( res == RIL_E_SUCCESS )
        {
            /* Request is from user and response is success,
            ** so update data base with user preference.
            */
            qcril_qmi_prov_update_db_with_user_pref(UIM_UICC_SUBSCRIPTION_DEACTIVATE,
                    iccid);
        }

        // Check if this is user trigger request received from IRadio
        auto& msgList = getNasModule().getPendingMessageList();
        auto pendingMsg = msgList.find(RilRequestEnableUiccAppMessage::get_class_message_id());
        if (pendingMsg)
        {
            auto msg = std::static_pointer_cast<RilRequestEnableUiccAppMessage>(pendingMsg);
            //Deactivation of SIM card triggerd through IRadio interface.
            auto respData = std::make_shared<QcRilRequestMessageCallbackPayload>(
                    res, nullptr);
            msgList.erase(pendingMsg);
            msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respData);
            if (res == RIL_E_SUCCESS)
            {
                qcril_qmi_prov_send_unsol_uicc_applications_enablement_change();
            }
        }
        else
        {
            // Check if deactivation is from OEMHOOK
            found_req_res = qcril_reqlist_query_by_event( QCRIL_DEFAULT_INSTANCE_ID,
                                                          QCRIL_DEFAULT_MODEM_ID,
                                                          QCRIL_EVT_QMI_PROV_DEACTIVATE_SUB_STATUS,
                                                          &req_info );

            QCRIL_LOG_INFO("found req - %d, res - %d", found_req_res, res );
            if ( found_req_res == RIL_E_SUCCESS )
            {
               qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID,
                                                   req_info.t,
                                                   req_info.request,
                                                   res,
                                                   &resp );
                qcril_send_request_response( &resp );
                qcril_qmi_prov_send_unsol_sub_pref_change();
            }
        }

    }

    QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_prov_manual_provisioning_hndlr

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_RIL_REQUEST_MANUAL_PROVISIONING.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_prov_manual_provisioning_hndlr
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr // Output parameter
)
{
    int                              send_resp = TRUE;
    int                              apps_prov = FALSE;
    int                              apps_deprov = FALSE;
    qcril_qmi_prov_action_type       prov_action = QCRIL_QMI_PROV_ACTION_NONE;
    RIL_Errno                        res = RIL_E_SUCCESS;
    RIL_UIM_UiccSubActStatus         act_status;
    qcril_qmi_prov_state_type        prop_state = QCRIL_QMI_PROV_STATE_NONE;
    qcril_uim_subs_mode_pref         mode_pref = QCRIL_SUBS_MODE_UNKNOWN;
    QCRIL_NOTUSED(ret_ptr);

    QCRIL_LOG_FUNC_ENTRY();

    if ( params_ptr->datalen > 0 && params_ptr->data != NULL )
    {
        do
        {
            prop_state = *(qcril_qmi_prov_state_type*)params_ptr->data;

            QCRIL_LOG_INFO("proposed provisioning state - %d", prop_state);
            /* Update provisioning state machine with proposed state */
            res = qcril_qmi_prov_set_prov_state(prop_state, &prov_action);
            if ( res != RIL_E_SUCCESS || prop_state == QCRIL_QMI_PROV_STATE_NONE )
            {
                /* If defer flag is set, don't send response. */
                if ( prov_action == QCRIL_QMI_PROV_ACTION_DEFER )
                {
                    QCRIL_LOG_INFO("Differ this request as already manual provisioning in-progress");
                    send_resp = FALSE;
                }
                else if ( prop_state == QCRIL_QMI_PROV_STATE_NONE )
                {
                    QCRIL_LOG_INFO("request is to just to reset the status...");
                    send_resp = FALSE;
                }
                break;
            }

            /* Based on current state, mark applications as pending prov/deprov */
            if ( RIL_E_SUCCESS != qcril_qmi_prov_evaluate_and_mark_apps() )
            {
                res = RIL_E_GENERIC_FAILURE;
                break;
            }

            act_status = qcril_qmi_prov_get_act_status_from_state(prop_state);

            /* First send request for one app. Once activation response from UIM
            ** is received, send another request to UIM for second app if present. */
            if ( act_status == UIM_UICC_SUBSCRIPTION_ACTIVATE )
             {
                 qcril_qmi_prov_get_sub_mode_pref_for_activation(&mode_pref);
             }
             else
             {
                 qcril_qmi_prov_get_sub_mode_pref_for_deactivation(&mode_pref);
             }

             if ( mode_pref != QCRIL_SUBS_MODE_UNKNOWN )
             {
                 res = qcril_qmi_prov_activate_deactivate_app( act_status, mode_pref);
                 if ( res == RIL_E_SUCCESS )
                 {
                     send_resp = FALSE;
                 }
             }
             else
             {
                 QCRIL_LOG_INFO("Checking if all apps are activated/deactivated.");
                 if ( act_status == UIM_UICC_SUBSCRIPTION_ACTIVATE )
                 {
                     apps_prov = qcril_qmi_prov_all_apps_provisioned();
                 }
                 else
                 {
                     apps_deprov = qcril_qmi_prov_all_apps_deprovisioned();
                 }

                 /* If all apps are activate/deactivated send success response */
                 if ( apps_prov || apps_deprov )
                 {
                     qcril_qmi_prov_set_curr_sub_prov_status(act_status);
                 }
                 else
                 {
                     res = RIL_E_GENERIC_FAILURE;
                     QCRIL_LOG_INFO("No app available on SIM card or SIM card is absent");
                 }
             }

        } while ( FALSE );
    }

    /* If activation/deactivation request is not sent to UIM, send response here */
    if ( send_resp == TRUE )
    {
        qcril_qmi_prov_check_and_send_prov_resp(res, prop_state);
    }
}

/*=========================================================================
  FUNCTION:  qcril_qmi_prov_activate_deactivate_app

===========================================================================*/
/*!
    @brief
    Core function of provisioning module.
    Sends request to UIM for activation/deactivation.

    @return
    None.
*/
/*=========================================================================*/
RIL_Errno qcril_qmi_prov_activate_deactivate_app
(
    RIL_UIM_UiccSubActStatus act_status,
    qcril_uim_subs_mode_pref mode_pref
)
{
    RIL_Errno                        ril_req_res = RIL_E_SUCCESS;
    RIL_UIM_SelectUiccSub            select_uicc_sub;
    qcril_uim_uicc_subs_info_type    subs_info;

    memset( &subs_info, NIL, sizeof(subs_info));
    memset( &select_uicc_sub, NIL, sizeof(select_uicc_sub) );

    /* fill data required for UIM to activate/deactivate. */
    select_uicc_sub.slot       = qmi_ril_get_sim_slot();
    select_uicc_sub.sub_type   = (RIL_UIM_SubscriptionType) qcril_qmi_get_modem_stack_id(); // TODO map??
    select_uicc_sub.act_status = act_status;

    subs_info.subs_mode_pref = mode_pref;
    memcpy( &subs_info.uicc_subs_info, &select_uicc_sub, sizeof( RIL_UIM_SelectUiccSub ) );

    std::shared_ptr<UimChangeSubscriptionReqMsg> req =
            std::make_shared<UimChangeSubscriptionReqMsg>(subs_info);

    if(req == nullptr)
    {
        QCRIL_LOG_DEBUG( "Failed in resource allocation for act status: %d", act_status);
        ril_req_res = RIL_E_GENERIC_FAILURE;
    }

    /* if success, update app status from
    ** pending provisioning/pending deprovisioning -> provisioning/deprovisioning
    ** if failure, update app status to not_provisioned
    */
    qcril_qmi_prov_update_app_provision_status(
                            select_uicc_sub.act_status,
                            ril_req_res, ril_req_res);

    if(req != nullptr)
    {
        if(act_status == UIM_UICC_SUBSCRIPTION_ACTIVATE)
        {
            QCRIL_LOG_DEBUG( "Activate sub: slot %d subs_mode_pref %d", select_uicc_sub.slot, mode_pref);
            NasProvRequestActivateCallback cb("set-cb-token", (uint32_t)(-1));
            req->setCallback(&cb);
        }
        else
        {
            QCRIL_LOG_DEBUG( "Deactivate sub: slot %d subs_mode_pref %d", select_uicc_sub.slot, mode_pref);
            NasProvRequestDeactivateCallback cb("set-cb-token", (uint32_t)(-1));
            req->setCallback(&cb);
        }
        req->dispatch();
    }

    QCRIL_LOG_INFO("completed with %d", (int) ril_req_res);
    return ril_req_res;
}


/*=========================================================================
  FUNCTION:  qcril_qmi_prov_all_apps_deprovisioned

===========================================================================*/
/*!
    @brief
    Check if all SIM applications are deprovisioned.

    @return
    None.
*/
/*=========================================================================*/
int qcril_qmi_prov_all_apps_deprovisioned(void)
{
  int res                                              = FALSE;
  qcril_card_app_provision_status gw_provision_state   = QCRIL_APP_STATUS_NOT_PROVISIONED;
  qcril_card_app_provision_status cdma_provision_state = QCRIL_APP_STATUS_NOT_PROVISIONED;

  QCRIL_LOG_FUNC_ENTRY();

  gw_provision_state   = qcril_qmi_prov_get_gw_provision_state();
  cdma_provision_state = qcril_qmi_prov_get_cdma_provision_state();

  QCRIL_LOG_INFO("gw_provision_state: %d cdma_provision_state: %d",
                                                gw_provision_state,
                                                cdma_provision_state);

  if ( ( (gw_provision_state   == QCRIL_APP_STATUS_DEPROVISIONED)  &&
         (cdma_provision_state == QCRIL_APP_STATUS_DEPROVISIONED) ) ||
       ( (gw_provision_state   == QCRIL_APP_STATUS_DEPROVISIONED) &&
         (cdma_provision_state == QCRIL_APP_STATUS_NOT_PROVISIONED ) ) ||
       ( (cdma_provision_state == QCRIL_APP_STATUS_DEPROVISIONED) &&
         (gw_provision_state   == QCRIL_APP_STATUS_NOT_PROVISIONED ) ) )
  {
    res = TRUE;
  }
  else
  {
    res = FALSE;
  }
  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
  return res;
}

/*=========================================================================
  FUNCTION:  qcril_qmi_prov_all_apps_provisioned

===========================================================================*/
/*!
    @brief
    Check if all SIM applications are provisioned.

    @return
    None.
*/
/*=========================================================================*/

int qcril_qmi_prov_all_apps_provisioned(void)
{
  int res                                              = FALSE;
  qcril_card_app_provision_status gw_provision_state   = QCRIL_APP_STATUS_NOT_PROVISIONED;
  qcril_card_app_provision_status cdma_provision_state = QCRIL_APP_STATUS_NOT_PROVISIONED;

  QCRIL_LOG_FUNC_ENTRY();

  gw_provision_state   = qcril_qmi_prov_get_gw_provision_state();
  cdma_provision_state = qcril_qmi_prov_get_cdma_provision_state();

  QCRIL_LOG_INFO("gw_provision_state: %d cdma_provision_state: %d",
                                                gw_provision_state,
                                                cdma_provision_state);

  /* In case of failure as well app provision state will be not provisioned */
  if ( ( (gw_provision_state   == QCRIL_APP_STATUS_PROVISIONED) &&
         (cdma_provision_state == QCRIL_APP_STATUS_PROVISIONED)) ||
       ( (gw_provision_state   == QCRIL_APP_STATUS_PROVISIONED) &&
         (cdma_provision_state == QCRIL_APP_STATUS_NOT_PROVISIONED ) ) ||
       ( (cdma_provision_state == QCRIL_APP_STATUS_PROVISIONED) &&
         (gw_provision_state   == QCRIL_APP_STATUS_NOT_PROVISIONED ) ) )
  {
    res = TRUE;
  }
  else
  {
    res = FALSE;
  }
  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
  return res;
}

/*=========================================================================
  FUNCTION:  qcril_qmi_prov_is_any_app_being_provisioned

===========================================================================*/
/*!
    @brief
    Check if any SIM application is in process of provisioning.

    @return
    None.
*/
/*=========================================================================*/
int qcril_qmi_prov_is_any_app_being_provisioned(void)
{
  int res                                              = FALSE;
  qcril_card_app_provision_status gw_provision_state   = QCRIL_APP_STATUS_NOT_PROVISIONED;
  qcril_card_app_provision_status cdma_provision_state = QCRIL_APP_STATUS_NOT_PROVISIONED;

  QCRIL_LOG_FUNC_ENTRY();

  gw_provision_state   = qcril_qmi_prov_get_gw_provision_state();
  cdma_provision_state = qcril_qmi_prov_get_cdma_provision_state();

  QCRIL_LOG_INFO("gw_provision_state: %d cdma_provision_state: %d",
                                                gw_provision_state,
                                                cdma_provision_state);

  if ((gw_provision_state   == QCRIL_APP_STATUS_PROVISIONING) ||
      (cdma_provision_state == QCRIL_APP_STATUS_PROVISIONING) ||
      (gw_provision_state   == QCRIL_APP_STATUS_PENDING_PROVISIONING) ||
      (cdma_provision_state == QCRIL_APP_STATUS_PENDING_PROVISIONING) )
  {
    res = TRUE;
  }
  else
  {
    res = FALSE;
  }
  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
  return res;
}

/*=========================================================================
  FUNCTION:  qcril_qmi_prov_is_any_app_being_deprovisioned

===========================================================================*/
/*!
    @brief
    Check if any SIM application is in process of deprovisioning.

    @return
    None.
*/
/*=========================================================================*/
int qcril_qmi_prov_is_any_app_being_deprovisioned(void)
{
  int res                                              = FALSE;
  qcril_card_app_provision_status gw_provision_state   = QCRIL_APP_STATUS_NOT_PROVISIONED;
  qcril_card_app_provision_status cdma_provision_state = QCRIL_APP_STATUS_NOT_PROVISIONED;

  QCRIL_LOG_FUNC_ENTRY();

  gw_provision_state   = qcril_qmi_prov_get_gw_provision_state();
  cdma_provision_state = qcril_qmi_prov_get_cdma_provision_state();

  QCRIL_LOG_INFO("gw_provision_state: %d cdma_provision_state: %d",
                gw_provision_state,
                cdma_provision_state);
  if ((gw_provision_state   == QCRIL_APP_STATUS_DEPROVISIONING) ||
      (cdma_provision_state == QCRIL_APP_STATUS_DEPROVISIONING) ||
      (gw_provision_state   == QCRIL_APP_STATUS_PENDING_DEPROVISIONING) ||
      (cdma_provision_state == QCRIL_APP_STATUS_PENDING_DEPROVISIONING) )
  {
    res = TRUE;
  }
  else
  {
    res = FALSE;
  }
  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
  return res;
}

/*=========================================================================
  FUNCTION:  qcril_qmi_prov_is_any_app_provisioned

===========================================================================*/
/*!
    @brief
    Check if any SIM application provisioned.

    @return
    None.
*/
/*=========================================================================*/
int qcril_qmi_prov_is_any_app_provisioned()
{
  int res                                              = FALSE;
  qcril_card_app_provision_status gw_provision_state   = QCRIL_APP_STATUS_NOT_PROVISIONED;
  qcril_card_app_provision_status cdma_provision_state = QCRIL_APP_STATUS_NOT_PROVISIONED;

  QCRIL_LOG_FUNC_ENTRY();

  gw_provision_state   = qcril_qmi_prov_get_gw_provision_state();
  cdma_provision_state = qcril_qmi_prov_get_cdma_provision_state();

  QCRIL_LOG_INFO("gw_provision_state: %d cdma_provision_state: %d",
                                                 gw_provision_state,
                                                 cdma_provision_state);

  if ( (gw_provision_state   == QCRIL_APP_STATUS_PROVISIONED) ||
       (cdma_provision_state == QCRIL_APP_STATUS_PROVISIONED) )
  {
    res = TRUE;
  }
  else
  {
    res = FALSE;
  }

  QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
  return res;
}

/*===========================================================================

    qcril_qmi_prov_get_sub_mode_pref_for_activation

============================================================================*/
/*!
    @brief
    Get sub mode pref for activation.

    @return
    None
*/
/*=========================================================================*/
void qcril_qmi_prov_get_sub_mode_pref_for_activation(qcril_uim_subs_mode_pref *mode_pref)
{
    int gw_app_index                                     = QCRIL_INVALID_APP_INDEX;
    int cdma_app_index                                   = QCRIL_INVALID_APP_INDEX;
    qcril_card_app_provision_status gw_provision_state   = QCRIL_APP_STATUS_NOT_PROVISIONED;
    qcril_card_app_provision_status cdma_provision_state = QCRIL_APP_STATUS_NOT_PROVISIONED;

    QCRIL_LOG_FUNC_ENTRY();

    gw_provision_state   = qcril_qmi_prov_get_gw_provision_state();
    cdma_provision_state = qcril_qmi_prov_get_cdma_provision_state();

    if ( gw_provision_state == QCRIL_APP_STATUS_PENDING_PROVISIONING )
    {
        gw_app_index = qcril_qmi_prov_get_gw_app_index();
    }
    if ( cdma_provision_state == QCRIL_APP_STATUS_PENDING_PROVISIONING )
    {
        cdma_app_index = qcril_qmi_prov_get_cdma_app_index();
    }

    if ( ( gw_app_index != QCRIL_INVALID_APP_INDEX ) && ( cdma_app_index != QCRIL_INVALID_APP_INDEX ) )
    {
        *mode_pref = QCRIL_SUBS_MODE_GW_1X;
    }
    else if( gw_app_index != QCRIL_INVALID_APP_INDEX )
    {
        *mode_pref = QCRIL_SUBS_MODE_GW;
    }
    else if( cdma_app_index != QCRIL_INVALID_APP_INDEX )
    {
        *mode_pref = QCRIL_SUBS_MODE_1X;
    }
    else
    {
        *mode_pref = QCRIL_SUBS_MODE_UNKNOWN;
    }

    QCRIL_LOG_INFO("sub mode pref for activation: %d", *mode_pref);

    return;
}

/*===========================================================================

    qcril_qmi_prov_get_sub_mode_pref_for_deactivation

============================================================================*/
/*!
    @brief
    Get sub mode pref for deactivation.

    @return
    None
*/
/*=========================================================================*/
void qcril_qmi_prov_get_sub_mode_pref_for_deactivation(qcril_uim_subs_mode_pref *mode_pref)
{
    int gw_app_index                                     = QCRIL_INVALID_APP_INDEX;
    int cdma_app_index                                   = QCRIL_INVALID_APP_INDEX;
    qcril_card_app_provision_status gw_provision_state   = QCRIL_APP_STATUS_NOT_PROVISIONED;
    qcril_card_app_provision_status cdma_provision_state = QCRIL_APP_STATUS_NOT_PROVISIONED;

    QCRIL_LOG_FUNC_ENTRY();

    gw_provision_state   = qcril_qmi_prov_get_gw_provision_state();
    cdma_provision_state = qcril_qmi_prov_get_cdma_provision_state();

    if ( cdma_provision_state == QCRIL_APP_STATUS_PENDING_DEPROVISIONING )
    {
        cdma_app_index = qcril_qmi_prov_get_cdma_app_index();
    }
    if ( gw_provision_state == QCRIL_APP_STATUS_PENDING_DEPROVISIONING )
    {
        gw_app_index = qcril_qmi_prov_get_gw_app_index();
    }

    if ( ( gw_app_index != QCRIL_INVALID_APP_INDEX ) && ( cdma_app_index != QCRIL_INVALID_APP_INDEX ) )
    {
        *mode_pref = QCRIL_SUBS_MODE_GW_1X;
    }
    else if( gw_app_index != QCRIL_INVALID_APP_INDEX )
    {
        *mode_pref = QCRIL_SUBS_MODE_GW;
    }
    else if( cdma_app_index != QCRIL_INVALID_APP_INDEX )
    {
        *mode_pref = QCRIL_SUBS_MODE_1X;
    }
    else
    {
        *mode_pref = QCRIL_SUBS_MODE_UNKNOWN;
    }

    QCRIL_LOG_INFO("sub mode pref for deactivation: %d", *mode_pref);

    return;
}

/*===========================================================================

    qcril_qmi_prov_check_and_send_prov_resp

============================================================================*/
/*!
    @brief
    Send appropriate response based on current prov state.

    @return
    None
*/
/*=========================================================================*/
void qcril_qmi_prov_check_and_send_prov_resp(RIL_Errno res, qcril_qmi_prov_state_type state)
{
    qcril_evt_e_type               event = (qcril_evt_e_type)0;
    int                            send_event = TRUE;
    qcril_qmi_prov_state_type      curr_state = QCRIL_QMI_PROV_STATE_NONE;
    qcril_qmi_prov_status_type     prov_status;

    QCRIL_LOG_FUNC_ENTRY();

    memset(&prov_status, NIL, sizeof(prov_status));
    curr_state = qcril_qmi_prov_get_prov_state();
    switch ( state )
    {
        case QCRIL_QMI_PROV_STATE_FM_START:
            event = QCRIL_EVT_QMI_PROV_FM_START_STATUS;
            break;

        case QCRIL_QMI_PROV_STATE_FM_APPLY:
            event = QCRIL_EVT_QMI_PROV_FM_APPLY_STATUS;
            break;

        case QCRIL_QMI_PROV_STATE_USER_ACTIVATE:
            event = QCRIL_EVT_QMI_PROV_ACTIVATE_SUB_STATUS;
            break;

        case QCRIL_QMI_PROV_STATE_USER_DEACTIVATE:
            event = QCRIL_EVT_QMI_PROV_DEACTIVATE_SUB_STATUS;
            break;

        case QCRIL_QMI_PROV_STATE_CARD_UP:
            event = QCRIL_EVT_QMI_PROV_STATUS_CHANGED;
            break;

        default:
            send_event = FALSE;
            break;
    }

    /* If state is FM_START is success, wait for FM_APPLY before changing state to NONE.
    ** In all other cases move to NONE. */
    if ( (state == curr_state) &&
        !( (state == QCRIL_QMI_PROV_STATE_FM_START) && (res == RIL_E_SUCCESS) ) )
    {
        qcril_qmi_prov_set_prov_state(QCRIL_QMI_PROV_STATE_NONE, NULL);
    }

    QCRIL_LOG_INFO("state %d, event to post %d, should send_event %d", state, event, send_event);
    if ( send_event == TRUE )
    {
        qcril_event_queue ( QCRIL_DEFAULT_INSTANCE_ID,
                            QCRIL_DEFAULT_MODEM_ID,
                            QCRIL_DATA_ON_STACK,
                            event,
                            &res,
                            sizeof(res),
                            (RIL_Token)QCRIL_TOKEN_ID_INTERNAL );
    }

    QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

    qcril_qmi_prov_evaluate_and_mark_apps_to_deactivate

============================================================================*/
/*!
    @brief
    Mark applications to deactivate if they are in ready state.

    @return
    None
*/
/*=========================================================================*/
RIL_Errno qcril_qmi_prov_evaluate_and_mark_apps_to_deactivate(void)
{
    int                       app_index;
    RIL_Errno                 res = RIL_E_SUCCESS;
    qcril_instance_id_e_type  instance_id  = qmi_ril_get_process_instance_id(); //TODO: uncomment
    auto card_status = std::make_shared<UimGetCardStatusRequestSyncMsg>(instance_id);
    std::shared_ptr<RIL_UIM_CardStatus> ril_card_status = nullptr;

    /* 3gpp sim app state */
    qcril_card_app_provision_status gw_provision_state;
    /* 3gpp2 sim app state */
    qcril_card_app_provision_status cdma_provision_state;

    QCRIL_LOG_FUNC_ENTRY();

    gw_provision_state   = qcril_qmi_prov_get_gw_provision_state();
    cdma_provision_state = qcril_qmi_prov_get_cdma_provision_state();

    /* Retrieve card status info */
    if (card_status != nullptr &&
        card_status->dispatchSync(ril_card_status) == Message::Callback::Status::SUCCESS &&
        ril_card_status != nullptr &&
        ril_card_status->err == RIL_UIM_E_SUCCESS)
    {
        /* Mark 3gpp sim app */
        app_index = qcril_qm_prov_retrieve_gw_app_index(ril_card_status);
        if (app_index != QCRIL_INVALID_APP_INDEX)
        {

            QCRIL_LOG_DEBUG("3gpp app index: %d, 3gpp app state %d",
                             app_index,
                             ril_card_status->applications[app_index].app_state);

            /* Deactivate apps irrespective of current app state.
            UIM should return success immediately if app is already deactivated */
            if ( ril_card_status->applications[app_index].app_state !=
                                                RIL_UIM_APPSTATE_UNKNOWN )
            {
                qcril_qmi_prov_set_gw_app_index(app_index);
                qcril_qmi_prov_set_gw_provision_state(QCRIL_APP_STATUS_PENDING_DEPROVISIONING);
            }
        }

        /* Mark 3gpp2 sim apps */
        app_index = qcril_qmi_prov_retrieve_cdma_app_index(ril_card_status);
        if (app_index != QCRIL_INVALID_APP_INDEX)
        {
            QCRIL_LOG_DEBUG("3gpp2 app index: %d, 3gpp2 app state %d",
                            app_index,
                            ril_card_status->applications[app_index].app_state);

            /* Deactivate apps irrespective of current app state.
            UIM should return success immediately if app is already deactivated */
            if ( ril_card_status->applications[app_index].app_state !=
                                                    RIL_UIM_APPSTATE_UNKNOWN )
            {
                qcril_qmi_prov_set_cdma_app_index(app_index);
                qcril_qmi_prov_set_cdma_provision_state(QCRIL_APP_STATUS_PENDING_DEPROVISIONING);
            }
        }
    }
    else
    {
        res = RIL_E_GENERIC_FAILURE;
        QCRIL_LOG_DEBUG("Card status failed to retrieve");
    }

    QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
    return res;
}

/*===========================================================================

    qcril_qmi_prov_evaluate_and_mark_apps_to_activate

============================================================================*/
/*!
    @brief
    Mark applications to activate if they are in ready state.

    @return
    None
*/
/*=========================================================================*/
RIL_Errno qcril_qmi_prov_evaluate_and_mark_apps_to_activate(void)
{
    qcril_instance_id_e_type  instance_id  = qmi_ril_get_process_instance_id(); //TODO: uncomment
    int                       app_index;
    RIL_Errno                 res = RIL_E_SUCCESS;
    auto card_status = std::make_shared<UimGetCardStatusRequestSyncMsg>(instance_id);
    std::shared_ptr<RIL_UIM_CardStatus> ril_card_status = nullptr;

    /* 3gpp sim app state */
    qcril_card_app_provision_status gw_provision_state;
    /* 3gpp2 sim app state */
    qcril_card_app_provision_status cdma_provision_state;

    QCRIL_LOG_FUNC_ENTRY();

    gw_provision_state   = qcril_qmi_prov_get_gw_provision_state();
    cdma_provision_state = qcril_qmi_prov_get_cdma_provision_state();

    /* retrieve card status info */
    if (card_status != nullptr &&
        card_status->dispatchSync(ril_card_status) == Message::Callback::Status::SUCCESS &&
        ril_card_status != nullptr &&
        ril_card_status->err == RIL_UIM_E_SUCCESS)
    {
        /* Mark 3gpp sim app */
        if ((gw_provision_state == QCRIL_APP_STATUS_DEPROVISIONED) ||
            (gw_provision_state == QCRIL_APP_STATUS_NOT_PROVISIONED))
        {
            app_index = qcril_qm_prov_retrieve_gw_app_index(ril_card_status);
            if (app_index != QCRIL_INVALID_APP_INDEX)
            {
                QCRIL_LOG_DEBUG("3gpp app index: %d, 3gpp app state %d",
                                 app_index,
                                 ril_card_status->applications[app_index].app_state);

                /* Activate apps irrespective of current app state.
                UIM should return success immediately if app is already activated */
                if ( ril_card_status->applications[app_index].app_state !=
                                                    RIL_UIM_APPSTATE_UNKNOWN )
                {
                    qcril_qmi_prov_set_gw_app_index(app_index);
                    qcril_qmi_prov_set_gw_provision_state(QCRIL_APP_STATUS_PENDING_PROVISIONING);

                }
            }
        }
        else
        {
            QCRIL_LOG_DEBUG("3gpp app already or being provisioned");
        }

        /* Mark 3gpp2 sim app */
        if ((cdma_provision_state == QCRIL_APP_STATUS_DEPROVISIONED) ||
            (cdma_provision_state == QCRIL_APP_STATUS_NOT_PROVISIONED))
        {
            app_index = qcril_qmi_prov_retrieve_cdma_app_index(ril_card_status);
            if (app_index != QCRIL_INVALID_APP_INDEX)
            {
                QCRIL_LOG_DEBUG("3gpp2 app index: %d, 3gpp2 app state %d",
                                 app_index,
                                 ril_card_status->applications[app_index].app_state);

                /* Activate apps irrespective of current app state.
                UIM should return success immediately if app is already activated */
                if ( ril_card_status->applications[app_index].app_state !=
                                                    RIL_UIM_APPSTATE_UNKNOWN )
                {
                    qcril_qmi_prov_set_cdma_app_index(app_index);
                    qcril_qmi_prov_set_cdma_provision_state(QCRIL_APP_STATUS_PENDING_PROVISIONING);
                }
            }
        }
        else
        {
            QCRIL_LOG_DEBUG("3gpp2 app already or being provisioned");
        }
    }
    else
    {
        res = RIL_E_GENERIC_FAILURE;
        QCRIL_LOG_DEBUG("Card status failed to retrieve");
    }

    QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
    return res;
}

/*===========================================================================

    qcril_qm_prov_retrieve_gw_app_index

============================================================================*/
/*!
    @brief
    Retrieve GW app index. If there are multiple gw apps preference
    will be give to USIM.

    @return
    None
*/
/*=========================================================================*/
int qcril_qm_prov_retrieve_gw_app_index
(
    std::shared_ptr<RIL_UIM_CardStatus> ril_card_status
)
{
    int index = 0;
    int usim_index = QCRIL_INVALID_APP_INDEX;
    int sim_index = QCRIL_INVALID_APP_INDEX;
    RIL_UIM_AppStatus *app_status = NULL;

    QCRIL_LOG_FUNC_ENTRY();

    if (ril_card_status)
    {
        for (index = 0; index < RIL_UIM_CARD_MAX_APPS; index++)
        {
            app_status = &ril_card_status->applications[index];

            if(app_status->app_type == RIL_UIM_APPTYPE_USIM)
            {
                usim_index = index;
                QCRIL_LOG_INFO( "USIM APP present index - %d", usim_index );
                break;
            }

            if( (sim_index == -1) && (app_status->app_type == RIL_UIM_APPTYPE_SIM) )
            {
                sim_index = index;
                QCRIL_LOG_INFO( "SIM APP present index - %d", sim_index );
            }
        }
    }

    if(usim_index != QCRIL_INVALID_APP_INDEX)
    {
        index = usim_index;
    }
    else if(sim_index != QCRIL_INVALID_APP_INDEX)
    {
        index = sim_index;
    }
    else
    {
        index = QCRIL_INVALID_APP_INDEX;
    }

    QCRIL_LOG_INFO( "index - %d", index );
    return index;
}


/*===========================================================================

    qcril_qmi_prov_retrieve_cdma_app_index

============================================================================*/
/*!
    @brief
    Retrieve CDMA app index. If multiple CDMA apps are available, preference
    would be given to CSIM.

    @return
    None
*/
/*=========================================================================*/
int qcril_qmi_prov_retrieve_cdma_app_index
(
    std::shared_ptr<RIL_UIM_CardStatus> ril_card_status
)
{
    int index = 0;
    int csim_index = QCRIL_INVALID_APP_INDEX;
    int ruim_index = QCRIL_INVALID_APP_INDEX;
    RIL_UIM_AppStatus *app_status = NULL;

    QCRIL_LOG_FUNC_ENTRY();

    if (ril_card_status)
    {
        for (index = 0; index < RIL_UIM_CARD_MAX_APPS; index++)
        {
            app_status = &ril_card_status->applications[index];

            if(app_status->app_type == RIL_UIM_APPTYPE_CSIM)
            {
                csim_index = index;
                QCRIL_LOG_INFO( "CSIM APP present index - %d", csim_index );
                break;
            }

            if( (ruim_index == -1) && (app_status->app_type == RIL_UIM_APPTYPE_RUIM) )
            {
                ruim_index = index;
                QCRIL_LOG_INFO( "RUIM APP present index - %d", ruim_index );
            }
        }
    }

    if(csim_index != QCRIL_INVALID_APP_INDEX)
    {
        index = csim_index;
    }
    else if(ruim_index != QCRIL_INVALID_APP_INDEX)
    {
        index = ruim_index;
    }
    else
    {
        index = QCRIL_INVALID_APP_INDEX;
    }

    QCRIL_LOG_INFO( "index - %d", index );
    return index;
}

/*===========================================================================

    qcril_qmi_prov_subs_activate_followup

============================================================================*/
/*!
    @brief
    Handler for QCRIL_EVT_QMI_PROV_ACTIVATE_FOLLOW_UP from UIM..

    @return
    None
*/
/*=========================================================================*/
void qcril_qmi_prov_subs_activate_followup
(
  qcril_provisioning_response_info *provision_info_ptr
)
{
    qcril_qmi_prov_state_type            prov_state = QCRIL_QMI_PROV_STATE_NONE;
    int i = 0;
    RIL_Errno                            gw_res = RIL_E_GENERIC_FAILURE;
    RIL_Errno                            cdma_res = RIL_E_GENERIC_FAILURE;

    QCRIL_LOG_FUNC_ENTRY();

    if( provision_info_ptr != NULL && provision_info_ptr->response_info_len > 0)
    {
        QCRIL_LOG_DEBUG( "response_info_len %d", provision_info_ptr->response_info_len);
        for(i=0;i < provision_info_ptr->response_info_len; i++)
        {
            QCRIL_LOG_DEBUG( "i = %d, session_type %d", i,
                provision_info_ptr->response_info[i].session_type);
            switch ( provision_info_ptr->response_info[i].session_type )
            {
                case UIM_SESSION_TYPE_TER_GW_PROV:
                case UIM_SESSION_TYPE_SEC_GW_PROV:
                case UIM_SESSION_TYPE_PRI_GW_PROV:
                    gw_res =  (RIL_Errno)provision_info_ptr->response_info[i].err_code;
                    if ( provision_info_ptr->response_info[i].status == QCRIL_PROVISION_STATUS_FAILURE )
                    {
                        QCRIL_LOG_DEBUG( "UIM GW activate sub failure, session_type %d, gw_err_code %d",
                          provision_info_ptr->response_info[i].session_type, gw_res);
                    }
                    else if ( provision_info_ptr->response_info[i].status == QCRIL_PROVISION_STATUS_SUCCESS )
                    {
                        QCRIL_LOG_DEBUG( "UIM GW activate sub success, session_type %d, gw_err_code %d",
                          provision_info_ptr->response_info[i].session_type, gw_res);
                    }
                    break;

                case UIM_SESSION_TYPE_TER_1X_PROV:
                case UIM_SESSION_TYPE_SEC_1X_PROV:
                case UIM_SESSION_TYPE_PRI_1X_PROV:
                    cdma_res =  (RIL_Errno)provision_info_ptr->response_info[i].err_code;
                    if ( provision_info_ptr->response_info[i].status == QCRIL_PROVISION_STATUS_FAILURE )
                    {
                        QCRIL_LOG_DEBUG( "UIM CDMA activate sub failure, session_type %d, cdma_err_code %d",
                          provision_info_ptr->response_info[i].session_type, cdma_res);
                    }
                    else if ( provision_info_ptr->response_info[i].status == QCRIL_PROVISION_STATUS_SUCCESS )
                    {
                        QCRIL_LOG_DEBUG( "UIM CDMA activate sub success, session_type %d, cdma_err_code %d",
                          provision_info_ptr->response_info[i].session_type, cdma_res);
                    }
                    break;

                default:
                    break;
            }
        }

        qcril_qmi_prov_update_app_provision_status( UIM_UICC_SUBSCRIPTION_ACTIVATE,
                                                                    gw_res,
                                                                    cdma_res);
    }
    else
    {
        qcril_qmi_prov_update_app_provision_status( UIM_UICC_SUBSCRIPTION_ACTIVATE,
                                                                    RIL_E_GENERIC_FAILURE,
                                                                    RIL_E_GENERIC_FAILURE);
    }

    qcril_reqlist_free(QCRIL_DEFAULT_INSTANCE_ID,
                                     (void *)(intptr_t)QCRIL_SUB_PROVISION_INTERNAL_TOKEN);

    if ( qcril_qmi_prov_is_any_app_being_provisioned() == FALSE )
    {
        prov_state = qcril_qmi_prov_get_prov_state();

        /* No pending apps, check if any app provisioned. If so, return SUCCESS.*/
        if ( qcril_qmi_prov_is_any_app_provisioned() )
        {
            qcril_qmi_prov_check_and_send_prov_resp(RIL_E_SUCCESS, prov_state);
        }
        else
        {
            qcril_qmi_prov_check_and_send_prov_resp(RIL_E_GENERIC_FAILURE, prov_state);
        }
    }

    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_prov_subs_activate_followup

/*===========================================================================

    qcril_qmi_prov_subs_deactivate_followup

============================================================================*/
/*!
    @brief
    Handler for QCRIL_EVT_QMI_PROV_DEACTIVATE_FOLLOW_UP from UIM..

    @return
    None
*/
/*=========================================================================*/
void qcril_qmi_prov_subs_deactivate_followup
(
  qcril_provisioning_response_info *provision_info_ptr
)
{
    qcril_qmi_prov_state_type            prov_state = QCRIL_QMI_PROV_STATE_NONE;
    int i = 0;
    RIL_Errno                            gw_res = RIL_E_GENERIC_FAILURE;
    RIL_Errno                            cdma_res = RIL_E_GENERIC_FAILURE;

    QCRIL_LOG_FUNC_ENTRY();

    if( provision_info_ptr != NULL && provision_info_ptr->response_info_len > 0)
    {
        QCRIL_LOG_DEBUG( "response_info_len %d", provision_info_ptr->response_info_len);
        for(i=0;i < provision_info_ptr->response_info_len; i++)
        {
            QCRIL_LOG_DEBUG( "i = %d, session_type %d", i, provision_info_ptr->response_info[i].session_type);
            switch ( provision_info_ptr->response_info[i].session_type )
            {
                case UIM_SESSION_TYPE_TER_GW_PROV:
                case UIM_SESSION_TYPE_SEC_GW_PROV:
                case UIM_SESSION_TYPE_PRI_GW_PROV:
                    gw_res =  (RIL_Errno)provision_info_ptr->response_info[i].err_code;
                    if ( provision_info_ptr->response_info[i].status == QCRIL_PROVISION_STATUS_FAILURE )
                    {
                        QCRIL_LOG_DEBUG( "UIM GW deactivate sub failure, session_type %d, gw_err_code %d",
                          provision_info_ptr->response_info[i].session_type, gw_res);
                    }
                    else if ( provision_info_ptr->response_info[i].status == QCRIL_PROVISION_STATUS_SUCCESS )
                    {
                        QCRIL_LOG_DEBUG( "UIM GW deactivate sub success, session_type %d, gw_err_code %d",
                          provision_info_ptr->response_info[i].session_type, gw_res);
                    }
                    break;

                case UIM_SESSION_TYPE_TER_1X_PROV:
                case UIM_SESSION_TYPE_SEC_1X_PROV:
                case UIM_SESSION_TYPE_PRI_1X_PROV:
                    cdma_res =  (RIL_Errno)provision_info_ptr->response_info[i].err_code;
                    if ( provision_info_ptr->response_info[i].status == QCRIL_PROVISION_STATUS_FAILURE )
                    {
                        QCRIL_LOG_DEBUG( "UIM CDMA deactivate sub failure, session_type %d, cdma_err_code %d",
                          provision_info_ptr->response_info[i].session_type, cdma_res);
                    }
                    else if ( provision_info_ptr->response_info[i].status == QCRIL_PROVISION_STATUS_SUCCESS )
                    {
                        QCRIL_LOG_DEBUG( "UIM CDMA deactivate sub success, session_type %d, cdma_err_code %d",
                          provision_info_ptr->response_info[i].session_type, cdma_res);
                    }
                    break;

                default:
                    break;
            }
        }

        qcril_qmi_prov_update_app_provision_status( UIM_UICC_SUBSCRIPTION_DEACTIVATE,
                                                                    gw_res,
                                                                    cdma_res);
    }
    else
    {
        qcril_qmi_prov_update_app_provision_status( UIM_UICC_SUBSCRIPTION_DEACTIVATE,
                                                                    RIL_E_GENERIC_FAILURE,
                                                                    RIL_E_GENERIC_FAILURE);
    }

    qcril_reqlist_free(QCRIL_DEFAULT_INSTANCE_ID,
                                     (void *)(intptr_t)QCRIL_SUB_PROVISION_INTERNAL_TOKEN);

    if ( qcril_qmi_prov_is_any_app_being_deprovisioned() == FALSE )
    {
        prov_state = qcril_qmi_prov_get_prov_state();

        /* send sucess if all apps are deprovisioned, failure otherwise.*/
        if ( qcril_qmi_prov_all_apps_deprovisioned() )
        {
            qcril_qmi_prov_check_and_send_prov_resp(RIL_E_SUCCESS, prov_state);
        }
        else
        {
            qcril_qmi_prov_check_and_send_prov_resp(RIL_E_GENERIC_FAILURE, prov_state);
        }
    }
    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_prov_subs_deactivate_followup


/*===========================================================================

    qcril_qmi_prov_backup_iccid_for_prov
============================================================================*/
/*!
    @brief
    Backup the iccid to iccid_prov for user action provisoning

    @return
    None
*/
/*=========================================================================*/
void qcril_qmi_prov_backup_iccid_for_prov()
{
    int slot = qmi_ril_get_sim_slot();

    QCRIL_LOG_FUNC_ENTRY();

    memset(prov_common_cache.prov_info[slot].iccid_under_prov, 0,
                           sizeof(prov_common_cache.prov_info[slot].iccid_under_prov));

    QCRIL_LOG_DEBUG("Backing up iccid to provision in cache");
    strlcpy(prov_common_cache.prov_info[slot].iccid_under_prov,
                           prov_common_cache.prov_info[slot].iccid,
                           sizeof(prov_common_cache.prov_info[slot].iccid_under_prov));

    QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

    qcril_qmi_prov_reset_prov_pref_info

============================================================================*/
/*!
    @brief
    Reset provisioning module internal cache.

    @return
    None
*/
/*=========================================================================*/
void qcril_qmi_prov_reset_prov_pref_info(int reset_state)
{
    int slot;

    QCRIL_LOG_FUNC_ENTRY();

    for (slot = 0; slot < RIL_UIM_MAX_CARD_COUNT; slot++) {
        prov_common_cache.prov_info[slot].gw_app_index          = QCRIL_INVALID_APP_INDEX;
        prov_common_cache.prov_info[slot].cdma_app_index        = QCRIL_INVALID_APP_INDEX;
        prov_common_cache.prov_info[slot].user_prov_pref        = QCRIL_INVALID_PROV_PREF;
        prov_common_cache.prov_info[slot].curr_prov_status      = QCRIL_INVALID_PROV_PREF;
        prov_common_cache.prov_info[slot].gw_provision_state    =
                                                         QCRIL_APP_STATUS_NOT_PROVISIONED;
        prov_common_cache.prov_info[slot].cdma_provision_state  =
                                                        QCRIL_APP_STATUS_NOT_PROVISIONED;

        memset(prov_common_cache.prov_info[slot].iccid, 0, sizeof(prov_common_cache.prov_info[slot].iccid));

        if ( reset_state )
        {
            prov_common_cache.prov_info[slot].state             = QCRIL_QMI_PROV_STATE_NONE;

            // Only during init or resuming from SSR need to reset state, clear any pending manual prov requests.
            qcril_reqlist_free(QCRIL_DEFAULT_INSTANCE_ID,
                               (void *)(intptr_t)QCRIL_SUB_PROVISION_INTERNAL_TOKEN);
        }
    }
    QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

    qcril_qmi_prov_set_curr_sub_prov_status

============================================================================*/
/*!
    @brief
    Update curr_sub_prov_status in cache.

    @return
    None
*/
/*=========================================================================*/
void qcril_qmi_prov_set_curr_sub_prov_status
(
    int curr_prov_status
)
{
    int      slot = qmi_ril_get_sim_slot();
    prov_common_cache.prov_info[slot].curr_prov_status = curr_prov_status;

}

/*===========================================================================

    qcril_qmi_prov_get_curr_sub_prov_status

============================================================================*/
/*!
    @brief
    Retrieve curr_sub_prov_status from cache.

    @return
    None
*/
/*=========================================================================*/
int qcril_qmi_prov_get_curr_sub_prov_status(void)
{
    int       slot = qmi_ril_get_sim_slot();
    int       curr_prov_status = QCRIL_INVALID_PROV_PREF;

    curr_prov_status = prov_common_cache.prov_info[slot].curr_prov_status;

    return curr_prov_status;
}

/*===========================================================================

    qcril_qmi_prov_get_user_pref_from_cache

============================================================================*/
/*!
    @brief
    Retrieve user_pref from cache.

    @return
    None
*/
/*=========================================================================*/
int qcril_qmi_prov_get_user_pref_from_cache(void)
{
    int        slot = qmi_ril_get_sim_slot();
    int        user_prov_pref = QCRIL_INVALID_PROV_PREF;

    user_prov_pref = prov_common_cache.prov_info[slot].user_prov_pref;

    return user_prov_pref;
}

/*===========================================================================

    qcril_qmi_prov_update_user_pref_cache

============================================================================*/
/*!
    @brief
    Update user_pref in cache.

    @return
    None
*/
/*=========================================================================*/
void qcril_qmi_prov_update_user_pref_cache(RIL_UIM_UiccSubActStatus new_user_pref)
{
    int      slot = qmi_ril_get_sim_slot();
    prov_common_cache.prov_info[slot].user_prov_pref = new_user_pref;

}

/*===========================================================================

    qcril_qmi_prov_set_gw_provision_state

============================================================================*/
/*!
    @brief
    Update gw app provisioning state.

    @return
    None
*/
/*=========================================================================*/
void qcril_qmi_prov_set_gw_provision_state(qcril_card_app_provision_status prov_state)
{
    int       slot = qmi_ril_get_sim_slot();
    prov_common_cache.prov_info[slot].gw_provision_state = prov_state;
}

/*===========================================================================

    qcril_qmi_prov_set_cdma_provision_state

============================================================================*/
/*!
    @brief
    Update cdma app provisioning state.

    @return
    None
*/
/*=========================================================================*/
void qcril_qmi_prov_set_cdma_provision_state(qcril_card_app_provision_status prov_state)
{
    int       slot = qmi_ril_get_sim_slot();
    prov_common_cache.prov_info[slot].cdma_provision_state = prov_state;
}

/*===========================================================================

    qcril_qmi_prov_get_gw_provision_state

============================================================================*/
/*!
    @brief
    retrieve gw app provisioning state from cache.

    @return
    None
*/
/*=========================================================================*/
qcril_card_app_provision_status qcril_qmi_prov_get_gw_provision_state()
{
    int       slot = qmi_ril_get_sim_slot();
    qcril_card_app_provision_status prov_state = QCRIL_APP_STATUS_NOT_PROVISIONED;

    prov_state = prov_common_cache.prov_info[slot].gw_provision_state;

    return prov_state;
}


/*===========================================================================

    qcril_qmi_prov_get_cdma_provision_state

============================================================================*/
/*!
    @brief
    retrieve cdma app provisioning state from cache.

    @return
    None
*/
/*=========================================================================*/
qcril_card_app_provision_status qcril_qmi_prov_get_cdma_provision_state()
{
    int       slot = qmi_ril_get_sim_slot();
    qcril_card_app_provision_status prov_state = QCRIL_APP_STATUS_NOT_PROVISIONED;

    prov_state = prov_common_cache.prov_info[slot].cdma_provision_state;

    return prov_state;
}

/*===========================================================================

    qcril_qmi_prov_get_gw_app_index

============================================================================*/
/*!
    @brief
    retrieve gw app index from cache.

    @return
    None
*/
/*=========================================================================*/
int qcril_qmi_prov_get_gw_app_index()
{
    int       slot = qmi_ril_get_sim_slot();
    int       app_index = QCRIL_INVALID_APP_INDEX;

    app_index = prov_common_cache.prov_info[slot].gw_app_index;

    return app_index;
}

/*===========================================================================

    qcril_qmi_prov_get_cdma_app_index

============================================================================*/
/*!
    @brief
    retrieve cdma app index from cache.

    @return
    None
*/
/*=========================================================================*/
int qcril_qmi_prov_get_cdma_app_index()
{
    int      slot = qmi_ril_get_sim_slot();
    int      app_index = QCRIL_INVALID_APP_INDEX;

    app_index = prov_common_cache.prov_info[slot].cdma_app_index;

    return app_index;
}

/*===========================================================================

    qcril_qmi_prov_set_gw_app_index

============================================================================*/
/*!
    @brief
    update gw app index in cache.

    @return
    None
*/
/*=========================================================================*/
void qcril_qmi_prov_set_gw_app_index(int app_index)
{
    int        slot = qmi_ril_get_sim_slot();
    prov_common_cache.prov_info[slot].gw_app_index = app_index;
}

/*===========================================================================

    qcril_qmi_prov_set_cdma_app_index

============================================================================*/
/*!
    @brief
    update cdma app index in cache.

    @return
    None
*/
/*=========================================================================*/
int qcril_qmi_prov_set_cdma_app_index(int app_index)
{
    int        slot = qmi_ril_get_sim_slot();
    prov_common_cache.prov_info[slot].cdma_app_index = app_index;
    return app_index;
}

/*===========================================================================

    qcril_qmi_prov_update_app_provision_status

============================================================================*/
/*!
    @brief
    Update app provision status.

    @return
    None
*/
/*=========================================================================*/
void qcril_qmi_prov_update_app_provision_status
(
    RIL_UIM_UiccSubActStatus provision_act,
    RIL_Errno                gw_res,
    RIL_Errno                cdma_res
)
{
    int                             slot = qmi_ril_get_sim_slot();
    int                             gw_app_index = QCRIL_INVALID_APP_INDEX;
    int                             cdma_app_index = QCRIL_INVALID_APP_INDEX;
    int                             *gw_app_index_ptr = NULL;
    int                             *cdma_app_index_ptr = NULL;
    qcril_card_app_provision_status *gw_provision_status_ptr = NULL;
    qcril_card_app_provision_status *cdma_provision_status_ptr = NULL;

    QCRIL_LOG_FUNC_ENTRY();

    gw_app_index   = qcril_qmi_prov_get_gw_app_index();
    cdma_app_index = qcril_qmi_prov_get_cdma_app_index();

    QCRIL_LOG_DEBUG("provision_action: %d, gw_result: %d, cmda_result: %d",
                                         provision_act, gw_res, cdma_res);

    if (gw_app_index != -1)
    {
        gw_provision_status_ptr =
            &prov_common_cache.prov_info[slot].gw_provision_state;
        gw_app_index_ptr = &prov_common_cache.prov_info[slot].gw_app_index;
        if(gw_provision_status_ptr)
        {
            QCRIL_LOG_DEBUG("retrieve GW privision status %d",
                        *gw_provision_status_ptr);
        }
    }
    if (cdma_app_index != -1)
    {
        cdma_provision_status_ptr =
            &prov_common_cache.prov_info[slot].cdma_provision_state;
        cdma_app_index_ptr = &prov_common_cache.prov_info[slot].cdma_app_index;
        if(cdma_provision_status_ptr)
        {
            QCRIL_LOG_DEBUG("retrieve CDMA privision status %d",
                *cdma_provision_status_ptr);
        }
    }

    if ((gw_provision_status_ptr && gw_app_index_ptr) || (cdma_provision_status_ptr && cdma_app_index_ptr))
    {
        if (provision_act == UIM_UICC_SUBSCRIPTION_ACTIVATE)
        {
            if (gw_res != RIL_E_SUCCESS)
            {
                /* Important Note::Failure to activate will be marked as not_provisioned */
                if(gw_provision_status_ptr && gw_app_index_ptr)
                {
                    *gw_app_index_ptr = -1;
                    *gw_provision_status_ptr = QCRIL_APP_STATUS_NOT_PROVISIONED;
                }
            }
            else if(gw_provision_status_ptr && gw_app_index_ptr)
            {
                if (*gw_provision_status_ptr == QCRIL_APP_STATUS_PENDING_PROVISIONING)
                {
                    *gw_provision_status_ptr = QCRIL_APP_STATUS_PROVISIONING;
                }
                else if (*gw_provision_status_ptr == QCRIL_APP_STATUS_PROVISIONING)
                {
                    *gw_provision_status_ptr = QCRIL_APP_STATUS_PROVISIONED;
                }
            }

            if (cdma_res != RIL_E_SUCCESS)
            {
                /* Important Note::Failure to activate will be marked as not_provisioned */
                if(cdma_provision_status_ptr && cdma_app_index_ptr)
                {
                    *cdma_app_index_ptr = -1;
                    *cdma_provision_status_ptr = QCRIL_APP_STATUS_NOT_PROVISIONED;
                }
            }
            else if(cdma_provision_status_ptr && cdma_app_index_ptr)
            {
                if (*cdma_provision_status_ptr == QCRIL_APP_STATUS_PENDING_PROVISIONING)
                {
                    *cdma_provision_status_ptr = QCRIL_APP_STATUS_PROVISIONING;
                }
                else if (*cdma_provision_status_ptr == QCRIL_APP_STATUS_PROVISIONING)
                {
                    *cdma_provision_status_ptr = QCRIL_APP_STATUS_PROVISIONED;
                }
            }
        }
        else if (provision_act == UIM_UICC_SUBSCRIPTION_DEACTIVATE)
        {
            if (gw_res != RIL_E_SUCCESS)
            {
                 /* Important Note::Failure to deactivate will be marked as provisioned
                 ** as we try to deactivate only if app is provisioned earlier. */
                if(gw_provision_status_ptr && gw_app_index_ptr)
                {
                    *gw_provision_status_ptr = QCRIL_APP_STATUS_PROVISIONED;
                }
            }
            else if(gw_provision_status_ptr && gw_app_index_ptr)
            {
                if (*gw_provision_status_ptr == QCRIL_APP_STATUS_PENDING_DEPROVISIONING)
                {
                    *gw_provision_status_ptr = QCRIL_APP_STATUS_DEPROVISIONING;
                }
                else if (*gw_provision_status_ptr == QCRIL_APP_STATUS_DEPROVISIONING)
                {
                    *gw_provision_status_ptr = QCRIL_APP_STATUS_DEPROVISIONED;
                }
            }

            if (cdma_res != RIL_E_SUCCESS)
            {
                 /* Important Note::Failure to deactivate will be marked as provisioned
                 ** as we try to deactivate only if app is provisioned earlier. */
                if(cdma_provision_status_ptr && cdma_app_index_ptr)
                {
                    *cdma_provision_status_ptr = QCRIL_APP_STATUS_PROVISIONED;
                }
            }
            else if(cdma_provision_status_ptr && cdma_app_index_ptr)
            {
                if (*cdma_provision_status_ptr == QCRIL_APP_STATUS_PENDING_DEPROVISIONING)
                {
                    *cdma_provision_status_ptr = QCRIL_APP_STATUS_DEPROVISIONING;
                }
                else if (*cdma_provision_status_ptr == QCRIL_APP_STATUS_DEPROVISIONING)
                {
                    *cdma_provision_status_ptr = QCRIL_APP_STATUS_DEPROVISIONED;
                }
            }
        }

        /* if any of the app is provisioned mark curr_prov_status as Activated
        ** else curr_sub_prov_status will remain as invalid.*/
        if ( ( prov_common_cache.prov_info[slot].gw_provision_state ==
                                            QCRIL_APP_STATUS_PROVISIONED ) ||
             ( prov_common_cache.prov_info[slot].cdma_provision_state ==
                                                         QCRIL_APP_STATUS_PROVISIONED ) )
        {
            qcril_qmi_prov_set_curr_sub_prov_status(UIM_UICC_SUBSCRIPTION_ACTIVATE);
        }
        /* We will hit this if any app is not provisioned, mark status as
        ** deactivated even if one app is deprovisioned */
        else if ( ( prov_common_cache.prov_info[slot].gw_provision_state ==
                                            QCRIL_APP_STATUS_DEPROVISIONED ) ||
                  ( prov_common_cache.prov_info[slot].cdma_provision_state ==
                                                              QCRIL_APP_STATUS_DEPROVISIONED ) )
        {
            qcril_qmi_prov_set_curr_sub_prov_status(UIM_UICC_SUBSCRIPTION_DEACTIVATE);
        }

        if(gw_provision_status_ptr)
        {
            QCRIL_LOG_DEBUG("gw_provision status %d", *gw_provision_status_ptr);
        }
        if(cdma_provision_status_ptr)
        {
            QCRIL_LOG_DEBUG("cdma_provision status %d", *cdma_provision_status_ptr);
        }
    }

}

/*=========================================================================
  FUNCTION:  qcril_qmi_prov_get_prov_state
===========================================================================*/
/*!
    @brief
    Get prov state.

    @return
    None.
*/
/*=========================================================================*/
qcril_qmi_prov_state_type qcril_qmi_prov_get_prov_state(void)
{
    int        slot = qmi_ril_get_sim_slot();
    qcril_qmi_prov_state_type state = QCRIL_QMI_PROV_STATE_NONE;

    state = prov_common_cache.prov_info[slot].state;

    return state;
}

/*=========================================================================
  FUNCTION:  qcril_qmi_prov_get_act_status_from_state
===========================================================================*/
/*!
    @brief
    Based on state, user preference return appropriate act_status.

    @return
    None.
*/
/*=========================================================================*/
RIL_UIM_UiccSubActStatus qcril_qmi_prov_get_act_status_from_state(int state)
{
    int                      user_prov_pref = QCRIL_INVALID_PROV_PREF;
    RIL_UIM_UiccSubActStatus act_status     = UIM_UICC_SUBSCRIPTION_ACTIVATE;

    user_prov_pref = qcril_qmi_prov_get_user_pref_from_cache();

    switch(state)
    {
        case QCRIL_QMI_PROV_STATE_USER_ACTIVATE:
            act_status = UIM_UICC_SUBSCRIPTION_ACTIVATE;
            break;

        case QCRIL_QMI_PROV_STATE_FM_START:
        case QCRIL_QMI_PROV_STATE_USER_DEACTIVATE:
            act_status = UIM_UICC_SUBSCRIPTION_DEACTIVATE;
            break;

        /* FM apply/card status changed, decide activate/deactivate
        ** based on user preference.*/
        case QCRIL_QMI_PROV_STATE_FM_APPLY:
        case QCRIL_QMI_PROV_STATE_CARD_UP:
            act_status = (RIL_UIM_UiccSubActStatus) user_prov_pref;
            break;
    }

    return act_status;
}

/*=========================================================================
  FUNCTION:  qcril_qmi_prov_set_prov_state
===========================================================================*/
/*!
    @brief
    Handle prov state machine.

    @return
    None.
*/
/*=========================================================================*/
RIL_Errno qcril_qmi_prov_set_prov_state
(
    qcril_qmi_prov_state_type   state,
    qcril_qmi_prov_action_type  *prov_action
)
{
    int                              slot = qmi_ril_get_sim_slot();
    RIL_Errno                        res = RIL_E_GENERIC_FAILURE;
    qcril_qmi_prov_state_type        curr_state = QCRIL_QMI_PROV_STATE_NONE;

    QCRIL_LOG_FUNC_ENTRY();

    curr_state = qcril_qmi_prov_get_prov_state();

    QCRIL_LOG_INFO("Current state - %d, req state - %d", curr_state, state);

    do
    {

        switch( curr_state )
        {
            case QCRIL_QMI_PROV_STATE_NONE:
                prov_common_cache.prov_info[slot].state = state;
                res = RIL_E_SUCCESS;
                break;

            /* Only FM_APPLY/FM_START/NONE are valid transitions from FM_START
               FM_START might come when already in FM_START state if flex map
               failed on other RILD.*/
            case QCRIL_QMI_PROV_STATE_FM_START:
                if ( state == QCRIL_QMI_PROV_STATE_FM_APPLY ||
                     state == QCRIL_QMI_PROV_STATE_NONE     ||
                     state == QCRIL_QMI_PROV_STATE_FM_START )
                {
                    prov_common_cache.prov_info[slot].state = state;
                    res = RIL_E_SUCCESS;
                }
                break;

            /* Only Valid transition from curr_state is to NONE */
            case QCRIL_QMI_PROV_STATE_FM_APPLY:
            case QCRIL_QMI_PROV_STATE_USER_ACTIVATE:
            case QCRIL_QMI_PROV_STATE_USER_DEACTIVATE:
                if ( state == QCRIL_QMI_PROV_STATE_NONE )
                {
                    prov_common_cache.prov_info[slot].state = state;
                    res = RIL_E_SUCCESS;
                }
                break;

            /* If provisioning currently on going due card UP, defer FM_START request. */
            case QCRIL_QMI_PROV_STATE_CARD_UP:
                if ( state == QCRIL_QMI_PROV_STATE_NONE )
                {
                    prov_common_cache.prov_info[slot].state = state;
                    res = RIL_E_SUCCESS;
                }
                else if ( ( state == QCRIL_QMI_PROV_STATE_FM_START ) &&  prov_action )
                {
                    *prov_action = QCRIL_QMI_PROV_ACTION_DEFER;
                }
                break;

             default:
                QCRIL_LOG_INFO("should not be here curr_state - %d", curr_state);
                break;

        }


    } while(FALSE);

    QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
    return res;
}

/*=========================================================================
  FUNCTION:  qcril_qmi_prov_evaluate_and_mark_apps
===========================================================================*/
/*!
    @brief
    Evaluate application to activate/deactivate.

    @return
    None.
*/
/*=========================================================================*/
RIL_Errno qcril_qmi_prov_evaluate_and_mark_apps(void)
{
    int                              user_prov_pref = QCRIL_INVALID_PROV_PREF;
    RIL_Errno                        res = RIL_E_SUCCESS;
    qcril_qmi_prov_state_type        curr_state;

    QCRIL_LOG_FUNC_ENTRY();

    curr_state = qcril_qmi_prov_get_prov_state();

    QCRIL_LOG_INFO("Current state - %d", curr_state);

    do
    {
        user_prov_pref = qcril_qmi_prov_get_user_pref_from_cache();

        switch(curr_state)
        {
            case QCRIL_QMI_PROV_STATE_USER_ACTIVATE:
                res = qcril_qmi_prov_evaluate_and_mark_apps_to_activate();
                break;

            case QCRIL_QMI_PROV_STATE_FM_START:
            case QCRIL_QMI_PROV_STATE_USER_DEACTIVATE:
                res = qcril_qmi_prov_evaluate_and_mark_apps_to_deactivate();
                break;

            /* FM apply/card status changed, decide activate/deactivate
            ** based on user preference.*/
            case QCRIL_QMI_PROV_STATE_FM_APPLY:
            case QCRIL_QMI_PROV_STATE_CARD_UP:
                if ( user_prov_pref == UIM_UICC_SUBSCRIPTION_ACTIVATE )
                {
                    res = qcril_qmi_prov_evaluate_and_mark_apps_to_activate();
                }
                else if ( user_prov_pref == UIM_UICC_SUBSCRIPTION_DEACTIVATE )
                {
                    res = qcril_qmi_prov_evaluate_and_mark_apps_to_deactivate();
                }
                else
                {
                    QCRIL_LOG_INFO("Invalid user provisioning preference - %d", user_prov_pref);
                }
                break;
            default:
                break;
        }

    } while(FALSE);

    QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
    return res;
}

/*=========================================================================
  FUNCTION:  qcril_qmi_prov_update_iccid_hndlr

===========================================================================*/
/*!
    @brief
    Handle QCRIL_EVT_HOOK_SET_SUB_PROVISION_PREFERENCE_REQ

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_prov_update_iccid_hndlr
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
    char *iccid_ptr;
    int slot = qmi_ril_get_sim_slot();
    QCRIL_NOTUSED(ret_ptr);

    QCRIL_LOG_FUNC_ENTRY();

    iccid_ptr = (char *) params_ptr->data;

    if ( params_ptr->data != NULL && params_ptr->datalen > 0 )
    {
        strlcpy( prov_common_cache.prov_info[slot].iccid,
                 iccid_ptr,
                 QMI_DMS_UIM_ID_MAX_V01 + 1 );

        QCRIL_LOG_INFO("updated iccid - %s", prov_common_cache.prov_info[slot].iccid);
        qcril_qmi_nas_fetch_user_prov_pref_from_database();
    }

    QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:  qcril_qmi_prov_get_iccid_from_cache

===========================================================================*/
/*!
    @brief
    Get iccid from internal cache. iccid will be null terminated string.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_prov_get_iccid_from_cache(char *iccid)
{
    int slot = qmi_ril_get_sim_slot();
    if(NULL != iccid)
    {
        strlcpy(iccid,
                prov_common_cache.prov_info[slot].iccid,
                QMI_DMS_UIM_ID_MAX_V01 + 1 );
    }
}

/*=========================================================================
  FUNCTION:  qcril_qmi_prov_fetch_backup_iccid_and_reset

===========================================================================*/
/*!
    @brief
    Get iccid under prov from internal cache.
    iccid will be null terminated string. It will reset the backup
    iccid cache.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_prov_fetch_backup_iccid_and_reset(char *iccid)
{
    int slot = qmi_ril_get_sim_slot();
    if(NULL != iccid)
    {
        strlcpy(iccid,
                prov_common_cache.prov_info[slot].iccid_under_prov,
                sizeof(prov_common_cache.prov_info[slot].iccid_under_prov));

        QCRIL_LOG_INFO("Reset iccid_under_prov");
        memset(prov_common_cache.prov_info[slot].iccid_under_prov, 0,
                        sizeof(prov_common_cache.prov_info[slot].iccid_under_prov));
    }
}

/*=========================================================================
  FUNCTION:  qcril_qmi_prov_fill_prov_preference_info

===========================================================================*/
/*!
    @brief
    Fetach current subscription status, user preference from internal cache.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_prov_fill_prov_preference_info(RIL_SubProvStatus *payload)
{
    if(NULL != payload)
    {
        memset(payload, 0, sizeof(*payload));
        payload->user_preference = qcril_qmi_prov_get_user_pref_from_cache();
        payload->current_sub_preference = qcril_qmi_prov_get_curr_sub_prov_status();
    }
}

/*=========================================================================
  FUNCTION:  qcril_qmi_prov_update_db_with_user_pref

===========================================================================*/
/*!
    @brief
    Update database with current user preference for subscription.

    @return
    None.
*/
/*=========================================================================*/
RIL_Errno qcril_qmi_prov_update_db_with_user_pref
(
    RIL_UIM_UiccSubActStatus user_prov_pref,
    char* iccid
)
{
    int                  prev_userpref = QCRIL_INVALID_PROV_PREF;
    RIL_Errno            res = RIL_E_SUCCESS;

    QCRIL_LOG_FUNC_ENTRY();

    res = (RIL_Errno) qcril_db_query_user_pref_from_prov_table(iccid,&prev_userpref);

    do
    {
        if ( res != RIL_E_SUCCESS )
        {
            QCRIL_LOG_INFO("error while querying user preference");
            break;
        }

        if ( prev_userpref == QCRIL_INVALID_PROV_PREF )
        {
            QCRIL_LOG_INFO("iccid %s not present", iccid);
            qcril_db_insert_new_iccid_into_prov_table(iccid,user_prov_pref);
        }
        else if ( prev_userpref != user_prov_pref )
        {
            res = (RIL_Errno) qcril_db_update_user_pref_prov_table(iccid,user_prov_pref);
            if ( res == RIL_E_SUCCESS )
            {
                /* update cache as well.*/
                qcril_qmi_prov_update_user_pref_cache(user_prov_pref);
                QCRIL_LOG_INFO(" iccid - %s prev user pref - %d, new user pref - %d", iccid, prev_userpref, user_prov_pref);
            }
        }

    }while(0);

    QCRIL_LOG_FUNC_RETURN_WITH_RET(res);
    return res;
}


/*=========================================================================
  FUNCTION:  qcril_qmi_nas_fetch_user_prov_pref_from_database

===========================================================================*/
/*!
    @brief
    Fetch user subscription preference from database.

    @return
    None.
*/
/*=========================================================================*/
int qcril_qmi_nas_fetch_user_prov_pref_from_database(void)
{

    int                pref = QCRIL_INVALID_PROV_PREF;
    char               iccid[QMI_DMS_UIM_ID_MAX_V01 + 1];
    RIL_Errno          res = RIL_E_SUCCESS;

    QCRIL_LOG_FUNC_ENTRY();

    memset(iccid, NIL,sizeof(iccid));
    qcril_qmi_prov_get_iccid_from_cache(iccid);

    res = (RIL_Errno) qcril_db_query_user_pref_from_prov_table(iccid,&pref);

    QCRIL_LOG_INFO("user preference from db - %d", pref);

    if ( pref == QCRIL_INVALID_PROV_PREF )
    {
        /* for new card, set user preference by default as activate.*/
        QCRIL_LOG_INFO("New card inserted... set user preference to activate");
        pref = UIM_UICC_SUBSCRIPTION_ACTIVATE;
        qcril_qmi_prov_update_db_with_user_pref(UIM_UICC_SUBSCRIPTION_ACTIVATE, iccid);
    }

    qcril_qmi_prov_update_user_pref_cache((RIL_UIM_UiccSubActStatus)pref); //NAS REFACTOR:

    QCRIL_LOG_FUNC_RETURN_WITH_RET(pref);
    return pref;
}


