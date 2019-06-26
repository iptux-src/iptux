glib_dep = dependency('glib-2.0', version: '>=2.35')

sources = files([
    'utils.cpp',
])

inc = include_directories('..')

libiptux_utils = static_library('iptux-utils',
    sources,
    dependencies: [glib_dep],
    include_directories: inc,
)

