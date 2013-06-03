# iptux: 飞鸽传书GNU/Linux版

[![Build Status](https://travis-ci.org/iptux-src/iptux.png?branch=master)](https://travis-ci.org/iptux-src/iptux)

# Build

## Linux (Ubuntu)

```
sudo apt-get install git libgtk2.0-dev libgconf2-dev g++ make autoconf libtool automake
git clone git://github.com/iptux-src/iptux.git
autoreconf -i
./configure
make
sudo make install
iptux
```

TODO

## Mac OSX

```
brew install autoconf gettext gtk+ gconf
git clone git://github.com/iptux-src/iptux.git
autoreconf -i
./configure CPPFLAGS="-I/usr/local/opt/gettext/include" PKG_CONFIG_PATH=/opt/X11/lib/pkgconfig
make
sudo make install
iptux
```


## 贡献

* [launchpad](http://translations.launchpad.net/iptux/trunk) ,为iptux贡献翻译。页面由lidaobing(lidaobing@gmail.com)提供；
* 欢迎为iptux(最新版)制作二进制包、提供补丁。

## 声明

请总是使用最新版本！！

* 老版本中出现的bug可能已被修正；
* 许多新特性需要您的试用

## 基本

兼容Win版飞鸽传书、飞秋和android版飞鸽协议，实现局域网的通信，文件传输

## 提高

自定义一部分命令字，实现文件共享功能，群组通信，自动识别编码

## 相关
请查看Wiki标签，那里可能有你需要的内容！

## 必须

* 打开防火墙的 TCP/UDP 2425  端口
* 运行命令: `gconftool-2 --recursive-unset /apps/iptux` (使用时机: 从<=0.4.5升级到>=0.5.0)
* 运行命令: `sudo gtk-update-icon-cache PREFIX/share/icons/hicolor` (使用时机: 图标显示异常，PREFIX为程序安装目录)


## TODO
错误不可避免，请发送错误简介到开发组！https://groups.google.com/group/iptux

同时可以去wiki的[http://code.google.com/p/iptux/wiki/TODO TODO]页面把自己希望的功能和改进留言在那里。
