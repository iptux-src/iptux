pkg_check_modules (IPTUX REQUIRED glib-2.0>=2.32.0 gthread-2.0>=2.14.0 gtk+-3.0 jsoncpp>=1.0)
include_directories("${PROJECT_SOURCE_DIR}/src" "${PROJECT_BINARY_DIR}/src" "${IPTUX_INCLUDE_DIRS}")
link_libraries("${IPTUX_LIBRARIES}")
link_libraries("${Intl_LIBRARIES}")
if(GST_FOUND)
	include_directories("${GST_INCLUDE_DIRS}")
	link_libraries("${GST_LIBRARIES}")
endif(GST_FOUND)
add_executable (iptuxbin
	iptux.cpp
	)
set_target_properties(iptuxbin PROPERTIES OUTPUT_NAME iptux)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(iptuxbin iptux Threads::Threads)

install (TARGETS iptuxbin DESTINATION bin)
