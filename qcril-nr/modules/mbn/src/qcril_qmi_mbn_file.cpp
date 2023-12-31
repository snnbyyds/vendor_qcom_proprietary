/******************************************************************************
#  Copyright (c) 2014, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

/******************************************************************************
  @file    qcril_qmi_mbn_file.c
  @brief   utils for parsing MBN files

  DESCRIPTION
    Provide interface to get MBN configuration information.

******************************************************************************/


//===========================================================================
//
//                           INCLUDE FILES
//
//===========================================================================

#ifdef ANDROID

#ifdef TEST_XML_PARSER_STANDALONE
#undef ANDROID
#endif
#ifdef TEST_MBN_PARSER_STANDALONE
#undef ANDROID
#endif

#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <libgen.h>

#ifndef LIBXML_UNSUPPORTED
#include <libxml/parser.h>
#include <libxml/xpath.h>
#else
#define xmlDocPtr void*
#define xmlNodePtr void*
#define xmlChar unsigned char
#endif /* LIBXML_UNSUPPORTED */

#ifdef ANDROID
#include "qcrili.h"
#include "qmi_errors.h"
#include "qcril_log.h"
#include "modules/mbn/qcril_qmi_pdc.h"
#else
#include <stdint.h>
#endif

#include "qcril_qmi_oem_events.h"
#include "interfaces/mbn/mbn.h"

#ifndef ANDROID
#define QCRIL_LOG_ERROR printf
#define QCRIL_LOG_INFO  printf
#define QCRIL_LOG_FUNC_ENTRY() do {} while(0)
#define QCRIL_LOG_FUNC_RETURN() do {} while(0)
#define qcril_malloc    malloc
#define qcril_free      free

#ifndef QMI_RIL_UTF
typedef enum {
    RIL_E_SUCCESS = 0,
    RIL_E_GENERIC_FAILURE = 2,
} RIL_Errno;
#endif
#endif

//===========================================================================
//
//                    INTERNAL DEFINITIONS AND TYPES
//
//===========================================================================
// MBN file related
#define MCFG_MBN_CONFIG_LEN_OFFSET      10
#define MCFG_MBN_CONFIG_DESC_OFFSET     12
#define MCFG_MBN_QC_VERSION_TYPE_ID     5
#define MCFG_MBN_OEM_VERSION_TYPE_ID    1

#define MCFG_MBN_CONFIG_INFO_MAGIC      "MCFG_TRL"
#define MCFG_MBN_CONFIG_INFO_MAGIC_LEN  8

// XML item format related:
#define PDC_XML_ROOT_NAME               "NvData"
#define PDC_XML_NV_ITEM_NAME            "NvItemData"
#define PDC_XML_EFS_ITEM_NAME           "NvEfsItemData"
#define PDC_XML_REFERENCE_NAME          "NvItemDataREFERENCE"
#define PDC_XML_DEVICE_NAME             "NvItemDataDEVICE"

typedef struct __attribute__ ((__packed__)) {
  uint8_t type;
  uint16_t length;
  uint8_t value[0];
} mcfg_config_info_unit_t;

typedef struct {
  int result;
  int index;
  int id_len;
  int ref_len;
  int dev_len;
  char val_str[0];
} mcfg_diff_unsol_msg_type;

#ifdef ANDROID
#define MBN_DIFF_VALUE_LENGTH_LIMIT     6144
#define TOO_LARGET_STRING               "value too large, not display"
#endif

