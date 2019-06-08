glib_dep = dependency('glib-2.0', version: '>=2.35')
jsoncpp_dep = dependency('jsoncpp', version: '>=1.0')
glog_dep = dependency('libglog')

# conf_data = configuration_data()
# conf_data.set_quoted('VERSION', meson.project_version())
# # the 1_ prefix is used for compatible with FeiQ
# conf_data.set_quoted('IPTUX_VERSION', '1_iptux '+meson.project_version())
# conf_data.set_quoted('GETTEXT_PACKAGE', meson.project_name())
# conf_data.set('CMAKE_INSTALL_FULL_DATADIR',
#     join_paths(get_option('prefix'), get_option('datadir')))
# if gstreamer_dep.found()
#     conf_data.set('GST_FOUND', true)
# endif
# configure_file(input : 'config.h.in',
#                output : 'config.h',
#                configuration : conf_data)

# test_conf_data = configuration_data()
# test_conf_data.set_quoted('PROJECT_ROOT_PATH', meson.source_root())
# configure_file(input : 'TestConfig.h.in',
#                output : 'TestConfig.h',
#                configuration : test_conf_data)

core_sources = files([
    'AnalogFS.cpp',
    'Command.cpp',
    'CoreThread.cpp',
    'Event.cpp',
    'TcpData.cpp',
    'UdpData.cpp',
    'Models.cpp',
    'support.cpp',
    'utils.cpp',
    'ProgramData.cpp',
    'output.cpp',
    'Exception.cpp',
    'IptuxConfig.cpp',
    'SendFile.cpp',
    'RecvFile.cpp',
    'RecvFileData.cpp',
    'SendFileData.cpp',
    'CommandMode.cpp',
    'TransAbstract.cpp',
    'TransFileModel.cpp',
])

inc = include_directories('..')

libiptux_core = static_library('iptux-core',
    core_sources,
    dependencies: [glib_dep, jsoncpp_dep, glog_dep],
    include_directories: inc
)

core_test_helper_sources = files([
    'TestHelper.cpp',
])
libiptux_core_test_helper = static_library('iptux-test-helper',
    core_test_helper_sources,
    dependencies: [glib_dep, jsoncpp_dep, glog_dep],
    link_with: [libiptux_core],
    include_directories: inc
)

gtest_inc = include_directories('../googletest/include')
thread_dep = dependency('threads')
core_test_sources = files([
    'CoreThreadTest.cpp',
    'UdpDataTest.cpp',
    'ModelsTest.cpp',
    'UtilsTest.cpp',
    'ProgramDataTest.cpp',
    'IptuxConfigTest.cpp',
    'CommandModeTest.cpp',
    'TestMain.cpp',
])
libiptux_core_test = executable('libiptux_core_test',
    core_test_sources,
    dependencies: [glib_dep, jsoncpp_dep, thread_dep],
    link_with: [libiptux_core, libgtest, libiptux_core_test_helper],
    include_directories: [inc, gtest_inc]
)
test('iptux core unit test', libiptux_core_test)


