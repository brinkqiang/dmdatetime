
rmdir /S /Q build
mkdir build
pushd build
cmake -A x64 -DCMAKE_BUILD_TYPE=debug ..
cmake --build . --config debug
popd

rem pause