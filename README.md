# iptux: LAN communication software

[![Snapcraft](https://snapcraft.io/iptux/badge.svg)](https://snapcraft.io/iptux)
[![CI](https://github.com/iptux-src/iptux/workflows/CI/badge.svg)](https://github.com/iptux-src/iptux/actions)
[![CodeFactor](https://www.codefactor.io/repository/github/iptux-src/iptux/badge)](https://www.codefactor.io/repository/github/iptux-src/iptux)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/d0340710e474453aa5d4c6943cadeb80)](https://app.codacy.com/app/lidaobing/iptux?utm_source=github.com&utm_medium=referral&utm_content=iptux-src/iptux&utm_campaign=badger)
[![codecov](https://codecov.io/gh/iptux-src/iptux/branch/master/graph/badge.svg)](https://codecov.io/gh/iptux-src/iptux/branch/master)
[![Total alerts](https://img.shields.io/lgtm/alerts/g/iptux-src/iptux.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/iptux-src/iptux/alerts/)
[![Weblate Translation Status](https://hosted.weblate.org/widgets/iptux/-/iptux/svg-badge.svg)](https://hosted.weblate.org/engage/iptux/)

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [Install](#install)
  - [Linux](#linux)
  - [Mac OS X](#mac-os-x)
- [Build from source](#build-from-source)
  - [Linux (Debian and Ubuntu)](#linux-debian-and-ubuntu)
  - [Mac OS X](#mac-os-x-1)
- [Usage](#usage)
  - [Compatible list](#compatible-list)
- [Develop](#develop)
- [Contributing](#contributing)
  - [How to update `po/iptux.pot`](#how-to-update-poiptuxpot)
- [Stargazers over time](#stargazers-over-time)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Install

### Linux

[![Get it from the Snap Store](https://snapcraft.io/static/images/badges/en/snap-store-white.svg)](https://snapcraft.io/iptux)

* for Ubuntu 14.04, please check 0.6.x branch: https://github.com/iptux-src/iptux/tree/iptux-0-6
* for Ubuntu 16.04, please check 0.7.x branch: https://github.com/iptux-src/iptux/tree/iptux-0-7

### Mac OS X

```
brew tap iptux-src/iptux
brew install iptux
```

## Build from source

### Linux (Debian and Ubuntu)

```sh
sudo apt-get install git libgoogle-glog-dev libgtk-3-dev libglib2.0-dev libjsoncpp-dev g++ meson libsigc++-2.0-dev
git clone git://github.com/iptux-src/iptux.git
cd iptux
meson builddir && ninja -C builddir
sudo ninja -C builddir install
iptux
```

### Mac OS X

```sh
brew install meson gettext gtk+3 jsoncpp glog gtk-mac-integration libsigc++@2
git clone git://github.com/iptux-src/iptux.git
cd iptux
meson builddir
ninja -C builddir install
iptux
```

## Usage

* adjust firewall to allow use the TCP/UDP 2425 port.
* then run `iptux`.

### Compatible list

check https://github.com/iptux-src/iptux/wiki/Compatible-List

## Develop

* use `meson -Ddev=true builddir` to build an iptux which can use resource in source directory.
* start 2 iptux on one machine for test
  * It's a known bug that you can not send file between 127.0.0.2 and 127.0.0.3
```sh
iptux -b 127.0.0.2 &
iptux -b 127.0.0.3 &
```


## Contributing

* Help improve [translation](https://hosted.weblate.org/projects/iptux/#languages), we are using weblate for translation
* Test the [compatibility](https://github.com/iptux-src/iptux/wiki/Compatible-List),
* Fix [bugs](https://github.com/iptux-src/iptux/issues).

### How to update `po/iptux.pot`

```
meson builddir
find . \( -name '*.cpp' -or -name '*.desktop.in' -or -name '*.ui' \) | grep -v Test | grep -v builddir | sort > po/POTFILES
ninja -C builddir iptux-pot
ninja -C builddir iptux-update-po
for f in  po/*.po; do echo -n "$f: "; msgfmt -v $f; done
```

## Stargazers over time

[![Stargazers over time](https://starcharts.herokuapp.com/iptux-src/iptux.svg)](https://starcharts.herokuapp.com/iptux-src/iptux)
