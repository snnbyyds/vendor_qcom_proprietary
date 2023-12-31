/******************************************************************************

                          N E T M G R _ E X E C . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_exec.h
  @brief   Network Manager executive header file

  DESCRIPTION
  Header file for NetMgr executive control module.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010-2013,2015,2017,2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
02/17/10   ar         Initial version

******************************************************************************/

#ifndef __NETMGR_EXEC_H__
#define __NETMGR_EXEC_H__

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/
#include "comdef.h"

#include "ds_cmdq.h"
#include "netmgr.h"
#include "netmgr_defs.h"
#include "netmgr_netlink.h"
#include "netmgr_main.h"
#include "netmgr_qmi.h"
#include "netmgr_tc.h"
#include "netmgr_sm.h"
#include "semaphore.h"

/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

/*---------------------------------------------------------------------------
   Type of a Executive event data
---------------------------------------------------------------------------*/
typedef struct netmgr_exec_cmd_data_s {
  netmgr_sm_events_t            type;            /* Event type */
  int                           link;
  int socket_info;                               /* Will be used internally for tracking
                                                    the UNIX socket used to send response
                                                    back to client */
  int                           ret_code;        /* Return code for dynamic link creation */
  union {
    netmgr_user_cmd_data_t      usr_cmd;
    netmgr_qmi_cmd_t            qmi_msg;
    netmgr_qmi_qos_flow_info_t  qos_flow;
    netmgr_msg_t                kif_msg;
    struct connect_msg_s {
      netmgr_nl_msg_t           nlmsg_info;
      netmgr_address_set_t     *addr_info_ptr;
      boolean                   reconfig_required;
#ifdef FEATURE_DATA_IWLAN
      int                       txn_id;
#endif
    } connect_msg;
    struct disconnect_msg_s {
      netmgr_address_set_t     *addr_info_ptr;
      boolean                   teardown_iface;
#ifdef FEATURE_DATA_IWLAN
      int                       txn_id;
#endif
    } disconnect_msg;
    char wlan_ifname[NETMGR_IF_NAME_MAX_LEN];    /* Wlan ifname used for WLAN UP/DOWN EV */
  } info;                                        /* Payload info */
} netmgr_exec_cmd_data_t;

/*---------------------------------------------------------------------------
   Type of a Executive command
---------------------------------------------------------------------------*/
typedef struct netmgr_exec_cmd_s {
  ds_cmd_t                  cmd;                 /* Command object         */
  netmgr_exec_cmd_data_t    data;                /* Command data           */
  int                       tracker;             /* 1 if alloc, else 0     */
  int                       repost_count;        /* Number of time command
                                                    has been reposted      */
} netmgr_exec_cmd_t;


/*---------------------------------------------------------------------------
   Type representing collection of state information for each network link
---------------------------------------------------------------------------*/
struct netmgr_exec_link_s {
  struct stm_state_machine_s *sm;
};


/*---------------------------------------------------------------------------
   Type representing collection of state information for module
---------------------------------------------------------------------------*/
struct netmgr_exec_state_s {
  struct netmgr_exec_link_s   links[NETMGR_MAX_LINK];   /* Link state info */
  struct ds_cmdq_info_s       cmdq;  /* Command queue for async processing */
  int                         nlink;            /* Number of network links */
};


/*===========================================================================
                            GLOBAL FUNCTION DECLARATIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  netmgr_exec_get_cmd
===========================================================================*/
/*!
@brief
  Function to get a command buffer for asynchronous processing

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - Allocated heap memory
*/
/*=========================================================================*/
netmgr_exec_cmd_t * netmgr_exec_get_cmd ( void );


/*===========================================================================
  FUNCTION  netmgr_exec_release_cmd
===========================================================================*/
/*!
@brief
  Function to release a command buffer

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - Returns memory to heap
*/
/*=========================================================================*/
void netmgr_exec_release_cmd ( netmgr_exec_cmd_t * );


/*===========================================================================
  FUNCTION  netmgr_exec_put_cmd
===========================================================================*/
/*!
@brief
  Function to post a command buffer for asynchronous processing

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_exec_put_cmd ( const netmgr_exec_cmd_t * cmdbuf );


/*===========================================================================
  FUNCTION  netmgr_exec_wait
===========================================================================*/
/*!
@brief
  Forces calling thread to wait on exit of command queue thread.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - Calling thread is blocked indefinitely
*/
/*=========================================================================*/
void netmgr_exec_wait ( void );


/*===========================================================================
  FUNCTION  netmgr_exec_init
===========================================================================*/
/*!
@brief
 Main initialization routine of the executive control module.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - Initializes the QMI Driver
*/
/*=========================================================================*/
void netmgr_exec_init (int nlink, netmgr_ctl_port_config_type links[]);


/*===========================================================================
  FUNCTION  netmgr_exec_init_disabled_link
===========================================================================*/
/*!
@brief
  Enables the state machine for a link that is originally disabled
  (i.e. not statically inited)

@return
  NETMGR_SUCCESS if state machine was inited, NETMGR_FAILURE otherwise

@note
  - Dependencies
    - None

  - Side Effects
    - Initializes the QMI Driver
*/
/*=========================================================================*/
int netmgr_exec_init_disabled_link(int link_num,
                                   netmgr_ctl_port_config_type links[]);


/*===========================================================================
  FUNCTION  netmgr_exec_process_msg
===========================================================================*/
/*!
@brief
  Process messages that need to be run in the executive thread context. These
  messages cannot be processed as part of the state machine.

@return
  None
*/
/*=========================================================================*/
void
netmgr_exec_process_msg (ds_cmd_t * cmd, void * data);

#endif /* __NETMGR_EXEC_H__ */
