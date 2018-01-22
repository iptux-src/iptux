<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [iptux: 飞鸽传书 GNU/Linux 版](#iptux-%E9%A3%9E%E9%B8%BD%E4%BC%A0%E4%B9%A6-gnulinux-%E7%89%88)
  - [Install](#install)
    - [Linux (Debian and Ubuntu)](#linux-debian-and-ubuntu)
    - [Mac OS X](#mac-os-x)
  - [Build from source](#build-from-source)
    - [Linux (Debian and Ubuntu)](#linux-debian-and-ubuntu-1)
    - [Mac OS X](#mac-os-x-1)
  - [贡献](#%E8%B4%A1%E7%8C%AE)
  - [声明](#%E5%A3%B0%E6%98%8E)
  - [基本](#%E5%9F%BA%E6%9C%AC)
  - [提高](#%E6%8F%90%E9%AB%98)
  - [相关](#%E7%9B%B8%E5%85%B3)
  - [必须](#%E5%BF%85%E9%A1%BB)
  - [TODO](#todo)
  - [How to update `po/iptux.pot`](#how-to-update-poiptuxpot)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# iptux: 飞鸽传书 GNU/Linux 版

[![Build Status](https://travis-ci.org/iptux-src/iptux.png?branch=master)](https://travis-ci.org/iptux-src/iptux)
[![GitHub version](https://badge.fury.io/gh/iptux-src%2Fiptux.png)](http://badge.fury.io/gh/iptux-src%2Fiptux)

* If anyone want to adopt this software, please [file an issue](https://github.com/iptux-src/iptux/issues/new)<br>
  at https://github.com/iptux-src/iptux/issues/new


## Install

### Linux (Debian and Ubuntu)

```
sudo apt-get install iptux
```

### Mac OS X

```
brew install https://raw.githubusercontent.com/iptux-src/iptux/master/homebrew/iptux.rb
```


## Build from source

### Linux (Debian and Ubuntu)

```
sudo apt-get install git libgtk2.0-dev libglib2.0-dev libgstreamer1.0-dev libjsoncpp-dev g++ make cmake
git clone git://github.com/iptux-src/iptux.git
cd iptux
mkdir build && cd build && cmake .. && make
sudo make install
iptux
```

### Mac OS X

```
brew install gettext gtk+ cmake jsoncpp gstreamer
git clone git://github.com/iptux-src/iptux.git
cd iptux
mkdir build && cd build && cmake .. && make
sudo make install
iptux
```


## 贡献

* [Launchpad](http://translations.launchpad.net/iptux/trunk) 为 iptux 贡献翻译。页面由 LI Daobing &lt;lidaobing@gmail.com&gt; 提供；
* 欢迎为 iptux (最新版) 制作二进制包、提供补丁。

## 声明

请总是使用最新版本！！

* 老版本中出现的 bug 可能已被修正；
* 许多新特性需要您的试用。

## 基本

兼容 Windows 版[飞鸽传书](http://www.ipmsg.org.cn/)、[飞秋](http://www.feiq18.com/)和 Android 版飞鸽协议，也兼容日本 SHIROUZU Hiroaki (白水啓章) 先生原著的 [IP Messenger](http://ipmsg.org/) 实现局域网的通信，文件传输。

## 提高

自定义一部分命令字，实现文件共享功能，群组通信，自动识别编码

## 相关
请查看 Wiki 标签，那里可能有你需要的内容！

## 必须

* 打开防火墙的 TCP/UDP 2425 端口
* 运行命令: `sudo gtk-update-icon-cache PREFIX/share/icons/hicolor` (使用时机: 图标显示异常，PREFIX 为程序安装目录)


## TODO
错误不可避免，请发送错误报告到 https://github.com/iptux-src/iptux/issues

## How to update `po/iptux.pot`

```
xgettext \
  --output=po/iptux.pot \
  --files-from=po/POTFILES.in \
  --language=C++ \
  --keyword=_ \
  --from-code=utf-8 \
  --package-name=iptux \
  --msgid-bugs-address=https://github.com/iptux-src/iptux/issues/new \
  --package-version=0.7.3
```
