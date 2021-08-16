cmake_minimum_required(VERSION 3.7.2)
project(osmc-core CXX)

enable_testing()
include(CheckCXXCompilerFlag)

set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)

include_directories(${gtest_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src)


file(GLOB_RECURSE CPP_FILES src/*.cpp)
file(GLOB C_FILES src/*.c)

add_library(osmc-core STATIC ${C_FILES} ${CPP_FILES} src/CallBackTimer.h)
target_link_libraries(osmc-core)
set_target_properties(osmc-core
        PROPERTIES POSITION_INDEPENDENT_CODE ON)
#set_target_properties(rabbitmq PROPERTIES PREFIX "")

target_include_directories(osmc-core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)

#################################### TESTS ####################################

list(FILTER CPP_FILES EXCLUDE REGEX ".*src\\/main\\.cpp")




# ------------------------------- UNIT TESTS ----------------------------------
file(GLOB CPP_TEST_FILES test/*.cpp)
list(FILTER CPP_TEST_FILES EXCLUDE REGEX ".*_it.cpp")

add_executable(unit-test-${PROJECT_NAME} ${C_FILES} ${CPP_FILES} ${CPP_TEST_FILES} test/core_test.cpp)
target_link_libraries(unit-test-${PROJECT_NAME} gtest_main)

if (WIN32)
    add_test(unit-test-${PROJECT_NAME} wine ${PROJECT_NAME} --gtest_output=xml:gtestresults.xml)
else ()
    add_test(NAME unit-test-${PROJECT_NAME} COMMAND unit-test-${PROJECT_NAME}  --gtest_output=xml:gtestresults.xml)
endif ()

# ---------------------------- Integration TESTS ------------------------------

file(GLOB CPP_TEST_FILES test/*.cpp)
list(FILTER CPP_TEST_FILES INCLUDE REGEX ".*_it.cpp")

add_executable(it-test-${PROJECT_NAME} ${C_FILES} ${CPP_FILES} ${CPP_TEST_FILES})
target_link_libraries(it-test-${PROJECT_NAME} gtest_main)