/*===========================================================================

  FUNCTION: mcfg_get_config_info

===========================================================================*/
/*!
    @brief
    This function will get the config_info for a given MBN file

    @return
    RIL_E_GENERIC_FAILURE -> Failed  RIL_E_SUCCESS -> Success

    @side effect
    If success, the function will allocate the memory, assigned to parameter
    config_info. The caller should free it after use.
*/
/*=========================================================================*/
static int mcfg_get_config_info(const char* file_name, char** config_info, uint32_t* config_len)
{
  int fd = -1;
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  off_t pos;
  uint16_t len;
  uint32_t info_offset;
  char* buf;

  *config_info = NULL;
  *config_len = 0;

  do {
    if ( (fd = open( file_name, O_RDONLY )) == -1 )
    {
      QCRIL_LOG_ERROR("Failed to open file name %s: %s", file_name, strerror(errno));
      break;
    }
    pos = lseek(fd, -4, SEEK_END);
    if ( pos == -1 )
    {
      QCRIL_LOG_ERROR("Failed to seek file: %s", strerror(errno));
      break;
    }
    if ( read(fd, &info_offset, 4) == -1 )
    {
      QCRIL_LOG_ERROR("Failed to read the offset where stores config_info: %s",strerror(errno));
      break;
    }

    /* just jump to the pos that stores length */
    pos = lseek(fd, 0 - (off_t)info_offset + MCFG_MBN_CONFIG_LEN_OFFSET, SEEK_END);
    if ( pos == -1 )
    {
      QCRIL_LOG_ERROR("Failed to seek file: %s", strerror(errno));
      break;
    }
    if ( read(fd, (uint8_t*)&len, sizeof(len)) == -1 )
    {
      QCRIL_LOG_ERROR("Failed to read the length of config_info: %s", strerror(errno));
      break;
    }
    /* now the file pointer points to config_info */
    if ( (buf = static_cast<char *>(qcril_malloc(len))) == NULL )
    {
      QCRIL_LOG_ERROR("Failed to allocate the memory: %s", strerror(errno));
      break;
    }
    if ( read(fd, buf, len) != len )
    {
      qcril_free(buf);
      QCRIL_LOG_ERROR("Failed to read the config_info: %s", strerror(errno));
      break;
    }

    *config_info = buf;
    *config_len = len;
    result = RIL_E_SUCCESS;
  } while (0);

  if (fd != -1)
    close(fd);
  return result;
}

/*===========================================================================

  FUNCTION: mcfg_get_tr_tlv

===========================================================================*/
/*!
    @brief
    This function will get the TLV for a given MBN file and specified
    type id;

    @return
    0 -> Failed  other value -> Success

    @side effect
    If success, the function will allocate the memory, assigned to parameter
    value. The caller should free it after use.
*/
/*=========================================================================*/
static int mcfg_get_tr_tlv(const char* file_name, const char type, char** value, uint32_t* len)
{
  char* config_info = NULL;
  int result = RIL_E_GENERIC_FAILURE;
  uint32_t index = 0;
  uint32_t config_info_len;
  uint16_t length;
  char* buf;

  *value = NULL;
  *len = 0;

  do
  {
    QCRIL_LOG_INFO("file name: %s", file_name);
    if (mcfg_get_config_info(file_name, &config_info, &config_info_len) != RIL_E_SUCCESS)
    {
      QCRIL_LOG_ERROR("Failed to get config_info from file: %s", file_name);
      break;
    }
    /* skip the the MAGIC number part */
    index += MCFG_MBN_CONFIG_INFO_MAGIC_LEN;
    while ( index < config_info_len - 3 )
    {
      QCRIL_LOG_INFO("type_id: %d", config_info[index]);
      if (config_info[index] == type)
        break;
      index++;
      length = (uint16_t)config_info[index] + (((uint16_t)config_info[index+1]) << 8);
      index += 2 + length;
    }
    /* TLV is not found */
    if ( index >= ( config_info_len - 3 ) )
    {
      QCRIL_LOG_ERROR("Failed to found TLV with type id: 0x%02x", type);
      break;
    }

    index++;
    length = (uint16_t)config_info[index] + (((uint16_t)config_info[index+1]) << 8);
    if ( ( buf = static_cast<char *>(qcril_malloc(length))) == NULL )
    {
      QCRIL_LOG_ERROR("Failed to allocate memory for type id: 0x%02x", type);
      break;
    }
    memcpy(buf, config_info+index+2, length);
    *value = buf;
    *len = length;
    result = RIL_E_SUCCESS;
  } while (0);

  if (config_info)
    qcril_free(config_info);

  return result;
}

/*===========================================================================

  FUNCTION: mcfg_get_qc_version

===========================================================================*/
/*!
    @brief
    This function will get the qc_version for a given MBN file

    @return
    0 -> Failed  other value -> Success

    @side effect
    If success, the function will allocate the memory, assigned to parameter
    qc_version. The caller should free it after use.
*/
/*=========================================================================*/
static inline int mcfg_get_qc_version(const char* file_name, char** qc_version, uint32_t* len)
{

  return mcfg_get_tr_tlv(file_name, MCFG_MBN_QC_VERSION_TYPE_ID,
                                qc_version, len);
}

/*===========================================================================

  FUNCTION: mcfg_get_oem_version

===========================================================================*/
/*!
    @brief
    This function will get the oem_version for a given MBN file

    @return
    0 -> Failed  other value -> Success

    @side effect
    If success, the function will allocate the memory, assigned to parameter
    oem_version. The caller should free it after use.
*/
/*=========================================================================*/
static inline int mcfg_get_oem_version(const char* file_name, char** oem_version, uint32_t* len)
{

  return mcfg_get_tr_tlv(file_name, MCFG_MBN_OEM_VERSION_TYPE_ID,
                                oem_version, len);
}

