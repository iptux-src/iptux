glib_dep = dependency('glib-2.0', version: '>=2.35')
jsoncpp_dep = dependency('jsoncpp', version: '>=1.0')
glog_dep = dependency('libglog')
gflags_dep = dependency('gflags')

header_files = files([
    'CoreThread.h',
    'Event.h',
    'Exception.h',
    'IptuxConfig.h',
    'Models.h',
    'ProgramData.h',
    'TransFileModel.h',
])
install_headers(header_files, subdir: 'iptux-core')

core_sources = files([
    'CoreThread.cpp',
    'Event.cpp',
    'Exception.cpp',
    'IptuxConfig.cpp',
    'Models.cpp',
    'ProgramData.cpp',
    'TransFileModel.cpp',
])

core_sources += files([
    'internal/AnalogFS.cpp',
    'internal/Command.cpp',
    'internal/CommandMode.cpp',
    'internal/RecvFile.cpp',
    'internal/RecvFileData.cpp',
    'internal/SendFile.cpp',
    'internal/SendFileData.cpp',
    'internal/support.cpp',
    'internal/TcpData.cpp',
    'internal/TransAbstract.cpp',
    'internal/UdpData.cpp',
])

inc = include_directories('..')

thread_dep = dependency('threads')
libiptux_core = shared_library('iptux-core',
    core_sources,
    dependencies: [glib_dep, jsoncpp_dep, glog_dep, gflags_dep, thread_dep],
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
    requires_private: ['libglog', 'gflags'],
    libraries: [libiptux_core, glog_dep],
)

core_test_helper_sources = files([
    'TestHelper.cpp',
])
libiptux_core_test_helper = static_library('iptux-test-helper',
    core_test_helper_sources,
    dependencies: [glib_dep, jsoncpp_dep, glog_dep],
    link_with: [libiptux_core, libiptux_utils_test_helper],
    include_directories: inc
)

gtest_inc = include_directories('../googletest/include')
core_test_sources = files([
    'CoreThreadTest.cpp',
    'internal/CommandModeTest.cpp',
    'internal/CommandTest.cpp',
    'internal/UdpDataTest.cpp',
    'IptuxConfigTest.cpp',
    'ModelsTest.cpp',
    'ProgramDataTest.cpp',
    'TestMain.cpp',
])
libiptux_core_test = executable('libiptux_core_test',
    core_test_sources,
    dependencies: [glib_dep, jsoncpp_dep, thread_dep],
    link_with: [libiptux_core, libgtest, libiptux_core_test_helper],
    include_directories: [inc, gtest_inc]
)
test('iptux core unit test', libiptux_core_test)


