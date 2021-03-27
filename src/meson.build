libgtest_sources = files(['googletest/src/gtest-all.cc'])
libgtest = static_library('gtest',
    libgtest_sources,
    include_directories: include_directories('googletest/include', 'googletest')
    )

gstreamer_dep = dependency('gstreamer-1.0', version: '>=1.0', required:false)

conf_data = configuration_data()
conf_data.set_quoted('VERSION', meson.project_version())
# the 1_ prefix is used for compatible with FeiQ
conf_data.set_quoted('IPTUX_VERSION', '1_iptux '+meson.project_version())
conf_data.set_quoted('GETTEXT_PACKAGE', meson.project_name())
conf_data.set('SHARE_DIR', join_paths(get_option('prefix'), get_option('datadir')))
if get_option('dev')
    conf_data.set('SHARE_IPTUX_DIR', join_paths(meson.source_root(), 'share'))
else
    conf_data.set('SHARE_IPTUX_DIR', join_paths(get_option('prefix'), get_option('datadir'), 'iptux'))
endif

if gstreamer_dep.found()
    conf_data.set('GST_FOUND', true)
endif

if host_machine.system() == 'darwin'
    conf_data.set('SYSTEM_DARWIN', 1)
else
    conf_data.set('SYSTEM_DARWIN', 0)
endif

configure_file(input : 'config.h.in',
               output : 'config.h',
               configuration : conf_data)

test_conf_data = configuration_data()
test_conf_data.set_quoted('PROJECT_ROOT_PATH', meson.source_root())
configure_file(input : 'TestConfig.h.in',
               output : 'TestConfig.h',
               configuration : test_conf_data)

subdir('api')
subdir('iptux-utils')
subdir('iptux-core')
subdir('iptux')
subdir('main')
