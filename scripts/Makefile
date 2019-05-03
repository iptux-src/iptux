all:

cppcheck:
	cppcheck -D__cplusplus -I src `pkg-config --cflags-only-I gtk+-3.0 gstreamer-1.0 jsoncpp` --enable=all --error-exitcode=1 --inconclusive ./src/iptux/ ./src/main/
fast-cppcheck:
	cppcheck -q -I src --enable=all --error-exitcode=1 --inconclusive ./src/iptux/ ./src/main/
docker:
	docker build -f iptux-travis-gcc.Dockerfile -t lidaobing/iptux-travis-gcc:0.0.3 .
travis-linux:
	docker run -t -v `pwd`:/ws lidaobing/iptux-travis-gcc:0.0.3 /ws/scripts/build_in_docker_ubuntu1804.sh
travis-linux-codecov:
	docker run -t -v `pwd`:/ws lidaobing/iptux-travis-gcc:0.0.3 /ws/scripts/build_in_docker_ubuntu1804.sh
