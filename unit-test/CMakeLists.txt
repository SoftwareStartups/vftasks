# Copyright (c) 2010-2011 Vector Fabrics B.V. All rights reserved.

# This file contains proprietary and confidential information of Vector
# Fabrics and all use (including distribution) is subject to the conditions of
# the license agreement between you and Vector Fabrics. Without such a license
# agreement in place, no usage or distribution rights are granted by Vector
# Fabrics.

file(GLOB src src/*.cpp)
include_directories(../include ../src)
add_executable(unit_test ${src})
enable_testing()
add_test(unit_test unit_test)

# workaround for missing cmake feature:
# (http://stackoverflow.com/questions/733475/cmake-ctest-make-test-doesnt-build-tests)
add_custom_target(run_tests COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS unit_test)

if (${CMAKE_USE_PTHREADS_INIT})
  target_link_libraries(unit_test vftasks ${CMAKE_THREAD_LIBS_INIT} ${CPPUNIT_LIB})
else (${CMAKE_USE_PTHREADS_INIT})
  target_link_libraries(unit_test vftasks ${CPPUNIT_LIB})
endif (${CMAKE_USE_PTHREADS_INIT})
