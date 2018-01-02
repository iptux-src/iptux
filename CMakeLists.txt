cmake_minimum_required (VERSION 3.1)
project (iptux
	VERSION 0.6.5)


if (APPLE AND EXISTS /usr/local/opt/gettext)
    list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/gettext")
    set(CMAKE_EXE_LINKER_FLAGS "-L/usr/local/opt/gettext/lib")
endif()

include(GNUInstallDirs)
include(FindPkgConfig)
include(FindGettext)
include(FindIntl)

set(CMAKE_C_FLAGS "-Wall -O2")
set(CMAKE_CXX_FLAGS "-Wall -O2")

if(NOT Intl_FOUND)
	message(FATAL_ERROR "libintl not found")
endif(NOT Intl_FOUND)

pkg_check_modules(GST gstreamer-1.0>=1.0)

configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
  )

add_subdirectory(src)
add_subdirectory(po)
add_subdirectory(pixmaps)
add_subdirectory(sound)
add_subdirectory(desktop)
