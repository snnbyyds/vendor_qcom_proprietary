/*
 * Copyright (c) 2019,2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


#ifndef __CNSS_CLI_H__
#define __CNSS_CLI_H__

#define CNSS_CLI_MAX_PAYLOAD 1024

#ifdef ANDROID
#define CNSS_USER_SERVER "/data/vendor/wifi/sockets/cnss_user_server"
#define CNSS_USER_CLIENT "/data/vendor/wifi/sockets/cnss_user_client"
#else
#define CNSS_USER_SERVER "/var/run/cnss_user_server"
#define CNSS_USER_CLIENT "/var/run/cnss_user_client"
#endif

enum cnss_cli_cmd_type {
	CNSS_CLI_CMD_NONE = 0,
	QDSS_TRACE_START,
	QDSS_TRACE_STOP,
	QDSS_TRACE_CONFIG_DOWNLOAD
};

struct cnss_cli_msg_hdr {
	enum cnss_cli_cmd_type type;
	int len;
	int resp_status;
};

struct cnss_cli_qdss_trace_stop_data {
	unsigned long long option;
};

#endif

