# User Variables
set (CHROMATIX_VERSION 0x0310)

set (SUPPORT_FUNCTION_CALL_TRACE 1)

# NO DATA FOR SECTION: Custom additions (begin)

# Common Include
include (${CAMX_CDK_PATH}/../linuxembedded/common.cmake)

# Files and Build Type
add_library( com.qtistatic.stats.af SHARED
    ../../src/camxaf.cpp
)

# Libraries to compile
target_link_libraries (com.qtistatic.stats.af camxgenerated)
target_link_libraries (com.qtistatic.stats.af camxosutils)
target_link_libraries (com.qtistatic.stats.af camxutils)
target_link_libraries (com.qtistatic.stats.af dl)
target_link_libraries (com.qtistatic.stats.af cutils)
target_link_libraries (com.qtistatic.stats.af log)
target_link_libraries (com.qtistatic.stats.af sync)
# Include Paths
include_directories (${CAMX_C_INCLUDES})
include_directories (../..)
include_directories (${CAMX_CDK_PATH}/generated/g_parser)
include_directories (${CAMX_CDK_PATH}/generated/g_chromatix)
include_directories (${CAMX_CDK_PATH})
include_directories (${CAMX_CDK_PATH}/stats)
include_directories (${CAMX_CDK_PATH}/fd)
include_directories (${CAMX_VENDOR_PATH}/node/staticafalgo)
include_directories (${CAMX_VENDOR_PATH}/node/staticafalgo/inc)

# Dependencies


# Definitions
add_definitions (${CAMX_CFLAGS})
add_definitions (${CAMX_CPPFLAGS})
# NO DATA FOR SECTION: Custom additions (end)

# Auto-Gen
add_dependencies(com.qtistatic.stats.af autogen)

install(TARGETS com.qtistatic.stats.af LIBRARY DESTINATION lib/${CAMX_LIB_OUTPUT_PATH})

if (CONFIG STREQUAL "Debug")
    add_definitions( -g )
endif()

