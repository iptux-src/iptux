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

# add_custom_command(OUTPUT IptuxResource.h
# 		MAIN_DEPENDENCY resources/iptux.gresource.xml
# 		COMMAND glib-compile-resources --generate-header --manual-register --target=IptuxResource.h
# 			--sourcedir=${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_CURRENT_SOURCE_DIR}/resources/iptux.gresource.xml
# 		)

# add_custom_command(OUTPUT IptuxResource.cpp
# 		MAIN_DEPENDENCY resources/iptux.gresource.xml
# 		COMMAND glib-compile-resources --generate-source --manual-register --target=IptuxResource.cpp
# 			--sourcedir=${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_CURRENT_SOURCE_DIR}/resources/iptux.gresource.xml
# 		)
