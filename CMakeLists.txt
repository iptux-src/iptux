cmake_minimum_required (VERSION 3.1)
project (iptux
	VERSION 0.8.0)

set(VERSION_SUFFIX "-git")

set (CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

if(CMAKE_COMPILER_IS_GNUCXX)
    include(CodeCoverage)
    setup_target_for_coverage(${PROJECT_NAME}_coverage tests coverage)
endif()


if (APPLE AND EXISTS /usr/local/opt/gettext)
    list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/gettext")
    set(CMAKE_EXE_LINKER_FLAGS "-L/usr/local/opt/gettext/lib")
endif()

include(GNUInstallDirs)
include(FindPkgConfig)
include(FindGettext)
include(FindIntl)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -pthread -g -O0 -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGTK_DISABLE_SINGLE_INCLUDES -DGSEAL_ENABLE")

if(NOT Intl_FOUND)
	message(FATAL_ERROR "libintl not found")
endif(NOT Intl_FOUND)

pkg_check_modules(GST gstreamer-1.0>=1.0)

configure_file (
  "${PROJECT_SOURCE_DIR}/src/iptux/config.h.in"
  "${PROJECT_BINARY_DIR}/src/iptux/config.h"
  )

enable_testing()
add_subdirectory(src)
add_subdirectory(po)
add_subdirectory(pixmaps)
add_subdirectory(sound)
add_subdirectory(desktop)
