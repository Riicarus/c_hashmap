cmake_minimum_required(VERSION 3.20.0)
project(c_hashmap_lib VERSION 1.0 LANGUAGES C)

set(CMAKE_BUILD_TYPE "Release")
if(NEED_TEST)
    set(CMAKE_BUILD_TYPE "Debug")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O3")
    set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
endif(NEED_TEST)

# include .c files
file(GLOB SRC ${PROJECT_SOURCE_DIR}/src/*.c)
# include .h files
include_directories(${PROJECT_SOURCE_DIR}/include)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
# create shared lib(.so)
add_library(c_hashmap SHARED ${SRC})
# create static lib(.a)
add_library(c_hashmap_static STATIC ${SRC})

set_target_properties(c_hashmap_static PROPERTIES OUTPUT_NAME c_hashmap)
set_target_properties(c_hashmap PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(c_hashmap_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

option(NEED_TEST OFF)
if(NEED_TEST)
    # build test for c_hashmap
    set(TEST_SRC ${PROJECT_SOURCE_DIR}/test/map_test.c)
    add_executable(map_test ${TEST_SRC})
    # link lib for map_test
    target_link_libraries(map_test c_hashmap)
endif(NEED_TEST)