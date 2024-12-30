glib_dep = dependency('glib-2.0', version: '>=2.35')
jsoncpp_dep = dependency('jsoncpp', version: '>=1.0')
glog_dep = dependency('libglog')
gflags_dep = dependency('gflags')
sigc_dep = dependency('sigc++-2.0')

ccore_sources = files([
])

ccore_sources += files([
])


thread_dep = dependency('threads')

if get_option('static-link')
    libiptux_ccore = static_library('iptux-core',
        ccore_sources,
        dependencies: [glib_dep, jsoncpp_dep, glog_dep, gflags_dep, sigc_dep, thread_dep],
    )
else
    libiptux_ccore = shared_library('iptux-core',
        ccore_sources,
        dependencies: [glib_dep, jsoncpp_dep, glog_dep, gflags_dep, sigc_dep, thread_dep],
        install: true,
        version: so_version,
    )
    pkg = import('pkgconfig')
    pkg.generate(
        description: 'Communicate and Share File in LAN',
        name: 'iptux-core',
        version: so_version,
        requires: [jsoncpp_dep, glib_dep, sigc_dep, thread_dep],
        requires_private: [glog_dep, gflags_dep],
        libraries: [libiptux_ccore, glog_dep],
    )
endif

