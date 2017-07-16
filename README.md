[![Build Status](https://travis-ci.org/drizt/torrent-file-editor.svg?branch=master)](https://travis-ci.org/drizt/torrent-file-editor)
[![Crowdin](https://d322cqt584bo4o.cloudfront.net/torrent-file-editor/localized.svg)](https://crowdin.com/project/torrent-file-editor)

Torrent File Editor
===================

Qt based GUI tool designed to create and edit .torrent files

Author: Ivan Romanov <[drizt@land.ru](mailto:drizt@land.ru)>  
License: GNU General Public License v3.0 or later  
Homepage: https://sourceforge.net/projects/torrent-file-editor  
Sources: https://github.com/drizt/torrent-file-editor  
Crowdin translations: https://crowdin.com/project/torrent-file-editor

Build Instructions
------------------

Need to have
 - CMake >= 2.8.11
 - Qt4 or Qt5
 - QJSON >= 0.8.0 if used Qt4
 - [Sparkle](http://sparkle-project.org/) only for Mac OS X

**Linux:**

Will build Qt4 version by default

    mkdir build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release -DQT5_BUILD=OFF ..
    make

**Mac OS X:**

Only Qt5 version

    mkdir build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make
    make dmg # to build dmg package

**Windows important note**

Only Qt4 version for a while.
I use Fedora 26 MinGW to build Windows versions. Furthermore I build
portable static versions. Any other build way is not tested and may
not work. It is on my TODO list.

Fedora hasn't a MinGW QJSON package. You need to build your own version.
It is easy:

    wget https://github.com/flavio/qjson/archive/master.tar.gz -O qjson-master.tar.gz
    tar zxf qjson-master.tar.gz
    mkdir qjson-master/win32
    mkdir qjson-master/win64
    cd qjson-master/win32
    mingw32-cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DQT4_BUILD=ON  -DQT_INCLUDE_DIRS_NO_SYSTEM=ON -DQT_USE_IMPORTED_TARGETS=OFF ..
    make
    sudo make install # will be carefull, it installs qjson to system folders
    cd ../win64
    mingw64-cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DQT4_BUILD=ON  -DQT_INCLUDE_DIRS_NO_SYSTEM=ON -DQT_USE_IMPORTED_TARGETS=OFF ..
    make
    sudo make install # be careful, it installs qjson to system folders

**Windows x32:**

    mkdir build && cd build
    mingw32-cmake -DCMAKE_BUILD_TYPE=Release ..
    make

**Windows x64:**

    mkdir build && cd build
    mingw64-cmake -DCMAKE_BUILD_TYPE=Release ..
    make

How Can I Help?
---------------

The Project is translated from English to several languages.
There are Arabian, Czech, Dutch, French, German, Spanish, Hungarian, Italian, Korean,
Russian, Simplified Chinese, Portuguese and Turkish translations available.
I would be glad if you add new translations. You can translate the
project to your native language with [Crowdin](https://crowdin.com/project/torrent-file-editor).
It is not difficult and no special knowledges are required.
Or you can correct my English. I know it is not good. Anyway you can
always email <[drizt@land.ru](mailto:drizt@land.ru)> me.

Also feel free to open an issue on GitHub or send me pull requests.
