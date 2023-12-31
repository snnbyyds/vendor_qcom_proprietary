/******************************************************************************

                          N E T M G R _ C L I E N T . C

******************************************************************************/

/******************************************************************************

  @file    netmgr_client.c
  @brief   Network Manager client module

  DESCRIPTION
  Implementation of NetMgr client module.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010, 2014-2016, 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
04/21/10   ar         Initial version

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <stdlib.h>
#include <assert.h>

#include "netmgr.h"
#include "netmgr_defs.h"
#include "netmgr_netlink.h"
#include "netmgr_util.h"
#include "netmgr_unix.h"
#include "netmgr_tipc.h"

/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

/* Maximum number of registered clients */
#define NETMGR_CLIENT_MAX (5)

/* Communication method for listener*/
#define NETMGR_CLIENT_TIPC_LISTEN 1

typedef struct netmgr_client_s {
    boolean               is_valid;
    netmgr_event_ind_f    cb_f;
    void                * data;
} netmgr_client_t;

typedef struct listener_info_s {
  netmgr_socklthrd_info_t  sk_thrd_info;    /* Listener thread info        */
  netmgr_socklthrd_fdmap_t sk_thrd_fdmap[1];/* Array of fdmap structs used
                                             ** by listener thread.        */
  netmgr_nl_sk_info_t      ev_sk;           /* Netlink event socket info   */
  boolean                  running;
} netmgr_listener_info_t;

LOCAL struct netmgr_client_state_s {
  netmgr_listener_info_t   recv_socket;
  netmgr_client_t   client_info[NETMGR_CLIENT_MAX];
  pthread_mutex_t   mutex;                /* Mutex for multi-threaded access */
} netmgr_client_state;

LOCAL boolean netmgr_client_initialized = FALSE;

#define NETMGR_CLIENT_LOCK()                                            \
        if( 0>pthread_mutex_lock( &netmgr_client_state.mutex ) ) {      \
          netmgr_log_sys_err("Failed to lock mutex");                   \
          NETMGR_LOG_FUNC_EXIT;                                         \
          return NETMGR_FAILURE;                                        \
        }
#define NETMGR_CLIENT_UNLOCK()                                          \
        if( 0>pthread_mutex_unlock( &netmgr_client_state.mutex ) ) {    \
          netmgr_log_sys_err("Failed to unlock mutex");                 \
          NETMGR_LOG_FUNC_EXIT;                                         \
          return NETMGR_FAILURE;                                        \
        }


/* Macros for client handle */
#define CLIENT_HNDL_DEFAULT    0xBEEF0000
#define CLIENT_HNDL_MASK_ID    0x0000FFFF
#define CLIENT_HNDL_MASK_TOKEN 0xFFFF0000
#define CLIENT_MAKE_HNDL(id)                                         \
        (CLIENT_HNDL_DEFAULT | (id & CLIENT_HNDL_MASK_ID))
#define CLIENT_GET_ID_FROM_HNDL(hndl)                                \
        (hndl & CLIENT_HNDL_MASK_ID)
#define IS_VALID_HNDL(hndl)                                          \
        ((CLIENT_HNDL_DEFAULT == (hndl & CLIENT_HNDL_MASK_TOKEN)) && \
         (NETMGR_CLIENT_MAX > CLIENT_GET_ID_FROM_HNDL(hndl)))
#define NETMGR_PROCESS_NAME "netmgrd"

/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  netmgr_client_alloc
===========================================================================*/
/*!
@brief
  Function to allocate a client instance

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int netmgr_client_alloc
(
  netmgr_client_t  **client_pptr,
  unsigned int      *index_ptr
)
{
  unsigned int index = 0;

  NETMGR_ASSERT( client_pptr );
  NETMGR_ASSERT( index_ptr );

  NETMGR_LOG_FUNC_ENTRY;

  *client_pptr = NULL;

  /* Loop over all client records to find one avaiable */
  for( index=0; index < NETMGR_CLIENT_MAX; index++ )
  {
    /* Verify client free */
    if( !netmgr_client_state.client_info[index].is_valid ) {
      netmgr_client_state.client_info[index].is_valid = TRUE;
      *index_ptr = index;
      *client_pptr = &netmgr_client_state.client_info[index];
      break;
    }
  }

  NETMGR_LOG_FUNC_EXIT;
  return (*client_pptr)? NETMGR_SUCCESS : NETMGR_FAILURE;
}


