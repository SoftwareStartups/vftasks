# Copyright (c) 2010-2011 Vector Fabrics B.V. All rights reserved.

# This file contains proprietary and confidential information of Vector
# Fabrics and all use (including distribution) is subject to the conditions of
# the license agreement between you and Vector Fabrics. Without such a license
# agreement in place, no usage or distribution rights are granted by Vector
# Fabrics.

set(config_file ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf)

configure_file(config/doxygen.conf.in ${config_file})

add_custom_target(
  doc
  COMMAND ${DOXYGEN} ${config_file}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

