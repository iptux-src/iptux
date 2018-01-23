# 0.7.4 (2018-XX-XX)

* [#?] fix bug in save share management.
* [#97] don't download googletest if already installed.

# 0.7.3 (2018-01-20)

* [#98] fix typo, thanks to @hosiet.
* [#100] fix crash on the context menu of statusicon.

# 0.7.2 (2018-01-16)

* [?] fix crash when clicking on the popup menu of the mainwindow.

* [?] introduce gtest.
* [?] support specify config from command line.
* [#92] clean the icon namespace.
* [?] update po/iptux.pot, update `zh_CN` translation.

# 0.7.1 (2018-01-14)

* [?] fix build guide in `README.md` and homebrew.
* [#80] honor the default `CMAKE_C_FLAGS` and `CMAKE_CXX_FLAGS`, thanks to @hosiet.
* [#81] fix crash on status icon click.

# 0.7.0 (2018-01-10)

* [#33] refactor src/AnalogFS.cpp to make the bug log has more information.
* [#61] switch from autotools to cmake.
* [#67] improve compile under MacOS, thanks to @jiegec.
* [#70] switch config system from gconf to jsoncpp1.
* [#74] fix critial warning on peer window.

# 0.6.4 (2017-08-22)

* [#58] fix compile problem under gcc 7.

# 0.6.3 (2015-09-29)

* [#44] Add "Keywords" entries to iptux.desktop, etc.
* [#43] Remove deprecated "Encoding" in group "Desktop Entry".
* [#45] Upgrade to GStreamer from 0.10 to 1.0.

# 0.6.2 (2014-02-06)

* [#26] iptux --version should work without DISPLAY env.
* [#28] code.google.com -> github.com

# 0.6.1 (2013-12-14)

* [#20] fix compile problem under MacOSX 10.9

# 0.6.0 (2013-06-05)

* [#8] fix autoreconf warning
* [#6] add travis support
* [#4] buildable under Linux
* [#1] buildable under Mac OSX


# OLD RELEASES

* 2012-2-29  把文件接收发送界面放在了聊天窗口内，根据此功能需要底层数据结构作相应改动
* 2011-12-24 改进了如下问题
           1.资源文件不存在是程序异常退出
           2.广播网段过大时，启动时界面不响应
           3.正在文件传输过程中终止文件传输时程序异常退出
           4.在没有任务栏的系统中点主窗口的X按钮后主窗口无法调出
           5.与windows版,adroid版信鸽发送单个文件时对方不能接收
           6.往adroid版信鸽发多个文件时对方不能接收
           7.改进了接收文件夹时的协议，与原版飞鸽一致，接收与发送时加入了文件时间信息(飞鸽类软件都有这一项)，改正了接收文件夹时的一个逻辑错误（这个错误会导致文件已经接收完，但是还不断去tcp连接中读取数据，这样界面上一直会显示没有正常结束）
* 2009-11-20 源码包不能安装，更新

* 2009-11-19 修正已发现的致命错误，并更新源码包

* 2009-11-01 发布最新版iptux-0.5.1源码包

* 2009-10-10 貌似0.5.0这个版本中的问题很多啊，这几天我都在不停的改版本号。

* 2009-10-09 降低iptux对`Gtk+`高版本库的依赖，现在iptux需要的`Gtk+`库的最低版本为2.12.0，如果还低于此版本，那我只能表示很抱歉了。

* 2009-10-08 发布最新版iptux-0.5.0源码包。另，鉴于很多哥们的库版本比较低，所以这一次除了发布一个正规的版本(0.5.0)外，顺便也发布了一个对库文件要求较低的版本(0.5.0-lv)，所以下载的时候请参考自己的实际情形选择下载。

* 2009-09-30 郁闷，有些人加入了项目之后都不做事，这算什么嘛？我决定清理成员了

* 2009-05-12 继续0.4.6版开发，加入聊天窗口中对http://等url的识别和链接功能

* 2009-03-09 鉴于0.4.5版中存在一个严重错误，所以建议尽快更新为0.4.5-1版

* 2009-03-02 发布最新iptux-0.4.5源码包

* 2009-01-17 发布最新iptux-0.4.4源码包

* 2008-12-31 新加入项目成员 pentie@gmail.com ,PT

* 2008-12-24 发布iptux-0.4.3 .rpm包，由网友 liangsuilong 提供

* 2008-12-17 发布iptux-0.4.3 .deb及二进制包，由网友 mdjhu@sina.com 提供

* 2008-12-16 发布最新iptux-0.4.3源码包

* 2008-12-07 发布最新iptux-0.4.2相关包，二进制包由网友 mdjhu@sina.com 提供

* 2008-12-04 新上传三个二进制包，由网友 mdjhu@sina.com 提供
