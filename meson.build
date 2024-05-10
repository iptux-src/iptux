project('iptux', 'cpp',
    license: 'GPL2+',
    version: '0.9.0',
    default_options: ['warning_level=3', 'cpp_std=c++14'])
add_global_arguments('-Werror=format', language : 'cpp')
add_project_arguments(
  '-DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_36',
  language: 'cpp')
so_version = '0.9.0'
subdir('src')
subdir('share')
subdir('po')
subdir('examples')
subdir('scripts')
