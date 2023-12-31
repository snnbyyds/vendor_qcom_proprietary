/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
Copyright (c) 2015-2022 by Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

              Diag Qshrink4 Support

GENERAL DESCRIPTION

Implementation of qshrink 4 database reading using diag command request/responses

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
#include <stdlib.h>
#include "comdef.h"
#include "stringl.h"
#include "event.h"
#include "msg.h"
#include "log.h"
#include "diag_lsm.h"
#include "diag_lsmi.h"
#include "diag_shared_i.h"
#include "diagdiag.h"
#include "stdio.h"
#include "string.h"
#include <malloc.h>
#include "diag_lsm_dci.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#if !defined(TARGET_FSM_PRODUCTS) && !defined(FEATURE_LE_DIAG)
#include <log/log.h>
#endif
#include "errno.h"

#define std_strlprintf     snprintf

#define QSR4_DB_CMD_REQ_BUF_SIZE 50
#define QSR4_DB_READ_BUF_SIZE 5000
#define MAX_QSR4_DB_FILE_READ_PER_RSP 4000

int fd_qsr4_xml[NUM_PROC] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
int db_thread_initialized = 0;
qsr4_db_file_parser_state parser_state;
int qsr4_db_file_fd = -1;
int qshrink_kill_thread = 0;
int wait_index = 0;
static int guid_perif_status[NUM_PROC][NUM_PERIPHERALS];
static int periph_up = 0;
char qsr4_file_name_curr[NUM_PROC][FILE_NAME_LEN];
unsigned char qsr4_db_cmd_req_buf[QSR4_DB_CMD_REQ_BUF_SIZE];

extern int use_qmdl2_v2;
extern int use_qmdl2_v2_hdlc_disable;
int qshrink4_filelist_rsp[NUM_PROC] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

typedef PACK(struct) {
   unsigned int peripheral_mask;
   unsigned int peripheral_type;
} qsr4_db_peripheral_info;

typedef PACK(struct) {
	uint32 data1;
	uint16 data2;
	uint16 data3;
	char data4[8];
} GUID;

typedef PACK(struct) {
	uint16 num_read;
	uint32 offset;
	uint8 read_status;
} qsr4_db_file_block_status;

typedef PACK(struct) {
	int peripheral;
	uint8 guid[GUID_LEN];
	uint32 file_len;
	uint8 read_completed;
	uint8 *buf;
	struct qsr4_db_file_list *next;
	uint32 num_blocks;
	int fd;
	qsr4_db_file_block_status *head;
} qsr4_db_file_list;

static qsr4_db_file_list* head = NULL;

struct qsr4_db_write_buf {
	int data_ready;
	qsr4_db_file_list *write_entry;
	pthread_mutex_t read_mutex;
	pthread_mutex_t write_mutex;
	pthread_cond_t write_cond;
	pthread_cond_t read_cond;
};

struct qsr4_db_read_buf_pool {
	unsigned char* db_read_buf;
	int data_ready;
	pthread_mutex_t read_rsp_mutex;
	pthread_mutex_t write_rsp_mutex;
	pthread_cond_t write_rsp_cond;
	pthread_cond_t read_rsp_cond;
};

static struct qsr4_db_read_buf_pool qsr4_db_buffer_pool[2];
struct qsr4_db_write_buf qsr4_db_write_buf_pool[1];

pthread_t qsr4_db_parser_thread_hdl;
pthread_t db_write_thread_hdl;
pthread_mutex_t qsr4_read_db_mutex;
pthread_cond_t qsr4_read_db_cond;

volatile int curr_read_idx = 0;
volatile int curr_write_idx = 0;

qsr4_db_peripheral_info periph_info;
int in_write = 0;
int in_wait_for_peripheral_status = 0;
static int create_qsr4_db_file(qsr4_db_file_list* entry, int peripheral_type)
{
	int qsr4_file_fd = -1;
	char read_buf[100];
	GUID* guid_val;

	if (!entry || (peripheral_type < MSM || peripheral_type > MDM_2))
		return qsr4_file_fd;

	if (qsr4_file_fd < 0) {
		guid_val = (GUID *)&(entry->guid[0]);

		std_strlprintf(read_buf, 100, "%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", guid_val->data1,
			       guid_val->data2, guid_val->data3,guid_val->data4[0], guid_val->data4[1],
			       guid_val->data4[2], guid_val->data4[3], guid_val->data4[4], guid_val->data4[5],
			       guid_val->data4[6], guid_val->data4[7]);
		(void)std_strlprintf(qsr4_file_name_curr[peripheral_type],
							 FILE_NAME_LEN, "%s%s%s%s",
							 output_dir[peripheral_type],"/", read_buf,
							 ".qdb");
		qsr4_file_fd = open(qsr4_file_name_curr[peripheral_type],
							O_CREAT | O_RDWR | O_SYNC | O_TRUNC,
							0644);
	}
	return qsr4_file_fd;

}

static int diag_qsr4_get_cmd_code_for_peripheral(int peripheral)
{
	switch (peripheral) {
	case DIAG_MODEM_PROC:
		return DIAGDIAG_QSR4_FILE_OP_MODEM;
	case DIAG_LPASS_PROC:
		return DIAGDIAG_QSR4_FILE_OP_ADSP;
	case DIAG_WCNSS_PROC:
		return DIAGDIAG_QSR4_FILE_OP_WCNSS;
	case DIAG_SENSORS_PROC:
		return DIAGDIAG_QSR4_FILE_OP_SLPI;
	default:
		return -1;
	}

}

static void insert_diag_qsr4_db_guid_to_list(file_info* db_file_info, int peripheral_type, int peripheral)
{
	qsr4_db_file_list *temp = NULL;
	char guid[100], diagid[30], process_name[100];
	char read_buf[100];
	GUID* guid_val;
	qsr4_db_file_list *entry = NULL;
	int guid_ret, diagid_ret, process_name_ret;
	diag_id_list *item = NULL;

	if (!db_file_info || (peripheral_type < MSM || peripheral_type > MDM_2) ||
		(peripheral <  DIAG_MODEM_PROC || peripheral >  DIAG_SENSORS_PROC))
		return;

	temp = (qsr4_db_file_list*)malloc(sizeof(qsr4_db_file_list));
	if (!temp)
		return;
	memcpy(temp->guid, db_file_info->guid, GUID_LEN);
	guid_val = (GUID*)&(db_file_info->guid[0]);
	if (hdlc_disabled[peripheral_type] || use_qmdl2_v2) {
		add_guid_to_qshrink4_header(&db_file_info->guid[0], peripheral_type, peripheral);
	} else {

		item = get_diag_id(peripheral_type, peripheral);

		std_strlprintf(read_buf, 100, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", guid_val->data1,
			       guid_val->data2, guid_val->data3, guid_val->data4[0], guid_val->data4[1],
			       guid_val->data4[2], guid_val->data4[3], guid_val->data4[4], guid_val->data4[5],
			       guid_val->data4[6], guid_val->data4[7]);

		guid_ret = std_strlprintf(guid, 100, "%s%s%s\n", "<guid>", read_buf, "</guid>");

		if (item) {
			diagid_ret = std_strlprintf(diagid, 30, "%s%02d%s\n", "<diag_id>", item->diag_id, "</diag_id>");

			process_name_ret = std_strlprintf(process_name, 100, "%s%s%s\n", "<process_name>", item->process_name, "</process_name>");
		}
		if (fd_qsr4_xml[peripheral_type] >= 0) {
			write(fd_qsr4_xml[peripheral_type], guid, guid_ret);
			if (item) {
				write(fd_qsr4_xml[peripheral_type], diagid, diagid_ret);
				write(fd_qsr4_xml[peripheral_type], process_name, process_name_ret);
			}
		}
	}
	temp->file_len = db_file_info->file_len;
	temp->peripheral = peripheral;
	temp->read_completed = 0;
	temp->next = NULL;
	temp->head = NULL;
	if (head == NULL)
		head = temp;
	else {
		entry = head;
		while (entry->next != NULL) {
			if (memcmp(entry->guid, temp->guid, GUID_LEN) == 0) {
				free(temp);
				temp = NULL;
				return;
			}
			entry = (qsr4_db_file_list *)(entry->next);
		}
		entry->next = (struct qsr4_db_file_list *)temp;
		temp->next = NULL;
	}

	return;
}

