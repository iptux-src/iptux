gtk_dep = dependency('gtk+-3.0', version: '>=3.12')
jsoncpp_dep = dependency('jsoncpp', version: '>=1.0')

glib_compile_resources = find_program('glib-compile-resources')

iptux_resource_h = custom_target('iptux_resource_h',
  output : 'IptuxResource.h',
  input : 'resources/iptux.gresource.xml',
  command : [glib_compile_resources,
    '--generate-header',
    '--manual-register',
    '--target=@OUTPUT@',
    '--sourcedir=' + join_paths(meson.current_source_dir(), 'resources'),
    '@INPUT@'],
   install : true,
   install_dir : 'subdir')
iptux_resource_cpp = custom_target('iptux_resource_cpp',
  output : 'IptuxResource.cpp',
  input : 'resources/iptux.gresource.xml',
  command : [glib_compile_resources,
    '--generate-source',
    '--manual-register',
    '--target=@OUTPUT@',
    '--sourcedir=' + join_paths(meson.current_source_dir(), 'resources'),
    '@INPUT@'],
   install : true,
   install_dir : 'subdir')

library('iptux',
    [
        iptux_resource_h,
        iptux_resource_cpp
    ],
    dependencies: [gtk_dep, jsoncpp_dep]
)
