/******************************************************************************
#  Copyright (c) 2012, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

/******************************************************************************
  @file    qcril_qmi_pil_monitor.c

  DESCRIPTION
    monitor the state of ADSP from PIL ADSP state file and inform the changes
    to the interested clients

******************************************************************************/

#define __STDC_FORMAT_MACROS 1

#include "qcrili.h"
#include "qcril_other.h"

#ifdef FEATURE_TARGET_GLIBC_x86
   extern "C" size_t strlcat(char *, const char *, size_t);
   extern "C" size_t strlcpy(char *, const char *, size_t);
#endif

#include <pthread.h>
#include <poll.h>
#include <fcntl.h>
#include <sstream>

#define TAG "PIL_Monitor"

#include <framework/Log.h>

#include "qcril_qmi_pil_monitor.h"
#include "qcril_qmi_singleton_agent.h"

class qcril_qmi_pil_monitor : public qcril_qmi_singleton_agent<qcril_qmi_pil_monitor>
{
public:
    static constexpr const char *PIL_DEVICE_NAMES[] = {"adsp", "bg-wear", "slatefw"};

    const qcril_qmi_pil_state& get_pil_state() const { return pil_state; }
    int register_for_state_change(qcril_qmi_pil_state_change_hdlr hdlr);
    int deregister_for_state_change(qcril_qmi_pil_state_change_hdlr hdlr);

private:
    qcril_qmi_pil_monitor();
    void thread_func();

    void set_pil_state(qcril_qmi_pil_state_type new_pil_state);
    void inform_registered_client_pil_changed();
    std::string get_pil_device(std::string name);

private: // data member
    qcril_qmi_pil_state pil_state;

    const static int MAX_REGISTERED_CLIENT = 4;
    qcril_qmi_pil_state_change_hdlr hdlr[MAX_REGISTERED_CLIENT];

friend class qcril_qmi_singleton_agent<qcril_qmi_pil_monitor>;
};

//===========================================================================
// qcril_qmi_pil_monitor::register_for_state_change
//===========================================================================
int qcril_qmi_pil_monitor::register_for_state_change(qcril_qmi_pil_state_change_hdlr hdlr)
{
   int result = -1;

   for (int i=0; i<MAX_REGISTERED_CLIENT; i++)
   {
      if (NULL == this->hdlr[i])
      {
         this->hdlr[i] = hdlr;
         result = 0;
         break;
      }
   }

   QCRIL_LOG_FUNC_RETURN_WITH_RET(result);
   return result;
} // qcril_qmi_pil_monitor::register_for_state_change

//===========================================================================
// qcril_qmi_pil_monitor::deregister_for_state_change
//===========================================================================
int qcril_qmi_pil_monitor::deregister_for_state_change(qcril_qmi_pil_state_change_hdlr hdlr)
{
   int result = -1;

   for (int i=0; i<MAX_REGISTERED_CLIENT; i++)
   {
      if (hdlr == this->hdlr[i])
      {
         this->hdlr[i] = NULL;
         result = 0;
         break;
      }
   }

   QCRIL_LOG_FUNC_RETURN_WITH_RET(result);
   return result;
} // qcril_qmi_pil_monitor::deregister_for_state_change

//===========================================================================
// qcril_qmi_pil_monitor::qcril_qmi_pil_monitor
//===========================================================================
qcril_qmi_pil_monitor::qcril_qmi_pil_monitor() : qcril_qmi_singleton_agent<qcril_qmi_pil_monitor>(QMI_RIL_IMS_PIL_MONITOR_THREAD_NAME)
{
   pil_state.state = QCRIL_QMI_PIL_STATE_UNKNOWN;
   memset(hdlr, 0, sizeof(hdlr));
} // qcril_qmi_pil_monitor::qcril_qmi_pil_monitor