static int diag_qsr4_db_file_mem_init(qsr4_db_file_list** file_entry)
{
	qsr4_db_file_block_status* file_block_offset = NULL;
	qsr4_db_file_list* entry = NULL;
	uint32 num_blocks = 0;

	if (!file_entry)
		return FALSE;

	entry = *file_entry;
	if (!entry)
		return FALSE;

	num_blocks = ((entry->file_len) / MAX_QSR4_DB_FILE_READ_PER_RSP);
	if (((entry->file_len) % MAX_QSR4_DB_FILE_READ_PER_RSP) > 0)
		num_blocks++;

	entry->num_blocks = num_blocks;
	entry->buf = malloc(entry->file_len);
	if (entry->buf == NULL)
		return FALSE;

	file_block_offset = calloc(num_blocks, sizeof(qsr4_db_file_block_status));
	if (file_block_offset == NULL) {
		free(entry->buf);
		return FALSE;
	}
	if (entry->head == NULL)
		entry->head = file_block_offset;

	return TRUE;

}

static int diag_send_qsr4_db_file_list_cmd_req(int peripheral, int peripheral_type)
{
	int offset = 0;
	int length = 0;
	unsigned char* ptr = qsr4_db_cmd_req_buf;
	diag_qsr_header_req* req = NULL;

	if ((peripheral_type < MSM || peripheral_type > MDM_2)||
		(peripheral <  DIAG_MODEM_PROC || peripheral >  DIAG_SENSORS_PROC))
		return FALSE;

	*(int*)ptr = USER_SPACE_RAW_DATA_TYPE;
	offset += sizeof(int);
	if (peripheral_type) {
		*(int*)(ptr + offset) = -peripheral_type;
		offset += sizeof(int);
	}
	ptr = ptr + offset;
	req = (diag_qsr_header_req*)ptr;
	req->cmd_code = DIAG_SUBSYS_CMD_VER_2_F;
	req->subsys_id = DIAG_SUBSYS_DIAG_SERV;
	req->subsys_cmd_code = diag_qsr4_get_cmd_code_for_peripheral(peripheral);
	req->version = 1;
	req->opcode = DIAGDIAG_FILE_LIST_OPERATION;
	length = sizeof(diag_qsr_header_req);
	if (length + offset <= QSR4_DB_CMD_REQ_BUF_SIZE)
		diag_send_data(qsr4_db_cmd_req_buf, offset + length);
	return TRUE;
}

static int process_qsr4_db_file_list_response(int peripheral_type, int peripheral)
{
	int i = 0, counter = 0;
	int number_of_files;
	diag_qsr_file_list_rsp* rsp;
	unsigned char* buf_ptr;

	buf_ptr = &(qsr4_db_buffer_pool[curr_write_idx].db_read_buf[0]);
	if (buf_ptr[0] == DIAG_BAD_CMD_F) {
		qshrink4_filelist_rsp[peripheral_type] = 0;
		return FALSE;
	} else {
		rsp = (diag_qsr_file_list_rsp*)buf_ptr;
		if ((rsp->rsp_header.version != 1) || (rsp->rsp_header.opcode != DIAGDIAG_FILE_LIST_OPERATION)) {
			qshrink4_filelist_rsp[peripheral_type] = 0;
			return FALSE;
		}

		if (rsp->status == 0) {
			number_of_files = rsp->num_files;
			for (i = 0; i < number_of_files; i++) {
				if (rsp->info[i].file_len) {
					insert_diag_qsr4_db_guid_to_list(&rsp->info[i], peripheral_type, peripheral);
					counter++;
				}
				if (!counter) {
					qshrink4_filelist_rsp[peripheral_type] = 0;
					return FALSE;
				}
			}
			qshrink4_filelist_rsp[peripheral_type] = 1;
			return TRUE;
		} else {
			qshrink4_filelist_rsp[peripheral_type] = 0;
			return FALSE;
		}
	}

}

static int diag_send_qsr4_file_open_cmd_req(qsr4_db_file_list* entry, int peripheral_type, int peripheral)
{
	int offset = 0;
	int length = 0;
	unsigned char* ptr = qsr4_db_cmd_req_buf;
	diag_qsr_file_open_req* req_ptr;

	if (!entry || (peripheral_type < MSM || peripheral_type > MDM_2) ||
		(peripheral <  DIAG_MODEM_PROC || peripheral >  DIAG_SENSORS_PROC))
		return FALSE;

	*(int*)ptr = USER_SPACE_RAW_DATA_TYPE;
	offset += sizeof(int);
	if (peripheral_type) {
		*(int*)(ptr + offset) = -peripheral_type;
		offset += sizeof(int);
	}
	ptr = ptr + offset;
	req_ptr = (diag_qsr_file_open_req*)ptr;

	length = sizeof(diag_qsr_file_open_req);
	if (length + offset <= QSR4_DB_CMD_REQ_BUF_SIZE) {
		req_ptr->req.cmd_code = DIAG_SUBSYS_CMD_VER_2_F;
		req_ptr->req.subsys_id = DIAG_SUBSYS_DIAG_SERV;
		req_ptr->req.subsys_cmd_code = diag_qsr4_get_cmd_code_for_peripheral(peripheral);
		req_ptr->req.version = 1;
		req_ptr->req.opcode = DIAGDIAG_FILE_OPEN_OPERATION;
		memcpy(req_ptr->guid, entry->guid, GUID_LEN);
		diag_send_data(qsr4_db_cmd_req_buf, offset + length);
	}
	return TRUE;

}

static int process_qsr_db_file_open_rsp(void)
{
	diag_qsr_file_open_rsp* ptr;
	unsigned char* buf_ptr;
	buf_ptr = &(qsr4_db_buffer_pool[curr_write_idx].db_read_buf[0]);

	if (buf_ptr[0] == DIAG_BAD_CMD_F)
		return -1;
	else {
		ptr = (diag_qsr_file_open_rsp*)buf_ptr;
		if ((ptr->rsp_header.version != 1) || (ptr->rsp_header.opcode != DIAGDIAG_FILE_OPEN_OPERATION))
			return FALSE;
		if (ptr->status == 0)
			return ptr->read_file_fd;
		else
			return -1;
	}
}

static int diag_send_qsr4_file_read_cmd_req(uint16 read_file_fd,
					    int peripheral_type,
					    int peripheral,
					    unsigned int file_offset,
					    int file_len)
{
	int offset = 0;
	int length = 0;
	unsigned char* ptr = qsr4_db_cmd_req_buf;
	diag_qsr_file_read_req* req;

	if ((peripheral_type < MSM || peripheral_type > MDM_2) ||
		(peripheral <  DIAG_MODEM_PROC || peripheral >  DIAG_SENSORS_PROC))
		return FALSE;

	offset += sizeof(int);
	*(int*)ptr = USER_SPACE_RAW_DATA_TYPE;
	if (peripheral_type) {
		*(int*)(ptr + offset) = -peripheral_type;
		offset += sizeof(int);
	}
	ptr = ptr + offset;
	req = (diag_qsr_file_read_req*)ptr;
	req->req.cmd_code = DIAG_SUBSYS_CMD_VER_2_F;
	req->req.subsys_id = DIAG_SUBSYS_DIAG_SERV;
	req->req.subsys_cmd_code = diag_qsr4_get_cmd_code_for_peripheral(peripheral);
	req->req.version = 1;
	req->req.opcode = DIAGDIAG_FILE_READ_OPERATION;
	req->read_file_fd = read_file_fd;
	req->offset = file_offset;
	req->req_bytes = file_len;
	length = sizeof(diag_qsr_file_read_req);
	if (length + offset <= QSR4_DB_CMD_REQ_BUF_SIZE)
		diag_send_data(qsr4_db_cmd_req_buf, offset + length);
	return TRUE;
}

