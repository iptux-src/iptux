#!/bin/sh

set -ex

cd `dirname $0`/..
mkdir build && cd build && cmake .. && make && ctest --verbose
make install
