#!/bin/bash

set -ex

apt-get install -y lcov

cd `dirname $0`/..
mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make
make iptux_coverage
ctest --verbose
make install
lcov --directory . --capture --output-file coverage.info # capture coverage info
lcov --remove coverage.info '/usr/*' --output-file coverage.info # filter out system
lcov --remove coverage.info '*Test*' --output-file coverage.info # filter out system
lcov --remove coverage.info '*gtest*' --output-file coverage.info # filter out system
lcov --list coverage.info #debug info
# Uploading report to CodeCov
bash <(curl -s https://codecov.io/bash) || echo "Codecov did not collect coverage reports"
