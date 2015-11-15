[![build status](https://secure.travis-ci.org/drizt/torrent-file-editor.png)](http://travis-ci.org/drizt/torrent-file-editor)
[![Crowdin](https://d322cqt584bo4o.cloudfront.net/torrent-file-editor/localized.svg)](https://crowdin.com/project/torrent-file-editor)

Torrent File Editor
===================

Qt based GUI tool designed to create and edit .torrent files

Author: Ivan Romanov <[drizt@land.ru](mailto:drizt@land.ru)>  
License: GNU General Public License v3.0 or later  
Home Page: http://sourceforge.net/projects/torrent-file-editor/  
Sources: http://github.com/drizt/torrent-file-editor  
Crowdin translations: https://crowdin.com/project/torrent-file-editor

Build Instructions
------------------

Need to have
 - CMake >= 2.8.11
 - Qt4 or Qt5
 - QJSON >= 0.8.0 if used Qt4
 - [Sparkle](http://sparkle-project.org/) only for Mac OS X

**Linux:**

By default will be built Qt4 version

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
I use Fedora 22 MinGW to build Windows version. Furthemore I build
portable static version. Any another build way not tested and may will
not work. It is in my TODO list.

Fedora hasn't MinGW QJSON package. Need to build own version. It is
easy:

    wget https://github.com/flavio/qjson/archive/master.tar.gz -O qjson-master.tar.gz
    tar zxf qjson-master.tar.gz
    mkdir qjson-master/win32
    mkdir qjson-master/win64
    cd qjson-master/win32
    mingw32-cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DQT4_BUILD=ON ..
    make
    sudo make install # will be carefull, it installs qjson to system folders
    cd ../win64
    mingw64-cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DQT4_BUILD=ON ..
    make
    sudo make install # will be carefull, it installs qjson to system folders

**Windows x32:**

    mkdir build && cd build
    mingw32-cmake -DCMAKE_BUILD_TYPE=Release ..
    make

**Windows x64:**

    mkdir build && cd build
    mingw64-cmake -DCMAKE_BUILD_TYPE=Release ..
    make

How I Can Help?
---------------

Now project translated only to three languages. It is English, Russian
and Czech. I would glad to add new translations. You can translate
project to your native language with [Crowdin](https://crowdin.com/project/torrent-file-editor).
It is not difficult and not required special knowledges. Or you can
correct my English. I know it is not good. Anyway you can
always email <[drizt@land.ru](mailto:drizt@land.ru)> me.

Also feel free to open an issue on github or send me pull requests.
