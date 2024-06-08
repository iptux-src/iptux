project('iptux', 'cpp',
    license: 'GPL2+',
    version: '0.9.1',
    default_options: ['warning_level=3', 'cpp_std=c++14'])
add_global_arguments('-Werror=format', language : 'cpp')
if get_option('sanitize-address')
  add_project_arguments('-fsanitize=address', language: 'cpp')
  add_project_link_arguments('-fsanitize=address', language: 'cpp')
endif

so_version = '0.9.1'
subdir('src')
subdir('share')
subdir('po')
subdir('examples')
subdir('scripts')
