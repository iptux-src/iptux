libgtest_sources = files('googletest/src/gtest-all.cc')
libgtest = static_library(
  'gtest',
  libgtest_sources,
  include_directories: include_directories('googletest/include', 'googletest'),
)

conf_data = configuration_data()
conf_data.set_quoted('VERSION', meson.project_version())
# the 1_ prefix is used for compatible with FeiQ
conf_data.set_quoted('IPTUX_VERSION', '1_iptux ' + meson.project_version())
conf_data.set_quoted('GETTEXT_PACKAGE', meson.project_name())
conf_data.set(
  'SHARE_DIR',
  join_paths(get_option('prefix'), get_option('datadir')),
)
if get_option('dev')
  conf_data.set(
    'SHARE_IPTUX_DIR',
    join_paths(meson.current_source_dir(), '../share'),
  )
else
  conf_data.set(
    'SHARE_IPTUX_DIR',
    join_paths(get_option('prefix'), get_option('datadir'), 'iptux'),
  )
endif

if host_machine.system() == 'darwin'
  conf_data.set('SYSTEM_DARWIN', 1)
else
  conf_data.set('SYSTEM_DARWIN', 0)
endif

appindicator_dep = dependency('ayatana-appindicator3-0.1', required: get_option('appindicator'))
if appindicator_dep.found()
  conf_data.set('HAVE_APPINDICATOR', 1)
else
  conf_data.set('HAVE_APPINDICATOR', 0)
endif

configure_file(
  input: 'config.h.in',
  output: 'config.h',
  configuration: conf_data,
)

test_conf_data = configuration_data()
test_conf_data.set_quoted('CURRENT_SOURCE_PATH', meson.current_source_dir())
configure_file(
  input: 'TestConfig.h.in',
  output: 'TestConfig.h',
  configuration: test_conf_data,
)

subdir('api')
subdir('ccore')
subdir('iptux-utils')
subdir('iptux-core')
subdir('iptux')
subdir('main')
