all:

cppcheck:
	cppcheck -D__cplusplus -I src `pkg-config --cflags-only-I gtk+-3.0 gstreamer-1.0 jsoncpp` --enable=all --error-exitcode=1 --inconclusive ./src/iptux/ ./src/main/
fast-cppcheck:
	cppcheck -q -I src --enable=all --error-exitcode=1 --inconclusive ./src/iptux/ ./src/main/
docker:
	docker build -f iptux-travis-gcc.Dockerfile -t lidaobing/iptux-travis-gcc:0.0.3 .
travis-linux:
	echo $$TARGET
	echo $$CODECOV_TOKEN
	docker run -t -v `pwd`:/ws lidaobing/iptux-travis-gcc:0.0.3 \
    env TARGET=$$TARGET CODECOV_TOKEN=$$CODECOV_TOKEN /ws/scripts/build_in_docker_ubuntu1804.sh
travis-linux-codecov:
	echo $$TARGET
	echo $$CODECOV_TOKEN
	docker run -t -v `pwd`:/ws lidaobing/iptux-travis-gcc:0.0.3 \
    env TARGET=$$TARGET CODECOV_TOKEN=$$CODECOV_TOKEN /ws/scripts/build_in_docker_ubuntu1804.sh
