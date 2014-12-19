#!/usr/bin/bash

set -e

name=torrent-file-editor
version=0.1.0

git clean -dfx .
mkdir win32
pushd win32
mingw32-cmake .. -DCMAKE_EXE_LINKER_FLAGS=-static -DCMAKE_BUILD_TYPE=Release
make -j5
mv ${name}.exe ../${name}-${version}-x32.exe
popd

mkdir win64
pushd win64
mingw64-cmake .. -DCMAKE_EXE_LINKER_FLAGS=-static -DCMAKE_BUILD_TYPE=Release
make -j5
mv ${name}.exe ../${name}-${version}-x64.exe
popd