static int process_qsr_db_file_read_rsp(qsr4_db_file_list** file_entry, uint32 offset,
					uint32* ret_bytes, int peripheral_type)
{
	(void) peripheral_type;
	diag_qsr_file_read_rsp* rsp = NULL;
	unsigned char* buf_ptr = NULL;
	(void) file_entry;
	(void) offset;
	(void) ret_bytes;

	buf_ptr = &(qsr4_db_buffer_pool[curr_write_idx].db_read_buf[0]);

	if (buf_ptr[0] == DIAG_BAD_CMD_F)
		return 0;
	else {
		rsp = (diag_qsr_file_read_rsp*)buf_ptr;
		if ((rsp->version != 1) || (rsp->opcode != DIAGDIAG_FILE_READ_OPERATION) ||
			(rsp->status))
			return FALSE;

		return TRUE;
	}

}
static int process_qsr_db_file_read_delayed_rsp(qsr4_db_file_list** file_entry, uint16* rsp_count,
						uint32* ret_bytes, int peripheral_type)
{
	diag_qsr_file_read_rsp* rsp;
	uint16 bytes_read;
	uint8_t* file_block_src_ptr;
	qsr4_db_file_block_status* file_block_offset;
	qsr4_db_file_list* entry = *file_entry;
	unsigned char* buf_ptr;
	int offset_index = 0;
	(void)peripheral_type;

	buf_ptr = &(qsr4_db_buffer_pool[curr_write_idx].db_read_buf[0]);
	if (buf_ptr[0] == DIAG_BAD_CMD_F)
		return FALSE;
	else {
		rsp = (diag_qsr_file_read_rsp*)buf_ptr;
		if ((rsp->version != 1) || (rsp->opcode != DIAGDIAG_FILE_READ_OPERATION) ||
			(rsp->status))
			return FALSE;

		if (rsp->status == 0) {
			bytes_read = rsp->num_read;
			*ret_bytes = rsp->num_read;
			*rsp_count = rsp->rsp_cnt;
			offset_index = ((rsp->offset) / MAX_QSR4_DB_FILE_READ_PER_RSP);
			if((rsp->offset) % MAX_QSR4_DB_FILE_READ_PER_RSP)
				offset_index++;
			file_block_src_ptr = (uint8 *)entry->head;
			file_block_offset = (qsr4_db_file_block_status *) (&file_block_src_ptr[0] + (offset_index*sizeof(qsr4_db_file_block_status)));
			if (file_block_offset != NULL )
				file_block_offset->read_status = 1;
			if (rsp->offset < entry->file_len)
				memcpy(&entry->buf[rsp->offset], rsp->data, bytes_read);
		}

		return TRUE;
	}

}
static int diag_send_qsr4_file_close_send_req(int read_file_fd, int peripheral_type, int peripheral)
{
	int offset = 0;
	int length = 0;
	unsigned char* ptr = qsr4_db_cmd_req_buf;
	diag_qsr_file_close_req* req_ptr;

	if ((peripheral_type < MSM || peripheral_type > MDM_2)||
		(peripheral <  DIAG_MODEM_PROC || peripheral >  DIAG_SENSORS_PROC))
		return FALSE;

	if (read_file_fd < 0) {
		DIAG_LOGE("diag:%s:invalid read_file_fd = %d\n", __func__, read_file_fd);
		return FALSE;
	}

	*(int*)ptr = USER_SPACE_RAW_DATA_TYPE;
	offset += sizeof(int);
	if (peripheral_type) {
		*(int*)(ptr + offset) = -peripheral_type;
		offset += sizeof(int);
	}
	ptr = ptr + offset;
	req_ptr = (diag_qsr_file_close_req*)ptr;

	length = sizeof(diag_qsr_file_close_req);
	if (length + offset <= QSR4_DB_CMD_REQ_BUF_SIZE) {
		req_ptr->req.cmd_code = DIAG_SUBSYS_CMD_VER_2_F;
		req_ptr->req.subsys_id = DIAG_SUBSYS_DIAG_SERV;
		switch (peripheral) {
		case DIAG_MODEM_PROC:
			req_ptr->req.subsys_cmd_code = DIAGDIAG_QSR4_FILE_OP_MODEM;
			break;
		case DIAG_LPASS_PROC:
			req_ptr->req.subsys_cmd_code = DIAGDIAG_QSR4_FILE_OP_ADSP;
			break;
		case DIAG_WCNSS_PROC:
			req_ptr->req.subsys_cmd_code = DIAGDIAG_QSR4_FILE_OP_WCNSS;
			break;
		case DIAG_SENSORS_PROC:
			req_ptr->req.subsys_cmd_code = DIAGDIAG_QSR4_FILE_OP_SLPI;
			break;
		}
		req_ptr->req.version = 1;
		req_ptr->req.opcode = DIAGDIAG_FILE_CLOSE_OPERATION;
		req_ptr->read_file_fd = read_file_fd;
		diag_send_data(qsr4_db_cmd_req_buf, offset + length);
	}
	return TRUE;

}
static int process_qsr_db_file_close_rsp(void)
{
	diag_qsr_file_close_rsp* rsp;
	unsigned char* buf_ptr;
	buf_ptr = &(qsr4_db_buffer_pool[curr_write_idx].db_read_buf[0]);
	qsr4_db_buffer_pool[curr_write_idx].data_ready = 0;
	if (buf_ptr[0] == DIAG_BAD_CMD_F)
		return 0;
	else {
		rsp = (diag_qsr_file_close_rsp*)buf_ptr;
		if ((rsp->rsp_header.version != 1) || (rsp->rsp_header.opcode != DIAGDIAG_FILE_CLOSE_OPERATION))
			return FALSE;
		if (rsp->status == 0) {
			return 1;
		} else
			return 0;
	}

}
static int wait_for_response(void)
{
	struct timespec time;
	struct timeval now;
	int rt = 0;

	gettimeofday(&now, NULL);
	time.tv_sec = now.tv_sec + (10000 / 1000) * 2;
	time.tv_nsec = now.tv_usec + (10000 % 1000) * 1000000;
	wait_index = curr_write_idx;
	pthread_mutex_lock(&(qsr4_db_buffer_pool[curr_write_idx].write_rsp_mutex));
	if (!qsr4_db_buffer_pool[curr_write_idx].data_ready)
		rt = pthread_cond_timedwait(&(qsr4_db_buffer_pool[curr_write_idx].write_rsp_cond), &(qsr4_db_buffer_pool[curr_write_idx].write_rsp_mutex), &time);
	return rt;
}

static int diag_qsr4_reset_db_read_buffer(void)
{
	qsr4_db_buffer_pool[curr_write_idx].data_ready = 0;
	pthread_mutex_lock(&(qsr4_db_buffer_pool[curr_write_idx].read_rsp_mutex));
	pthread_cond_signal(&(qsr4_db_buffer_pool[curr_write_idx].read_rsp_cond));
	pthread_mutex_unlock(&(qsr4_db_buffer_pool[curr_write_idx].read_rsp_mutex));
	pthread_mutex_unlock(&(qsr4_db_buffer_pool[curr_write_idx].write_rsp_mutex));
	curr_write_idx = !curr_write_idx;
	return 0;
}