/*===========================================================================
  FUNCTION  netmgr_client_notify
===========================================================================*/
/*!
@brief
  Function to generate event indication for all registered clients.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int netmgr_client_notify
(
  netmgr_nl_event_info_t  *event_info
)
{
  int index = 0;

  NETMGR_ASSERT( event_info );

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_CLIENT_LOCK();

  /* Loop over all client records */
  for( index=0; index < NETMGR_CLIENT_MAX; index++ )
  {
    /* Verify client registered */
    if( netmgr_client_state.client_info[index].is_valid &&
        netmgr_client_state.client_info[index].cb_f ) {
      /* Invoke clinet callback */
      netmgr_client_state.client_info[index].cb_f( event_info->event,
                                                   event_info,
                                                   netmgr_client_state.client_info[index].data );
      netmgr_log_high( "Notified client: hdl=0x%p event=%d link=%d flow=0x%08x",
                       &netmgr_client_state.client_info[index],
                       event_info->event, event_info->link, event_info->flow_info.flow_id );

    }
  }

  NETMGR_CLIENT_UNLOCK();

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_client_recv_nl_msg
===========================================================================*/
/*!
@brief
  Virtual function registered with the socket listener thread to receive
  incoming messages over the NETLINK event socket.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_client_recv_nl_msg (int fd)
{
  struct msghdr * msgh = NULL;
  struct sockaddr_nl * nladdr = NULL;
  struct iovec * iov = NULL;
  unsigned int  msglen = 0;
  netmgr_nl_msg_t * nlmsg = NULL;

  NETMGR_CLIENT_LOCK();
  if( fd != netmgr_client_state.recv_socket.ev_sk.sk_fd ) {
    netmgr_log_err( "unexpected socket fd(%d), expected fd(%d)\n",
                    fd, netmgr_client_state.recv_socket.ev_sk.sk_fd );
    NETMGR_CLIENT_UNLOCK();
    return NETMGR_FAILURE;
  }
  NETMGR_CLIENT_UNLOCK();

  /* Allocate event message buffer */
  nlmsg = netmgr_malloc( sizeof(netmgr_nl_msg_t) );
  if( NULL == nlmsg ) {
    netmgr_log_err("failed to allocate message buffer!\n");
    goto bail;
  } else {
    /* Read netlink message from the socket */
    if( NETMGR_SUCCESS != netmgr_nl_recv_msg( fd, &msgh, &msglen ) ) {
      netmgr_log_err("netmgr_nl_recv_msg failed!\n");
      goto bail;
    }

    nladdr = msgh->msg_name;
    iov = msgh->msg_iov;

    /* Decode the message in structure */
    memset( nlmsg, 0x0, sizeof(netmgr_nl_msg_t) );
    if( NETMGR_SUCCESS !=
        netmgr_nl_decode_nlmsg( (char*)iov->iov_base, msglen, nlmsg ) ) {
      netmgr_log_err("netmgr_nl_decode_nlmsg failed!\n");
      goto bail;
    }

    /* Verify message is NetMgr event type */
    if( NETMGR_NL_PARAM_EVENT & nlmsg->param_mask ) {
      /* Generate indication to client */
      netmgr_client_notify( &nlmsg->event_info );
    } else {
      netmgr_log_low("Unknown Netlink msg type, ignoring!\n" );
    }

    /* Release NetLink message buffer */
    netmgr_nl_release_msg( msgh );
    netmgr_free( nlmsg );
  }
  return NETMGR_SUCCESS;