//===========================================================================
// qcril_qmi_pil_monitor::thread_func
//===========================================================================
void qcril_qmi_pil_monitor::thread_func()
{
    QCRIL_LOG_FUNC_ENTRY();
    int   ret;
    char  rd_buf[QCRIL_QMI_PIL_MONITOR_MAX_BUF_SIZE];
    qcril_qmi_pil_state      prev_pil_state;
    qcril_qmi_pil_state_type new_state = QCRIL_QMI_PIL_STATE_UNKNOWN;

    std::vector<struct pollfd> fds;
    std::vector<std::string> devices;

    for (const char *name: PIL_DEVICE_NAMES)
    {
        std::string device_name = get_pil_device(name);
        if (!device_name.empty())
        {
            ret = open(device_name.c_str(), O_RDONLY);
            if (ret >= 0)
            {
                struct pollfd fd{};
                fd.fd = ret;
                fd.events = POLLPRI;

                fds.push_back(fd);
                devices.push_back(device_name);
            }
        }
    }
    QCRIL_LOG_INFO("fds.size = %d", fds.size());
    if (fds.size() > 0)
    {
        while (TRUE)
        {
            ret = poll(fds.data(), fds.size(), -1);
            QCRIL_LOG_INFO("poll result: %d", ret);

            prev_pil_state = get_pil_state();
            new_state = QCRIL_QMI_PIL_STATE_UNKNOWN;

            for (int i = 0; i < fds.size(); i++)
            {
                memset(rd_buf, 0, QCRIL_QMI_PIL_MONITOR_MAX_BUF_SIZE);
                ret = read(fds[i].fd, (void *)rd_buf, QCRIL_QMI_PIL_MONITOR_MAX_BUF_SIZE);
                lseek(fds[i].fd, 0, SEEK_SET);
                QCRIL_LOG_INFO("from device: %s read buf: %s", devices[i].c_str(), rd_buf);

                if (!strncmp(rd_buf, "ONLINE", strlen("ONLINE")))
                {
                    if (new_state != QCRIL_QMI_PIL_STATE_OFFLINE)
                    {
                        new_state = QCRIL_QMI_PIL_STATE_ONLINE;
                    }
                }
                else if (!strncmp(rd_buf, "OFFLINE", strlen("OFFLINE")))
                {
                    new_state = QCRIL_QMI_PIL_STATE_OFFLINE;
                }
            }
            set_pil_state(new_state);

            if (memcmp(&prev_pil_state, &(get_pil_state()), sizeof(qcril_qmi_pil_state)))
            {
                QCRIL_LOG_INFO("state changed from %d to %d. Informing client...",
                    prev_pil_state.state, get_pil_state().state);
                inform_registered_client_pil_changed();
            }
        }
    }
    else
    {
        QCRIL_LOG_INFO("Failed to get PIL device...");
        set_pil_state(QCRIL_QMI_PIL_STATE_UNKNOWN);
    }

    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_pil_monitor::thread_func

//===========================================================================
// qcril_qmi_pil_monitor::set_pil_state
//===========================================================================
void qcril_qmi_pil_monitor::set_pil_state(qcril_qmi_pil_state_type new_pil_state)
{
    QCRIL_LOG_INFO("set new pil state to %d", new_pil_state);
    pil_state.state = new_pil_state;
} // qcril_qmi_pil_monitor::set_pil_state

//===========================================================================
// qcril_qmi_pil_monitor::inform_register_client_pil_change
//===========================================================================
void qcril_qmi_pil_monitor::inform_registered_client_pil_changed()
{
    int i;
    for (i=0; i<MAX_REGISTERED_CLIENT ; i++)
    {
        if (NULL != hdlr[i])
        {
            hdlr[i](&get_pil_state());
        }
    }
} // inform_register_client_pil_change

//===========================================================================
// qcril_qmi_pil_monitor::get_pil_device
//===========================================================================
std::string qcril_qmi_pil_monitor::get_pil_device(std::string name)
{
    QCRIL_LOG_FUNC_ENTRY();

    char device[QCRIL_QMI_PIL_DEVICE_FILE_NAME_SIZE];
    char rd_buf[QCRIL_QMI_PIL_MONITOR_MAX_BUF_SIZE] = {0};
    FILE* fd = NULL;
    std::string dev_name;
    qcril_other_dirlist *dir_list = NULL;
    qcril_other_dirlist *dir_trav = NULL;

    if (!name.empty())
    {
        QCRIL_LOG_INFO("Requested device = %s", name.c_str());
        qmi_ril_retrieve_directory_list(QCRIL_QMI_PIL_DEVICE_DIR,
            QCRIL_QMI_PIL_DEVICE_DIR_SUBSTR, &dir_list);
        dir_trav = dir_list;

        while(dir_trav != NULL)
        {
            memset(device, 0, sizeof(device));
            snprintf(device, sizeof(device),
                QCRIL_QMI_PIL_DEVICE_DIR "/%s/" QCRIL_QMI_PIL_DEVICE_NAME_FILE,
                dir_trav->dir_name);
            fd = fopen(device, "r");
            if (fd != NULL)
            {
                memset(rd_buf, 0, QCRIL_QMI_PIL_MONITOR_MAX_BUF_SIZE);
                fseek(fd, SEEK_SET, 0);
                fread(rd_buf, QCRIL_QMI_PIL_MONITOR_MAX_BUF_SIZE, 1, fd);
                QCRIL_LOG_INFO("PIL device: %s - %s", device, rd_buf);

                if (!strncmp(rd_buf, name.c_str(), name.size()))
                {
                    std::stringstream temp_dev_name;
                    temp_dev_name << QCRIL_QMI_PIL_DEVICE_DIR;
                    temp_dev_name << "/";
                    temp_dev_name << dir_trav->dir_name;
                    temp_dev_name << "/";
                    temp_dev_name << QCRIL_QMI_PIL_DEVICE_STATE_FILE;
                    dev_name = temp_dev_name.str();
                    fclose(fd);
                    break;
                }
                fclose(fd);
            }
            else
            {
                QCRIL_LOG_INFO("Unable to open file %s", device);
            }
            dir_trav = dir_trav->next;
        }

        qmi_ril_free_directory_list(dir_list);
    }
    else
    {
        QCRIL_LOG_ERROR("Invalid inputs");
    }

    QCRIL_LOG_FUNC_RETURN();
    return dev_name;
} // qcril_qmi_pil_monitor::get_pil_device

//===========================================================================
// qcril_qmi_pil_monitor::deregister_for_state_change
//===========================================================================
int qcril_qmi_pil_init_monitor()
{
    return qcril_qmi_pil_monitor::get_instance()->init();
} // qcril_qmi_pil_init_monitor

//===========================================================================
// qcril_qmi_pil_register_for_state_change
//===========================================================================
int qcril_qmi_pil_register_for_state_change(qcril_qmi_pil_state_change_hdlr hdlr)
{
    return qcril_qmi_pil_monitor::get_instance()->register_for_state_change(hdlr);
} // qcril_qmi_pil_register_for_state_change

//===========================================================================
// qcril_qmi_pil_deregister_for_state_change
//===========================================================================
int qcril_qmi_pil_deregister_for_state_change(qcril_qmi_pil_state_change_hdlr hdlr)
{
    return qcril_qmi_pil_monitor::get_instance()->deregister_for_state_change(hdlr);
} // qcril_qmi_pil_deregister_for_state_change

//===========================================================================
// qcril_qmi_pil_get_pil_state
//===========================================================================
const qcril_qmi_pil_state* qcril_qmi_pil_get_pil_state()
{
    return &(qcril_qmi_pil_monitor::get_instance()->get_pil_state());
} // qcril_qmi_pil_get_pil_state
