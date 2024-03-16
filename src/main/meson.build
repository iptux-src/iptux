iptux_sources = files(['iptux.cpp', 'iptux_crash_utils.cpp'])
executable('iptux',
    iptux_sources,
    include_directories: inc,
    dependencies: [gtk_dep, jsoncpp_dep, thread_dep, glog_dep, gflags_dep, sigc_dep],
    link_with: [libiptux],
    export_dynamic: true,
    install: true,
    link_args: ['-ldl'],
)
