# Copyright (c) 2010-2011 Vector Fabrics B.V. All rights reserved.

# This file contains proprietary and confidential information of Vector
# Fabrics and all use (including distribution) is subject to the conditions of
# the license agreement between you and Vector Fabrics. Without such a license
# agreement in place, no usage or distribution rights are granted by Vector
# Fabrics.

include_directories(../include)
add_executable(streams src/streams.c)
target_link_libraries(streams vftasks pthread)

add_executable(2d_partitioned src/2d_partitioned.c)
target_link_libraries(2d_partitioned vftasks pthread)
