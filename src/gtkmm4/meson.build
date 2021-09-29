gtkmm4_dep = dependency('gtkmm-4.0')

iptux_sources = files([
  'iptux-gtkmm4.cpp',
  'Iptux.cpp'
  ])
executable('iptux-gtkmm4',
    iptux_sources,
    include_directories: inc,
    dependencies: [gtkmm4_dep],
    link_with: [libiptux_core],
    export_dynamic: true,
    install: true,
)
