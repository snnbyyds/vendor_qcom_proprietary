#pragma once
/** @file  IOpener.idl */

// =======================================================================
// Copyright (c) 2015-2019 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
// =======================================================================

/** @cond */

/** 0 is not a valid service ID. */

// AUTOGENERATED FILE: DO NOT EDIT

#include <cstdint>
#include "object.h"
#include "proxy_base.hpp"

static const uint32_t IOpener_INVALID_ID = UINT32_C(0);

class IIOpener {
   public:
    static const int32_t ERROR_NOT_FOUND = INT32_C(10);
    static const int32_t ERROR_PRIVILEGE = INT32_C(11);
    static const int32_t ERROR_NOT_SUPPORTED = INT32_C(12);

    virtual ~IIOpener() {}

    virtual int32_t open(uint32_t id_val, ProxyBase &obj_ref) = 0;

   protected:
    static const ObjectOp OP_open = 0;
};

class IOpener : public IIOpener, public ProxyBase {
   public:
    IOpener() {}
    IOpener(Object impl) : ProxyBase(impl) {}
    virtual ~IOpener() {}

    virtual int32_t open(uint32_t id_val, ProxyBase &obj_ref) {
        ObjectArg a[2];
        a[0].b = (ObjectBuf) {&id_val, sizeof(uint32_t)};

        int32_t result = invoke(OP_open, a, ObjectCounts_pack(1, 0, 0, 1));
        if (Object_OK != result) { return result; }

        obj_ref.consume(a[1].o);

        return result;
    }

};
