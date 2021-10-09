gtk_dep = dependency('gtk+-3.0', version: '>=3.22')
jsoncpp_dep = dependency('jsoncpp', version: '>=1.0')
glog_dep = dependency('libglog')
gflags_dep = dependency('gflags')
sigc_dep = dependency('sigc++-2.0')

sources = files([
    'AboutDialog.cpp',
    'Application.cpp',
    'callback.cpp',
    'DataSettings.cpp',
    'DetectPal.cpp',
    'dialog.cpp',
    'DialogBase.cpp',
    'DialogGroup.cpp',
    'DialogPeer.cpp',
    'EventAdaptor.cpp',
    'GioNotificationService.cpp',
    'GroupInfo.cpp',
    'GroupInfoManager.cpp',
    'LogSystem.cpp',
    'MainWindow.cpp',
    'RevisePal.cpp',
    'ShareFile.cpp',
    'TerminalNotifierNotificationService.cpp',
    'TransWindow.cpp',
    'UiCoreThread.cpp',
    'UiHelper.cpp',
    'UiModels.cpp',
    'UiProgramData.cpp',
    'WindowConfig.cpp',
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

# TODO: How to support C++?
# gnome = import('gnome')
# iptux_resource = gnome.compile_resources(
#     'IptuxResource',
#     'resources/iptux.gresource.xml',
#     source_dir: join_paths(meson.current_source_dir(), 'resources'),
# )

sources += [iptux_resource_cpp, iptux_resource_h]
inc = include_directories('..', '../api')

dependencies = [gtk_dep, jsoncpp_dep, glog_dep, gflags_dep, sigc_dep]

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
    'AboutDialogTest.cpp',
    'ApplicationTest.cpp',
    'DataSettingsTest.cpp',
    'DetectPalTest.cpp',
    'DialogGroupTest.cpp',
    'DialogPeerTest.cpp',
    'LogSystemTest.cpp',
    'MainWindowTest.cpp',
    'RevisePalTest.cpp',
    'ShareFileTest.cpp',
    'TestHelper.cpp',
    'TestMain.cpp',
    'TransWindowTest.cpp',
    'UiCoreThreadTest.cpp',
    'UiHelperTest.cpp',
    'UiModelsTest.cpp',
    'UiProgramDataTest.cpp',
])
libiptux_test = executable('libiptux_test',
    test_sources,
    dependencies: [gtk_dep, jsoncpp_dep, thread_dep, sigc_dep],
    link_with: [libiptux, libgtest,libiptux_core_test_helper],
    include_directories: [inc, gtest_inc]
)
test('iptux ui unit test', libiptux_test, is_parallel : false)