static int diag_query_file_list_for_guid(int peripheral_type, int peripheral)
{
	int ret, rt;

	parser_state = DB_PARSER_STATE_ON;
	ret = diag_send_qsr4_db_file_list_cmd_req(peripheral, peripheral_type);
	if (!ret || qshrink_kill_thread) {
		if (qshrink_kill_thread) {
			DIAG_LOGE("diag: In %s: Kill initiated (qshrink_kill_thread:%d) while sending File List cmd\n",__func__, qshrink_kill_thread);
		} else {
			DIAG_LOGE("diag: In %s failed to send file list cmd for periph type %d periph %d\n", __func__, peripheral_type, peripheral);
		}
		parser_state = DB_PARSER_STATE_OFF;
		return 0;
	}
	parser_state = DB_PARSER_STATE_LIST;
	rt = wait_for_response();
	if (rt == ETIMEDOUT || qshrink_kill_thread) {
		pthread_mutex_unlock(&(qsr4_db_buffer_pool[curr_write_idx].write_rsp_mutex));
		if (qshrink_kill_thread) {
			DIAG_LOGE("diag: In %s: Kill initiated (qshrink_kill_thread:%d) while waiting for File List rsp\n",__func__, qshrink_kill_thread);
		} else {
			DIAG_LOGE("diag:In %s time out while waiting for file list cmd rsp for periph type:%d periph:%d\n", __func__, peripheral_type, peripheral);
		}
		parser_state = DB_PARSER_STATE_OFF;
		return 0;
	}

	ret = process_qsr4_db_file_list_response(peripheral_type, peripheral);
	diag_qsr4_reset_db_read_buffer();
	if (!ret || qshrink_kill_thread) {
		if (qshrink_kill_thread)
			DIAG_LOGE("diag: In %s: Kill initiated (qshrink_kill_thread:%d) while processing for File list rsp\n",__func__, qshrink_kill_thread);
		parser_state = DB_PARSER_STATE_OFF;
		return 0;
	}

	parser_state = DB_PARSER_STATE_OFF;
	guid_perif_status[peripheral_type][peripheral] = DB_PARSER_STATE_GUID_DOWNLOADED;
	return 0;
}
static int diag_read_qsr4_db_from_peripheral(int peripheral_type, int peripheral)
{
	qsr4_db_file_list* entry;
	int ret;
	uint32 ret_bytes;
	unsigned int offset = 0;
	int read_file_fd = -1;
	unsigned int file_len;
	int status = 0;
	int rt, delete_qdb_file = 0;
	uint32 i;
	uint16 rsp_count = 0;
	qsr4_db_file_block_status* read_entry;
	uint8 * block;
	struct stat file_stat;

	parser_state = DB_PARSER_STATE_ON;
	if (guid_perif_status[peripheral_type][peripheral] == DB_PARSER_STATE_GUID_DOWNLOADED) {
		entry = head;
		while(entry != NULL && entry->read_completed)
			entry = (qsr4_db_file_list *)entry->next;

		while (entry != NULL && entry->read_completed == 0) {
			if (entry->peripheral != peripheral) {
				parser_state = DB_PARSER_STATE_OFF;
				return 0;
			}
			ret = diag_qsr4_db_file_mem_init(&entry);
			if (!ret || qshrink_kill_thread) {
				if (qshrink_kill_thread) {
					DIAG_LOGE("diag: In %s: Kill initiated (qshrink_kill_thread:%d) while mem_init\n",__func__, qshrink_kill_thread);
				} else {
					DIAG_LOGE("diag: In %s failed to create memory to store database file\n",__func__);
				}
				parser_state = DB_PARSER_STATE_OFF;
				return 0;
			}
			file_len = entry->file_len;
			ret = diag_send_qsr4_file_open_cmd_req(entry, peripheral_type, peripheral);
			if (!ret || qshrink_kill_thread) {
				if (qshrink_kill_thread) {
					DIAG_LOGE("diag: In %s: Kill initiated (qshrink_kill_thread:%d) while sending File open cmd\n",__func__, qshrink_kill_thread);
				} else {
					DIAG_LOGE("diag: In %s failed to send file open cmd for periph type %d periph %d\n", __func__, peripheral_type, peripheral);
				}
				free(entry->buf);
				entry->buf = NULL;
				free(entry->head);
				entry->head = NULL;
				entry->fd = -1;
				continue;
			}
			parser_state = DB_PARSER_STATE_OPEN;
			rt = wait_for_response();
			if (rt == ETIMEDOUT || qshrink_kill_thread) {
				pthread_mutex_unlock(&(qsr4_db_buffer_pool[curr_write_idx].write_rsp_mutex));
				if (qshrink_kill_thread) {
					DIAG_LOGE("diag: In %s: Kill initiated (qshrink_kill_thread:%d) while waiting for File open rsp\n",__func__, qshrink_kill_thread);
				} else {
					DIAG_LOGE("diag:In %s time out while waiting for file open cmd rsp for periph type:%d periph:%d\n",__func__, peripheral_type, peripheral);
				}
				free(entry->buf);
				entry->buf = NULL;
				free(entry->head);
				entry->head = NULL;
				entry->fd = -1;
				goto close;
			}
			read_file_fd = process_qsr_db_file_open_rsp();
			diag_qsr4_reset_db_read_buffer();
			if (read_file_fd < 0 || qshrink_kill_thread) {
				if (qshrink_kill_thread)
					DIAG_LOGE("diag: In %s: Kill initiated (qshrink_kill_thread:%d) while processing for File open rsp\n",__func__, qshrink_kill_thread);
				free(entry->buf);
				entry->buf = NULL;
				free(entry->head);
				entry->head = NULL;
				entry->fd = -1;
				goto close;
			}
			pthread_mutex_lock(&(qsr4_db_write_buf_pool[0].write_mutex));
			qsr4_db_file_fd = create_qsr4_db_file(entry, peripheral_type);
			if (qsr4_db_file_fd < 0 || qshrink_kill_thread) {
				pthread_mutex_unlock(&(qsr4_db_write_buf_pool[0].write_mutex));
				if (qshrink_kill_thread) {
					DIAG_LOGE("diag: In %s: Kill initiated (qshrink_kill_thread:%d) while creating qsr4 db file\n",__func__, qshrink_kill_thread);
				} else {
					DIAG_LOGE("diag: In %s Failed to create database file for periph type %d periph %d\n", __func__, peripheral_type, peripheral);
				}
				free(entry->buf);
				entry->buf = NULL;
				free(entry->head);
				entry->head = NULL;
				entry->fd = -1;
				goto close;
			}
			pthread_mutex_unlock(&(qsr4_db_write_buf_pool[0].write_mutex));
			if (read_file_fd >= 0) {
				ret = diag_send_qsr4_file_read_cmd_req(read_file_fd, peripheral_type, peripheral, offset, file_len);
				if (!ret || qshrink_kill_thread) {
					if (qshrink_kill_thread) {
						DIAG_LOGE("diag: In %s: Kill initiated (qshrink_kill_thread:%d) while sending File read cmd\n",__func__, qshrink_kill_thread);
					} else {
						DIAG_LOGE("diag: In %s failed to send file read cmd for periph type %d periph %d\n", __func__, peripheral_type, peripheral);
					}
					free(entry->buf);
					entry->buf = NULL;
					free(entry->head);
					entry->head = NULL;
					entry->fd = -1;
					delete_qdb_file = 1;
					goto close;
				}
				parser_state = DB_PARSER_STATE_READ;
				rt = wait_for_response();
				if (rt == ETIMEDOUT || qshrink_kill_thread) {
					pthread_mutex_unlock(&(qsr4_db_buffer_pool[curr_write_idx].write_rsp_mutex));
					if (qshrink_kill_thread) {
						DIAG_LOGE("diag: In %s: Kill initiated (qshrink_kill_thread:%d) while waiting for File read cmd\n",__func__, qshrink_kill_thread);
					} else {
						DIAG_LOGE("diag:In %s time out while waiting for file read cmd rsp for periph type:%d periph:%d\n",__func__, peripheral_type, peripheral);
					}
					free(entry->buf);
					entry->buf = NULL;
					free(entry->head);
					entry->head = NULL;
					entry->fd = -1;
					delete_qdb_file = 1;
					goto close;
				}
				status = process_qsr_db_file_read_rsp(&entry, offset, &ret_bytes, peripheral_type);
				diag_qsr4_reset_db_read_buffer();
				if (!status || qshrink_kill_thread) {
					if (qshrink_kill_thread)
						DIAG_LOGE("diag: In %s: Kill initiated (qshrink_kill_thread:%d) while processing for File read rsp\n",__func__, qshrink_kill_thread);
					free(entry->buf);
					entry->buf = NULL;
					free(entry->head);
					entry->head = NULL;
					entry->fd = -1;
					delete_qdb_file = 1;
					goto close;
				}
				do {
					rt = wait_for_response();
					if (rt == ETIMEDOUT || qshrink_kill_thread) {
						pthread_mutex_unlock(&(qsr4_db_buffer_pool[curr_write_idx].write_rsp_mutex));
						if (qshrink_kill_thread) {
							DIAG_LOGE("diag: In %s: Kill initiated (qshrink_kill_thread:%d) while waiting for File delayed read rsp\n",__func__, qshrink_kill_thread);
						} else {
							DIAG_LOGE("diag:In %s time out while waiting for file read delayed cmd rsp for periph type:%d periph:%d\n",__func__, peripheral_type, peripheral);
						}
						break;
					}
					status = process_qsr_db_file_read_delayed_rsp(&entry, &rsp_count, &ret_bytes, peripheral_type);
					diag_qsr4_reset_db_read_buffer();
					if (!status || qshrink_kill_thread) {
						if (qshrink_kill_thread)
							DIAG_LOGE("diag: In %s: Kill initiated (qshrink_kill_thread:%d) while processing for File delayed read rsp\n",__func__, qshrink_kill_thread);
						free(entry->buf);
						entry->buf = NULL;
						free(entry->head);
						entry->head = NULL;
						entry->fd = -1;
						delete_qdb_file = 1;
						goto close;
					}
				} while (rsp_count > 0 && rsp_count >= 0x1000);

				/* Resend read command for failed blocks*/
				for (i = 0; i < entry->num_blocks; i++) {
					block = (uint8 *)entry->head;
					read_entry = (qsr4_db_file_block_status*)(&block[0] + (i * (sizeof(qsr4_db_file_block_status))));
					if (!read_entry->read_status) {
						offset = i * MAX_QSR4_DB_FILE_READ_PER_RSP;
						ret = diag_send_qsr4_file_read_cmd_req(read_file_fd, peripheral_type, peripheral, offset, MAX_QSR4_DB_FILE_READ_PER_RSP);
						if (!ret || qshrink_kill_thread) {
							if (qshrink_kill_thread) {
								DIAG_LOGE("diag: In %s: Kill initiated (qshrink_kill_thread:%d) while sending File read cmd\n",__func__, qshrink_kill_thread);
							} else {
								DIAG_LOGE("diag: In %s failed to send file read cmd for periph type %d periph %d\n", __func__, peripheral_type, peripheral);
							}
							free(entry->buf);
							entry->buf = NULL;
							free(entry->head);
							entry->head = NULL;
							entry->fd = -1;
							delete_qdb_file = 1;
							goto close;
						}
						parser_state = DB_PARSER_STATE_READ;
						rt = wait_for_response();
						if (rt == ETIMEDOUT || qshrink_kill_thread) {
							pthread_mutex_unlock(&(qsr4_db_buffer_pool[curr_write_idx].write_rsp_mutex));
							if (qshrink_kill_thread) {
								DIAG_LOGE("diag: In %s: Kill initiated (qshrink_kill_thread:%d) while waiting for File read rsp\n",__func__, qshrink_kill_thread);
							} else {
								DIAG_LOGE("diag:In %s time out while waiting for file read cmd rsp for periph type:%d periph:%d\n",__func__, peripheral_type, peripheral);
							}
							free(entry->buf);
							entry->buf = NULL;
							free(entry->head);
							entry->head = NULL;
							entry->fd = -1;
							delete_qdb_file = 1;
							goto close;
						}
						status = process_qsr_db_file_read_rsp(&entry, offset, &ret_bytes, peripheral_type);
						diag_qsr4_reset_db_read_buffer();
						if (!status || qshrink_kill_thread) {
							if (qshrink_kill_thread)
								DIAG_LOGE("diag: In %s: Kill initiated (qshrink_kill_thread:%d) while processing file read rsp\n",__func__, qshrink_kill_thread);
							free(entry->buf);
							entry->buf = NULL;
							free(entry->head);
							entry->head = NULL;
							entry->fd = -1;
							delete_qdb_file = 1;
							goto close;
						}
						rt = wait_for_response();
						if (rt == ETIMEDOUT || qshrink_kill_thread) {
							pthread_mutex_unlock(&(qsr4_db_buffer_pool[curr_write_idx].write_rsp_mutex));
							if (qshrink_kill_thread) {
								DIAG_LOGE("diag: In %s: Kill initiated (qshrink_kill_thread:%d) while waiting for file delayed read rsp\n",__func__, qshrink_kill_thread);
							} else {
								DIAG_LOGE("diag:In %s time out while waiting for file read delayed cmd rsp for periph type:%d periph:%d\n",__func__, peripheral_type, peripheral);
							}
							free(entry->buf);
							entry->buf = NULL;
							free(entry->head);
							entry->head = NULL;
							entry->fd = -1;
							delete_qdb_file = 1;
							goto close;
						}
						status = process_qsr_db_file_read_delayed_rsp(&entry, &rsp_count, &ret_bytes, peripheral_type);
						diag_qsr4_reset_db_read_buffer();
						if (!status || qshrink_kill_thread) {
							if (qshrink_kill_thread)
								DIAG_LOGE("diag: In %s: Kill initiated (qshrink_kill_thread:%d) while processing file delayed read rsp\n",__func__, qshrink_kill_thread);
							free(entry->buf);
							entry->buf = NULL;
							free(entry->head);
							entry->head = NULL;
							entry->fd = -1;
							delete_qdb_file = 1;
							goto close;
						}
					}
				}

				entry->read_completed = 1;
				qsr4_db_write_buf_pool[0].write_entry = entry;
				entry->fd = qsr4_db_file_fd;
				pthread_mutex_lock(&(qsr4_db_write_buf_pool[0].write_mutex));
				pthread_mutex_lock(&(qsr4_db_write_buf_pool[0].read_mutex));
				if (qsr4_db_write_buf_pool[0].data_ready) {
					pthread_mutex_unlock(&(qsr4_db_write_buf_pool[0].write_mutex));
					pthread_cond_wait(&(qsr4_db_write_buf_pool[0].read_cond),
									  &(qsr4_db_write_buf_pool[0].read_mutex));
					pthread_mutex_lock(&(qsr4_db_write_buf_pool[0].write_mutex));
				}

				pthread_mutex_unlock(&(qsr4_db_write_buf_pool[0].read_mutex));
				qsr4_db_write_buf_pool[0].data_ready = 1;
				pthread_cond_signal(&qsr4_db_write_buf_pool[0].write_cond);
				pthread_mutex_unlock(&(qsr4_db_write_buf_pool[0].write_mutex));
			close:
				if (delete_qdb_file &&
					!stat(qsr4_file_name_curr[peripheral_type], &file_stat)) {
					/* Convert size to KB */
					file_stat.st_size /= 1024;
					if (unlink(qsr4_file_name_curr[peripheral_type])) {
						DIAG_LOGE("diag: %s, Unable to delete qdb file: %s, errno: %d\n",
								__func__, qsr4_file_name_curr[peripheral_type], errno);
					} else {
						DIAG_LOGE("diag: %s, Deleting qdb %s of size %lld KB\n",
								__func__, qsr4_file_name_curr[peripheral_type],
								(long long int)file_stat.st_size);
					}
				}

				ret = diag_send_qsr4_file_close_send_req(read_file_fd, peripheral_type, peripheral);
				if (!ret || qshrink_kill_thread) {
					if (qshrink_kill_thread) {
						DIAG_LOGE("diag: In %s: Kill initiated (qshrink_kill_thread:%d) while sending file close cmd\n",__func__, qshrink_kill_thread);
					} else {
						DIAG_LOGE("diag: In %s failed to send file close cmd for periph type %d periph %d\n", __func__, peripheral_type, peripheral);
					}
					parser_state = DB_PARSER_STATE_OFF;
					return 0;
				}
				parser_state = DB_PARSER_STATE_CLOSE;
				rt = wait_for_response();
				if (rt == ETIMEDOUT || qshrink_kill_thread) {
					pthread_mutex_unlock(&(qsr4_db_buffer_pool[curr_write_idx].write_rsp_mutex));
					if (qshrink_kill_thread) {
						DIAG_LOGE("diag: In %s: Kill initiated (qshrink_kill_thread:%d) while waiting for file close rsp\n",__func__, qshrink_kill_thread);
					} else {
						DIAG_LOGE("diag:In %s time out while waiting for file close cmd rsp for periph type:%d periph:%d\n",__func__, peripheral_type, peripheral);
					}
					if (entry->buf){
						free(entry->buf);
						entry->buf = NULL;
					}
					if (entry->head){
						free(entry->head);
						entry->head = NULL;
					}
					entry->fd = -1;
					continue;
				}
				process_qsr_db_file_close_rsp();
				diag_qsr4_reset_db_read_buffer();
				entry = (qsr4_db_file_list*) entry->next;
				offset = 0;
				i = 0;
				if (qshrink_kill_thread) {
					parser_state = DB_PARSER_STATE_OFF;
					return 0;
				}

			}
		}
	}
	parser_state = DB_PARSER_STATE_OFF;
	return 0;
}
void diag_notify_parser_thread(int peripheral_type, int peripheral_mask)
{
	periph_info.peripheral_type = peripheral_type;
	periph_info.peripheral_mask = peripheral_mask;
	DIAG_LOGD("diag: In %s waiting for 1 second for peripheral to complete its registration entries \n", __func__);
	sleep(1);
	pthread_mutex_lock(&qsr4_read_db_mutex);
	periph_up = 1;
	pthread_cond_signal(&qsr4_read_db_cond);
	pthread_mutex_unlock(&qsr4_read_db_mutex);
}

