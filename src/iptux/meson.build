gtk_dep = dependency('gtk+-3.0', version: '>=3.12')
jsoncpp_dep = dependency('jsoncpp', version: '>=1.0')
gstreamer_dep = dependency('gstreamer-1.0', version: '>=1.0')

glib_compile_resources = find_program('glib-compile-resources')

sources = files([
    'AnalogFS.cpp',
    'Application.cpp',
    'Command.cpp',
    'CoreThread.cpp',
    'CoreThreadTest.cpp',
    'DataSettings.cpp',
    'DetectPal.cpp',
    'DialogBase.cpp',
    'DialogGroup.cpp',
    'DialogPeer.cpp',
    'Event.cpp',
    'Exception.cpp',
    'HelpDialog.cpp',
    'IptuxConfig.cpp',
    'IptuxConfigTest.cpp',
    'LogSystem.cpp',
    'LogSystemTest.cpp',
    'MainWindow.cpp',
    'Models.cpp',
    'ModelsTest.cpp',
    'ProgramData.cpp',
    'ProgramDataTest.cpp',
    'RecvFile.cpp',
    'RecvFileData.cpp',
    'RevisePal.cpp',
    'SendFile.cpp',
    'SendFileData.cpp',
    'ShareFile.cpp',
    'SoundSystem.cpp',
    'StatusIcon.cpp',
    'TcpData.cpp',
    'TestHelper.cpp',
    'TestMain.cpp',
    'TransAbstract.cpp',
    'TransFileModel.cpp',
    'TransWindow.cpp',
    'UdpData.cpp',
    'UdpDataTest.cpp',
    'UiCoreThread.cpp',
    'UiCoreThreadTest.cpp',
    'UiHelper.cpp',
    'UiModels.cpp',
    'UiProgramData.cpp',
    'UiProgramDataTest.cpp',
    'UtilsTest.cpp',
    'WindowConfig.cpp',
    'callback.cpp',
    'dialog.cpp',
    'global.cpp',
    'output.cpp',
    'support.cpp',
    'utils.cpp',
    'wrapper.cpp'
])

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

sources += [iptux_resource_h, iptux_resource_cpp]
inc = include_directories('..')

library('iptux',
    sources,
    dependencies: [gtk_dep, jsoncpp_dep, gstreamer_dep],
    include_directories: inc
)


