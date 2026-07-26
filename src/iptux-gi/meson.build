gnome = import('gnome')

gobject_dep = dependency('gobject-2.0')
sigc_dep = dependency('sigc++-2.0')
dependencies = [gobject_dep, sigc_dep]

libsources = files('iptux-service.cpp')
libiptux_gi = shared_library('iptux-gi',
  sources: libsources,
  dependencies: dependencies,
  install: true,
  include_directories: include_directories('..', '../api'),
)

gir = gnome.generate_gir(libiptux_gi,
  sources: ['iptux-service.h', 'iptux-service.cpp'],
  nsversion: '1.0',
  namespace: 'Iptux',
  symbol_prefix: 'iptux',
  identifier_prefix: 'Iptux',
  includes: ['GObject-2.0'],
  install: true
)
