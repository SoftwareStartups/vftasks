# Copyright (c) 2010-2012 Vector Fabrics B.V. All rights reserved.

# This file contains proprietary and confidential information of Vector
# Fabrics and all use (including distribution) is subject to the conditions of
# the license agreement between you and Vector Fabrics. Without such a license
# agreement in place, no usage or distribution rights are granted by Vector
# Fabrics.

PROJECT(Pareon)

include_directories(../include)
add_library(vftasks tasks.c sync_1d.c sync_2d.c streams.c semaphore.c timer.c)

install(TARGETS vftasks DESTINATION lib/${CMAKE_LIBRARY_ARCHITECTURE})

SET(CPACK_PACKAGE_NAME "${PACKAGENAME}${MAJOR}${MINOR}")
SET(CPACK_PACKAGE_VENDOR "Vector Fabrics B.V.")
SET(CPACK_PACKAGE_FILE_NAME "${PACKAGENAME}${MAJOR}${MINOR}-${MAJOR}.${MINOR}.${BUILD}")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Vector Fabrics Pareon parallelization library")
SET(CPACK_PACKAGE_CONTACT "info@vectorfabrics.com")

SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "all")
SET(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${BUILD}") # use hudson build id

SET(CPACK_GENERATOR "DEB;RPM")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Vector Fabrics B.V. <info@vectorfabrics.com>")

SET(CPACK_PACKAGING_INSTALL_PREFIX
  "/opt/vectorfabrics/${PACKAGENAME}-${MAJOR}.${MINOR}.${BUILD}")

INCLUDE(CPack)
