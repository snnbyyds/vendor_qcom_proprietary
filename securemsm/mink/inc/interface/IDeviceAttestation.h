/*
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
/** @file  IDeviceAttestation.idl */
/**
 * IDAError provides a list of all the errors that might be returned
 * by the methods of the IAttestationReport, IAttestationBuilder, and
 * IDeviceAttestation interfaces.
 */
/** @cond */
#pragma once
// AUTOGENERATED FILE: DO NOT EDIT

#include <stdint.h>
#include "object.h"

#define IDAError_NO_MEMORY INT32_C(10)
#define IDAError_INVALID_BUFFER INT32_C(11)
#define IDAError_INVALID_CERTIFICATE INT32_C(12)
#define IDAError_MAX_APP_DATA_LIMIT_REACHED INT32_C(13)
#define IDAError_INVALID_SECURITY_LEVEL INT32_C(14)
#define IDAError_INVALID_ATTESTATION_CONTEXT INT32_C(15)
#define IDAError_INVALID_SIGNING_KEY INT32_C(16)
#define IDAError_INVALID_NONCE INT32_C(17)
#define IDAError_INVALID_REPORT_OFFSET INT32_C(18)
#define IDAError_ATTESTATION_REPORT_FAILURE INT32_C(19)
#define IDAError_WARM_UP_FAILURE INT32_C(20)


static inline int32_t
IDAError_release(Object self)
{
  return Object_invoke(self, Object_OP_release, 0, 0);
}

static inline int32_t
IDAError_retain(Object self)
{
  return Object_invoke(self, Object_OP_retain, 0, 0);
}


#define IAttestationReport_OP_getSize 0
#define IAttestationReport_OP_getBytes 1

static inline int32_t
IAttestationReport_release(Object self)
{
  return Object_invoke(self, Object_OP_release, 0, 0);
}

static inline int32_t
IAttestationReport_retain(Object self)
{
  return Object_invoke(self, Object_OP_retain, 0, 0);
}

static inline int32_t
IAttestationReport_getSize(Object self, uint64_t *attestationReportSize_ptr)
{
  ObjectArg a[1];
  a[0].b = (ObjectBuf) { attestationReportSize_ptr, sizeof(uint64_t) };

  return Object_invoke(self, IAttestationReport_OP_getSize, a, ObjectCounts_pack(0, 1, 0, 0));
}

static inline int32_t
IAttestationReport_getBytes(Object self, uint64_t offset_val, void *attestation_ptr, size_t attestation_len, size_t *attestation_lenout)
{
  ObjectArg a[2];
  a[0].b = (ObjectBuf) { &offset_val, sizeof(uint64_t) };
  a[1].b = (ObjectBuf) { attestation_ptr, attestation_len * 1 };

  int32_t result = Object_invoke(self, IAttestationReport_OP_getBytes, a, ObjectCounts_pack(1, 1, 0, 0));

  *attestation_lenout = a[1].b.size / 1;

  return result;
}


#define IAttestationBuilder_SECURITY_LEVEL_UNRESTRICTED UINT32_C(1)
#define IAttestationBuilder_SECURITY_LEVEL_RESTRICTED UINT32_C(2)
#define IAttestationBuilder_SECURITY_LEVEL_SECURERESTRICTED UINT32_C(3)
#define IAttestationBuilder_SECURITY_LEVEL_HARDWARE UINT32_C(4)
#define IAttestationBuilder_ATTESTATION_CONTEXT_GENERIC UINT32_C(1)
#define IAttestationBuilder_ATTESTATION_CONTEXT_REGISTRATION UINT32_C(2)
#define IAttestationBuilder_ATTESTATION_CONTEXT_PROVISIONING UINT32_C(3)
#define IAttestationBuilder_ATTESTATION_CONTEXT_CERT_ISSUANCE UINT32_C(4)
#define IAttestationBuilder_ATTESTATION_CONTEXT_PROOF_OF_POSSESSION UINT32_C(5)
#define IAttestationBuilder_ATTESTATION_CONTEXT_LICENSING UINT32_C(6)
#define IAttestationBuilder_OPT_ADDON_NONE UINT64_C(0x0000000000000000)
#define IAttestationBuilder_OPT_ADDON_LOCATION UINT64_C(0x0000000000000001)
#define IAttestationBuilder_OPT_ADDON_RTIC UINT64_C(0x0000000000000002)
#define IAttestationBuilder_OPT_ADDON_TRUSTEDTIME UINT64_C(0x0000000000000004)
#define IAttestationBuilder_OPT_ADDON_CONNSEC_CELLULAR UINT64_C(0x0000000000000008)
#define IAttestationBuilder_FORMAT_EAT UINT32_C(1)
#define IAttestationBuilder_KEY_DEMO UINT32_C(1)
#define IAttestationBuilder_KEY_QDAK UINT32_C(2)

