# Copyright (c) 2010-2011 Vector Fabrics B.V. All rights reserved.

# This file contains proprietary and confidential information of Vector
# Fabrics and all use (including distribution) is subject to the conditions of
# the license agreement between you and Vector Fabrics. Without such a license
# agreement in place, no usage or distribution rights are granted by Vector
# Fabrics.

include(CheckCCompilerFlag)

cmake_minimum_required(VERSION 2.6)

macro(append_if list condition var)
  if (${condition})
    list(APPEND ${list} ${var})
  endif()
endmacro()

project(vftasks)

check_c_compiler_flag(-Wall HAS_WALL_FLAG)
check_c_compiler_flag(-pedantic HAS_PEDANTIC_FLAG)
append_if(VFTASKS_COMPILER_FLAGS HAS_WALL_FLAG -Wall)
append_if(VFTASKS_COMPILER_FLAGS HAS_PEDANTIC_FLAG -pedantic)
add_definitions(${VFTASKS_COMPILER_FLAGS})
set(CMAKE_C_FLAGS_RELEASE "-DVFTASKS_ABORT_ON_FAILURE ${CMAKE_C_FLAGS_RELEASE}")

find_library(CPPUNIT_LIB cppunit)
find_library(PTHREAD_LIB pthread)
find_program(DOXYGEN doxygen)

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(example)

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
