iptux_sources = files(['iptux.cpp'])
executable('iptux',
    iptux_sources,
    include_directories: inc,
    dependencies: [gtk_dep, jsoncpp_dep, gstreamer_dep],
    link_with: [libiptux],
    install: true
)
