/******************************************************************************
#  Copyright (c) 2014, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/******************************************************************************

  @file    qmi_ril_platform_dep.c
  @brief   Provides interface to functions where conditional platform
           dependency is there.

******************************************************************************/
#include <sys/ioctl.h>

#define TAG "RIL-PlatformDep"

#include "framework/Log.h"
#include "qcrili.h"
#include "qcril_qmi_client.h"
#include "qmi_ril_platform_dep.h"
#include "qcril_memory_management.h"
#ifdef FEATURE_QCRIL_SHM
#ifndef QMI_RIL_UTF
#include "sys_health_mon.h"
#endif
#endif

#define NAS_MDM_SHUTDOWN_IOCTL_CMD   (0x4004cc0a)

static const char *modem_node_name[QCRIL_MAX_INSTANCE_ID] = { "/dev/mdm",
                                                              "/dev/mdm1"};

typedef struct {
    uint32_t timer_id;
    qmi_client_recv_msg_async_cb resp_cb;
    void    *cb_data;
} async_data_type;

static void qcril_qmi_async_callback_timeout(qcril_timed_callback_handler_params_type* handler_params)
{
    QCRIL_NOTUSED(handler_params);
    int ret;
    if(qmi_ril_is_feature_supported(QMI_RIL_FEATURE_SHM))
    {
#ifdef FEATURE_QCRIL_SHM
#ifndef QMI_RIL_UTF
        /* since async callback does not come in time, check system health */
        ret = check_system_health();
        if (ret < 0) {
            QCRIL_LOG_ESSENTIAL("QCRIL_WARNING:IO: Check system health returned %d", ret);
        }
#endif
#endif
    }
}

/*===========================================================================

  FUNCTION  qcril_process_mdm_shutdown

===========================================================================*/
/*!
    @brief
    Process modem shutdown request

    @return
    0 on success
*/
/*=========================================================================*/
RIL_Errno qcril_process_mdm_shutdown
(
    void
)
{
    int                                   ioctl_ret_code;
    int                                   ioctl_err_code;
    const char                           *modem_dev_node_name = modem_node_name[QCRIL_DEFAULT_INSTANCE_ID];
    qcril_instance_id_e_type              ril_instance_id;
    RIL_Errno                             ril_req_res = RIL_E_GENERIC_FAILURE;

    ioctl_ret_code = E_SUCCESS;
    ioctl_err_code = E_SUCCESS;

    ril_instance_id = qmi_ril_get_process_instance_id();
    if ((ril_instance_id == QCRIL_SECOND_INSTANCE_ID) &&
         qmi_ril_is_feature_supported( QMI_RIL_FEATURE_DSDA2 ))
    {
        modem_dev_node_name = modem_node_name[ril_instance_id];
    }

#ifdef FEATURE_QCRIL_8064

    fd = open(modem_dev_node_name, O_RDONLY | O_NONBLOCK);
    if ( fd >= RIL_VALID_FILE_HANDLE)
    {
        ioctl_ret_code = ioctl(fd, NAS_MDM_SHUTDOWN_IOCTL_CMD, &ioctl_err_code);
        if ( ioctl_ret_code < NAS_NIL || ioctl_err_code != E_SUCCESS )
        {
            QCRIL_LOG_ERROR("error while issuing ioctl SHUTDOWN_CHARM, ret %d, err %d", ioctl_ret_code, ioctl_err_code);
        }
        else
        {
            ril_req_res = RIL_E_SUCCESS;
        }

        close(fd);
    }
    else
    {
        QCRIL_LOG_ERROR("Could not open device %s, fd %d", modem_dev_node_name, fd);
    }

#else

    ril_req_res = RIL_E_SUCCESS;

#endif

    return ril_req_res;
}

