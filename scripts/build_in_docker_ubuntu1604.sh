#!/bin/sh

set -ex

cd `dirname $0`/..
apt-get update
apt-get install -y libgtk-3-dev libglib2.0-dev libgstreamer1.0-dev libjsoncpp-dev g++ make cmake
mkdir build && cd build && cmake .. && make && make test
make install
