gnome = import('gnome')

gobject_dep = dependency('gobject-2.0')
sigc_dep = dependency('sigc++-2.0')
jsoncpp_dep = dependency('jsoncpp', version: '>=1.0')
dependencies = [gobject_dep, sigc_dep, jsoncpp_dep]

libsources = files('iptux-service.cpp', 'iptux-config.cpp', 'iptux-pal.cpp')
libiptux_gi = shared_library('iptux-gi',
  sources: libsources,
  dependencies: dependencies,
  install: true,
  include_directories: include_directories('..', '../api'),
  link_with: [libiptux_core],
)

version_parts = meson.project_version().split('.')
nsversion = version_parts[0] + '.' + version_parts[1]

gir = gnome.generate_gir(libiptux_gi,
  sources: ['iptux-service.h', 'iptux-service.cpp', 'iptux-config.h', 'iptux-config.cpp', 'iptux-pal.h', 'iptux-pal.cpp'],
  nsversion: nsversion,
  namespace: 'Iptux',
  symbol_prefix: 'iptux',
  identifier_prefix: 'Iptux',
  includes: ['GObject-2.0', 'Gio-2.0'],
  install: true
)