static int diag_qshrink4_query_file_list(void)
{
	int dev_idx;

	parser_state = DB_PARSER_STATE_OFF;
	for (dev_idx = NUM_PROC-1; dev_idx >= 0; dev_idx--) {
		if (periph_info.peripheral_type & (1 << dev_idx)) {
			if (periph_info.peripheral_mask & DIAG_CON_MPSS) {
				diag_query_file_list_for_guid(dev_idx, DIAG_MODEM_PROC);
				if (qshrink_kill_thread)
					return 0;
			}

			if (periph_info.peripheral_mask & DIAG_CON_WCNSS) {
				diag_query_file_list_for_guid(dev_idx, DIAG_WCNSS_PROC);
				if (qshrink_kill_thread)
					return 0;
			}

			continue;

			if (periph_info.peripheral_mask & DIAG_CON_LPASS) {
				diag_query_file_list_for_guid(dev_idx, DIAG_LPASS_PROC);
				if (qshrink_kill_thread)
					return 0;
			}

			if (periph_info.peripheral_mask & DIAG_CON_SENSORS) {
				diag_query_file_list_for_guid(dev_idx, DIAG_SENSORS_PROC);
				if (qshrink_kill_thread)
					return 0;
			}
			if (periph_info.peripheral_mask & DIAG_CON_WDSP) {
				if (qshrink_kill_thread)
					return 0;
			}
			if (periph_info.peripheral_mask & DIAG_CON_CDSP) {
				if (qshrink_kill_thread)
					return 0;
			}
			if (periph_info.peripheral_mask & DIAG_CON_NPU) {
				if (qshrink_kill_thread)
					return 0;
			}
			if (periph_info.peripheral_mask & DIAG_CON_NSP1) {
				if (qshrink_kill_thread)
					return 0;
			}
			if (periph_info.peripheral_mask & DIAG_CON_GPDSP0) {
				if (qshrink_kill_thread)
					return 0;
			}
			if (periph_info.peripheral_mask & DIAG_CON_GPDSP1) {
				if (qshrink_kill_thread)
					return 0;
			}
			if (periph_info.peripheral_mask & DIAG_CON_HELIOS_M55) {
				if (qshrink_kill_thread)
					return 0;
			}
			if (periph_info.peripheral_mask & DIAG_CON_APSS) {
				if (qshrink_kill_thread)
					return 0;
			}
		}
	}
	return 0;
}

