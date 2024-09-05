set -e

rm -rf build

echo "TEST DEBUG BUILD"
mkdir build
cd build && cmake ..
make
./tests/tests

echo "TEST RELEASE BUILD"
cmake -DCMAKE_BUILD_TYPE=Release -DCPPY3_BUILD_EXECUTABLES=On ..
cmake --build .
./tests/tests

echo "TESTS PASSED."
