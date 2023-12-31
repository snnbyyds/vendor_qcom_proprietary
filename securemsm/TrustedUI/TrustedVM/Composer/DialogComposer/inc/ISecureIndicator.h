/*************************************************************
Copyright (c) 2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
**************************************************************/
/** @cond */
#pragma once
// AUTOGENERATED FILE: DO NOT EDIT

#include <stdint.h>
#include "object.h"

#define ISecureIndicator_ERROR_INVALID_PARAM INT32_C(10)
#define ISecureIndicator_ERROR_PROVISION INT32_C(11)
#define ISecureIndicator_ERROR_STORE INT32_C(12)
#define ISecureIndicator_ERROR_LOAD INT32_C(13)

#define ISecureIndicator_OP_isIndicatorProvisioned 0
#define ISecureIndicator_OP_getMaxIndicatorResolution 1
#define ISecureIndicator_OP_storeIndicator 2
#define ISecureIndicator_OP_getIndicator 3
#define ISecureIndicator_OP_removeIndicator 4

static inline int32_t
ISecureIndicator_release(Object self)
{
  return Object_invoke(self, Object_OP_release, 0, 0);
}

static inline int32_t
ISecureIndicator_retain(Object self)
{
  return Object_invoke(self, Object_OP_retain, 0, 0);
}

static inline int32_t
ISecureIndicator_isIndicatorProvisioned(Object self, uint32_t *isProvisioned_ptr)
{
  ObjectArg a[1];
  a[0].b = (ObjectBuf) { isProvisioned_ptr, sizeof(uint32_t) };

  return Object_invoke(self, ISecureIndicator_OP_isIndicatorProvisioned, a, ObjectCounts_pack(0, 1, 0, 0));
}

static inline int32_t
ISecureIndicator_getMaxIndicatorResolution(Object self, uint32_t *maxWidth_ptr, uint32_t *maxHeight_ptr)
{
  ObjectArg a[1];
  struct {
    uint32_t m_maxWidth;
    uint32_t m_maxHeight;
  } o;
  a[0].b = (ObjectBuf) { &o, 8 };

  int32_t result = Object_invoke(self, ISecureIndicator_OP_getMaxIndicatorResolution, a, ObjectCounts_pack(0, 1, 0, 0));

  *maxWidth_ptr = o.m_maxWidth;
  *maxHeight_ptr = o.m_maxHeight;

  return result;
}

static inline int32_t
ISecureIndicator_storeIndicator(Object self, const void *indicator_ptr, size_t indicator_len)
{
  ObjectArg a[1];
  a[0].bi = (ObjectBufIn) { indicator_ptr, indicator_len * 1 };

  return Object_invoke(self, ISecureIndicator_OP_storeIndicator, a, ObjectCounts_pack(1, 0, 0, 0));
}

static inline int32_t
ISecureIndicator_getIndicator(Object self, void *indicator_ptr, size_t indicator_len, size_t *indicator_lenout)
{
  ObjectArg a[1];
  a[0].b = (ObjectBuf) { indicator_ptr, indicator_len * 1 };

  int32_t result = Object_invoke(self, ISecureIndicator_OP_getIndicator, a, ObjectCounts_pack(0, 1, 0, 0));

  *indicator_lenout = a[0].b.size / 1;

  return result;
}

static inline int32_t
ISecureIndicator_removeIndicator(Object self)
{
  return Object_invoke(self, ISecureIndicator_OP_removeIndicator, 0, 0);
}



