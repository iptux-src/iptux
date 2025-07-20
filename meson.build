project('iptux', 'cpp',
    license: 'GPL2+',
    version: '0.9.4',
    meson_version: '>=0.53',
    default_options: ['warning_level=2', 'cpp_std=c++17'])

# Ubuntu 22.04
add_project_arguments('-DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_70', language : 'cpp')


add_project_arguments('-Werror', language : 'cpp')
add_project_arguments('-Wno-cast-function-type-mismatch', '-Wno-cast-function-type', language : 'cpp')

so_version = 1
subdir('src')
subdir('share')
subdir('po')
subdir('examples')
subdir('scripts')

if meson.version().version_compare('>=0.57')
  summary({
    'prefix': get_option('prefix'),
    'bindir': get_option('bindir'),
    'libdir': get_option('libdir'),
    'data': get_option('datadir'),
    }, section: 'Directories:')

  summary_deps = {
    'meson': meson.version(),
    'glib-2.0': glib_dep,
    'gtk+-3.0': gtk_dep,
    'jsoncpp': jsoncpp_dep,
    'sigc++-2.0': sigc_dep,
    }

  if host_machine.system() == 'darwin'
    summary_deps += {'gtk-mac-integration-gtk3': gtk_mac_integration_dep}
  endif

  if host_machine.system() == 'linux'
    summary_deps += {'ayatana-appindicator3-0.1': appindicator_dep}
  endif

  summary_deps += {
    'glib-compile-resources': glib_compile_resources,
    'appstreamcli': ascli_exe,
  }

  summary(summary_deps, section: 'Dependencies:')

  summary({
    'dev': get_option('dev'),
    'static-link': get_option('static-link'),
    'appindicator': appindicator_dep.found(),
    }, section: 'Options:')
endif
