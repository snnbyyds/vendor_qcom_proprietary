/* ===================================================================
** Copyright (c) 2017-2019, 2021 Qualcomm Technologies, Inc.
** All Rights Reserved.
** Confidential and Proprietary - Qualcomm Technologies, Inc.
**
** FILE: see_salt.cpp
** DESC:
** ================================================================ */
#include <iostream>

#include "see_salt.h"
#include "salt_usta.h"
using namespace std;
static int salt_instance_num = 0;

see_salt* see_salt::get_instance(see_sensor_stream_path_e stream_path_e)
{
   return new see_salt(stream_path_e);
}

// constructor
see_salt::see_salt(see_sensor_stream_path_e stream_path_e)
{
   _salt_instance_num = ++salt_instance_num;
   _stream_path_e = stream_path_e;
}

see_salt::~see_salt()
{
   usta_destroy_instance( this);
}

void see_salt::append_sensor(sensor &sens)
{
   _sensor_list.push_back(sens);
}

int see_salt::suid_to_handle( sens_uid *target)
{
   for ( int handle = 0; handle < (int)_sensor_list.size(); handle++) {
      sensor *psens = &_sensor_list[handle];
      sens_uid *psuid = psens->get_suid();
      if ( target->low == psuid->low &&
           target->high == psuid->high) {
         return handle;
      }
   }
   char buffer[96];
   sprintf( buffer, "suid [hi %" PRIx64 " lo %" PRIx64 "] not found\n",
            target->high,
            target->low);
   throw std::runtime_error( std::string( buffer));
   return -1;
}

/**
 * @brief send_request message to USTA
 * @param [i] see_client_request_message
 * @return salt_rc
 */
salt_rc see_salt::send_request(see_client_request_message &client_req)
{
   /* get target sensor handle */
   sens_uid *suid = client_req.get_suid();
   int handle = suid_to_handle( suid);
   salt_rc rc = usta_send_request( this, handle, client_req);
   return rc;
}

/**
 * @brief - delete stream or disconnect qmi
 * @param [i] sens_uid
 * @param [i] delete_or_disconnect: true == delete
 * @return int 0 == successful
 */
salt_rc see_salt::stop_request(sens_uid *suid,
                               bool delete_or_disconnect)
{
   int handle = suid_to_handle( suid);
   salt_rc rc = usta_stop_request( this, handle, delete_or_disconnect);
   return rc;
}

std::string see_client_request_message::get_client_symbolic( see_std_client_processor_e client)
{
   if ( client == SEE_STD_CLIENT_PROCESSOR_SSC) { return  "SNS_STD_CLIENT_PROCESSOR_SSC"; }
   if ( client == SEE_STD_CLIENT_PROCESSOR_APSS) { return "SNS_STD_CLIENT_PROCESSOR_APSS"; }
   if ( client == SEE_STD_CLIENT_PROCESSOR_ADSP) { return "SNS_STD_CLIENT_PROCESSOR_ADSP"; }
   if ( client == SEE_STD_CLIENT_PROCESSOR_MDSP) { return "SNS_STD_CLIENT_PROCESSOR_MDSP"; }
   if ( client == SEE_STD_CLIENT_PROCESSOR_CDSP) { return "SNS_STD_CLIENT_PROCESSOR_CDSP"; }
   return "";
}

std::string see_client_request_message::get_wakeup_symbolic( see_client_delivery_e wakeup)
{
   if ( wakeup == SEE_CLIENT_DELIVERY_WAKEUP) { return "SNS_CLIENT_DELIVERY_WAKEUP"; }
   if ( wakeup == SEE_CLIENT_DELIVERY_NO_WAKEUP) { return "SNS_CLIENT_DELIVERY_NO_WAKEUP"; }
   return "";
}

/**
 * This sleep can pull the processor awake from suspend mode
 * @param duration - seconds
 */
void see_salt::sleep( float duration)
{
   cout << "sleep( " << to_string( duration) << ") seconds" << endl;
   uint32_t msec = (uint32_t)( duration * 1000);
   sleep_and_awake( msec);
   cout << "awake" << endl;
}

/**
 * @brief register an event callback function for input sensor
 * @param [i] pointer to sensor's suid
 * @param [i] pointer to event callback function
 */
void see_salt::update_event_cb(sens_uid *suid, event_cb_func event_cb)
{
   if (suid) {
      int handle = suid_to_handle( suid);
      if ( handle >= 0) {
         usta_register_event_cb(this, handle, event_cb);
      }
   }
}

// Direct Channel Interface
#ifdef SNS_SUPPORT_DIRECT_CHANNEL
/**
 * @brief - create DRM channel
 * @param channel info
 * @param channel handle
 * @return int 0 == successful
 */
salt_rc see_salt::create_channel_dc(see_create_dc_channel_info &dc_channel_info, unsigned long &channel_handle)
{
    salt_rc rc = usta_create_dc_channel(this , dc_channel_info, channel_handle);
    return rc;
}

/**
 * @brief - close DRM channel
 * @param channel handle
 * @return int 0 == successful
 */
salt_rc see_salt::close_channel_dc( unsigned long &channel_handle)
{
    salt_rc rc = usta_close_dc_channel(this , channel_handle);
    return rc;
}

/**
 * @brief - send direct channel request
 * @param channel andle
 * @param request info
 * @param false: start request, true: stop request
 * @return int 0 == successful
 */
salt_rc see_salt::send_request_dc(unsigned long &channel_handle, see_dc_std_req_info &req_info, bool is_delete_request_info)
{
    salt_rc rc = usta_send_request_dc_channel(this, channel_handle, req_info, is_delete_request_info);
    return rc;
}

/**
 * @brief - update direct channel offset
 * @param channel handle
 * @param offset
 * @return int 0 == successful
 */
salt_rc see_salt::update_offset_ts_dc(unsigned long &channel_handle, int64_t offset)
{
    salt_rc rc = usta_update_offset_ts_dc_channel(this, channel_handle, offset);
    return rc;
}
#endif

/**
 * @brief - get direct channel sensor attributes
 * @param direct channel attributes structure
 */
void see_salt::get_direct_channel_attributes(direct_channel_attributes &dc_attributes)
{
    usta_get_direct_channel_attributes(this, dc_attributes);
}

