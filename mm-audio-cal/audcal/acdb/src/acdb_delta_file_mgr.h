#ifndef __ACDB_DELTA_FILE_MGR_H__
#define __ACDB_DELTA_FILE_MGR_H__
/*===========================================================================
    @file   acdb_delta_file_mgr.h

    This file contains the implementation of the delta acdb file accessor interfaces.

                    Copyright (c) 2014-2018 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Confidential and Proprietary - Qualcomm Technologies, Inc.
========================================================================== */
/* $Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acdb/src/acdb_delta_file_mgr.h#9 $ */

/*===========================================================================
    EDIT HISTORY FOR MODULE

    This section contains comments describing changes made to the module.
    Notice that changes are listed in reverse chronological order. Please
    use ISO format for dates.

    when        who     what, where, why
    ----------  ---     -----------------------------------------------------
    2014-05-28  mh      SW migration from 32-bit to 64-bit architecture
    2013-11-28  avi     Delta ACDB file manager initial checkin.
===========================================================================*/

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */
#include "acdb.h"
#include "acdb_file_mgr.h"

/* ---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
* Class Definitions
*--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Function Declarations and Documentation
 *--------------------------------------------------------------------------- */

typedef struct _AcdbDeltaFileVersion AcdbDeltaFileVersion;
#include "acdb_begin_pack.h"
struct _AcdbDeltaFileVersion{
   uint32_t majorVersion;
   uint32_t minorVersion;
}
#include "acdb_end_pack.h"
;


typedef struct _AcdbCmdDeltaFileInfo AcdbCmdDeltaFileInfo;
#include "acdb_begin_pack.h"
struct _AcdbCmdDeltaFileInfo{
   uint32_t fileIndex;
   AcdbFileInfo nFileInfo;
   uint32_t isFileUpdated;
   uint32_t deltaFileExists;
	uint32_t nFileSize;
	uint8_t *pFileBuf;
}
#include "acdb_end_pack.h"
;

typedef struct _AcdbCmdDeltaFileDataInstance AcdbCmdDeltaFileDataInstance;
#include "acdb_begin_pack.h"
struct _AcdbCmdDeltaFileDataInstance{
   uint32_t *pTblId;
   uint32_t *pIndicesCount;
   uint32_t *pMid;
   uint32_t *pPid;
   uint32_t *pDataLen;
   uint8_t *pIndices;
   uint8_t *pData;
}
#include "acdb_end_pack.h"
;

typedef struct _AcdbCmdDeltaFileDataInstanceV2 AcdbCmdDeltaFileDataInstanceV2;
#include "acdb_begin_pack.h"
struct _AcdbCmdDeltaFileDataInstanceV2{
   uint32_t *pTblId;
   uint32_t *pIndicesCount;
   uint32_t *pDataLen;
   uint8_t *pIndices;
   uint8_t *pData;
}
#include "acdb_end_pack.h"
;

typedef struct _AcdbCmdInstanceDeltaFileData AcdbCmdInstanceDeltaFileData;
#include "acdb_begin_pack.h"
struct _AcdbCmdInstanceDeltaFileData{
   uint32_t *pTblId;
   uint32_t *pIndicesCount;
   uint32_t *pMid;
   uint32_t *pIid;
   uint32_t *pPid;
   uint32_t *pDataLen;
   uint8_t *pIndices;
   uint8_t *pData;
}
#include "acdb_end_pack.h"
;

typedef struct _AcdbCmdDeltaFileIndexCmdType AcdbCmdDeltaFileIndexCmdType;
#include "acdb_begin_pack.h"
struct _AcdbCmdDeltaFileIndexCmdType{
   uint32_t *pTblId;
   uint32_t *pIndicesCount;
   uint8_t *pIndices;
}
#include "acdb_end_pack.h"
;
enum AcdbDeltaDataCmds {
    ACDBDELTADATACMD_INIT_DELTA_ACDB_DATA= 0,
	ACDBDELTADATACMD_DELTA_RESET,
   ACDBDELTADATACMD_GET_DELTA_ACDB_DATA_COUNT,
   ACDBDELTADATACMD_GET_DELTA_ACDB_DATA_FOR_ONE_FILE_V0,
   ACDBDELTADATACMD_GET_DELTA_ACDB_FILE_INDEX,
   ACDBDELTADATACMD_SET_DELTA_ACDB_FILE_UPDATED,
   ACDBDELTADATACMD_SAVE_DELTA_ACDB_DATA,
   ACDBDELTADATACMD_DELETE_DELTA_ACDB_FILES,
   ACDBDELTADATACMD_FREE_DELTA_ACDB_BUF,
   ACDBDELTADATACMD_GET_INSTANCE_DELTA_ACDB_DATA_COUNT,
   ACDBDELTADATACMD_GET_INSTANCE_DELTA_ACDB_DATA,
   ACDBDELTADATACMD_GET_DELTA_ACDB_DATA_COUNT_FOR_ONE_FILE,
   ACDBDELTADATACMD_GET_DELTA_ACDB_DATA_FOR_ONE_FILE_V1,
   ACDBDELTADATCMD_GET_DELTA_FILE_VERSION,
};

int32_t acdb_delta_data_ioctl (uint32_t nCommandId,
                        uint8_t *pInput,
                        uint32_t nInputSize,
                        uint8_t *pOutput,
                        uint32_t nOutputSize);

#endif /* __ACDB_DELTA_FILE_MGR_H__ */

