rm -rf build/
cmake -B build -DNEED_TEST=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=1
cmake --build build
build/map_test