static int get_nv_detail(xmlDocPtr pdoc, xmlNodePtr item, xmlChar **ref_val, xmlChar **dev_val)
{
    RIL_Errno res = RIL_E_GENERIC_FAILURE;
#ifndef LIBXML_UNSUPPORTED
    xmlNodePtr cur, member;

    do
    {
        cur = item->xmlChildrenNode;
        // search NvItemDataREFERENCE
        while (cur != NULL)
        {
            if (!xmlStrcmp(cur->name, (const xmlChar *)PDC_XML_REFERENCE_NAME))
                break;
            cur = cur->next;
        }
        if (cur == NULL)
        {
            QCRIL_LOG_ERROR("failed to find " PDC_XML_REFERENCE_NAME);
            break;
        }

        member = cur->xmlChildrenNode;
        while (member != NULL)
        {
            if (!xmlStrcmp(member->name, (const xmlChar *)"Member"))
            {
                *ref_val = xmlNodeListGetString(pdoc, member->xmlChildrenNode, 1);
                break;
            }
            member = member->next;
        }
        if (member == NULL)
        {
            QCRIL_LOG_ERROR("failed to find value");
            break;
        }

        // search NvItemDataDevice
        cur = cur->next;
        while (cur != NULL)
        {
            if (!xmlStrcmp(cur->name, (const xmlChar *)PDC_XML_DEVICE_NAME))
                break;
            cur = cur->next;
        }
        if (cur == NULL)
        {
            QCRIL_LOG_ERROR("failed to find " PDC_XML_DEVICE_NAME);
            xmlFree(*ref_val);
            break;
        }

        member = cur->xmlChildrenNode;
        while (member != NULL)
        {
            if (!xmlStrcmp(member->name, (const xmlChar *)"Member"))
            {
                *dev_val = xmlNodeListGetString(pdoc, member->xmlChildrenNode, 1);
                break;
            }
            member = member->next;
        }
        if (member == NULL)
        {
            QCRIL_LOG_ERROR("failed to find value");
            xmlFree(*ref_val);
            break;
        }
        res = RIL_E_SUCCESS;
    } while (0);
#endif /* LIBXML_UNSUPPORTED */

    return res;
}

static int parse_mbn_diff_result(char* file_name)
{
    RIL_Errno res = RIL_E_GENERIC_FAILURE;
#ifndef LIBXML_UNSUPPORTED
    xmlDocPtr pdoc;
    xmlNodePtr proot, cur;
    xmlChar *id, *ref_val, *dev_val;
    int index = 0;

    do
    {
        pdoc = xmlParseFile(file_name);
        if (pdoc == NULL)
        {
            QCRIL_LOG_ERROR("failed to parse XML file %s", file_name);
            break;
        }

        proot = xmlDocGetRootElement(pdoc);
        if (proot == NULL)
        {
            QCRIL_LOG_ERROR("failed to get root element");
            xmlFreeDoc(pdoc);
            break;
        }

        if (xmlStrcmp(proot->name, (const xmlChar *)PDC_XML_ROOT_NAME))
        {
            QCRIL_LOG_ERROR("document of the wrong type, root node != " PDC_XML_ROOT_NAME);
            xmlFreeDoc(pdoc);
            break;
        }

        cur = proot->xmlChildrenNode;
        while (cur != NULL)
        {
            if ( (!xmlStrcmp(cur->name, (const xmlChar *)PDC_XML_NV_ITEM_NAME))
                    || (!xmlStrcmp(cur->name, (const xmlChar *)PDC_XML_EFS_ITEM_NAME)) )
            {
                id = xmlGetProp(cur, (const xmlChar*)"id");
                if (id != NULL)
                {
                    QCRIL_LOG_INFO("id = %s\n", id);
                    if (get_nv_detail(pdoc, cur, &ref_val, &dev_val) == RIL_E_SUCCESS)
                    {
                        QCRIL_LOG_INFO("ref_val = %s, dev_val = %s\n", ref_val, dev_val);
#ifdef ANDROID
                        qcril_qmi_mbn_diff_send_unsol_msg(RIL_E_SUCCESS, index++, id, ref_val, dev_val);
#endif
                        xmlFree(ref_val);
                        xmlFree(dev_val);
                    }
                    else
                    {
                        QCRIL_LOG_ERROR("failed to get NV detail");
                        xmlFree(id);
                        break;
                    }
                    xmlFree(id);
                }
            }
            cur = cur->next;
        }
        xmlFreeDoc(pdoc);
        if (cur == NULL)
        {
            res = RIL_E_SUCCESS;
        }
     } while (0);
#ifdef ANDROID
    // send a message to ATEL, indicating that everything is done
    qcril_qmi_mbn_diff_send_unsol_msg(res, -1, NULL, NULL, NULL);
#endif /* ANDROID */
#endif /* LIBXML_UNSUPPORTED */
    return res;
}

