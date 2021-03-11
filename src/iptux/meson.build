gtk_dep = dependency('gtk+-3.0', version: '>=3.22')
jsoncpp_dep = dependency('jsoncpp', version: '>=1.0')
gstreamer_dep = dependency('gstreamer-1.0', version: '>=1.0', required:false)
glog_dep = dependency('libglog')
gflags_dep = dependency('gflags')

sources = files([
    'Application.cpp',
    'DataSettings.cpp',
    'DetectPal.cpp',
    'DialogBase.cpp',
    'DialogGroup.cpp',
    'DialogPeer.cpp',
    'HelpDialog.cpp',
    'LogSystem.cpp',
    'MainWindow.cpp',
    'RevisePal.cpp',
    'ShareFile.cpp',
    'SoundSystem.cpp',
    'StatusIcon.cpp',
    'TransWindow.cpp',
    'UiCoreThread.cpp',
    'UiHelper.cpp',
    'UiModels.cpp',
    'UiProgramData.cpp',
    'WindowConfig.cpp',
    'callback.cpp',
    'dialog.cpp',
    'global.cpp',
])

glib_compile_resources = find_program('glib-compile-resources')

iptux_resource_h = custom_target('iptux_resource_h',
  output : 'IptuxResource.h',
  input : 'resources/iptux.gresource.xml',
  depend_files: 'resources/gtk/menus.ui',
  command : [glib_compile_resources,
    '--generate-header',
    '--manual-register',
    '--target=@OUTPUT@',
    '--sourcedir=' + join_paths(meson.current_source_dir(), 'resources'),
    '@INPUT@'])
iptux_resource_cpp = custom_target('iptux_resource_cpp',
  output : 'IptuxResource.cpp',
  input : 'resources/iptux.gresource.xml',
  depend_files: 'resources/gtk/menus.ui',
  command : [glib_compile_resources,
    '--generate-source',
    '--manual-register',
    '--target=@OUTPUT@',
    '--sourcedir=' + join_paths(meson.current_source_dir(), 'resources'),
    '@INPUT@'])

# TODO: How to support C++?
# gnome = import('gnome')
# iptux_resource = gnome.compile_resources(
#     'IptuxResource',
#     'resources/iptux.gresource.xml',
#     source_dir: join_paths(meson.current_source_dir(), 'resources'),
# )

sources += [iptux_resource_cpp, iptux_resource_h]
inc = include_directories('..')

dependencies = [gtk_dep, jsoncpp_dep, gstreamer_dep, glog_dep, gflags_dep]

if host_machine.system() == 'darwin'
    gtk_mac_integration_dep = dependency('gtk-mac-integration-gtk3')
    dependencies += [gtk_mac_integration_dep]
    sources += ['Darwin.cpp']
endif

libiptux = static_library('iptux',
    sources,
    dependencies: dependencies,
    link_with: [libiptux_core],
    include_directories: inc,
)

gtest_inc = include_directories('../googletest/include')
thread_dep = dependency('threads')
test_sources = files([
    'ApplicationTest.cpp',
    'DataSettingsTest.cpp',
    'DetectPalTest.cpp',
    'DialogGroupTest.cpp',
    'DialogPeerTest.cpp',
    'HelpDialogTest.cpp',
    'LogSystemTest.cpp',
    'TestHelper.cpp',
    'TestMain.cpp',
    'TransWindowTest.cpp',
    'UiCoreThreadTest.cpp',
    'UiProgramDataTest.cpp',
])
libiptux_test = executable('libiptux_test',
    test_sources,
    dependencies: [gtk_dep, jsoncpp_dep, gstreamer_dep, thread_dep],
    link_with: [libiptux, libgtest,libiptux_core_test_helper],
    include_directories: [inc, gtest_inc]
)
test('iptux ui unit test', libiptux_test)


