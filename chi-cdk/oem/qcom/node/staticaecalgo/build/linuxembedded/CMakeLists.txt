# User Variables
set (CHROMATIX_VERSION 0x0310)

set (SUPPORT_FUNCTION_CALL_TRACE 1)

# NO DATA FOR SECTION: Custom additions (begin)

# Common Include
include (${CAMX_CDK_PATH}/../linuxembedded/common.cmake)

# Files and Build Type
add_library( com.qtistatic.stats.aec SHARED
    ../../src/camxstaticaecalgoimpl.cpp
)

# Libraries to compile
target_link_libraries (com.qtistatic.stats.aec camxosutils)
target_link_libraries (com.qtistatic.stats.aec camxutils)
target_link_libraries (com.qtistatic.stats.aec camxgenerated)
target_link_libraries (com.qtistatic.stats.aec dl)
target_link_libraries (com.qtistatic.stats.aec cutils)
target_link_libraries (com.qtistatic.stats.aec log)
target_link_libraries (com.qtistatic.stats.aec sync)


# Include Paths
include_directories (${CAMX_C_INCLUDES})
include_directories (../..)
include_directories (${CAMX_CDK_PATH}/generated/g_chromatix)
include_directories (${CAMX_CDK_PATH}/generated/g_parser)
include_directories (${CAMX_CDK_PATH})
include_directories (${CAMX_CDK_PATH}/stats)
include_directories (${CAMX_VENDOR_PATH}/node/staticaecalgo)
include_directories (${CAMX_VENDOR_PATH}/node/staticaecalgo/inc)

# Dependencies


# Definitions
add_definitions (${CAMX_CFLAGS})
add_definitions (${CAMX_CPPFLAGS})
# NO DATA FOR SECTION: Custom additions (end)

# Auto-Gen
#add_dependencies(com.qtistatic.stats.aec autogen)

install(TARGETS com.qtistatic.stats.aec LIBRARY DESTINATION lib/${CAMX_LIB_OUTPUT_PATH})

if (CONFIG STREQUAL "Debug")
    add_definitions( -g )
endif()

