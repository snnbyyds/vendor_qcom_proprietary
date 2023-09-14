#pragma once
// AUTOGENERATED FILE: DO NOT EDIT

#include <cstdint>
#include "impl_base.hpp"
#include "ITrustedInputCallBack.hpp"

/*===================================================================================
  Copyright (c) 2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ===================================================================================*/

/*===================================================================================
FILE:  ITrustedInputCallBack.idl

DESCRIPTION:  This file contains interface to the VMClientApp and is used
by TrustedInput to either share the touchdata or indicate that it has timed-out
without any touch events or notify any error.

=====================================================================================*/

class TrustedInputCallBackImplBase : protected ImplBase, public ITrustedInputCallBack {
   public:
    TrustedInputCallBackImplBase() {}
    virtual ~TrustedInputCallBackImplBase() {}

   protected:
    virtual int32_t invoke(ObjectOp op, ObjectArg* a, ObjectCounts k) {
        switch (ObjectOp_methodID(op)) {
            case OP_notifyInput: {
                if (k != ObjectCounts_pack(1, 0, 0, 0)) {
                    break;
                }
                const void* inputData_ptr = (const void*)a[0].b.ptr;
                size_t inputData_len = a[0].b.size / 1;
                return notifyInput(inputData_ptr, inputData_len);
            }
            case OP_notifyTimeout: {
                return notifyTimeout();
            }
            case OP_notifyError: {
                if (k != ObjectCounts_pack(1, 0, 0, 0) ||
                    a[0].b.size != 4) {
                    break;
                }
                const int32_t* error_ptr = (const int32_t*)a[0].b.ptr;
                return notifyError(*error_ptr);
            }
            default: { return Object_ERROR_INVALID; }
        }
    }
};
