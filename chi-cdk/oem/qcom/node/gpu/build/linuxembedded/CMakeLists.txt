# User Variables
set (SUPPORT_FUNCTION_CALL_TRACE 1)

# NO DATA FOR SECTION: Custom additions (begin)

# Common Include
include (${CAMX_CDK_PATH}/../linuxembedded/common.cmake)

# Files and Build Type
add_library( com.qti.node.gpu SHARED
    ../../camxchinodegpu.cpp
)

# Libraries to compile
target_link_libraries (com.qti.node.gpu camxosutils)
target_link_libraries (com.qti.node.gpu camxutils)
target_link_libraries (com.qti.node.gpu dl)
target_link_libraries (com.qti.node.gpu libcom.qti.chinodeutils)
target_link_libraries (com.qti.node.gpu glib-2.0)

# Include Paths
include_directories (${CAMX_C_INCLUDES})
include_directories (../..)
#include_directories (../../../../../../camx/build/infrastructure/makemeister/../../../../chi-cdk/vendor/node/gpu/build/android/frameworks/native/libs/arect/include)
#include_directories (../../../../../../camx/build/infrastructure/makemeister/../../../../chi-cdk/vendor/node/gpu/build/android/frameworks/native/libs/nativewindow/include)
#include_directories (${TARGET_OUT_HEADERS}/adreno)
#include_directories (../../../../../../camx/build/infrastructure/makemeister/../../../../chi-cdk/vendor/node/gpu/build/android/system/media/camera/include)

# Dependencies


# Definitions
add_definitions (${CAMX_CFLAGS})
add_definitions (${CAMX_CPPFLAGS})
# NO DATA FOR SECTION: Custom additions (end)

# Auto-Gen
#add_dependencies(com.qti.node.gpu autogen)

install(TARGETS com.qti.node.gpu LIBRARY DESTINATION lib/${CAMX_LIB_OUTPUT_PATH})

if (CONFIG STREQUAL "Debug")
    add_definitions( -g )
endif()

