glib_dep = dependency('glib-2.0', version: '>=2.35')
jsoncpp_dep = dependency('jsoncpp', version: '>=1.0')
sigc_dep = dependency('sigc++-2.0')

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
    'internal/UdpDataService.cpp',
])

inc = include_directories('..', '../api')

thread_dep = dependency('threads')

if get_option('static-link')
    libiptux_core = static_library('iptux-core',
        core_sources,
        dependencies: [glib_dep, jsoncpp_dep, sigc_dep, thread_dep],
        link_with: [libiptux_utils],
        include_directories: inc,
    )
else
    libiptux_core = shared_library('iptux-core',
        core_sources,
        dependencies: [glib_dep, jsoncpp_dep, sigc_dep, thread_dep],
        link_with: [libiptux_utils],
        include_directories: inc,
        install: true,
        version: meson.project_version(),
        soversion: so_version,
    )
    pkg = import('pkgconfig')
    pkg.generate(
        description: 'Communicate and Share File in LAN',
        name: 'iptux-core',
        version: meson.project_version(),
        requires: [jsoncpp_dep, glib_dep, sigc_dep, thread_dep],
        libraries: [libiptux_core],
    )
endif


core_test_helper_sources = files([
    'TestHelper.cpp',
])
libiptux_core_test_helper = static_library('iptux-test-helper',
    core_test_helper_sources,
    dependencies: [glib_dep, jsoncpp_dep, sigc_dep],
    link_with: [libiptux_core, libiptux_utils_test_helper],
    include_directories: inc
)

gtest_inc = include_directories('../googletest/include')
core_test_sources = files([
    'CoreThreadTest.cpp',
    'internal/CommandModeTest.cpp',
    'internal/CommandTest.cpp',
    'internal/supportTest.cpp',
    'internal/UdpDataTest.cpp',
    'internal/UdpDataServiceTest.cpp',
    'IptuxConfigTest.cpp',
    'ModelsTest.cpp',
    'ProgramDataTest.cpp',
    'TestMain.cpp',
])
libiptux_core_test = executable('libiptux_core_test',
    core_test_sources,
    dependencies: [glib_dep, jsoncpp_dep, sigc_dep, thread_dep],
    link_with: [libiptux_core, libgtest, libiptux_core_test_helper],
    include_directories: [inc, gtest_inc]
)
if meson.version().version_compare('>=0.55')
  test('core', libiptux_core_test, is_parallel : false, protocol: 'gtest')
else
  test('core', libiptux_core_test, is_parallel : false)
endif
