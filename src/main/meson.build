iptux_sources = files(['iptux.cpp', '../iptux/UiCallbacks.cpp'])
gmodule_dep = dependency('gmodule-export-2.0')
executable('iptux',
    iptux_sources,
    include_directories: inc,
    dependencies: [gtk_dep, jsoncpp_dep, gstreamer_dep, thread_dep, glog_dep, gflags_dep, gmodule_dep],
    link_with: [libiptux],
    cpp_args: '-rdynamic',
    export_dynamic: true,
    install: true,
)
