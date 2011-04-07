# Copyright (c) 2010-2011 Vector Fabrics B.V. All rights reserved.

# This file contains proprietary and confidential information of Vector
# Fabrics and all use (including distribution) is subject to the conditions of
# the license agreement between you and Vector Fabrics. Without such a license
# agreement in place, no usage or distribution rights are granted by Vector
# Fabrics.

include(CheckCCompilerFlag)
include(CheckLibraryExists)

cmake_minimum_required(VERSION 2.6)

macro(append_if list condition var)
  if (${condition})
    list(APPEND ${list} ${var})
  endif()
endmacro()

project(vftasks)

check_c_compiler_flag(-Wall HAS_WALL_FLAG)
check_c_compiler_flag(-pedantic HAS_PEDANTIC_FLAG)
check_c_compiler_flag(-O3 HAS_O3_FLAG)

append_if(VFTASKS_COMPILER_FLAGS HAS_WALL_FLAG -Wall)
append_if(VFTASKS_COMPILER_FLAGS HAS_PEDANTIC -pedantic)
append_if(VFTASKS_COMPILER_FLAGS HAS_O3_FLAG -O3)
add_definitions(${VFTASKS_COMPILER_FLAGS})

find_library(PTHREAD_LIB pthread)
find_library(CPPUNIT cppunit)
find_program(DOXYGEN doxygen)

add_subdirectory(src)
add_subdirectory(include)

if (DOXYGEN)
  add_subdirectory(doc EXCLUDE_FROM_ALL)
endif (DOXYGEN)

if (PTHREAD_LIB)
  add_subdirectory(unit-test)
  enable_testing()

  if (CPPUNIT)
    add_subdirectory(example EXCLUDE_FROM_ALL)
  endif (CPPUNIT)
endif (PTHREAD_LIB)

# installation
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "")
endif (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_FILE_NAME "vftasks")
include(CPack)
