#!/usr/bin/bash

set -x
set -e

name=torrent-file-editor

exclude="debug debug-qt4 debug-qt5 release release-qt4 release-qt5 linux linux-qt4 linux-qt5 CMakeLists.txt.user build-mxe-shared.sh build-mxe-static.sh"
exclude=$(echo $exclude | sed  -r 's/[^ ]+/-e &/g')

git clean -dfx . $exclude

if [ -x "$(command -v i686-w64-mingw32.shared-qmake-qt4)" ]; then
    mkdir mxe32-qt4-static
    pushd mxe32-qt4-static
    i686-w64-mingw32.static-cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j5
    version=$(cat appversion)
    mv ${name}.exe ../${name}-${version}-mxe-qt4-static-x32.exe
    popd
fi

if [ -x "$(command -v i686-w64-mingw32.shared-qmake-qt5)" ]; then
    mkdir mxe32-qt5-static
    pushd mxe32-qt5-static
    i686-w64-mingw32.static-cmake -DCMAKE_BUILD_TYPE=Release -DQT5_BUILD=ON ..
    make -j5
    version=$(cat appversion)
    mv ${name}.exe ../${name}-${version}-mxe-qt5-static-x32.exe
    popd
fi

if [ -x "$(command -v x86_64-w64-mingw32.shared-qmake-qt4)" ]; then
    mkdir mxe64-qt4-static
    pushd mxe64-qt4-static
    x86_64-w64-mingw32.static-cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j5
    version=$(cat appversion)
    mv ${name}.exe ../${name}-${version}-mxe-qt4-static-x64.exe
    popd
fi

if [ -x "$(command -v x86_64-w64-mingw32.shared-qmake-qt5)" ]; then
    mkdir mxe64-qt5-static
    pushd mxe64-qt5-static
    x86_64-w64-mingw32.static-cmake -DCMAKE_BUILD_TYPE=Release -DQT5_BUILD=ON ..
    make -j5
    version=$(cat appversion)
    mv ${name}.exe ../${name}-${version}-mxe-qt5-static-x64.exe
    popd
fi
