iptux_sources = files(['iptux.cpp'])
link_args = []
main_deps = [gtk_dep, jsoncpp_dep, thread_dep, sigc_dep]

if host_machine.system() != 'windows'
    iptux_sources += files(['iptux_crash_utils.cpp'])
    link_args += ['-ldl']
else
    ws2_32 = cc.find_library('ws2_32')
    main_deps += [ws2_32]
endif

if host_machine.system() == 'windows'
    rc_files = files([
        'windows.rc',
    ])
    win = import('windows')
    iptux_sources += win.compile_resources(rc_files)
endif


executable('iptux',
    iptux_sources,
    include_directories: inc,
    dependencies: main_deps,
    link_with: [libiptux],
    export_dynamic: true,
    install: true,
    link_args: link_args,
)