#ifdef ANDROID

void qcril_qmi_mbn_diff_send_unsol_msg(int result, int index, xmlChar* id, xmlChar* ref_val, xmlChar* dev_val)
{
    int id_len, val_len, str_len, pos;
    boolean is_too_large;
    boolean is_valid_entry;
    mcfg_diff_unsol_msg_type* payload;

    do
    {
      // payload format:
      // result | index | id_len | ref value len | dev value len | id value | ref value | dev value
      is_valid_entry = ( id != NULL ) && (( ref_val != NULL) || ( dev_val != NULL));
      if (is_valid_entry)
      {
        id_len = strlen((char *)id);
        if ( ref_val != NULL )
          val_len = strlen((char *)ref_val);
        else
          val_len = 1;
        if ( dev_val != NULL )
          val_len += strlen((char *)dev_val);
        else
          val_len += 1;
        QCRIL_LOG_INFO("entry is valid, the total value len is %d", val_len);
        str_len = id_len + val_len;
      }
      else
      {
        val_len = 0;
        id_len = 0;
        str_len = 0;
        QCRIL_LOG_ERROR("entry is not valid");
      }

      is_too_large = (val_len > MBN_DIFF_VALUE_LENGTH_LIMIT);
      if ( is_too_large )
      {
        str_len = id_len + 2 * strlen(TOO_LARGET_STRING);
        QCRIL_LOG_INFO("data is too large from item %s, new lenght is %d", id, str_len);
      }

      if ( (payload = static_cast<mcfg_diff_unsol_msg_type *>(qcril_malloc(sizeof(mcfg_diff_unsol_msg_type) + str_len))) == NULL )
      {
        QCRIL_LOG_ERROR("Failed to allocate memory");
        break;
      }
      // build payload
      payload->result = result;
      payload->index = index;
      if (is_valid_entry)
      {
        payload->id_len = strlen((char *)id);
        memcpy(payload->val_str, id, payload->id_len);
        pos = payload->id_len;

        if ( is_too_large )
        {
          payload->ref_len = strlen(TOO_LARGET_STRING);
          memcpy(payload->val_str+pos, TOO_LARGET_STRING, payload->ref_len);
          pos += payload->ref_len;
        }
        else if ( ref_val != NULL)
        {
          payload->ref_len = strlen((char *)ref_val);
          memcpy(payload->val_str+pos, ref_val, payload->ref_len);
          pos += payload->ref_len;
        }
        else
        {
          payload->ref_len = 1;
          memcpy(payload->val_str+pos, " ", 1);
          pos += 1;
        }

        if ( is_too_large )
        {
          payload->dev_len = strlen(TOO_LARGET_STRING);
          memcpy(payload->val_str+pos, TOO_LARGET_STRING, payload->dev_len);
          pos += payload->dev_len;
        }
        else if ( dev_val != NULL)
        {
          payload->dev_len = strlen((char *)dev_val);
          memcpy(payload->val_str+pos, dev_val, payload->dev_len);
          pos += payload->dev_len;
        }
        else
        {
          payload->dev_len = 1;
          memcpy(payload->val_str+pos, " ", 1);
          pos += 1;
        }
      }
      else
      {
        payload->id_len = 0;
        payload->ref_len = 0;
        payload->dev_len = 0;
      }

      // send unsol message
      std::vector<uint8_t> mbn_diff((char *)payload, (char *)payload + sizeof(mcfg_diff_unsol_msg_type) + str_len);
      auto msg = std::make_shared<QcRilUnsolMbnValidateConfigMessage>(mbn_diff);
      if (msg != nullptr)
      {
        msg->broadcast();
      }

      qcril_free(payload);
    } while (0);
}

