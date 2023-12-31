# Common Include
include (${CAMX_CDK_PATH}/../linuxembedded/common.cmake)
# NO DATA FOR SECTION: Custom additions (begin)

# Files and Build Type
file(GLOB CAMXGENERATED_SRC
        ${CAMX_CDK_PATH}/generated/g_facedetection/*.cpp
        ${CAMX_CDK_PATH}/generated/g_sensor/*.cpp
        ${CAMX_CDK_PATH}/generated/g_parser/*.cpp
        )
if(NOT (IQSETTING EQUAL "OEM1"))
        file(GLOB CAMXCHROMATIX_SRC ${CAMX_CDK_PATH}/generated/g_chromatix/*.cpp)
endif()

add_library (camxgenerated STATIC
                ${CAMXGENERATED_SRC}
                ${CAMXCHROMATIX_SRC}
            )

# Libraries to compile


# Include Paths
include_directories (${CAMX_C_INCLUDES})
include_directories (../..)
include_directories (${CAMX_CDK_PATH}/generated/g_parser)

# Dependencies


# Definitions
add_definitions (${CAMX_CFLAGS})
add_definitions (${CAMX_CPPFLAGS})
# NO DATA FOR SECTION: Custom additions (end)

# Auto-Gen
#add_dependencies(camxgenerated autogen)

file(GLOB CAMXGENERATED_HEADER
        ${CAMX_CDK_PATH}/generated/g_facedetection/*.h
        ${CAMX_CDK_PATH}/generated/g_sensor/*.h
        ${CAMX_CDK_PATH}/generated/g_parser/*.h
        )
if(NOT (IQSETTING EQUAL "OEM1"))
        file(GLOB CAMXCHROMATIX_HEADER ${CAMX_CDK_PATH}/generated/g_chromatix/*.h)
endif()

install(TARGETS camxgenerated ARCHIVE DESTINATION lib)
install(FILES
        ${CAMXGENERATED_HEADER}
        ${CAMXCHROMATIX_HEADER}
        DESTINATION include)

if (CONFIG STREQUAL "Debug")
    add_definitions( -g )
endif()