#define IAttestationBuilder_OP_clearBytes 0
#define IAttestationBuilder_OP_addBytes 1
#define IAttestationBuilder_OP_build 2

static inline int32_t
IAttestationBuilder_release(Object self)
{
  return Object_invoke(self, Object_OP_release, 0, 0);
}

static inline int32_t
IAttestationBuilder_retain(Object self)
{
  return Object_invoke(self, Object_OP_retain, 0, 0);
}

static inline int32_t
IAttestationBuilder_clearBytes(Object self)
{
  return Object_invoke(self, IAttestationBuilder_OP_clearBytes, 0, 0);
}

static inline int32_t
IAttestationBuilder_addBytes(Object self, uint32_t securityLevel_val, const int8_t *label_ptr, size_t label_len, const void *bytes_ptr, size_t bytes_len)
{
  ObjectArg a[3];
  a[0].b = (ObjectBuf) { &securityLevel_val, sizeof(uint32_t) };
  a[1].bi = (ObjectBufIn) { label_ptr, label_len * sizeof(int8_t) };
  a[2].bi = (ObjectBufIn) { bytes_ptr, bytes_len * 1 };

  return Object_invoke(self, IAttestationBuilder_OP_addBytes, a, ObjectCounts_pack(3, 0, 0, 0));
}

static inline int32_t
IAttestationBuilder_build(Object self, uint32_t attestationContext_val, uint64_t ADDONOptions_val, uint32_t formatType_val, uint32_t keyType_val, const void *nonce_ptr, size_t nonce_len, uint64_t *reportStatus_ptr, Object *attestationReport_ptr)
{
  ObjectArg a[4];
  struct {
    uint64_t m_ADDONOptions;
    uint32_t m_attestationContext;
    uint32_t m_formatType;
    uint32_t m_keyType;
  } i;
  a[0].b = (ObjectBuf) { &i, 20 };
  i.m_attestationContext = attestationContext_val;
  i.m_ADDONOptions = ADDONOptions_val;
  i.m_formatType = formatType_val;
  i.m_keyType = keyType_val;
  a[1].bi = (ObjectBufIn) { nonce_ptr, nonce_len * 1 };
  a[2].b = (ObjectBuf) { reportStatus_ptr, sizeof(uint64_t) };

  int32_t result = Object_invoke(self, IAttestationBuilder_OP_build, a, ObjectCounts_pack(2, 1, 0, 1));

  *attestationReport_ptr = a[3].o;

  return result;
}


#define IDeviceAttestation_RTIC_AGE_LABEL INT32_C(-77200)
#define IDeviceAttestation_TRUSTEDTIME_AGE_LABEL INT32_C(-77250)
#define IDeviceAttestation_LOCATION_AGE_LABEL INT32_C(-77270)
#define IDeviceAttestation_CONNSEC_CELLULAR_AGE_LABEL INT32_C(-77280)

#define IDeviceAttestation_OP_start 0
#define IDeviceAttestation_OP_getWarmUpStatus 1
#define IDeviceAttestation_OP_warmUp 2

static inline int32_t
IDeviceAttestation_release(Object self)
{
  return Object_invoke(self, Object_OP_release, 0, 0);
}

static inline int32_t
IDeviceAttestation_retain(Object self)
{
  return Object_invoke(self, Object_OP_retain, 0, 0);
}

static inline int32_t
IDeviceAttestation_start(Object self, const void *licenseCert_ptr, size_t licenseCert_len, Object *attestationBuilder_ptr)
{
  ObjectArg a[2];
  a[0].bi = (ObjectBufIn) { licenseCert_ptr, licenseCert_len * 1 };

  int32_t result = Object_invoke(self, IDeviceAttestation_OP_start, a, ObjectCounts_pack(1, 0, 0, 1));

  *attestationBuilder_ptr = a[1].o;

  return result;
}

static inline int32_t
IDeviceAttestation_getWarmUpStatus(Object self, void *warmUpStatus_ptr, size_t warmUpStatus_len, size_t *warmUpStatus_lenout)
{
  ObjectArg a[1];
  a[0].b = (ObjectBuf) { warmUpStatus_ptr, warmUpStatus_len * 1 };

  int32_t result = Object_invoke(self, IDeviceAttestation_OP_getWarmUpStatus, a, ObjectCounts_pack(0, 1, 0, 0));

  *warmUpStatus_lenout = a[0].b.size / 1;

  return result;
}

static inline int32_t
IDeviceAttestation_warmUp(Object self, uint64_t options_val, uint64_t timeout_val, Object callback_val)
{
  ObjectArg a[2];
  struct {
    uint64_t m_options;
    uint64_t m_timeout;
  } i;
  a[0].b = (ObjectBuf) { &i, 16 };
  i.m_options = options_val;
  i.m_timeout = timeout_val;
  a[1].o = callback_val;

  return Object_invoke(self, IDeviceAttestation_OP_warmUp, a, ObjectCounts_pack(1, 0, 1, 0));
}



