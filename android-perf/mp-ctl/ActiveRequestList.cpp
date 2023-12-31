/******************************************************************************
  @file    ActiveRequestList.cpp
  @brief   Implementation of active request lists

  DESCRIPTION

  ---------------------------------------------------------------------------
  Copyright (c) 2011-2015,2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "ANDR-PERF-ACTIVEREQLIST"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <cutils/properties.h>
#include <fcntl.h>
#include <log/log.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ActiveRequestList.h"
#include "MpctlUtils.h"
#include "Request.h"
#include "ResourceInfo.h"

int ActiveRequestList::ACTIVE_REQS_MAX = property_get_bool("ro.config.low_ram", false)? 15:30;
void ActiveRequestList::Reset() {
    RequestListNode *current = NULL;
    RequestListNode *next = NULL;

    pthread_mutex_lock(&mMutex);
    current = mActiveListHead;

    if (NULL == current) {
        QLOGI("No activity present");
        pthread_mutex_unlock(&mMutex);
        return;
    }

    while (NULL != current) {
        next = current->mNext;
        if (NULL != current->mHandle) {
            delete current->mHandle;
        }
        free(current);
        current = next;
    }
    mActiveListHead = NULL;
    mActiveReqs = 0;
    pthread_mutex_unlock(&mMutex);
}

/* Always add new request to head of active list */
int ActiveRequestList::Add(Request *req, int req_handle) {
    RequestListNode *newnode = NULL;

    pthread_mutex_lock(&mMutex);
    if (mActiveReqs >= ACTIVE_REQS_MAX) {
        QLOGE("Max Active request limit reached");
        pthread_mutex_unlock(&mMutex);
        return FAILED;
    }
    newnode = (struct RequestListNode *)calloc(1, sizeof(struct RequestListNode));
    if (NULL == newnode) {
        QLOGE("Failed to create active request");
        pthread_mutex_unlock(&mMutex);
        return FAILED;
    } else {
        newnode->mHandle = req;
        newnode->mRetHandle = req_handle;

        newnode->mNext = mActiveListHead;
        mActiveListHead = newnode;
        mActiveReqs++;
        pthread_mutex_unlock(&mMutex);
    }
    return SUCCESS;
}

void ActiveRequestList::Remove(Request *req) {
    struct RequestListNode *del = NULL;
    struct RequestListNode *iter = NULL;

    pthread_mutex_lock(&mMutex);

    if (mActiveListHead == NULL) {
        QLOGI("No activity to remove");
        pthread_mutex_unlock(&mMutex);
        return;
    }

    if (mActiveListHead->mHandle == req) {
        del = mActiveListHead;
        mActiveListHead = mActiveListHead->mNext;
    } else {
        iter = mActiveListHead;
        while ((iter->mNext != NULL) && (iter->mNext->mHandle != req)) {
            iter = iter->mNext;
        }
        del = iter->mNext;
        if (del != NULL)
            iter->mNext = del->mNext;
    }

    /* Free list_node */
    if (del) {
        mActiveReqs--;
        free(del);
    }
    pthread_mutex_unlock(&mMutex);
}

bool ActiveRequestList::CanSubmit() {
    bool status = false;
    pthread_mutex_lock(&mMutex);
    if (mActiveReqs < ACTIVE_REQS_MAX) {
        status = true;
    }
    pthread_mutex_unlock(&mMutex);
    return status;
}

Request * ActiveRequestList::DoesExists(int handle) {
    RequestListNode *iter = NULL;
    Request *tmpreq = NULL;

    pthread_mutex_lock(&mMutex);
    iter = mActiveListHead;

    if (iter == NULL || handle < 1) {
        pthread_mutex_unlock(&mMutex);
        return NULL;
    }

    while ((iter->mNext != NULL) && (iter->mRetHandle != handle)) {
        iter = iter->mNext;
    }
    if (iter->mRetHandle == handle) {
        tmpreq = iter->mHandle;
    }
    pthread_mutex_unlock(&mMutex);
    return tmpreq;
}

