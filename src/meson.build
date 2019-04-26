subdir('iptux')
subdir('main')

libgtest_sources = files(['googletest/src/gtest-all.cc'])
libgtest = static_library('gtest',
    libgtest_sources,
    include_directories: include_directories('googletest/include', 'googletest')
    )
