#!/bin/bash

set -ex

env
export CODECOV_TOKEN=cec2d3eb-e3d2-414c-ae88-137cc880e0e1
env

cd $(dirname $0)/..
if [ "$TARGET" = "travis-linux-codecov" ]; then
    meson -D b_coverage=true builddir
else
    meson builddir
fi
ninja -v -C builddir
xvfb-run ninja -v -C builddir test
ninja -v -C builddir install
if [ "$TARGET" = "travis-linux-codecov" ]; then
    lcov --directory . --capture --output-file coverage.info; # capture coverage info
    lcov --remove coverage.info '/usr/*' --output-file coverage.info; # filter out system
    lcov --remove coverage.info '*Test*' --output-file coverage.info; # filter out system
    lcov --remove coverage.info '*gtest*' --output-file coverage.info; # filter out system
    lcov --list coverage.info; #debug info
    # Uploading report to CodeCov
    bash <(curl -s https://codecov.io/bash) || echo "Codecov did not collect coverage reports"
fi