bail:
  if( msgh )
    netmgr_nl_release_msg( msgh );
  if( nlmsg )
    netmgr_free( nlmsg );
  return NETMGR_FAILURE;
}
/*===========================================================================
  FUNCTION  netmgr_client_send_ping_msg
===========================================================================*/
/*!
@brief
  Function to query NetMgr for its readiness status. @param timeout is the
  interval to wait for a response.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_client_send_ping_msg
(
  int resp_timeout
)
{
  int                     rc = NETMGR_FAILURE;
  char                    *req_buffer = NULL;
  unsigned int            req_bufflen = 0;
  char                    *resp_buffer = NULL;
  unsigned int            resp_bufflen = 0;
  netmgr_nl_event_info_t  *event_info = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  if ((event_info = netmgr_malloc(sizeof(netmgr_nl_event_info_t))) == NULL)
  {
    netmgr_log_err("%s(): Failed to allocate buffer", __func__);
    goto free_buffers_and_exit;
  }

  memset(event_info, 0x0, sizeof(netmgr_nl_event_info_t));
  event_info->event = NETMGR_READY_REQ;

  if (netmgr_nl_encode_netmgr_event(event_info, &req_buffer, &req_bufflen) != NETMGR_SUCCESS)
  {
    netmgr_log_err("%s(): Failed to encode message", __func__);
    goto free_buffers_and_exit;
  }

  if (((req_buffer != NULL) && (req_bufflen == 0)) || (req_buffer == NULL))
  {
    netmgr_log_err("%s(): Invalid buffer allocated", __func__);
    goto free_buffers_and_exit;
  }

  if ((resp_buffer = netmgr_malloc(NETMGR_UNIX_MAX_MESSAGE_SIZE * sizeof(char *))) == NULL)
  {
    netmgr_log_err("%s(): Failed to allocate buffer", __func__);
    goto free_buffers_and_exit;
  }

  memset(resp_buffer, 0x0, sizeof(NETMGR_UNIX_MAX_MESSAGE_SIZE));
  if (netmgr_unix_ipc_send_msg(req_buffer, req_bufflen, &resp_buffer, &resp_bufflen, resp_timeout)
    != NETMGR_SUCCESS)
  {
    netmgr_log_err("%s(): netmgr_unix_send_msg failed", __func__);
    goto free_buffers_and_exit;
  }

  if(resp_buffer == NULL)
  {
    netmgr_log_err("%s(): Error in recv", __func__);
    goto free_buffers_and_exit;
  }

  if(resp_bufflen == 0)
  {
    netmgr_log_err("%s(): Client closed socket", __func__);
    goto free_buffers_and_exit;
  }

  memset(event_info, 0x0, sizeof(netmgr_nl_event_info_t));
  if(netmgr_nl_decode_netmgr_event(resp_buffer, resp_bufflen, event_info) != NETMGR_SUCCESS)
  {
    netmgr_log_err("%s(): netmgr_nl_decode_netmgr_event failed", __func__);
    goto free_buffers_and_exit;
  }

  if (event_info->event != NETMGR_READY_RESP)
  {
    netmgr_log_err("%s(): Invalid event received in response", __func__);
    goto free_buffers_and_exit;
  }

  rc = NETMGR_SUCCESS;

free_buffers_and_exit:
  if (event_info != NULL)
  {
    netmgr_free(event_info);
  }
  if (req_buffer != NULL)
  {
    netmgr_free(req_buffer);
  }
  if (resp_buffer != NULL)
  {
    netmgr_free(resp_buffer);
  }

  NETMGR_LOG_FUNC_EXIT;
  return rc;
}

/*===========================================================================
  FUNCTION  netmgr_client_send_user_cmd
===========================================================================*/
/*!
@brief
  Function to send proprietary commands to netmgr. @param timeout is the
  interval to wait for a response.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

*/
/*=========================================================================*/
int
netmgr_client_send_user_cmd
(
  int                     cmd,
  netmgr_user_cmd_data_t  *cmd_data,
  netmgr_nl_event_info_t  *resp_info,
  int                     resp_timeout
)
{

  int                     rc = NETMGR_FAILURE;
  char                    *req_buffer = NULL;
  unsigned int            req_bufflen = 0;
  char                    *resp_buffer = NULL;
  unsigned int            resp_bufflen = 0;
  netmgr_nl_event_info_t  *event_info = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  if ((cmd_data == NULL) || (resp_info == NULL))
  {
    netmgr_log_err("%s(): Invalid input arguments", __func__);
    goto free_buffers_and_exit;
  }

  if ((event_info = netmgr_malloc(sizeof(netmgr_nl_event_info_t))) == NULL)
  {
    netmgr_log_err("%s(): Failed to allocate buffer", __func__);
    goto free_buffers_and_exit;
  }

  memset(event_info, 0, sizeof(netmgr_nl_event_info_t));
  event_info->event = NETMGR_USER_CMD;
  event_info->param_mask |= NETMGR_EVT_PARAM_USER_CMD;
  event_info->user_cmd = (unsigned int)cmd;

  /* if cmd_id has associated command data with copy to event_info */
  if (cmd_data != NULL)
  {
    event_info->param_mask |= NETMGR_EVT_PARAM_CMD_DATA;
    memcpy(&(event_info->cmd_data), cmd_data, sizeof(netmgr_user_cmd_data_t));
  }

  if (netmgr_nl_encode_netmgr_event(event_info, &req_buffer, &req_bufflen) != NETMGR_SUCCESS)
  {
    netmgr_log_err("%s(): Failed to encode message", __func__);
    goto free_buffers_and_exit;
  }

  if (((req_buffer != NULL) && (req_bufflen == 0)) || (req_buffer == NULL))
  {
    netmgr_log_err("%s(): Invalid buffer allocated", __func__);
    goto free_buffers_and_exit;
  }

  if ((resp_buffer = netmgr_malloc(NETMGR_UNIX_MAX_MESSAGE_SIZE * sizeof(char *))) == NULL)
  {
    netmgr_log_err("%s(): Failed to allocate buffer", __func__);
    goto free_buffers_and_exit;
  }

  memset(resp_buffer, 0x0, sizeof(NETMGR_UNIX_MAX_MESSAGE_SIZE));
  if (netmgr_unix_ipc_send_msg(req_buffer, req_bufflen, &resp_buffer, &resp_bufflen, resp_timeout)
    != NETMGR_SUCCESS)
  {
    netmgr_log_err("%s(): netmgr_unix_send_msg failed", __func__);
    goto free_buffers_and_exit;
  }

  if(resp_buffer == NULL)
  {
    netmgr_log_err("%s(): Error in recv", __func__);
    goto free_buffers_and_exit;
  }

  if(resp_bufflen == 0)
  {
    netmgr_log_err("%s(): Client closed socket", __func__);
    goto free_buffers_and_exit;
  }

  if(netmgr_nl_decode_netmgr_event(resp_buffer, resp_bufflen, resp_info) != NETMGR_SUCCESS)
  {
    netmgr_log_err("%s(): netmgr_nl_decode_netmgr_event failed", __func__);
    goto free_buffers_and_exit;
  }

  rc = NETMGR_SUCCESS;

free_buffers_and_exit:
  if (event_info != NULL)
  {
    netmgr_free(event_info);
  }
  if (req_buffer != NULL)
  {
    netmgr_free(req_buffer);
  }
  if (resp_buffer != NULL)
  {
    netmgr_free(resp_buffer);
  }

  NETMGR_LOG_FUNC_EXIT;
  return rc;
}

