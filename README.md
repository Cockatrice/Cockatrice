<p align='center'><img src=https://cloud.githubusercontent.com/assets/9874850/7516775/b00b8e36-f4d1-11e4-8da4-3df294d01f86.png></p>

---

**Table of Contents** &nbsp;&nbsp; [Cockatrice](#cockatrice) | [Get Involved] (#get-involved-) | [Community Resources](#community-resources) | [Translation Status](#translation-status-) | [Building](#building-) | [Running](#running) | [License](#license-)

---

# Cockatrice

Cockatrice is an open-source multiplatform software for playing card games,
such as Magic: The Gathering, over a network. It is fully client-server based
to prevent any kind of cheating, though it supports single-player games without
a network interface as well. Both client and server are written in Qt, supporting both Qt4 and Qt5.<br>


# Get Involved [![Gitter chat](https://badges.gitter.im/Cockatrice/Cockatrice.png)](https://gitter.im/Cockatrice/Cockatrice)

Chat with the Cockatrice developers on Gitter. Come here to talk about the application, features, or just to hang out. For support regarding specific servers, please contact that server's admin or forum for support rather than asking here.<br>


# Community Resources
- [Cockatrice Official Wiki](https://github.com/Cockatrice/Cockatrice/wiki)
- [reddit r/Cockatrice](http://reddit.com/r/cockatrice)
- [Woogerworks](http://www.woogerworks.com) / [Chickatrice] (http://www.chickatrice.net/) / [Poixen](http://www.poixen.com/) (incomplete Serverlist)<br>


# Translation Status [![Cockatrice on Transiflex](https://ds0k0en9abmn1.cloudfront.net/static/charts/images/tx-logo-micro.646b0065fce6.png)](https://www.transifex.com/projects/p/cockatrice/)

Cockatrice uses Transifex for translations. You can help us bring Cockatrice/Oracle to your language or edit single wordings by clicking on the associated charts below.<br>
Our [project page](https://www.transifex.com/projects/p/cockatrice/) offers a detailed overview for contributors.

Language statistics for `Cockatrice` *(on the left)* and `Oracle` *(on the right)*:

[![Cockatrice translations](https://www.transifex.com/projects/p/cockatrice/resource/cockatrice/chart/image_png)](https://www.transifex.com/projects/p/cockatrice/resource/cockatrice/)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[![Oracle translations](https://www.transifex.com/projects/p/cockatrice/resource/oracle/chart/image_png)](https://www.transifex.com/projects/p/cockatrice/resource/oracle/)

Check out our [Translator FAQ](https://github.com/Cockatrice/Cockatrice/wiki/Translation-FAQ) for more information!<br>


# Building [![Build Status](https://travis-ci.org/Cockatrice/Cockatrice.svg?branch=master)](https://travis-ci.org/Cockatrice/Cockatrice)

**Detailed compiling instructions are on the Cockatrice wiki under [Compiling Cockatrice](https://github.com/Cockatrice/Cockatrice/wiki/Compiling-Cockatrice)**

Dependencies:
- [Qt](http://qt-project.org/) 
- [protobuf](http://code.google.com/p/protobuf/)
- [CMake](http://www.cmake.org/)

Oracle can optionally use zlib to load zipped files:
- [zlib](http://www.zlib.net/)

The server requires an additional dependency when compiled under Qt4:
- [libgcrypt](http://www.gnu.org/software/libgcrypt/)


To compile:

    mkdir build
    cd build
    cmake ..
    make
    make install

The following flags can be passed to `cmake`:

- `-DWITH_SERVER=1` Build the server.
- `-DWITH_CLIENT=0` Do not build the client.
- `-DWITH_ORACLE=0` Do not build oracle.
- `-DPORTABLE=1` Build portable versions of client & oracle.
- `-DWITH_QT4=1` Force compilation to use Qt4 instead of Qt5.
- `-DCMAKE_BUILD_TYPE=Debug` Compile in debug mode. Enables extra logging output, debug symbols, and much more verbose compiler warnings.
- `-DUPDATE_TRANSLATIONS=1` Configure `make` to update the translation .ts files for new strings in the source code. Note: Running `make clean` will remove the .ts files.

#### Building servatrice Docker container
`docker build -t servatrice .`<br>


# Running

`oracle` fetches card data  
`cockatrice` is the game client  
`servatrice` is the server<br>


# License [![GPLv2 License](https://img.shields.io/badge/license-GPLv2-blue.svg)](https://github.com/Cockatrice/Cockatrice/blob/master/COPYING)

Cockatrice is free software, licensed under the [GPLv2](https://github.com/Cockatrice/Cockatrice/blob/master/COPYING).

