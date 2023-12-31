/******************************************************************************
#  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#ifndef QCRIL_SCWS_H
#define QCRIL_SCWS_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include "framework/Log.h"
#include "qcril_scws_opt.h"


/*===========================================================================

                           DEFINES

===========================================================================*/
#define QCRIL_SCWS_MAX_SERVER_SOCKETS      3
#define QCRIL_SCWS_MAX_CLIENT_SOCKETS     15
#define QCRIL_SCWS_TEMP_BUFFER_SIZE      500


/*===========================================================================

                           TYPES

===========================================================================*/

/* -----------------------------------------------------------------------------
   TYPEDEF:     QCRIL_SCWS_SOCKET_STATE_ENUM_TYPE

   DESCRIPTION:
     This is the type that indicates the socket state.
-------------------------------------------------------------------------------*/
typedef enum
{
  QCRIL_SCWS_SOCKET_STATE_CLOSED,
  QCRIL_SCWS_SOCKET_STATE_LISTEN,
  QCRIL_SCWS_SOCKET_STATE_ESTABLISHED
} qcril_scws_socket_state_enum_type;


/* -----------------------------------------------------------------------------
   TYPEDEF:     QCRIL_SCWS_SLOT_ENUM_TYPE

   DESCRIPTION:
     Slot of the card.
-------------------------------------------------------------------------------*/
typedef enum
{
  QCRIL_SCWS_SLOT_1,
  QCRIL_SCWS_SLOT_2,
  QCRIL_SCWS_SLOT_3
} qcril_scws_slot_enum_type;


/* -----------------------------------------------------------------------------
   TYPEDEF:     QCRIL_SCWS_SEND_DATA_CB_TYPE

   DESCRIPTION:
     This is the callback used by the SCWS Agent to notify the modem when
     data is received from the client (browser) on the socket.
-------------------------------------------------------------------------------*/
typedef void qcril_scws_data_available_cb_type(
  uint32_t                  bip_id,
  qcril_scws_slot_enum_type slot_id,
  uint16_t                  data_len,
  uint8_t *                 data_ptr,
  uint16_t                  remaining_data_len);


/* -----------------------------------------------------------------------------
   TYPEDEF:     QCRIL_SCWS_CHANNEL_STATUS_CB_TYPE

   DESCRIPTION:
     This is the callback used by the SCWS Agent to notify the modem when
     there is a change in the channel status.
-------------------------------------------------------------------------------*/
typedef void qcril_scws_channel_status_cb_type(
  uint32_t                          bip_id,
  qcril_scws_slot_enum_type         slot_id,
  qcril_scws_socket_state_enum_type socket_state);


/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_SCWS_CONNECTED_SOCKET_TYPE

   DESCRIPTION:
     This structure contains the list of connected sockets. The socket descriptor
     can be -1 in case the socket is not in use.
-------------------------------------------------------------------------------*/
typedef struct
{
  int                                     connected_sd;
  boolean                                 valid_bip_id;
  uint32_t                                bip_id;
  uint16_t                                buffer_size;
  uint8_t                               * buffer_ptr;
  qcril_scws_opt_traffic_analyzer_type    traffic_analyzer;
} qcril_scws_connected_socket_type;


/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_SCWS_SERVER_SOCKET_TYPE

   DESCRIPTION:
     This structure contains the value of a server socket.
-------------------------------------------------------------------------------*/
typedef struct
{
  uint16_t                          local_port;
  pthread_t                         thread_id;
  fd_set                            fd_set;
  int                               server_sd;
  qcril_scws_slot_enum_type         slot_id;

  qcril_scws_connected_socket_type  connected_socket[QCRIL_SCWS_MAX_CLIENT_SOCKETS];
} qcril_scws_server_socket_type;



/*=========================================================================

  FUNCTION:  qcril_scws_initalize

===========================================================================*/
/*!
    @brief
    Initializes the SCWS Agent, indicating the callback functions.

    @return
    None
*/
/*=========================================================================*/
void qcril_scws_initalize(
  qcril_scws_data_available_cb_type  * data_available_cb,
  qcril_scws_channel_status_cb_type  * channel_status_cb);


/*=========================================================================

  FUNCTION:  qcril_scws_deinitalize

===========================================================================*/
/*!
    @brief
    De-initializes the SCWS Agent, freeing all allocated resources.

    @return
    None
*/
/*=========================================================================*/
void qcril_scws_deinitalize(void);


/*=========================================================================

  FUNCTION:  qcril_scws_open_channel

===========================================================================*/
/*!
    @brief
    Open channel from the UICC card.

    @return
    int: 0 if success, non 0 if fails
*/
/*=========================================================================*/
int qcril_scws_open_channel(
  uint16_t                  local_port,
  uint32_t                  bip_id,
  qcril_scws_slot_enum_type slot_id);


/*=========================================================================

  FUNCTION:  qcril_scws_close_channel

===========================================================================*/
/*!
    @brief
    Close channel from the UICC card.

    @return
    boolean: indicates if the command is successfull or not
*/
/*=========================================================================*/
boolean qcril_scws_close_channel(
  uint32_t                  bip_id,
  qcril_scws_slot_enum_type slot_id,
  boolean                   close_server);


/*=========================================================================

  FUNCTION:  qcril_scws_send_data

===========================================================================*/
/*!
    @brief
    Send data from the UICC card.

    @return
    boolean: indicates if the command is successfull or not
*/
/*=========================================================================*/
boolean qcril_scws_send_data(
  uint32_t                  bip_id,
  qcril_scws_slot_enum_type slot_id,
  const uint8_t *           data_ptr,
  uint16_t                  data_len);


/*=========================================================================

  FUNCTION:  qcril_scws_data_available_error

===========================================================================*/
/*!
    @brief
    Indicates an error for a previously sent data_available command.

    @return
    None
*/
/*=========================================================================*/
void qcril_scws_data_available_error(
  uint32_t                  bip_id,
  qcril_scws_slot_enum_type slot_id);


/*=========================================================================

  FUNCTION:  qcril_scws_card_error

===========================================================================*/
/*!
    @brief
    Notifies the agent of a card error, so that server socket can be cleaned.

    @return
    None
*/
/*=========================================================================*/
void qcril_scws_card_error(
  qcril_scws_slot_enum_type slot_id);


#endif /* QCRIL_SCWS_H */

