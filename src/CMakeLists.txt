PROJECT(Pareon)

include_directories(../include)
add_library(vftasks tasks.c sync_1d.c sync_2d.c streams.c semaphore.c timer.c)

install(TARGETS vftasks DESTINATION lib/${CMAKE_LIBRARY_ARCHITECTURE})

SET(CPACK_PACKAGE_NAME "${PACKAGENAME}${MAJOR}${MINOR}")
SET(CPACK_PACKAGE_VENDOR "SoftwareStartups.com")
SET(CPACK_PACKAGE_FILE_NAME "${PACKAGENAME}${MAJOR}${MINOR}-${MAJOR}.${MINOR}.${BUILD}")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Concurrent tasks and streaming library")
SET(CPACK_PACKAGE_CONTACT "info@softwarestartups.com")

SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "all")
SET(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${BUILD}") # use hudson build id

SET(CPACK_GENERATOR "DEB;RPM")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "SoftwareStartups.com <info@softwarestartups.com>")

SET(CPACK_PACKAGING_INSTALL_PREFIX "/opt/softwarestartups/${PACKAGENAME}-${MAJOR}.${MINOR}")

INCLUDE(CPack)
