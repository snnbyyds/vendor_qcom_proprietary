/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*
* Not a Contribution.
* Apache license notifications and license are retained
* for attribution purposes only.
*/
/*
 * Copyright (c) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef ANDROID_RIL_INTERNAL_H
#define ANDROID_RIL_INTERNAL_H

#include <functional>

namespace android {

#define RIL_SERVICE_NAME_BASE "slot"
#define RIL1_SERVICE_NAME "slot1"
#define RIL2_SERVICE_NAME "slot2"
#define RIL3_SERVICE_NAME "slot3"
#define RIL4_SERVICE_NAME "slot4"

/* Constants for response types */
#define RESPONSE_SOLICITED 0
#define RESPONSE_UNSOLICITED 1
#define RESPONSE_SOLICITED_ACK 2
#define RESPONSE_SOLICITED_ACK_EXP 3
#define RESPONSE_UNSOLICITED_ACK_EXP 4

// request, response, and unsolicited msg print macro
#define PRINTBUF_SIZE 8096

// Enable verbose logging
#define VDBG 0

#ifndef MIN
#define MIN(a,b) ((a)<(b) ? (a) : (b))
#endif

// Enable RILC log
#define RILC_LOG 0

#if RILC_LOG
    #define startRequest           snprintf(printBuf, PRINTBUF_SIZE, "(")
    #define closeRequest           snprintf(printBuf, PRINTBUF_SIZE, "%s)", printBuf)
    #define printRequest(token, req)           \
            RLOGD("[%04d]> %s %s", token, requestToString(req), printBuf)

    #define startResponse           snprintf(printBuf, PRINTBUF_SIZE, "%s {", printBuf)
    #define closeResponse           snprintf(printBuf, PRINTBUF_SIZE, "%s}", printBuf)
    #define printResponse           RLOGD("%s", printBuf)

    #define clearPrintBuf           printBuf[0] = 0
    #define removeLastChar          printBuf[strlen(printBuf)-1] = 0
    #define appendPrintBuf(x...)    snprintf(printBuf, PRINTBUF_SIZE, x)
#else
    #define startRequest
    #define closeRequest
    #define printRequest(token, req)
    #define startResponse
    #define closeResponse
    #define printResponse
    #define clearPrintBuf
    #define removeLastChar
    #define appendPrintBuf(x...)
#endif

typedef struct CommandInfo CommandInfo;

extern "C" const char * requestToString(int request);

typedef struct RequestInfo {
    int32_t token;      //this is not RIL_Token
    CommandInfo *pCI;
    struct RequestInfo *p_next;
    char cancelled;
    char local;         // responses to local commands do not go back to command process
    RIL_SOCKET_ID socket_id;
    int wasAckSent;    // Indicates whether an ack was sent earlier
} RequestInfo;

typedef struct CommandInfo {
    int requestNumber;
    int(*responseFunction) (int slotId, int responseType, int token,
            RIL_Errno e, void *response, size_t responselen);
} CommandInfo;

RequestInfo * addRequestToList(int serial, int slotId, int request);

typedef std::function<void(RIL_Errno errorCode, void* response, size_t responseLen)> SocketResponseCallback;

typedef struct SocketRequestInfo {
    // TODO: QCRIL reads the first 4 bytes of this struct for logging purposes.
    // TODO: Make sure the first 4 bytes are only accessed for logging purposes
    // TODO: and not to establish uniqueness (i.e. identifier) of a request.
    uint64_t token;
    SocketResponseCallback responseCallback;
} SocketRequestInfo;

SocketRequestInfo* addSocketRequestToList(uint64_t token, const SocketResponseCallback& responseCallback);

char * RIL_getServiceName();

void grabPartialWakeLock();
void releaseWakeLock();

void onNewCommandConnect(RIL_SOCKET_ID socket_id);

}   // namespace android

extern RIL_RadioFunctions *s_vendorFunctions;

#if defined(ANDROID_MULTI_SIM)
#define CALL_ONREQUEST(a, b, c, d, e) \
        s_vendorFunctions->onRequest((a), (b), (c), (d), ((RIL_SOCKET_ID)(e)))
#define CALL_ONSTATEREQUEST(a) s_vendorFunctions->onStateRequest((RIL_SOCKET_ID)(a))
#else
#define CALL_ONREQUEST(a, b, c, d, e) s_vendorFunctions->onRequest((a), (b), (c), (d))
#define CALL_ONSTATEREQUEST(a) s_vendorFunctions->onStateRequest()
#endif

bool socketDispatchVoid(int serial, int slotId, int request);
bool socketDispatchString(int serial, int slotId, int request, const char * str);
bool socketDispatchStrings(int serial, int slotId, int request, bool allowEmpty, int countStrings, ...);
bool socketDispatchInts(int serial, int slotId, int request, int countInts, ...);

#endif //ANDROID_RIL_INTERNAL_H
