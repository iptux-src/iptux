bot_sources = files(['main.cpp'])
executable('count-bot',
    bot_sources,
    include_directories: include_directories('../../src/api'),
    dependencies: [jsoncpp_dep, thread_dep, glog_dep, gflags_dep, glib_dep],
    link_with: [libiptux_core],
    install: false
)
