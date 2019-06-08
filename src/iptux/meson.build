gtk_dep = dependency('gtk+-3.0', version: '>=3.12')
jsoncpp_dep = dependency('jsoncpp', version: '>=1.0')
gstreamer_dep = dependency('gstreamer-1.0', version: '>=1.0', required:false)
glog_dep = dependency('libglog')

core_sources = files([
    '../iptux-core/AnalogFS.cpp',
    '../iptux-core/Command.cpp',
    '../iptux-core/CoreThread.cpp',
    '../iptux-core/Event.cpp',
    '../iptux-core/TcpData.cpp',
    '../iptux-core/UdpData.cpp',
    '../iptux-core/Models.cpp',
    '../iptux-core/support.cpp',
    '../iptux-core/utils.cpp',
    '../iptux-core/ProgramData.cpp',
    '../iptux-core/output.cpp',
    '../iptux-core/Exception.cpp',
    '../iptux-core/IptuxConfig.cpp',
    '../iptux-core/SendFile.cpp',
    '../iptux-core/RecvFile.cpp',
    '../iptux-core/RecvFileData.cpp',
    '../iptux-core/SendFileData.cpp',
    '../iptux-core/CommandMode.cpp',
    '../iptux-core/TransAbstract.cpp',
    '../iptux-core/TransFileModel.cpp',
])

sources = files([
    'Application.cpp',
    'DataSettings.cpp',
    'DetectPal.cpp',
    'DialogBase.cpp',
    'DialogGroup.cpp',
    'DialogPeer.cpp',
    'HelpDialog.cpp',
    'LogSystem.cpp',
    'MainWindow.cpp',
    'RevisePal.cpp',
    'ShareFile.cpp',
    'SoundSystem.cpp',
    'StatusIcon.cpp',
    'TransWindow.cpp',
    'UiCoreThread.cpp',
    'UiHelper.cpp',
    'UiModels.cpp',
    'UiProgramData.cpp',
    'WindowConfig.cpp',
    'callback.cpp',
    'dialog.cpp',
    'global.cpp',
])

glib_compile_resources = find_program('glib-compile-resources')

iptux_resource_h = custom_target('iptux_resource_h',
  output : 'IptuxResource.h',
  input : 'resources/iptux.gresource.xml',
  command : [glib_compile_resources,
    '--generate-header',
    '--manual-register',
    '--target=@OUTPUT@',
    '--sourcedir=' + join_paths(meson.current_source_dir(), 'resources'),
    '@INPUT@'])
iptux_resource_cpp = custom_target('iptux_resource_cpp',
  output : 'IptuxResource.cpp',
  input : 'resources/iptux.gresource.xml',
  command : [glib_compile_resources,
    '--generate-source',
    '--manual-register',
    '--target=@OUTPUT@',
    '--sourcedir=' + join_paths(meson.current_source_dir(), 'resources'),
    '@INPUT@'])

sources += [iptux_resource_h, iptux_resource_cpp]
inc = include_directories('..')

libiptux = static_library('iptux',
    sources + core_sources,
    dependencies: [gtk_dep, jsoncpp_dep, gstreamer_dep, glog_dep],
    include_directories: inc
)

gtest_inc = include_directories('../googletest/include')
thread_dep = dependency('threads')
core_test_sources = files([
    '../iptux-core/CoreThreadTest.cpp',
    '../iptux-core/UdpDataTest.cpp',
    '../iptux-core/ModelsTest.cpp',
    '../iptux-core/UtilsTest.cpp',
    '../iptux-core/ProgramDataTest.cpp',
    '../iptux-core/IptuxConfigTest.cpp',
    '../iptux-core/TestHelper.cpp',
    '../iptux-core/CommandModeTest.cpp',
])
test_sources = files([
    'LogSystemTest.cpp',
    'TestMain.cpp',
    'UiCoreThreadTest.cpp',
    'UiProgramDataTest.cpp',
])
libiptux_test = executable('libiptux_test',
    test_sources+core_test_sources,
    dependencies: [gtk_dep, jsoncpp_dep, gstreamer_dep, thread_dep],
    link_with: [libiptux, libgtest],
    include_directories: [inc, gtest_inc]
)
test('unit test', libiptux_test)


