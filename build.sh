#!/usr/bin/bash

set -x
set -e

name=torrent-file-editor

exclude="debug debug-qt4 debug-qt5 release release-qt4 release-qt5 linux linux-qt4 linux-qt5 CMakeLists.txt.user"
exclude=$(echo $exclude | sed  -r 's/[^ ]+/-e &/g')

git clean -dfx . $exclude
mkdir win32
pushd win32
mingw32-cmake .. -DQT4_BUILD=ON -DBUILD_SHARED=OFF -DCMAKE_EXE_LINKER_FLAGS=-static -DCMAKE_BUILD_TYPE=Release -DQT_INCLUDE_DIRS_NO_SYSTEM=ON
make -j5
version=$(cat appversion)
i686-w64-mingw32-strip -s ${name}.exe
mv ${name}.exe ../${name}-${version}-x32.exe
popd

mkdir win64
pushd win64
mingw64-cmake .. -DQT4_BUILD=ON -DBUILD_SHARED=OFF -DCMAKE_EXE_LINKER_FLAGS=-static -DCMAKE_BUILD_TYPE=Release -DQT_INCLUDE_DIRS_NO_SYSTEM=ON
make -j5
version=$(cat appversion)
x86_64-w64-mingw32-strip -s ${name}.exe
mv ${name}.exe ../${name}-${version}-x64.exe
popd
