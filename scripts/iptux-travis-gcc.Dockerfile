# Version: 0.0.5
FROM ubuntu:18.04
LABEL maintainer="LI Daobing <lidaobing@gmail.com>"
RUN echo 'deb http://mirrors.163.com/ubuntu bionic main restricted universe multiverse' > /etc/apt/sources.list
RUN echo 'deb http://mirrors.163.com/ubuntu bionic-security main restricted universe multiverse' >> /etc/apt/sources.list
RUN echo 'deb http://mirrors.163.com/ubuntu bionic-updates main restricted universe multiverse' >> /etc/apt/sources.list
ENV REFRESHED_AT 2019-05-30
RUN apt-get update
RUN apt-get install -y libgoogle-glog-dev libgtk-3-dev libglib2.0-dev libgstreamer1.0-dev libjsoncpp-dev g++ make meson lcov git xvfb
RUN apt-get install -y clang
