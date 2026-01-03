set -e
rm -rf build

trap 'echo "Exit code: $?"' EXIT

echo "TEST DEBUG BUILD"
mkdir build
cd build && cmake ..
make
./tests/tests

echo "TEST RELEASE BUILD"
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
./tests/tests

echo "=== TESTS PASSED ==="
