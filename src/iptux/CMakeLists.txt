pkg_check_modules (IPTUX REQUIRED glib-2.0>=2.32.0 gthread-2.0>=2.14.0 gtk+-2.0>=2.16.0 jsoncpp>=1.0)
include_directories("${PROJECT_SOURCE_DIR}/src" "${PROJECT_BINARY_DIR}/src" "${IPTUX_INCLUDE_DIRS}")
link_libraries("${IPTUX_LIBRARIES}")
link_libraries("${Intl_LIBRARIES}")
if(GST_FOUND)
	include_directories("${GST_INCLUDE_DIRS}")
	link_libraries("${GST_LIBRARIES}")
endif(GST_FOUND)

add_library(iptux
	STATIC
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
	WindowConfig.cpp
	)


add_executable(libiptux_test
	UtilsTest.cpp
	TestMain.cpp
	)
target_link_libraries(libiptux_test gtest iptux)
add_test(NAME libiptux_test COMMAND libiptux_test)
