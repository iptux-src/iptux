glib_dep = dependency('glib-2.0', version: '>=2.35')
jsoncpp_dep = dependency('jsoncpp', version: '>=1.0')
glog_dep = dependency('libglog')

header_files = files([
    'CoreThread.h',
    'ProgramData.h',
    'IptuxConfig.h',
    'Event.h',
    'TransFileModel.h',
    'Models.h',
])
install_headers(header_files, subdir: 'iptux-core')

core_sources = files([
    'internal/AnalogFS.cpp',
    'internal/Command.cpp',
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

thread_dep = dependency('threads')
libiptux_core = shared_library('iptux-core',
    core_sources,
    dependencies: [glib_dep, jsoncpp_dep, glog_dep, thread_dep],
    link_with: [libiptux_utils],
    include_directories: inc,
    install: true,
    version: so_version,
)

pkg = import('pkgconfig')
pkg.generate(
    description: 'Communicate and Share File in LAN',
    name: 'iptux-core',
    version: so_version,
    requires: ['jsoncpp >= 1.0', 'glib-2.0 >= 2.35'],
    requires_private: ['libglog'],
    libraries: [libiptux_core, glog_dep],
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


