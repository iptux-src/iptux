# Version: 0.0.1
FROM ubuntu:16.04
MAINTAINER LI Daobing <lidaobing@gmail.com>
RUN echo 'deb http://mirrors.163.com/ubuntu xenial main restricted universe multiverse' > /etc/apt/sources.list
RUN echo 'deb http://mirrors.163.com/ubuntu xenial-security main restricted universe multiverse' >> /etc/apt/sources.list
RUN echo 'deb http://mirrors.163.com/ubuntu xenial-updates main restricted universe multiverse' >> /etc/apt/sources.list
ENV REFRESHED_AT 2018-02-04
RUN apt-get update
RUN apt-get install -y libgtk-3-dev libglib2.0-dev libgstreamer1.0-dev libjsoncpp-dev g++ make cmake
