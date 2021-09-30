gtkmm4_dep = dependency('gtkmm-4.0')

iptux_sources = files([
  'iptux-gtkmm4.cpp',
  'Iptux.cpp',
  'MainWindow.cpp',
  'UiHelper.cpp',
  ])

glib_compile_resources = find_program('glib-compile-resources')

ui_files = [
    'resources/gtk/AboutDialog.ui',
    'resources/gtk/MainWindow.ui',
    'resources/gtk/DetectPal.ui',
    'resources/gtk/menus.ui',
]

iptux_resource_h = custom_target('iptux_resource_h',
  output : 'IptuxResource.h',
  input : 'resources/iptux.gresource.xml',
  depend_files: ui_files,
  command : [glib_compile_resources,
    '--generate-header',
    '--manual-register',
    '--target=@OUTPUT@',
    '--sourcedir=' + join_paths(meson.current_source_dir(), 'resources'),
    '@INPUT@'])
iptux_resource_cpp = custom_target('iptux_resource_cpp',
  output : 'IptuxResource.cpp',
  input : 'resources/iptux.gresource.xml',
  depend_files: ui_files,
  command : [glib_compile_resources,
    '--generate-source',
    '--manual-register',
    '--target=@OUTPUT@',
    '--sourcedir=' + join_paths(meson.current_source_dir(), 'resources'),
    '@INPUT@'])
iptux_sources += [iptux_resource_cpp, iptux_resource_h]

executable('iptux-gtkmm4',
    iptux_sources,
    include_directories: inc,
    dependencies: [gtkmm4_dep],
    link_with: [libiptux_core],
    export_dynamic: true,
    install: true,
)