static void* diag_qshrink4_db_parser_thread(void* param)
{
	int rc = 0;
	unsigned int local_peripheral_mask;
	(void)param;
	sigset_t set;
	int dev_idx;

	if ((sigemptyset((sigset_t *) &set) == -1) ||
	(sigaddset(&set, SIGUSR2) == -1) ||
	(sigaddset(&set, SIGTERM) == -1) ||
	(sigaddset(&set, SIGHUP) == -1) ||
	(sigaddset(&set, SIGUSR1) == -1) ||
	(sigaddset(&set, SIGINT) == -1))
		DIAG_LOGE("diag:%s: Failed to initialize block set\n", __func__);

	rc = pthread_sigmask(SIG_BLOCK, &set, NULL);
	if (rc != 0)
		DIAG_LOGE("diag:%s: Failed to block signal for qshrink parser thread\n", __func__);

	parser_state = DB_PARSER_STATE_OFF;
	while (1) {
		local_peripheral_mask = periph_info.peripheral_mask;
		for (dev_idx = NUM_PROC-1; dev_idx >= 0; dev_idx--) {
			if (periph_info.peripheral_type & (1 << dev_idx)) {
				if (periph_info.peripheral_mask & DIAG_CON_MPSS) {
					diag_read_qsr4_db_from_peripheral(dev_idx, DIAG_MODEM_PROC);
					periph_info.peripheral_mask = periph_info.peripheral_mask ^ DIAG_CON_MPSS;
					if (qshrink_kill_thread)
						return 0;
				}

				if (periph_info.peripheral_mask & DIAG_CON_WCNSS) {
					diag_read_qsr4_db_from_peripheral(dev_idx, DIAG_WCNSS_PROC);
					periph_info.peripheral_mask = periph_info.peripheral_mask ^ DIAG_CON_WCNSS;
					if (qshrink_kill_thread)
						return 0;
				}

				periph_info.peripheral_type = periph_info.peripheral_type ^ (1 << dev_idx);
				periph_info.peripheral_mask = local_peripheral_mask;
				continue;

				if (periph_info.peripheral_mask & DIAG_CON_LPASS) {
					diag_read_qsr4_db_from_peripheral(dev_idx, DIAG_LPASS_PROC);
					periph_info.peripheral_mask = periph_info.peripheral_mask ^ DIAG_CON_LPASS;
					if (qshrink_kill_thread)
						return 0;
				}

				if (periph_info.peripheral_mask & DIAG_CON_SENSORS) {
					diag_read_qsr4_db_from_peripheral(dev_idx, DIAG_SENSORS_PROC);
					periph_info.peripheral_mask = periph_info.peripheral_mask ^ DIAG_CON_SENSORS;
					if (qshrink_kill_thread)
						return 0;
				}
				if (periph_info.peripheral_mask & DIAG_CON_WDSP) {
					periph_info.peripheral_mask = periph_info.peripheral_mask ^ DIAG_CON_WDSP;
					if (qshrink_kill_thread)
						return 0;
				}
				if (periph_info.peripheral_mask & DIAG_CON_CDSP) {
					periph_info.peripheral_mask = periph_info.peripheral_mask ^ DIAG_CON_CDSP;
					if (qshrink_kill_thread)
						return 0;
				}
				if (periph_info.peripheral_mask & DIAG_CON_NPU) {
					periph_info.peripheral_mask = periph_info.peripheral_mask ^ DIAG_CON_NPU;
					if (qshrink_kill_thread)
						return 0;
				}
				if (periph_info.peripheral_mask & DIAG_CON_NSP1) {
					periph_info.peripheral_mask = periph_info.peripheral_mask ^ DIAG_CON_NSP1;
					if (qshrink_kill_thread)
						return 0;
				}
				if (periph_info.peripheral_mask & DIAG_CON_GPDSP0) {
					periph_info.peripheral_mask = periph_info.peripheral_mask ^ DIAG_CON_GPDSP0;
					if (qshrink_kill_thread)
						return 0;
				}
				if (periph_info.peripheral_mask & DIAG_CON_GPDSP1) {
					periph_info.peripheral_mask = periph_info.peripheral_mask ^ DIAG_CON_GPDSP1;
					if (qshrink_kill_thread)
						return 0;
				}
				if (periph_info.peripheral_mask & DIAG_CON_HELIOS_M55) {
					periph_info.peripheral_mask = periph_info.peripheral_mask ^ DIAG_CON_HELIOS_M55;
					if (qshrink_kill_thread)
						return 0;
				}
				if (periph_info.peripheral_mask & DIAG_CON_APSS) {
					periph_info.peripheral_mask = periph_info.peripheral_mask ^ DIAG_CON_APSS;
					if (qshrink_kill_thread)
						return 0;
				}
			}
		}
		periph_info.peripheral_mask = 0;
		pthread_mutex_lock(&qsr4_read_db_mutex);
		while (!periph_info.peripheral_mask) {
			in_wait_for_peripheral_status = 1;
			if (!periph_up)
				pthread_cond_wait(&qsr4_read_db_cond, &qsr4_read_db_mutex);
			periph_up = 0;
			in_wait_for_peripheral_status = 0;
			if (qshrink_kill_thread)
				return 0;
		}
		pthread_mutex_unlock(&qsr4_read_db_mutex);
	}
	return 0;
}

