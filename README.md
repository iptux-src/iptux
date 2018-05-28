# iptux: 飞鸽传书 GNU/Linux 版

[![CodeFactor](https://www.codefactor.io/repository/github/iptux-src/iptux/badge)](https://www.codefactor.io/repository/github/iptux-src/iptux)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/d0340710e474453aa5d4c6943cadeb80)](https://app.codacy.com/app/lidaobing/iptux?utm_source=github.com&utm_medium=referral&utm_content=iptux-src/iptux&utm_campaign=badger)
[![Build Status](https://travis-ci.org/iptux-src/iptux.svg?branch=master)](https://travis-ci.org/iptux-src/iptux)
[![GitHub version](https://badge.fury.io/gh/iptux-src%2Fiptux.svg)](http://badge.fury.io/gh/iptux-src%2Fiptux)
[![Join the chat at https://gitter.im/iptux-src/Lobby](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/iptux-src/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![codecov](https://codecov.io/gh/iptux-src/iptux/branch/master/graph/badge.svg)](https://codecov.io/gh/iptux-src/iptux/branch/master)

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
  - [Usage](#usage)
    - [Compatible list](#compatible-list)
  - [Contributing](#contributing)
    - [How to update `po/iptux.pot`](#how-to-update-poiptuxpot)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Install

### Linux (Debian and Ubuntu)

```
sudo apt-get install iptux
```

### Mac OS X

stable version without gstreamer:

```sh
brew unlink gstreamer # check #211
brew install https://raw.githubusercontent.com/iptux-src/iptux/master/homebrew/iptux.rb
```

stable version with gstream: # much slower

```sh
brew install https://raw.githubusercontent.com/iptux-src/iptux/master/homebrew/iptux.rb --with-gstreamer
```

head version without gstreamer:

```sh
brew unlink gstreamer # check #211
brew install --HEAD https://raw.githubusercontent.com/iptux-src/iptux/master/homebrew/iptux.rb
```

head version with gstreamer: # much slower

```sh
brew install --HEAD https://raw.githubusercontent.com/iptux-src/iptux/master/homebrew/iptux.rb --with-gstreamer
```


## Build from source

### Linux (Debian and Ubuntu)

* for Ubuntu 14.04, please download from https://github.com/iptux-src/iptux/releases/tag/v0.6.4

```sh
sudo apt-get install git libgtk-3-dev libglib2.0-dev libjsoncpp-dev g++ make cmake
# if you need the sound support
sudo apt-get install libgstreamer1.0-dev gstreamer1.0-plugins-good gstreamer1.0-alsa 
# endif
git clone git://github.com/iptux-src/iptux.git
cd iptux
mkdir build && cd build && cmake .. && make
sudo make install
iptux
```

### Mac OS X

```sh
brew install gettext gtk+3 cmake jsoncpp
# if you need the sound support
brew install gstreamer
brew install gst-plugins-base --with-libogg --with-libvorbis
brew install gst-plugins-good
# endif
git clone git://github.com/iptux-src/iptux.git
cd iptux
mkdir build && cd build && cmake .. && make
sudo make install
iptux
```

## Usage

* adjust firewall to allow use the TCP/UDP 2425 port.
* then run `iptux`.

### Compatible list

check https://github.com/iptux-src/iptux/wiki/Compatible-List

## Contributing

You can help improve [translation](http://translations.launchpad.net/iptux/trunk), test the [compatibility](https://github.com/iptux-src/iptux/wiki/Compatible-List), fix [bugs](https://github.com/iptux-src/iptux/issues).

### How to update `po/iptux.pot`

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