/* Search for request in active list */
Request * ActiveRequestList::IsActive(int handle, Request *req) {
    int i = 0;
    RequestListNode *iter = NULL;
    Request *tmpreq = NULL;

    pthread_mutex_lock(&mMutex);
    iter = mActiveListHead;
    if ((iter == NULL) || (handle < 1) || (NULL == iter->mHandle) ||
        (NULL == req) || (req->GetNumLocks() < 0)) {
        pthread_mutex_unlock(&mMutex);
        return NULL;
    }

    while ((iter->mNext != NULL) && (iter->mRetHandle != handle)) {
        iter = iter->mNext;
    }
    if (iter->mRetHandle == handle) {
        if (*req != *(iter->mHandle)) {
            tmpreq = NULL;
        } else {
            tmpreq = iter->mHandle;
        }
    }
    pthread_mutex_unlock(&mMutex);
    return tmpreq;
}

int ActiveRequestList::GetNumActiveRequests() {
    int tmp = 0 ;
    pthread_mutex_lock(&mMutex);
    tmp = mActiveReqs;
    pthread_mutex_unlock(&mMutex);
    return tmp;
}

int ActiveRequestList::GetActiveReqsInfo(int *handles, int *pids) {
    RequestListNode *iter = NULL;
    int i = 0;

    if ((NULL == handles) || (NULL == pids)) {
        QLOGE("Invalid data structure to ActiveRequestList");
        return 0;
    }

    pthread_mutex_lock(&mMutex);
    iter = mActiveListHead;

    QLOGI("Total no. of active requests: %d", mActiveReqs);
    while (iter != NULL) {
        if((iter->mHandle != NULL) && (i < ACTIVE_REQS_MAX)) {
            pids[i] = iter->mHandle->GetPid();
            handles[i] = iter->mRetHandle;
            i++;
        }
        iter = iter->mNext;
    }
    pthread_mutex_unlock(&mMutex);
    return i;
}

void ActiveRequestList::Dump() {
    RequestListNode *iter = NULL;

    pthread_mutex_lock(&mMutex);
    iter = mActiveListHead;

    while (iter != NULL) {
        QLOGI("request handle:%d ", iter->mRetHandle);
        iter = iter->mNext;
    }
    pthread_mutex_unlock(&mMutex);
}

std::string ActiveRequestList::DumpActive() {
    using namespace std;
    string result;
    stringstream ss;

    ss << setfill('-') << setw(strlen("Active Clients") + 60);
    ss << "Active Clients" << setw(60) << "\n";
    ss << setfill(' ') << left << setw(15) << " Pid(Tid)" << left << setw(60) << "| PkgName";
    ss << left << setw(15) << "| Duration" << "| OpCodes" << "\n";
    ss << right << setfill('-') << setw(strlen("Active Clients") + 120) << "\n";

    RequestListNode *iter = NULL;
    pthread_mutex_lock(&mMutex);
    iter = mActiveListHead;
    stringstream temp;
    while (iter != NULL) {
        Request* handle = iter->mHandle;
        if (NULL != handle) {
            temp.clear();
            temp.str("");
            temp << " " << handle->GetPid() << "(" << handle->GetTid() << ")";
            ss << dec;
            ss << left << setfill(' ') << setw(15) << temp.str();

            temp.clear();
            temp.str("");
            temp << "| " << getPkgNameFromPid(handle->GetPid());
            ss << left << setw(60) << temp.str();

            temp.clear();
            temp.str("");
            temp << "| " << handle->GetDuration();
            ss << left << setw(15) << temp.str();

            ss << "| ";
            for (int i = 0; i < handle->GetNumLocks(); i++) {
                ResourceInfo* res = handle->GetResource(i);
                if (NULL != res) {
                    ss << hex;
                    ss << "0x";
                    ss << res->GetOpCode();
                    ss << " ";
                    ss << dec;
                    ss << res->GetValue();
                    ss << "    ";
                } else {
                    ss << "No Resources";
                    break;
                }
            }
        }
        ss << "\n";
        iter = iter->mNext;
    }
    pthread_mutex_unlock(&mMutex);
    result = ss.str();
    return result;
}

std::string ActiveRequestList::getPkgNameFromPid(int pid) {
    using namespace std;
    string pkg;
    char buf[512];
    snprintf(buf, sizeof(buf), "/proc/%d/cmdline", pid);
    ifstream in(buf);

    if (!in.is_open()) {
        QLOGE("Cannot open cmdline.");
        return "";
    }

    getline(in, pkg, '\0');
    if (pkg.length() <= 0) {
        QLOGE("Cannot get cmdline.");
        return "";
    }

    return pkg;
}
