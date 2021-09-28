gtkmm3_dep = dependency('gtkmm-3.0')

iptux_sources = files(['iptux-gtkmm3.cpp'])
executable('iptux-gtkmm3',
    iptux_sources,
    include_directories: inc,
    dependencies: [gtkmm3_dep],
    link_with: [libiptux_core],
    export_dynamic: true,
    install: true,
)
