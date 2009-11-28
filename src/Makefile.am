# main program
bin_PROGRAMS = iptux
iptux_SOURCES = AnalogFS.cpp Command.cpp CoreThread.cpp DataSettings.cpp \
	DetectPal.cpp DialogGroup.cpp DialogPeer.cpp HelpDialog.cpp LogSystem.cpp \
	MainWindow.cpp ProgramData.cpp RecvFile.cpp RecvFileData.cpp RevisePal.cpp \
	SendFile.cpp SendFileData.cpp ShareFile.cpp SoundSystem.cpp StatusIcon.cpp \
	TcpData.cpp UdpData.cpp callback.cpp dialog.cpp iptux.cpp mess.cpp output.cpp \
	support.cpp utils.cpp wrapper.cpp

AM_CXXFLAGS = -Wall

# set the include path found by configure
INCLUDES= $(all_includes) @PACKAGE_CFLAGS@

# the library search path.
iptux_LDFLAGS = $(all_libraries) @PACKAGE_LIBS@

noinst_HEADERS = AnalogFS.h Command.h CoreThread.h DataSettings.h DetectPal.h \
	DialogGroup.h DialogPeer.h HelpDialog.h LogSystem.h MainWindow.h ProgramData.h \
	RecvFile.h RecvFileData.h RevisePal.h SendFile.h SendFileData.h ShareFile.h \
	SoundSystem.h StatusIcon.h TcpData.h UdpData.h callback.h deplib.h dialog.h \
	ipmsg.h mess.h net.h output.h support.h sys.h utils.h wrapper.h

SUBDIRS = uninstall
