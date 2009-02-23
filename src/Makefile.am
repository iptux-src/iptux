# main program
bin_PROGRAMS = iptux
iptux_SOURCES = AboutIptux.cpp Command.cpp Control.cpp CoreThread.cpp \
	DetectPal.cpp DialogGroup.cpp DialogPeer.cpp IptuxSetting.cpp Log.cpp MainWindow.cpp \
	Pal.cpp RecvFile.cpp RevisePal.cpp SendFile.cpp ShareFile.cpp Sound.cpp \
	StatusIcon.cpp TcpData.cpp Transport.cpp UdpData.cpp baling.cpp dialog.cpp iptux.cpp \
	my_chooser.cpp my_entry.cpp my_file.cpp output.cpp support.cpp utils.cpp

AM_CXXFLAGS = -Wall

# set the include path found by configure
INCLUDES= $(all_includes) @PACKAGE_CFLAGS@

# the library search path.
iptux_LDADD = $(INTLLIBS) @PACKAGE_LIBS@
iptux_LDFLAGS = $(all_libraries)

noinst_HEADERS = AboutIptux.h Command.h Control.h CoreThread.h DetectPal.h \
	DialogGroup.h DialogPeer.h IptuxSetting.h Log.h MainWindow.h Pal.h RecvFile.h \
	RevisePal.h SendFile.h ShareFile.h Sound.h StatusIcon.h TcpData.h Transport.h \
	UdpData.h baling.h common.h dialog.h face.h ipmsg.h my_chooser.h my_entry.h \
	my_file.h net.h output.h support.h sys.h udt.h utils.h

SUBDIRS = uninstall
