/* Automatically generated nanopb constant definitions */
/* Generated by nanopb-0.3.6 at Tue Apr 14 12:31:27 2020. */

#include "sns_suid.pb.h"

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

const uint64_t sns_suid_sensor_suid_low_default = 12370169555311111083ull;
const uint64_t sns_suid_sensor_suid_high_default = 12370169555311111083ull;
const bool sns_suid_req_default_only_default = true;


const pb_field_t sns_suid_sensor_fields[3] = {
    PB_FIELD(  1, FIXED64 , REQUIRED, STATIC  , FIRST, sns_suid_sensor, suid_low, suid_low, &sns_suid_sensor_suid_low_default),
    PB_FIELD(  2, FIXED64 , REQUIRED, STATIC  , OTHER, sns_suid_sensor, suid_high, suid_low, &sns_suid_sensor_suid_high_default),
    PB_LAST_FIELD
};

const pb_field_t sns_suid_req_fields[4] = {
    PB_FIELD(  1, STRING  , REQUIRED, CALLBACK, FIRST, sns_suid_req, data_type, data_type, 0),
    PB_FIELD(  2, BOOL    , OPTIONAL, STATIC  , OTHER, sns_suid_req, register_updates, data_type, 0),
    PB_FIELD(  3, BOOL    , OPTIONAL, STATIC  , OTHER, sns_suid_req, default_only, register_updates, &sns_suid_req_default_only_default),
    PB_LAST_FIELD
};

const pb_field_t sns_suid_event_fields[3] = {
    PB_FIELD(  1, STRING  , REQUIRED, CALLBACK, FIRST, sns_suid_event, data_type, data_type, 0),
    PB_FIELD(  2, MESSAGE , REPEATED, CALLBACK, OTHER, sns_suid_event, suid, data_type, &sns_std_suid_fields),
    PB_LAST_FIELD
};


/* Check that field information fits in pb_field_t */
#if !defined(PB_FIELD_32BIT)
/* If you get an error here, it means that you need to define PB_FIELD_32BIT
 * compile-time option. You can do that in pb.h or on compiler command line.
 * 
 * The reason you need to do this is that some of your messages contain tag
 * numbers or field sizes that are larger than what can fit in 8 or 16 bit
 * field descriptors.
 */
PB_STATIC_ASSERT((pb_membersize(sns_suid_event, suid) < 65536), YOU_MUST_DEFINE_PB_FIELD_32BIT_FOR_MESSAGES_sns_suid_sensor_sns_suid_req_sns_suid_event)
#endif

#if !defined(PB_FIELD_16BIT) && !defined(PB_FIELD_32BIT)
/* If you get an error here, it means that you need to define PB_FIELD_16BIT
 * compile-time option. You can do that in pb.h or on compiler command line.
 * 
 * The reason you need to do this is that some of your messages contain tag
 * numbers or field sizes that are larger than what can fit in the default
 * 8 bit descriptors.
 */
PB_STATIC_ASSERT((pb_membersize(sns_suid_event, suid) < 256), YOU_MUST_DEFINE_PB_FIELD_16BIT_FOR_MESSAGES_sns_suid_sensor_sns_suid_req_sns_suid_event)
#endif


/* @@protoc_insertion_point(eof) */
