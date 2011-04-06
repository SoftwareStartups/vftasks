# Copyright (c) 2010-2011 Vector Fabrics B.V. All rights reserved.

# This file contains proprietary and confidential information of Vector
# Fabrics and all use (including distribution) is subject to the conditions of
# the license agreement between you and Vector Fabrics. Without such a license
# agreement in place, no usage or distribution rights are granted by Vector
# Fabrics.

include(CheckIncludeFile)
include(CheckIncludeFileCXX)

cmake_minimum_required(VERSION 2.6)

project(unit_test)

CHECK_INCLUDE_FILE(pthread.h pthread)
CHECK_INCLUDE_FILE_CXX(cppunit/TestRunner.h unittest)

include_directories(../include)
add_definitions(-O3)
add_library(vftasks tasks.c sync.c streams.c)


install(TARGETS vftasks DESTINATION lib)