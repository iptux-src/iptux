gtk_dep = dependency('gtk+-3.0', version: '>=3.12')
jsoncpp_dep = dependency('jsoncpp', version: '>=1.0')
gstreamer_dep = dependency('gstreamer-1.0', version: '>=1.0', required:false)
glog_dep = dependency('libglog')

conf_data = configuration_data()
conf_data.set_quoted('VERSION', meson.project_version())
# the 1_ prefix is used for compatible with FeiQ
conf_data.set_quoted('IPTUX_VERSION', '1_iptux '+meson.project_version())
conf_data.set_quoted('GETTEXT_PACKAGE', meson.project_name())
conf_data.set('CMAKE_INSTALL_FULL_DATADIR',
    join_paths(get_option('prefix'), get_option('datadir')))
if gstreamer_dep.found()
    conf_data.set('GST_FOUND', true)
endif
configure_file(input : 'config.h.in',
               output : 'config.h',
               configuration : conf_data)

test_conf_data = configuration_data()
test_conf_data.set_quoted('PROJECT_ROOT_PATH', meson.source_root())
configure_file(input : 'TestConfig.h.in',
               output : 'TestConfig.h',
               configuration : test_conf_data)

core_sources = files([
    '../iptux-core/Command.cpp',
    '../iptux-core/CoreThread.cpp',
    '../iptux-core/TcpData.cpp',
    '../iptux-core/UdpData.cpp',
    '../iptux-core/Models.cpp',
])

sources = files([
    'AnalogFS.cpp',
    'Application.cpp',
    'CommandMode.cpp',
    'DataSettings.cpp',
    'DetectPal.cpp',
    'DialogBase.cpp',
    'DialogGroup.cpp',
    'DialogPeer.cpp',
    'Event.cpp',
    'Exception.cpp',
    'HelpDialog.cpp',
    'IptuxConfig.cpp',
    'LogSystem.cpp',
    'MainWindow.cpp',
    'ProgramData.cpp',
    'RecvFile.cpp',
    'RecvFileData.cpp',
    'RevisePal.cpp',
    'SendFile.cpp',
    'SendFileData.cpp',
    'ShareFile.cpp',
    'SoundSystem.cpp',
    'StatusIcon.cpp',
    'TransAbstract.cpp',
    'TransFileModel.cpp',
    'TransWindow.cpp',
    'UiCoreThread.cpp',
    'UiHelper.cpp',
    'UiModels.cpp',
    'UiProgramData.cpp',
    'WindowConfig.cpp',
    'callback.cpp',
    'dialog.cpp',
    'global.cpp',
    'output.cpp',
    'support.cpp',
    'utils.cpp',
    'wrapper.cpp'
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
])
test_sources = files([
    'CommandModeTest.cpp',
    'IptuxConfigTest.cpp',
    'LogSystemTest.cpp',
    'ProgramDataTest.cpp',
    'TestHelper.cpp',
    'TestMain.cpp',
    'UiCoreThreadTest.cpp',
    'UiProgramDataTest.cpp',
    'UtilsTest.cpp'
])
libiptux_test = executable('libiptux_test',
    test_sources+core_test_sources,
    dependencies: [gtk_dep, jsoncpp_dep, gstreamer_dep, thread_dep],
    link_with: [libiptux, libgtest],
    include_directories: [inc, gtest_inc]
)
test('unit test', libiptux_test)


