bot_sources = files(['main.cpp'])
executable('count-bot',
    bot_sources,
    include_directories: inc,
    dependencies: [jsoncpp_dep, thread_dep, glog_dep, glib_dep],
    link_with: [libiptux_core],
    install: false
)
