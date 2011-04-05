# Copyright (c) 2010-2011 Vector Fabrics B.V. All rights reserved.

# This file contains proprietary and confidential information of Vector
# Fabrics and all use (including distribution) is subject to the conditions of
# the license agreement between you and Vector Fabrics. Without such a license
# agreement in place, no usage or distribution rights are granted by Vector
# Fabrics.

cmake_minimum_required(VERSION 2.6)

project(docs)

find_program(doxygen doxygen)

if (EXISTS ${doxygen})
  set(input ${CMAKE_CURRENT_SOURCE_DIR}/config/doxygen.conf)
  set(output ${CMAKE_BINARY_DIR}/html)

  add_custom_target(
    doxygen
    COMMAND ${doxygen} ${input}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${input}
    )

else (EXISTS ${doxygen})
  message(WARNING "not generating doxygen target: doxygen not installed")
endif(EXISTS ${doxygen})
