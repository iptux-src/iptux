glib_dep = dependency('glib-2.0', version: '>=2.35')
thread_dep = dependency('threads')

sources = files([
    'output.cpp',
    'utils.cpp',
    'Exception.cpp',
])

inc = include_directories('..', '../api')

libiptux_utils = static_library('iptux-utils',
    sources,
    dependencies: [glib_dep],
    include_directories: inc,
)

utils_test_helper_sources = files([
    'TestHelper.cpp',
])
libiptux_utils_test_helper = static_library('iptux-utils-test-helper',
    utils_test_helper_sources,
    dependencies: [glib_dep],
    link_with: [libiptux_utils],
    include_directories: inc
)

gtest_inc = include_directories('../googletest/include')
utils_test_sources = files([
    'UtilsTest.cpp',
    'TestMain.cpp',
])
libiptux_utils_test = executable('libiptux_utils_test',
    utils_test_sources,
    dependencies: [glib_dep, thread_dep],
    link_with: [libiptux_utils, libgtest, libiptux_utils_test_helper],
    include_directories: [inc, gtest_inc]
)
test('utils', libiptux_utils_test,
  protocol: 'gtest',
  )
