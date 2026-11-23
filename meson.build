project('iptux', 'cpp',
    license: 'GPL2+',
    version: '0.8.5',
    default_options: ['warning_level=3', 'cpp_std=c++14'])

glog_dep = dependency('libglog')
if glog_dep.version().version_compare('>=0.7.0')
    add_project_arguments('-DGLOG_USE_GLOG_EXPORT', language: 'cpp')
endif

so_version = '0.8.5'
subdir('src')
subdir('share')
subdir('po')
subdir('examples')
