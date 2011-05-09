# Copyright (c) 2010-2011 Vector Fabrics B.V. All rights reserved.

# This file contains proprietary and confidential information of Vector
# Fabrics and all use (including distribution) is subject to the conditions of
# the license agreement between you and Vector Fabrics. Without such a license
# agreement in place, no usage or distribution rights are granted by Vector
# Fabrics.

include_directories(../include)

if (WIN32)
  set(libs vftasks)
else (WIN32)
  set(libs vftasks pthread)
endif (WIN32)

add_executable(2d src/2d.c)

add_executable(2dsync src/2dsync.c)
target_link_libraries(2dsync ${libs})

add_executable(loop src/loop.c)

add_executable(partitioned_loop src/partitioned_loop.c)
target_link_libraries(partitioned_loop ${libs})

add_executable(streams src/streams.c)
target_link_libraries(streams ${libs})
