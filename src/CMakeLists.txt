pkg_check_modules (IPTUX REQUIRED glib-2.0>=2.32.0 gthread-2.0>=2.14.0 gtk+-2.0>=2.12.0 gconf-2.0>=2.4.0)
include_directories("${PROJECT_BINARY_DIR}" "${IPTUX_INCLUDE_DIRS}")
link_libraries("${IPTUX_LIBRARIES}")
link_libraries("${Intl_LIBRARIES}")
if(GST_FOUND)
	include_directories("${GST_INCLUDE_DIRS}")
	link_libraries("${GST_LIBRARIES}")
endif(GST_FOUND)

add_executable (iptux
	AnalogFS.cpp
	callback.cpp
	Command.cpp
	CoreThread.cpp
	DataSettings.cpp
	DetectPal.cpp
	dialog.cpp
	DialogBase.cpp
	DialogGroup.cpp
	DialogPeer.cpp
	HelpDialog.cpp
	iptux.cpp
	IptuxConfig.cpp
	LogSystem.cpp
	MainWindow.cpp
	mess.cpp
	output.cpp
	ProgramData.cpp
	RecvFile.cpp
	RecvFileData.cpp
	RevisePal.cpp
	SendFile.cpp
	SendFileData.cpp
	ShareFile.cpp
	SoundSystem.cpp
	StatusIcon.cpp
	support.cpp
	TcpData.cpp
	UdpData.cpp
	utils.cpp
	wrapper.cpp
	)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(iptux Threads::Threads)

install (TARGETS iptux DESTINATION bin)
