#!/usr/bin/bash

set -x
set -e

name=torrent-file-editor

exclude="debug debug-qt4 debug-qt5 release release-qt4 release-qt5 linux linux-qt4 linux-qt5 CMakeLists.txt.user build-mxe-shared.sh build-mxe-static.sh"
exclude=$(echo $exclude | sed  -r 's/[^ ]+/-e &/g')

git clean -dfx . $exclude

if [ -x "$(command -v i686-w64-mingw32.shared-qmake-qt4)" ]; then
    mkdir mxe32-qt4-shared
    pushd mxe32-qt4-shared
    i686-w64-mingw32.shared-cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j5
    version=$(cat appversion)
    destdir=../${name}-${version}-mxe-qt4-shared-x32
    mkdir $destdir
    mv ${name}.exe $destdir
    libdir=$(i686-w64-mingw32.shared-qmake-qt4 -query QT_INSTALL_BINS)
    cp $libdir/QtCore4.dll $destdir
    cp $libdir/QtGui4.dll $destdir
    cp $libdir/../../bin/libgcc_s_sjlj-1.dll $destdir
    cp $libdir/../../bin/libpng16-16.dll $destdir
    cp $libdir/../../bin/libqjson.dll $destdir
    cp $libdir/../../bin/libstdc++-6.dll $destdir
    cp $libdir/../../bin/zlib1.dll $destdir
    popd
fi

if [ -x "$(command -v i686-w64-mingw32.shared-qmake-qt5)" ]; then
    mkdir mxe32-qt5-shared
    pushd mxe32-qt5-shared
    i686-w64-mingw32.shared-cmake -DCMAKE_BUILD_TYPE=Release -DQT5_BUILD=ON ..
    make -j5
    version=$(cat appversion)
    destdir=../${name}-${version}-mxe-qt5-shared-x32
    mkdir $destdir
    mv ${name}.exe $destdir
    libdir=$(i686-w64-mingw32.shared-qmake-qt5 -query QT_INSTALL_BINS)
    cp $libdir/Qt5Core.dll $destdir
    cp $libdir/Qt5Gui.dll $destdir
    cp $libdir/Qt5Widgets.dll $destdir
    cp $libdir/../../bin/libbz2.dll $destdir
    cp $libdir/../../bin/libfreetype-6.dll $destdir
    cp $libdir/../../bin/libgcc_s_sjlj-1.dll $destdir
    cp $libdir/../../bin/libglib-2.0-0.dll $destdir
    cp $libdir/../../bin/libharfbuzz-0.dll $destdir
    cp $libdir/../../bin/libiconv-2.dll $destdir
    cp $libdir/../../bin/libintl-8.dll $destdir
    cp $libdir/../../bin/libpcre-1.dll $destdir
    cp $libdir/../../bin/libpcre2-16-0.dll $destdir
    cp $libdir/../../bin/libpng16-16.dll $destdir
    cp $libdir/../../bin/libstdc++-6.dll $destdir
    cp $libdir/../../bin/zlib1.dll $destdir
    mkdir $destdir/platforms
    cp $libdir/../plugins/platforms/qwindows.dll $destdir/platforms
    mkdir $destdir/styles
    cp $libdir/../plugins/styles/qwindowsvistastyle.dll $destdir/styles
    popd
fi

if [ -x "$(command -v x86_64-w64-mingw32.shared-qmake-qt4)" ]; then
    mkdir mxe64-qt4-shared
    pushd mxe64-qt4-shared
    x86_64-w64-mingw32.shared-cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j5
    version=$(cat appversion)
    destdir=../${name}-${version}-mxe-qt4-shared-x64
    mkdir $destdir
    mv ${name}.exe $destdir
    libdir=$(x86_64-w64-mingw32.shared-qmake-qt4 -query QT_INSTALL_BINS)
    cp $libdir/QtCore4.dll $destdir
    cp $libdir/QtGui4.dll $destdir
    cp $libdir/../../bin/libgcc_s_seh-1.dll $destdir
    cp $libdir/../../bin/libpng16-16.dll $destdir
    cp $libdir/../../bin/libqjson.dll $destdir
    cp $libdir/../../bin/libstdc++-6.dll $destdir
    cp $libdir/../../bin/zlib1.dll $destdir
    popd
fi

if [ -x "$(command -v x86_64-w64-mingw32.shared-qmake-qt5)" ]; then
    mkdir mxe64-qt5-shared
    pushd mxe64-qt5-shared
    x86_64-w64-mingw32.shared-cmake -DCMAKE_BUILD_TYPE=Release -DQT5_BUILD=ON ..
    make -j5
    version=$(cat appversion)
    destdir=../${name}-${version}-mxe-qt5-shared-x64
    mkdir $destdir
    mv ${name}.exe $destdir
    libdir=$(x86_64-w64-mingw32.shared-qmake-qt5 -query QT_INSTALL_BINS)
    cp $libdir/Qt5Core.dll $destdir
    cp $libdir/Qt5Gui.dll $destdir
    cp $libdir/Qt5Widgets.dll $destdir
    cp $libdir/../../bin/libbz2.dll $destdir
    cp $libdir/../../bin/libfreetype-6.dll $destdir
    cp $libdir/../../bin/libgcc_s_seh-1.dll $destdir
    cp $libdir/../../bin/libglib-2.0-0.dll $destdir
    cp $libdir/../../bin/libharfbuzz-0.dll $destdir
    cp $libdir/../../bin/libiconv-2.dll $destdir
    cp $libdir/../../bin/libintl-8.dll $destdir
    cp $libdir/../../bin/libpcre-1.dll $destdir
    cp $libdir/../../bin/libpcre2-16-0.dll $destdir
    cp $libdir/../../bin/libpng16-16.dll $destdir
    cp $libdir/../../bin/libstdc++-6.dll $destdir
    cp $libdir/../../bin/zlib1.dll $destdir
    mkdir $destdir/platforms
    cp $libdir/../plugins/platforms/qwindows.dll $destdir/platforms
    mkdir $destdir/styles
    cp $libdir/../plugins/styles/qwindowsvistastyle.dll $destdir/styles
    popd
fi