//===========================================================================
// QCRIL_EVT_QMI_RIL_PDC_PARSE_DIFF_RESULT
//===========================================================================
void qcril_qmi_pdc_parse_diff_result
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
)
{
#define MAX_DUMP_FILE_LENGTH 255
  char dump_file[MAX_DUMP_FILE_LENGTH];
  QCRIL_NOTUSED(ret_ptr);

  QCRIL_LOG_FUNC_ENTRY();
  do
  {
    if ( (NULL == params_ptr->data) || (0 >= params_ptr->datalen)
            || (MAX_DUMP_FILE_LENGTH <= params_ptr->datalen) )
    {
      QCRIL_LOG_ERROR("invalid parameter: wrong dump file name");
      break;
    }
    memset(dump_file, 0, MAX_DUMP_FILE_LENGTH);
    memcpy(dump_file, params_ptr->data, params_ptr->datalen);
    parse_mbn_diff_result(dump_file);
  } while (0);

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// QCRIL_EVT_HOOK_GET_QC_VERSION_OF_FILE
//===========================================================================
void qcril_qmi_pdc_get_qc_version_of_file
(
  std::shared_ptr<QcRilRequestGetQcVersionOfFileMessage> msg
)
{
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  char* qc_version;
  uint32_t qc_version_len = 0;

  QCRIL_LOG_FUNC_ENTRY();
  do
  {
    qc_version = NULL;
    result = static_cast<RIL_Errno>(mcfg_get_qc_version(msg->getFilePath().c_str(), &qc_version, &qc_version_len));
  } while (0);

  QCRIL_LOG_DEBUG("QC version is %s, result is %d", qc_version, result);

  auto fileVersionResp = std::make_shared<qcril::interfaces::FileVersionResp>();
  if(fileVersionResp)
  {
    fileVersionResp->setLength(qc_version_len);
    if(qc_version != NULL)
    {
      //fileVersionResp->setVersion(std::string(static_cast<const char*>((char*)qc_version)));
      std::vector<uint8_t> version;
      for (int index = 0; index < qc_version_len; index++) {
        version.push_back(qc_version[index]);
      }
      fileVersionResp->setVersion(version);
    }

    auto resp = std::make_shared<QcRilRequestMessageCallbackPayload>(result, fileVersionResp);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }

  if (qc_version !=  NULL)
    qcril_free(qc_version);
  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_FILE
//===========================================================================
void qcril_qmi_pdc_get_oem_version_of_file
(
  std::shared_ptr<QcRilRequestGetOemVersionOfFileMessage> msg
)
{
  RIL_Errno result = RIL_E_GENERIC_FAILURE;
  char *oem_version;
  uint32_t oem_version_len;

  QCRIL_LOG_FUNC_ENTRY();
  do
  {
    oem_version = NULL;
    result = static_cast<RIL_Errno>(mcfg_get_oem_version(
        msg->getFilePath().c_str(), &oem_version, &oem_version_len));
  } while (0);

  QCRIL_LOG_DEBUG("OEM version is %s, result is %d", oem_version, result);
  auto fileVersionResp = std::make_shared<qcril::interfaces::FileVersionResp>();
  if(fileVersionResp)
  {
    fileVersionResp->setLength(oem_version_len);
    if(oem_version != NULL)
    {
      //fileVersionResp->setVersion(std::string(static_cast<const char*>((char*)oem_version)));
      std::vector<uint8_t> version;
      for (int index = 0; index < oem_version_len; index++) {
        version.push_back(oem_version[index]);
      }
      fileVersionResp->setVersion(version);
    }

    auto resp = std::make_shared<QcRilRequestMessageCallbackPayload>(result, fileVersionResp);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, resp);
  }

  if (oem_version !=  NULL)
    qcril_free(oem_version);
  QCRIL_LOG_FUNC_RETURN();
}

#endif

#ifdef TEST_MBN_PARSER_STANDALONE
#define MBN_FILE_NAME   "mcfg_sw.mbn"
int main(int argc, char* argv[])
{
  char* qc_version;
  char* oem_version;
  uint32_t len;
  int i;

  if (mcfg_get_qc_version(MBN_FILE_NAME, &qc_version, &len) == RIL_E_SUCCESS)
  {
    for (i = 0; i < len; i++)
      printf("0x%02x ", (unsigned char)qc_version[i]);
    printf("\n");
    free(qc_version);
  }

  if (mcfg_get_oem_version(MBN_FILE_NAME, &oem_version, &len) == RIL_E_SUCCESS)
  {
    for (i = 0; i < len; i++)
      printf("0x%02x ", (unsigned char)oem_version[i]);
    printf("\n");
    free(oem_version);
  }
  exit(0);
}
#endif

#ifdef TEST_XML_PARSER_STANDALONE
#define MBN_DIFF_TEST_FILE      "diff_template_Comb_Attach_TGL-3CSFB-DSDS.xml"
int main(int argc, char* argv[])
{
    parse_mbn_diff_result(MBN_DIFF_TEST_FILE);

    exit(0);
}
#endif