/*===========================================================================
  FUNCTION  netmgr_nl_client_start_listener
===========================================================================*/
/*!
@brief
  Function to start event listener on Netlink socket.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_client_start_nl_listener( void )
{
  NETMGR_LOG_FUNC_ENTRY;

  if( netmgr_client_state.recv_socket.running ) {
    netmgr_log_low("Listener already running\n");
    return NETMGR_SUCCESS;
  }

  /* Initialize NetLink socket interface and listening thread */
  if( NETMGR_SUCCESS !=
      netmgr_nl_listener_init( &netmgr_client_state.recv_socket.sk_thrd_info,
                               netmgr_client_state.recv_socket.sk_thrd_fdmap,
                               ds_arrsize(netmgr_client_state.recv_socket.sk_thrd_fdmap),
                               &netmgr_client_state.recv_socket.ev_sk,
                               NETMGR_NL_TYPE,
                               NETMGR_NL_GRP_EVENTS,
                               netmgr_client_recv_nl_msg ) )
  {
    NETMGR_ABORT("Error on netmgr_nl_init\n");
    return NETMGR_FAILURE;
  }

  netmgr_client_state.recv_socket.running = TRUE;

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_client_stop_nl_listener
===========================================================================*/
/*!
@brief
  Function to stop event listener on Netlink socket.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_client_stop_nl_listener( void )
{
  NETMGR_LOG_FUNC_ENTRY;

  if( !netmgr_client_state.recv_socket.running ) {
    netmgr_log_low("Generic Listener not running\n");
    return NETMGR_SUCCESS;
  }

  /* Teardown Generic socket interface and listening thread */
  if( NETMGR_SUCCESS !=
      netmgr_nl_listener_teardown( &netmgr_client_state.recv_socket.sk_thrd_info,
                                   &netmgr_client_state.recv_socket.ev_sk ) )
  {
    NETMGR_ABORT("netmgr_client_stop_listener: Error on netmgr_nl_init\n");
    return NETMGR_FAILURE;
  }

  netmgr_client_state.recv_socket.running = FALSE;

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_client_recv_tipc_msg
===========================================================================*/
/*!
@brief
  Virtual function registered with the socket listener thread to receive
  incoming messages over the NETLINK event socket.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_client_recv_tipc_msg (int fd)
{
  struct msghdr * msgh = NULL;
  struct sockaddr_nl * nladdr = NULL;
  struct iovec * iov = NULL;
  unsigned int  msglen = 0;
  netmgr_nl_msg_t * nlmsg = NULL;

  NETMGR_CLIENT_LOCK();
  if( fd != netmgr_client_state.recv_socket.ev_sk.sk_fd ) {
    netmgr_log_err( "unexpected socket fd(%d), expected fd(%d)\n",
                    fd, netmgr_client_state.recv_socket.ev_sk.sk_fd );
    NETMGR_CLIENT_UNLOCK();
    return NETMGR_FAILURE;
  }
  NETMGR_CLIENT_UNLOCK();

  /* Allocate event message buffer */
  nlmsg = netmgr_malloc( sizeof(netmgr_nl_msg_t) );
  if( NULL == nlmsg ) {
    netmgr_log_err("failed to allocate message buffer!\n");
    goto bail;
  } else {
    /* Read netlink message from the socket */
    if( NETMGR_SUCCESS != netmgr_tipc_recv_msg( fd, &msgh, &msglen ) ) {
      netmgr_log_err("netmgr_tipc_recv_msg failed!\n");
      goto bail;
    }

    nladdr = msgh->msg_name;
    iov = msgh->msg_iov;

    /* Decode the message in structure */
    memset( nlmsg, 0x0, sizeof(netmgr_nl_msg_t) );
    if( NETMGR_SUCCESS !=
        netmgr_nl_decode_nlmsg( (char*)msgh, msglen, nlmsg) ) {
      netmgr_log_err("netmgr_tipc netmgr decode_nlmsg failed!\n");
      goto bail;
    }

    /* Verify message is NetMgr event type */
    if( NETMGR_NL_PARAM_EVENT & nlmsg->param_mask ) {
      /* Generate indication to client */
      netmgr_client_notify( &nlmsg->event_info );
    } else {
      netmgr_log_low("Unknown Netlink msg type, ignoring!\n" );
    }

    /* Release NetLink message buffer */
    netmgr_free( msgh );
    netmgr_free( nlmsg );
  }
  return NETMGR_SUCCESS;

