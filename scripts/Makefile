all:

cppcheck:
	cppcheck -D__cplusplus -I src `pkg-config --cflags-only-I gtk+-3.0 gstreamer-1.0 jsoncpp` --enable=all --error-exitcode=1 --inconclusive ./src/iptux/ ./src/main/
fast-cppcheck:
	cppcheck -q -I src --enable=all --error-exitcode=1 --inconclusive ./src/iptux/ ./src/main/