static void* diag_write_qshrink4_db_to_disk_thread(void* param)
{
	qsr4_db_file_list* entry;
	int ret, rc = 0;
	(void) param;
	sigset_t set;

	if ((sigemptyset((sigset_t *) &set) == -1) ||
	(sigaddset(&set, SIGUSR2) == -1) ||
	(sigaddset(&set, SIGTERM) == -1) ||
	(sigaddset(&set, SIGHUP) == -1) ||
	(sigaddset(&set, SIGUSR1) == -1) ||
	(sigaddset(&set, SIGINT) == -1))
		DIAG_LOGE("diag:%s: Failed to initialize block set\n", __func__);

	rc = pthread_sigmask(SIG_BLOCK, &set, NULL);
	if (rc != 0)
		DIAG_LOGE("diag:%s: Failed to block signal for qshrink writer thread\n", __func__);

	while (1) {

		pthread_mutex_lock(&qsr4_db_write_buf_pool[0].write_mutex);
		while (!qsr4_db_write_buf_pool[0].data_ready) {
			in_write = 1;
			pthread_cond_wait(&qsr4_db_write_buf_pool[0].write_cond, &qsr4_db_write_buf_pool[0].write_mutex);
			in_write = 0;
			if (qshrink_kill_thread)
				return 0;
		}
		entry = qsr4_db_write_buf_pool[0].write_entry;
		if (entry && entry->fd >= 0) {
			ret = write(entry->fd, entry->buf, entry->file_len);
			if (ret < 0) {
				DIAG_LOGE("diag:failed to write qsr4 db file err is %d\n", errno);
			}
			close(entry->fd);
			entry->fd = -1;
			if (entry->buf) {
				free(entry->buf);
				entry->buf = NULL;
			}
			if (entry->head) {
				free(entry->head);
				entry->head = NULL;
			}
		}
		qsr4_db_write_buf_pool[0].data_ready = 0;
		qsr4_db_write_buf_pool[0].write_entry = NULL;
		qsr4_db_file_fd = -1;
		pthread_mutex_lock(&(qsr4_db_write_buf_pool[0].read_mutex));
		pthread_cond_signal(&(qsr4_db_write_buf_pool[0].read_cond));
		pthread_mutex_unlock(&(qsr4_db_write_buf_pool[0].read_mutex));
		pthread_mutex_unlock(&(qsr4_db_write_buf_pool[0].write_mutex));

		if (qshrink_kill_thread)
			return 0;
	}
	return 0;
}

int create_diag_qshrink4_db_parser_thread(unsigned int peripheral_mask,unsigned int device_mask)
{
	uint16 remote_mask = 0;

	pthread_mutex_init(&qsr4_read_db_mutex, NULL);
	pthread_mutex_init(&(qsr4_db_buffer_pool[0].read_rsp_mutex), NULL);
	pthread_mutex_init(&(qsr4_db_buffer_pool[1].read_rsp_mutex), NULL);
	pthread_mutex_init(&(qsr4_db_buffer_pool[0].write_rsp_mutex), NULL);
	pthread_mutex_init(&(qsr4_db_buffer_pool[1].write_rsp_mutex), NULL);
	pthread_cond_init(&(qsr4_db_buffer_pool[0].read_rsp_cond), NULL);
	pthread_cond_init(&(qsr4_db_buffer_pool[0].write_rsp_cond), NULL);
	pthread_cond_init(&(qsr4_db_buffer_pool[1].read_rsp_cond), NULL);
	pthread_cond_init(&(qsr4_db_buffer_pool[1].write_rsp_cond), NULL);

	qsr4_db_buffer_pool[0].data_ready = 0;
	qsr4_db_buffer_pool[1].data_ready = 0;
	pthread_cond_init(&qsr4_read_db_cond, NULL);

	pthread_mutex_init(&(qsr4_db_write_buf_pool[0].write_mutex), NULL);
	pthread_cond_init(&(qsr4_db_write_buf_pool[0].write_cond), NULL);
	qsr4_db_write_buf_pool[0].data_ready = 0;

	diag_has_remote_device(&remote_mask);
	if (peripheral_mask) {
		periph_info.peripheral_type |= DIAG_MSM_MASK;
		periph_info.peripheral_mask = peripheral_mask;
	} else {
		periph_info.peripheral_mask = DIAG_CON_ALL & ~DIAG_CON_APSS;

		if (device_mask & DIAG_MSM_MASK)
			periph_info.peripheral_type = DIAG_MSM_MASK | (device_mask & (remote_mask << 1));
		else if ((device_mask & DIAG_MDM_MASK) || (device_mask & DIAG_MDM2_MASK))
			periph_info.peripheral_type = device_mask & (remote_mask << 1);
	}

	qsr4_db_buffer_pool[0].db_read_buf = malloc(QSR4_DB_READ_BUF_SIZE);

	if (!qsr4_db_buffer_pool[0].db_read_buf){
		DIAG_LOGE("diag:failed to create data base read buffer 0\n");
		return FALSE;
	}

	qsr4_db_buffer_pool[1].db_read_buf = malloc(QSR4_DB_READ_BUF_SIZE);
	if (!qsr4_db_buffer_pool[1].db_read_buf){
		DIAG_LOGE("diag:failed to create data base read buffer 1\n");
		free(qsr4_db_buffer_pool[0].db_read_buf);
		return FALSE;
	}

	db_thread_initialized = 1;
	diag_qshrink4_query_file_list();
	db_thread_initialized = 0;

	pthread_create(&qsr4_db_parser_thread_hdl, NULL, diag_qshrink4_db_parser_thread, NULL);
	if (qsr4_db_parser_thread_hdl == 0) {
		DIAG_LOGE("diag: Failed to create database parser thread\n");
		free(qsr4_db_buffer_pool[0].db_read_buf);
		free(qsr4_db_buffer_pool[1].db_read_buf);
		return FALSE;
	}

	pthread_create(&db_write_thread_hdl, NULL, diag_write_qshrink4_db_to_disk_thread, NULL);
	if (db_write_thread_hdl == 0) {
		DIAG_LOGE("diag: Failed to create database write thread\n");
		free(qsr4_db_buffer_pool[0].db_read_buf);
		free(qsr4_db_buffer_pool[1].db_read_buf);
		return FALSE;
	}
	db_thread_initialized = 1;
	return TRUE;
}

static int check_for_qsr_db_file_op_cmd(uint8* src_ptr)
{
	uint16 cmd_code;
	uint16 version;
	uint16 opcode;
	unsigned int i ;
	unsigned int offset = 0;
	if (!src_ptr)
		return FALSE;

	if (((*src_ptr == DIAG_SUBSYS_CMD_VER_2_F && *(src_ptr + 1) == DIAG_SUBSYS_DIAG_SERV) ||
		 (*src_ptr == DIAG_BAD_CMD_F && *(src_ptr + 1) == DIAG_SUBSYS_CMD_VER_2_F &&
		  *(src_ptr + 2) == DIAG_SUBSYS_DIAG_SERV)))
	{
		if (*src_ptr == DIAG_SUBSYS_CMD_VER_2_F) {
			memcpy(&cmd_code, src_ptr + 2, sizeof(cmd_code));
			for (i = 8; i < 12; i++)
				if (src_ptr[i] == ESC_CHAR || src_ptr[i]== CTRL_CHAR)
					offset++;
				memcpy(&version, src_ptr + 12 + offset, sizeof(version));
				memcpy(&opcode, src_ptr + 14 + offset, sizeof(opcode));
		} else {
			memcpy(&cmd_code,src_ptr + 3, sizeof(cmd_code));
			memcpy(&version, src_ptr + 5, sizeof(version));
			memcpy(&opcode, src_ptr + 7, sizeof(opcode));
		}

		switch (cmd_code) {
		case DIAGDIAG_QSR4_FILE_OP_MODEM:
			break;
		case DIAGDIAG_QSR4_FILE_OP_ADSP:
			break;
		case DIAGDIAG_QSR4_FILE_OP_WCNSS:
			break;
		case DIAGDIAG_QSR4_FILE_OP_SLPI:
			break;
		default:
			return FALSE;
		}
		if(version != 1)
			return FALSE;
		switch (opcode) {
		case DIAGDIAG_FILE_LIST_OPERATION:
			return TRUE;
		case DIAGDIAG_FILE_OPEN_OPERATION:
			return TRUE;
		case DIAGDIAG_FILE_READ_OPERATION:
			return TRUE;
		case DIAGDIAG_FILE_CLOSE_OPERATION:
			return TRUE;
		default:
			return FALSE;
		}

	}
	else
		return FALSE;
}

