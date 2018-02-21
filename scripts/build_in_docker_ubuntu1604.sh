#!/bin/bash

set -ex

env

export CODECOV_TOKEN=cec2d3eb-e3d2-414c-ae88-137cc880e0e1

apt-get update
apt-get install -y lcov git xvfb

cd `dirname $0`/..
mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make VERBOSE=1
make iptux_coverage VERBOSE=1
xvfb-run ctest --verbose
make install
lcov --directory . --capture --output-file coverage.info # capture coverage info
lcov --remove coverage.info '/usr/*' --output-file coverage.info # filter out system
lcov --remove coverage.info '*Test*' --output-file coverage.info # filter out system
lcov --remove coverage.info '*gtest*' --output-file coverage.info # filter out system
lcov --list coverage.info #debug info
# Uploading report to CodeCov
bash <(curl -s https://codecov.io/bash) || echo "Codecov did not collect coverage reports"
