#!/usr/bin/bash

set -e

version=beta2

git clean -dfx .
mkdir win32
pushd win32
mingw32-cmake .. -DCMAKE_BUILD_TYPE=Release
make -j5
popd

mkdir win64
pushd win64
mingw64-cmake .. -DCMAKE_BUILD_TYPE=Release
make -j5
popd

makensis -DWIN32 -DVERSION=$version torrentfileeditor.nsis
makensis -DWIN64 -DVERSION=$version torrentfileeditor.nsis
