cmake_minimum_required(VERSION 3.7.2)
project(osmc-core CXX)

include(FetchContent)
include(ExternalProject)

FetchContent_Declare(httprequest
        GIT_REPOSITORY https://github.com/elnormous/HTTPRequest.git
        GIT_TAG 4c3bbe94b60d67b58716dcd1696a34520e3bc073
#        GIT_SHALLOW ON #GIt shallow does not work with commit hash. See https://cmake.org/cmake/help/git-stage/module/ExternalProject.html under GIT_TAG <tag>
        SOURCE_DIR "${CMAKE_BINARY_DIR}/httprequest"
        )

FetchContent_Populate(httprequest)

enable_testing()
include(CheckCXXCompilerFlag)

set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB_RECURSE CPP_FILES src/*.cpp)
file(GLOB C_FILES src/*.c)

add_library(osmc-core STATIC ${C_FILES} ${CPP_FILES} src/CallBackTimer.h src/Webserver.cpp src/Webserver.h)
target_link_libraries(osmc-core httplib::httplib)
set_target_properties(osmc-core
        PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(osmc-core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)

#################################### TESTS ####################################

list(FILTER CPP_FILES EXCLUDE REGEX ".*src\\/main\\.cpp")

# ------------------------------- UNIT TESTS ----------------------------------
file(GLOB CPP_TEST_FILES test/*.cpp)
list(FILTER CPP_TEST_FILES EXCLUDE REGEX ".*_it.cpp")


add_executable(unit-test-${PROJECT_NAME} ${CPP_TEST_FILES})
target_include_directories(unit-test-${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../httprequest/include)
target_link_libraries(unit-test-${PROJECT_NAME} gtest_main osmc-core)


add_test(NAME unit-test-${PROJECT_NAME} COMMAND unit-test-${PROJECT_NAME}  --gtest_output=xml:gtestresults.xml)



