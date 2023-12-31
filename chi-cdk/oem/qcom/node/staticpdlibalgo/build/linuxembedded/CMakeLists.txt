# User Variables
set (CHROMATIX_VERSION 0x0310)

set (SUPPORT_FUNCTION_CALL_TRACE 1)

# NO DATA FOR SECTION: Custom additions (begin)

# Common Include
include (${CAMX_CDK_PATH}/../linuxembedded/common.cmake)

# Files and Build Type
add_library( com.qtistatic.stats.pdlib SHARED
    ../../src/camxpdlib.cpp
)

# Libraries to compile
target_link_libraries (com.qtistatic.stats.pdlib camxosutils)
target_link_libraries (com.qtistatic.stats.pdlib camxutils)
target_link_libraries (com.qtistatic.stats.pdlib camxgenerated)
target_link_libraries (com.qtistatic.stats.pdlib dl)
target_link_libraries (com.qtistatic.stats.pdlib cutils)
target_link_libraries (com.qtistatic.stats.pdlib log)
target_link_libraries (com.qtistatic.stats.pdlib sync)

# Include Paths
include_directories (${CAMX_C_INCLUDES})
include_directories (../..)
include_directories (${CAMX_CDK_PATH}/generated/g_chromatix)
include_directories (${CAMX_CDK_PATH}/generated/g_parser)
include_directories (../../../../../../camx/build/infrastructure/makemeister/../../../../chi-cdk/vendor/node/staticpdlibalgo/build/android/system/media/camera/include)
include_directories (${CAMX_CDK_PATH})
include_directories (${CAMX_CDK_PATH}/stats)
include_directories (${CAMX_VENDOR_PATH}/node/staticpdlibalgo)

# Dependencies


# Definitions
add_definitions (${CAMX_CFLAGS})
add_definitions (${CAMX_CPPFLAGS})
# NO DATA FOR SECTION: Custom additions (end)

# Auto-Gen
#add_dependencies(com.qtistatic.stats.pdlib autogen)
install(TARGETS com.qtistatic.stats.pdlib  LIBRARY DESTINATION lib/${CAMX_LIB_OUTPUT_PATH}
        )
if (CONFIG STREQUAL "Debug")
    add_definitions( -g )
endif()

