# Copyright (c) 2010-2011 Vector Fabrics B.V. All rights reserved.

# This file contains proprietary and confidential information of Vector
# Fabrics and all use (including distribution) is subject to the conditions of
# the license agreement between you and Vector Fabrics. Without such a license
# agreement in place, no usage or distribution rights are granted by Vector
# Fabrics.

cmake_minimum_required(VERSION 2.6)

project(vfmulti)

add_subdirectory(doc EXCLUDE_FROM_ALL)
add_subdirectory(unit-test EXCLUDE_FROM_ALL)
add_subdirectory(example EXCLUDE_FROM_ALL)


# library creation

file(GLOB vfstream_src vfstream/src/*.c)
file(GLOB vftasks_src vftasks/src/*.c)
file(GLOB vfsync_src vfsync/src/*.c)
file(GLOB unit_tst_src unit-test/src/*.cpp)
file(GLOB example_src example/src/*.c)

set(src ${vfstream_src} ${vftasks_src} ${vfsync_src})

include_directories(vfstream/include vftasks/include vfsync/include)

add_definitions(-O3)

# FIXME: rename vfstream. this name is currently used to not break vfana builds
add_library(vfstream ${src})


# testing

enable_testing()

add_test(unit_test unit-test/unit_test)
add_test(example example/example)


# installation

file(GLOB vfstream_incl vfstream/include/*.h)
file(GLOB vftasks_incl vftasks/include/*.h)
file(GLOB vfsync_incl vfsync/include/*.h)

set(inc_files ${vfstream_incl} ${vftasks_incl} ${vfsync_incl})

set(CMAKE_INSTALL_PREFIX $ENV{VFSTREAMINSTALL})
install(TARGETS vfstream DESTINATION lib)
install(FILES ${inc_files} DESTINATION include)
