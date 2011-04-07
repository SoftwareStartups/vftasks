# Copyright (c) 2010-2011 Vector Fabrics B.V. All rights reserved.

# This file contains proprietary and confidential information of Vector
# Fabrics and all use (including distribution) is subject to the conditions of
# the license agreement between you and Vector Fabrics. Without such a license
# agreement in place, no usage or distribution rights are granted by Vector
# Fabrics.

cmake_minimum_required(VERSION 2.6)

project(vftasks)

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(doc EXCLUDE_FROM_ALL)
add_subdirectory(unit-test)
add_subdirectory(example EXCLUDE_FROM_ALL)

enable_testing()

# installation
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
set(CMAKE_INSTALL_PREFIX "")
endif (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_FILE_NAME "vfstream")
include(CPack)

