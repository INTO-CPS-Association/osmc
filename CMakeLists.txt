cmake_minimum_required(VERSION 3.7.2)
set(CMAKE_MACOSX_RPATH 1)
include(TargetArch.cmake)
project(OSMCProject)
set(CMAKE_PROJECT_VERSION "1.0.0")
set(CMAKE_PROJECT_VERSION_MAJOR "")
set(CMAKE_PROJECT_VERSION_MINOR "")
set(CMAKE_PROJECT_VERSION_PATCH "")

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/build/install" )

message("FMU will be installed to: ${CMAKE_INSTALL_PREFIX}")

#include(toolchains/clang-format.cmake)
# You can tweak some common (for all subprojects) stuff here. For example:

#set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)

enable_testing()

#if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
#  message(SEND_ERROR "In-source builds are not allowed.")
#endif ()

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE ON)

# Remove 'lib' prefix for shared libraries on Windows
if (WIN32)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif ()

#set_property(GLOBAL PROPERTY GLOBAL_DEPENDS_DEBUG_MODE 1)
target_architecture(arch)
message(arch: ${arch})

if (NOT DEFINED THIRD_PARTY_LIBRARIES_ROOT)
    set(THIRD_PARTY_LIBRARIES_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/build/external")
    if (UNIX)
        if (APPLE)
            set(THIRD_PARTY_LIBRARIES_ROOT ${THIRD_PARTY_LIBRARIES_ROOT}/darwin)
        ELSE ()
            set(THIRD_PARTY_LIBRARIES_ROOT ${THIRD_PARTY_LIBRARIES_ROOT}/linux)
        ENDIF ()
        set(THIRD_PARTY_LIBRARIES_ROOT ${THIRD_PARTY_LIBRARIES_ROOT}-${arch})
    ELSEIF (WIN32)
    ENDIF ()
endif ()


message("Third parties libraries will use search path: ${THIRD_PARTY_LIBRARIES_ROOT}")


set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/googletest)
#add_subdirectory(thirdparty/restinio/dev)
set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
set(HTTPLIB_USE_ZLIB_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
set(HTTPLIB_USE_BROTLI_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/cpp-httplib)

add_subdirectory(osmc-core)
add_subdirectory(oosmc-fmu)