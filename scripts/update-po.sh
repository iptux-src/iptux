#!/bin/bash

set -e
cd "${MESON_SOURCE_ROOT}" || exit 1
git ls-files '*.cpp' '*.ui' '*.metainfo.xml' \
  | grep -v Test \
  | LC_ALL=C sort \
  > po/POTFILES
ninja -C "${MESON_BUILD_ROOT}" iptux-pot
ninja -C "${MESON_BUILD_ROOT}" iptux-update-po
for f in po/*.po; do
  echo -n "$f: "
  msgfmt -v "$f"
done
