# iptux: LAN communication software

[![Snapcraft](https://snapcraft.io/iptux/badge.svg)](https://snapcraft.io/iptux)
[![CI](https://github.com/iptux-src/iptux/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/iptux-src/iptux/actions/workflows/ci.yml?query=branch%3Amaster)
[![CodeFactor](https://www.codefactor.io/repository/github/iptux-src/iptux/badge)](https://www.codefactor.io/repository/github/iptux-src/iptux)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/0d2720ebbf474c02ac5ebc1036849889)](https://app.codacy.com/gh/iptux-src/iptux/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![codecov](https://codecov.io/gh/iptux-src/iptux/branch/master/graph/badge.svg)](https://codecov.io/gh/iptux-src/iptux/branch/master)
[![CodeQL](https://github.com/iptux-src/iptux/actions/workflows/codeql.yml/badge.svg)](https://github.com/iptux-src/iptux/actions/workflows/codeql.yml)
[![Weblate Translation Status](https://hosted.weblate.org/widgets/iptux/-/iptux/svg-badge.svg)](https://hosted.weblate.org/engage/iptux/)

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
sudo apt-get install git libgoogle-glog-dev libgtk-3-dev libglib2.0-dev libjsoncpp-dev g++ meson libsigc++-2.0-dev libayatana-appindicator3-dev appstream gettext
git clone git://github.com/iptux-src/iptux.git
cd iptux
meson setup build
meson compile -C build # or "ninja -C build" if meson version < 0.54
sudo meson install -C build
iptux
```

### Mac OS X

```sh
brew install meson gettext gtk+3 jsoncpp glog gtk-mac-integration libsigc++@2 appstream
git clone git://github.com/iptux-src/iptux.git
cd iptux
meson setup build
meson install -C build
iptux
```

## Usage

* adjust firewall to allow use the TCP/UDP 2425 port.
* then run `iptux`.

### Compatible list

check https://github.com/iptux-src/iptux/wiki/Compatible-List

## Develop

* use `meson setup -Ddev=true build` to build an iptux which can use resource in source directory.
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
meson setup build
meson compile update-po -C build
```

## Stats

![Alt](https://repobeats.axiom.co/api/embed/8944a2744839c5ea58b0ea10f46a1d31c7fefa07.svg "Repobeats analytics image")
