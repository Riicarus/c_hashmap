rm -rf build/
cmake -B build -DNEED_TEST=ON
cmake --build build
build/map_test