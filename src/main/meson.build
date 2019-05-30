glog_dep = dependency('libglog')

iptux_sources = files(['iptux.cpp'])
executable('iptux',
    iptux_sources,
    include_directories: inc,
    # dependencies: [libiptux],
    dependencies: [gtk_dep, jsoncpp_dep, gstreamer_dep, thread_dep, glog_dep],
    link_with: [libiptux],
    install: true
)
