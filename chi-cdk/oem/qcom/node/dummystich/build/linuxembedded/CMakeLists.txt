# User Variables
set (SUPPORT_FUNCTION_CALL_TRACE 1)

# NO DATA FOR SECTION: Custom additions (begin)

# Common Include
include (${CAMX_CDK_PATH}/../linuxembedded/common.cmake)

# Files and Build Type
add_library( com.qti.node.stich SHARED
    ../../camxchinodedummystich.cpp
)

# Libraries to compile
target_link_libraries (com.qti.node.stich dl)
target_link_libraries (com.qti.node.stich libcom.qti.chinodeutils)

# Include Paths
include_directories (${CAMX_C_INCLUDES})
include_directories (../..)
include_directories (${CAMX_CDK_PATH}/node/)
#include_directories (../../../../../../camx/build/infrastructure/makemeister/../../../../chi-cdk/vendor/node/dummystich/build/android/system/media/camera/include)

# Dependencies


# Definitions
add_definitions (${CAMX_CFLAGS})
add_definitions (${CAMX_CPPFLAGS})
# NO DATA FOR SECTION: Custom additions (end)

# Auto-Gen
#add_dependencies(com.qti.node.stich autogen)

install(TARGETS com.qti.node.stich LIBRARY DESTINATION lib/${CAMX_LIB_OUTPUT_PATH})

if (CONFIG STREQUAL "Debug")
    add_definitions( -g )
endif()