bail:
  netmgr_free( msgh );
  netmgr_free( nlmsg );
  return NETMGR_FAILURE;
}
/*===========================================================================
  FUNCTION  netmgr_client_start_tipc_listener
===========================================================================*/
/*!
@brief
  Function to start event listener on TIPC socket.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/

static int
netmgr_client_start_tipc_listener( void )
{
  NETMGR_LOG_FUNC_ENTRY;



  if( netmgr_client_state.recv_socket.running ) {
    netmgr_log_low("netmgr Listener already running\n");
    return NETMGR_SUCCESS;
  }

  /* Initialize NetLink socket interface and listening thread */
  if( NETMGR_SUCCESS !=
      netmgr_tipc_listener_init( &netmgr_client_state.recv_socket.sk_thrd_info,
                               netmgr_client_state.recv_socket.sk_thrd_fdmap,
                               ds_arrsize(netmgr_client_state.recv_socket.sk_thrd_fdmap),
                               &netmgr_client_state.recv_socket.ev_sk,
                               netmgr_client_recv_tipc_msg ) )
  {
    NETMGR_ABORT("Error on netmgr_tipc_init\n");
    return NETMGR_FAILURE;
  }

  netmgr_client_state.recv_socket.running = TRUE;

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_client_stop_tipc_listener
===========================================================================*/
/*!
@brief
  Function to stop event listener on TIPC socket.
  TIPC socet teardown is the same as nl_listener_teardown.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_client_stop_tipc_listener( void )
{
  NETMGR_LOG_FUNC_ENTRY;

  if( !netmgr_client_state.recv_socket.running ) {
    netmgr_log_low("Generic Listener not running\n");
    return NETMGR_SUCCESS;
  }

  /* Teardown TIPC socket interface and listening thread */
  if( NETMGR_SUCCESS !=
      netmgr_nl_listener_teardown( &netmgr_client_state.recv_socket.sk_thrd_info,
                                   &netmgr_client_state.recv_socket.ev_sk ) )
  {
    NETMGR_ABORT("%s: Error on netmgr_tipc teardown\n", __func__);
    return NETMGR_FAILURE;
  }

  netmgr_client_state.recv_socket.running = FALSE;

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}
/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  netmgr_client_register
===========================================================================*/
/*!
@brief
  Function to register client for NetMgr asynchronous envents.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_client_register
(
  netmgr_event_ind_f    cb_f,
  void                * data,
  netmgr_client_hdl_t * client_hndl,
  netmgr_client_listen_proto client_proto
)
{
  netmgr_client_t  *client_ptr = NULL;
  unsigned int  index = 0;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT( cb_f );

  if( client_proto > NETMGR_CLIENT_MAX_LISTEN) {
    netmgr_log_low("netmgr Listener invalid proto\n");
    NETMGR_LOG_FUNC_EXIT;
    return NETMGR_FAILURE;
  }

  /* Initialize module state if not cone already */
  if( !netmgr_client_initialized ) {
    memset( (void*)&netmgr_client_state, 0x0, sizeof(netmgr_client_state) );

    if( 0>pthread_mutex_init(&netmgr_client_state.mutex, NULL) ) {
      netmgr_log_sys_err("Failed to initialize mutex");
      NETMGR_LOG_FUNC_EXIT;
      return NETMGR_FAILURE;
    }

    netmgr_client_initialized = TRUE;
  }

  NETMGR_CLIENT_LOCK();

  if( NETMGR_SUCCESS != netmgr_client_alloc( &client_ptr, &index ) ) {
    netmgr_log_err("Failed on netmgr_client_alloc!\n");
    NETMGR_CLIENT_UNLOCK();
    NETMGR_LOG_FUNC_EXIT;
    return NETMGR_FAILURE;
  }
  if( client_ptr ) {
    /* Preserve client info */
    client_ptr->cb_f = cb_f;
    client_ptr->data = data;
  }
  else
  {
    netmgr_log_err("NULL client pointerFailed on netmgr_client_alloc!\n");
    NETMGR_CLIENT_UNLOCK();
    NETMGR_LOG_FUNC_EXIT;
    return NETMGR_FAILURE;
  }

  if( client_proto == NETMGR_CLIENT_TIPC_LISTEN ) {
    /* Start the event listener thread */
   netmgr_log_low("Starting netmgr_client_start_tipc_listener\n");

    if( NETMGR_SUCCESS != netmgr_client_start_tipc_listener() ) {
      netmgr_log_err("Failed on netmgr_client_start_tipc_listener! Fallback\n");
      netmgr_log_low("Starting netmgr_client_start_nl_listener\n");

      if( NETMGR_SUCCESS != netmgr_client_start_nl_listener() ) {

          netmgr_log_err("Failed on netmgr_client_start_listener!\n");
          client_ptr->is_valid = FALSE;
          NETMGR_CLIENT_UNLOCK();
          NETMGR_LOG_FUNC_EXIT;
        return NETMGR_FAILURE;
      }
    }
  }
  else {
    netmgr_log_low("Starting netmgr_client_start_nl_listener\n");

    if( NETMGR_SUCCESS != netmgr_client_start_nl_listener() ) {

      netmgr_log_err("Failed on netmgr_client_start_listener!\n");
      client_ptr->is_valid = FALSE;
      NETMGR_CLIENT_UNLOCK();
      NETMGR_LOG_FUNC_EXIT;
      return NETMGR_FAILURE;
    }
  }
  /* Return client handle.  */
  *client_hndl = CLIENT_MAKE_HNDL( index );

  NETMGR_CLIENT_UNLOCK();

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_client_release
===========================================================================*/
/*!
@brief
  Function to register client for NetMgr asynchronous envents.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_client_release
(
  const netmgr_client_hdl_t client_hndl
)
{
  int index = 0;

  NETMGR_LOG_FUNC_ENTRY;

  if( !IS_VALID_HNDL( client_hndl )) {
    netmgr_log_err("Invalid clinet handle specified!\n");
    NETMGR_LOG_FUNC_EXIT;
    return NETMGR_FAILURE;
  }

  NETMGR_CLIENT_LOCK();

  index = CLIENT_GET_ID_FROM_HNDL( client_hndl );
  /* Stop the event listener thread */
  if( NETMGR_SUCCESS != netmgr_client_stop_tipc_listener() ) {
     netmgr_log_err("netmgr_client_stop_tipc_listener!\n");

    if( NETMGR_SUCCESS != netmgr_client_stop_nl_listener() ) {
      netmgr_log_err("netmgr_client_stop_listener!\n");
    }
  }

  /* Client handle is record address */
  memset( (void*)&netmgr_client_state.client_info[index],
          0x0, sizeof(netmgr_client_state.client_info[index]) );

  NETMGR_CLIENT_UNLOCK();

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_client_thread_wait
===========================================================================*/
/*!
@brief
  Function to wait on client event listener thread.
  THIS IS FOR TEST PURPOSES ONLY.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_client_thread_wait( void )
{
  pthread_t generic_netlink_thread_id;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_CLIENT_LOCK();
  generic_netlink_thread_id = netmgr_client_state.recv_socket.sk_thrd_info.thrd;
  NETMGR_CLIENT_UNLOCK();

  if( 0 != pthread_join( generic_netlink_thread_id, NULL ) ) {
    NETMGR_ABORT("netmgr_client_thread_wait: pthread_join failed\n");
  }

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}