static void request_qsr4_database_read_buffer(void)
{
	pthread_mutex_lock(&(qsr4_db_buffer_pool[curr_read_idx].write_rsp_mutex));
	pthread_mutex_lock(&(qsr4_db_buffer_pool[curr_read_idx].read_rsp_mutex));
	if (qsr4_db_buffer_pool[curr_read_idx].data_ready) {
		pthread_mutex_unlock(&(qsr4_db_buffer_pool[curr_read_idx].write_rsp_mutex));
		pthread_cond_wait(&(qsr4_db_buffer_pool[curr_read_idx].read_rsp_cond),
						  &(qsr4_db_buffer_pool[curr_read_idx].read_rsp_mutex));
		pthread_mutex_lock(&(qsr4_db_buffer_pool[curr_read_idx].write_rsp_mutex));
	}
	pthread_mutex_unlock(&(qsr4_db_buffer_pool[curr_read_idx].read_rsp_mutex));
}

int parse_data_for_qsr4_db_file_op_rsp(uint8* ptr, int count_received_bytes, int index)
{
	uint8_t* src_ptr = NULL;
	unsigned char* dest_ptr = NULL;
	unsigned int src_length = 0, dest_length = 0;
	unsigned int len = 0;
	unsigned int i;
	uint8_t src_byte;
	int bytes_read = 0;
	uint16_t payload_len = 0;

	if (qshrink_kill_thread)
		return -1;

	while (bytes_read < count_received_bytes) {

		src_ptr = ptr + bytes_read;
		src_length = count_received_bytes - bytes_read;

		if (hdlc_disabled[index]) {
			payload_len = *(uint16_t *)(src_ptr + 2);
			if (db_thread_initialized && check_for_qsr_db_file_op_cmd(src_ptr + 4))
			{
				request_qsr4_database_read_buffer();
				dest_ptr = &(qsr4_db_buffer_pool[curr_read_idx].db_read_buf[0]);
				dest_length = QSR4_DB_READ_BUF_SIZE;
				memcpy(dest_ptr, src_ptr + 4, payload_len);
				qsr4_db_buffer_pool[curr_read_idx].data_ready = 1;
				pthread_cond_signal(&(qsr4_db_buffer_pool[curr_read_idx].write_rsp_cond));
				pthread_mutex_unlock(&(qsr4_db_buffer_pool[curr_read_idx].write_rsp_mutex));
				curr_read_idx = !curr_read_idx;
				bytes_read += payload_len + 5;

			}
			else
				bytes_read += payload_len + 5;

		} else {
			if (db_thread_initialized && check_for_qsr_db_file_op_cmd(src_ptr)) {
				request_qsr4_database_read_buffer();
				dest_ptr = &(qsr4_db_buffer_pool[curr_read_idx].db_read_buf[0]);
				dest_length = QSR4_DB_READ_BUF_SIZE;
				for (i = 0; i < src_length; i++) {
					src_byte = src_ptr[i];

					if (src_byte == ESC_CHAR) {
						if (i == (src_length - 1)) {
							i++;
							break;
						} else {
							dest_ptr[len++] = src_ptr[++i]
								^ 0x20;
						}
					} else if (src_byte == CTRL_CHAR) {
						if (i == 0 && src_length > 1)
							continue;
						dest_ptr[len++] = src_byte;
						i++;
						break;
					} else {
						dest_ptr[len++] = src_byte;
					}

					if (len >= dest_length) {
						i++;
						break;
					}
				}
				bytes_read += i;
				i = 0;
				len = 0;
				qsr4_db_buffer_pool[curr_read_idx].data_ready = 1;
				pthread_cond_signal(&(qsr4_db_buffer_pool[curr_read_idx].write_rsp_cond));
				pthread_mutex_unlock(&(qsr4_db_buffer_pool[curr_read_idx].write_rsp_mutex));
				curr_read_idx = !curr_read_idx;
			} else {
				for (i = 0; i < src_length; i++) {
					if (src_ptr[i] == CTRL_CHAR) {
						i++;
						break;
					}
				}
				bytes_read += i;
				i = 0;
				len = 0;

			}
		}
	}
	return 0;
}

void diag_kill_qshrink4_threads()
{
	int ret;
	qsr4_db_file_list* entry = NULL;
	qsr4_db_file_list* temp;
	entry = head;

	qshrink_kill_thread = 1;
	DIAG_LOGE("diag: %s: Initiate qshrink threads kill (qshrink_kill_thread: %d)\n",
		__func__, qshrink_kill_thread);

	pthread_cond_signal(&qsr4_db_buffer_pool[wait_index].write_rsp_cond);
	pthread_mutex_unlock(&(qsr4_db_buffer_pool[wait_index].write_rsp_mutex));

	if (in_wait_for_peripheral_status)
		pthread_cond_signal(&qsr4_read_db_cond);

	ret = pthread_join(qsr4_db_parser_thread_hdl, NULL);
	if (ret != 0) {
		DIAG_LOGE("diag: In %s, Error trying to join with qshrink4 read thread: %d\n",
				  __func__, ret);
	}

	if (in_write)
		pthread_cond_signal(&qsr4_db_write_buf_pool[0].write_cond);
	ret = pthread_join(db_write_thread_hdl, NULL);
	if (ret != 0) {
		DIAG_LOGE("diag: In %s, Error trying to join with qshrink4 write thread: %d\n",
				  __func__, ret);
	}

	pthread_mutex_destroy(&qsr4_read_db_mutex);
	pthread_mutex_destroy(&(qsr4_db_buffer_pool[0].read_rsp_mutex));
	pthread_mutex_destroy(&(qsr4_db_buffer_pool[1].read_rsp_mutex));
	pthread_mutex_destroy(&(qsr4_db_buffer_pool[0].write_rsp_mutex));
	pthread_mutex_destroy(&(qsr4_db_buffer_pool[1].write_rsp_mutex));
	pthread_cond_destroy(&(qsr4_db_buffer_pool[0].read_rsp_cond));
	pthread_cond_destroy(&(qsr4_db_buffer_pool[0].write_rsp_cond));
	pthread_cond_destroy(&(qsr4_db_buffer_pool[1].read_rsp_cond));
	pthread_cond_destroy(&(qsr4_db_buffer_pool[1].write_rsp_cond));
	pthread_cond_destroy(&qsr4_read_db_cond);
	pthread_mutex_destroy(&(qsr4_db_write_buf_pool[0].write_mutex));
	pthread_cond_destroy(&(qsr4_db_write_buf_pool[0].write_cond));

	if (qsr4_db_buffer_pool[0].db_read_buf)
		free(qsr4_db_buffer_pool[0].db_read_buf);
	if (qsr4_db_buffer_pool[1].db_read_buf)
		free(qsr4_db_buffer_pool[1].db_read_buf);

	while (entry != NULL) {
		if (entry->buf) {
			free(entry->buf);
			entry->buf = NULL;
		}
		if (entry->head) {
			free(entry->head);
			entry->head = NULL;
		}
		temp = (qsr4_db_file_list *)entry->next;
		free(entry);
		entry = NULL;
		entry = temp;
	}
	DIAG_LOGE("diag:In %s finished killing qshrink4 threads\n", __func__);
}
