cmake_minimum_required(VERSION 3.7.2)
project(osmc CXX)

message("Building osmc")

enable_testing()
include(CheckCXXCompilerFlag)

set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/fmi/include)
include_directories(${gtest_SOURCE_DIR}/include)

add_definitions(-DFMI_COSIMULATION)

if (NOT BUILD_SHARED_LIBS)
    add_definitions(-DAMQP_STATIC)
endif ()

file(GLOB_RECURSE CPP_FILES src/*.cpp)
list(FILTER CPP_FILES EXCLUDE REGEX "main.cpp")
file(GLOB C_FILES src/*.c)

# Main
add_executable(${PROJECT_NAME}-main ${C_FILES} ${CPP_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_link_libraries(${PROJECT_NAME}-main osmc-core)

message("Creating library osmc with files: ${C_FILES} and ${CPP_FILES}")
add_library(osmc SHARED ${C_FILES} ${CPP_FILES})
target_link_libraries(osmc osmc-core)
set_target_properties(osmc
        PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(osmc PROPERTIES PREFIX "")

if (UNIX AND NOT APPLE)
    # for Linux, BSD, Solaris, Minix
    target_link_libraries(osmc dl)
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
message("Compiler is clang not adding -static-libgcc")
else ()
    string(APPEND CMAKE_SHARED_LINKER_FLAGS "-static-libgcc -static-libstdc++")
endif ()

if (WIN32)
    #windows
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
endif ()

#################################### TESTS ####################################

list(FILTER CPP_FILES EXCLUDE REGEX ".*src\\/main\\.cpp")

if (NOT ("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}"))
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/modelDescription.xml
            DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif ()


# ------------------------------- UNIT TESTS ----------------------------------
file(GLOB CPP_TEST_FILES test/*.cpp)
list(FILTER CPP_TEST_FILES EXCLUDE REGEX ".*_it.cpp")

add_executable(unit-test-${PROJECT_NAME} ${C_FILES} ${CPP_FILES} ${CPP_TEST_FILES})
target_link_libraries(unit-test-${PROJECT_NAME} gtest_main osmc-core)

if (WIN32)
    add_test(NAME unit-test-${PROJECT_NAME} COMMAND wine unit-test-${PROJECT_NAME}   --gtest_output=xml:gtestresults.xml)
else ()
    add_test(NAME unit-test-${PROJECT_NAME} COMMAND unit-test-${PROJECT_NAME}  --gtest_output=xml:gtestresults.xml)
endif ()

# ---------------------------- Integration TESTS ------------------------------

file(GLOB CPP_TEST_FILES test/*.cpp)
list(FILTER CPP_TEST_FILES INCLUDE REGEX ".*_it.cpp")

add_executable(it-test-${PROJECT_NAME} ${C_FILES} ${CPP_FILES} ${CPP_TEST_FILES})
target_link_libraries(it-test-${PROJECT_NAME} gtest_main osmc-core)


#################################### INSTALL targets ####################################
target_architecture(arch)
message(${arch})

set(FMU_OS "unknown")


if (UNIX)
    if (APPLE)
        set(FMU_OS "darwin")
    ELSE ()
        set(FMU_OS "linux")
    ENDIF ()
ELSEIF (WIN32)
    set(FMU_OS "win")
ENDIF ()


if (${arch} STREQUAL "x86_64")
    set(FMU_OS "${FMU_OS}64")
ELSE ()
    set(FMU_OS "${FMU_OS}32")
ENDIF ()


install(TARGETS osmc DESTINATION "osmcfmu/binaries/${FMU_OS}")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/modelDescription.xml" DESTINATION "osmcfmu/resources")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/modelDescription.xml" DESTINATION "osmcfmu")

#################################### Custom targets ####################################

add_custom_target(fmu

        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/mkFmu.sh $<TARGET_FILE:osmc> ${CMAKE_CURRENT_SOURCE_DIR}/modelDescription.xml
        COMMENT "Building fmu"
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        )
