iptux_sources = files(['iptux.cpp'])

if host_machine.system() != 'windows'
    iptux_sources += files(['iptux_crash_utils.cpp'])
endif

executable('iptux',
    iptux_sources,
    include_directories: inc,
    dependencies: [gtk_dep, jsoncpp_dep, thread_dep, sigc_dep],
    link_with: [libiptux],
    export_dynamic: true,
    install: true,
    link_args: ['-ldl'],
)
