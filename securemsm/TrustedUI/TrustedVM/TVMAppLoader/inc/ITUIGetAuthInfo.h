#pragma once

/*
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/** @cond */

// AUTOGENERATED FILE: DO NOT EDIT

#include <stdint.h>
#include "object.h"

#define ITUIGetAuthInfo_ERROR_INVALID_PARAM INT32_C(10)
#define ITUIGetAuthInfo_ERROR_APP_LOAD INT32_C(11)
#define ITUIGetAuthInfo_ERROR_KEY_INVALID INT32_C(12)
#define ITUIGetAuthInfo_ERROR_HMAC_INVALID INT32_C(13)
#define ITUIGetAuthInfo_ERROR_INTERNAL INT32_C(14)

#define ITUIGetAuthInfo_OP_getAuthInfo 0

static inline int32_t
ITUIGetAuthInfo_release(Object self)
{
  return Object_invoke(self, Object_OP_release, 0, 0);
}

static inline int32_t
ITUIGetAuthInfo_retain(Object self)
{
  return Object_invoke(self, Object_OP_retain, 0, 0);
}

static inline int32_t
ITUIGetAuthInfo_getAuthInfo(Object self, uint32_t UID, void *key_ptr, size_t key_len, size_t *key_lenout, void *hmac_ptr, size_t hmac_len, size_t *hmac_lenout)
{
  ObjectArg a[3];
  a[0].bi = (ObjectBufIn) { &UID, sizeof(uint32_t) };
  a[1].b =  (ObjectBuf) { key_ptr, key_len * 1 };
  a[2].b =  (ObjectBuf) { hmac_ptr, hmac_len * 1 };

  int32_t result = Object_invoke(self, ITUIGetAuthInfo_OP_getAuthInfo, a, ObjectCounts_pack(1, 2, 0, 0));

  *key_lenout = a[1].b.size / 1;
  *hmac_lenout = a[2].b.size / 1;

  return result;
}
