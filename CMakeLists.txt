enable_language(C)
enable_language(CXX)
include(CheckCCompilerFlag)
include(FindThreads)

cmake_minimum_required(VERSION 2.6)

project(vftasks)

check_c_compiler_flag(-Wall HAS_WALL_FLAG)
if (${HAS_WALL_FLAG})
  add_definitions(-Wall)
endif (${HAS_WALL_FLAG})

set(CMAKE_C_FLAGS_RELEASE "-DVFTASKS_ABORT_ON_FAILURE ${CMAKE_C_FLAGS_RELEASE}")

find_library(CPPUNIT_LIB cppunit)
find_program(DOXYGEN doxygen)

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(example)
add_subdirectory(benchmark)

if (DOXYGEN)
  add_subdirectory(doc)
else (DOXYGEN)
  message(STATUS "Doxygen not found, not configuring doc target")
endif (DOXYGEN)

if (CMAKE_BUILD_TYPE STREQUAL debug)
  if (CPPUNIT_LIB)
    add_subdirectory(unit-test)
    enable_testing()
  else (CPPUNIT_LIB)
    message(STATUS "Cppunit not installed, not configuring unit_test target")
  endif (CPPUNIT_LIB)
endif (CMAKE_BUILD_TYPE STREQUAL debug)