//===========================================================================
// generic_async_qmi_cb
//===========================================================================
static void generic_async_qmi_cb
(
  qmi_client_type              user_handle,
  unsigned int                 msg_id,
  void                         *resp_c_struct,
  unsigned int                 resp_c_struct_len,
  void                         *resp_cb_data,
  qmi_client_error_type        transp_err
)
{
  async_data_type *generic_cb_data;

  generic_cb_data = (async_data_type *)resp_cb_data;
  /* cancel the timer */
  qcril_cancel_timed_callback((void*)(uintptr_t)generic_cb_data->timer_id);
  /* call the user-set callback func with user-set cb_data */
  generic_cb_data->resp_cb( user_handle,
                            msg_id,
                            resp_c_struct,
                            resp_c_struct_len,
                            generic_cb_data->cb_data,
                            transp_err );

  qcril_free(generic_cb_data);
}

/*
 * This function is to send qmi async message with
 * system health monitor check
 */
qmi_client_error_type
qmi_client_send_msg_async_with_shm
(
    qmi_client_type                 user_handle,
    unsigned long                   msg_id,
    void                            *req_c_struct,
    int                             req_c_struct_len,
    void                            *resp_c_struct,
    int                             resp_c_struct_len,
    qmi_client_recv_msg_async_cb    resp_cb,
    void                            *resp_cb_data,
    qmi_txn_handle                  *txn_handle
)
{
    async_data_type *generic_cb_data;
    qmi_client_error_type rc;
    struct timeval relativeTime;

    /* build generic_cb_data */
    generic_cb_data = qcril_malloc2(generic_cb_data);
    if (generic_cb_data == NULL)
    {
        QCRIL_LOG_ERROR("Failed to allocate generic cb data");
        return QMI_INTERNAL_ERR;
    }
    generic_cb_data->resp_cb = resp_cb;
    generic_cb_data->cb_data = resp_cb_data;

    /* create timer */
    relativeTime.tv_sec = 120; // 2min
    relativeTime.tv_usec = 0;
    qcril_setup_timed_callback_ex_params(
                                QCRIL_DEFAULT_INSTANCE_ID,
                                QCRIL_DEFAULT_MODEM_ID,
                                qcril_qmi_async_callback_timeout,
                                NULL,
                                &relativeTime,
                                &generic_cb_data->timer_id );

    /* call the qmi async send */
    rc = qmi_client_send_msg_async( user_handle,
                                    msg_id,
                                    req_c_struct,
                                    req_c_struct_len,
                                    resp_c_struct,
                                    resp_c_struct_len,
                                    generic_async_qmi_cb,
                                    generic_cb_data,
                                    txn_handle);
    if (rc != QMI_NO_ERR) {
        qcril_cancel_timed_callback((void*)(uintptr_t)generic_cb_data->timer_id);
        qcril_free(generic_cb_data);
    }

    return rc;
}

/*
 * This function is to send qmi sync message with
 * system health monitor check
 */
qmi_client_error_type
qmi_client_send_msg_sync_with_shm
(
    qmi_client_type    user_handle,
    int                msg_id,
    void               *req_c_struct,
    int                req_c_struct_len,
    void               *resp_c_struct,
    int                resp_c_struct_len,
    int                timeout_msecs
)
{
    qmi_client_error_type rc;
    int ret;
    rc = qmi_client_send_msg_sync(user_handle,
                                  msg_id,
                                  req_c_struct,
                                  req_c_struct_len,
                                  resp_c_struct,
                                  resp_c_struct_len,
                                  timeout_msecs);
    if (rc == QMI_TIMEOUT_ERR)
    {

        if(qmi_ril_is_feature_supported(QMI_RIL_FEATURE_SHM))
        {
#ifdef FEATURE_QCRIL_SHM
#ifndef QMI_RIL_UTF
            /* sync send timeout, and check system health */
            ret = check_system_health();
            if (ret < 0) {
                QCRIL_LOG_ESSENTIAL("QCRIL_WARNING:IO: Check system health returned %d", ret);
            }
#endif
#endif
        }

    }
    return rc;
}
