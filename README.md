<p align='center'><img src=https://cloud.githubusercontent.com/assets/9874850/7516775/b00b8e36-f4d1-11e4-8da4-3df294d01f86.png></p>

---

**Table of Contents** &nbsp;&nbsp; [Cockatrice](#cockatrice) | [Downloads](#downloads) | [Get Involved] (#get-involved-) | [Community](#community-resources) | [Translation](#translation-status-) | [Building](#building--) | [Running](#running) | [License](#license-)

---

<br><pre>
<b>If you're getting started &#8674; [view our webpage](https://cockatrice.github.io/)</b><br>
<b>If you're trying to get support or suggest changes &#8674; [file an issue](https://github.com/Cockatrice/Cockatrice/issues/new)</b>
</pre><br>


# Cockatrice

Cockatrice is an open-source multiplatform software for playing card games,
such as Magic: The Gathering, over a network. It is fully client-server based
to prevent any kind of cheating, though it supports single-player games without
a network interface as well. Both client and server are written in Qt, supporting both Qt4 and Qt5.<br>


# Downloads

We offer a download for all full releases and the latest development version (recommended). Full releases are checkpoints with major feature or UI enhancements between them, but are infrequently updated. The development version contains the most recently added features and bugfixes, but can be more unstable. Downloads for development versions are updated automatically with every change.
Downloads are hosted on [BinTray](https://bintray.com/cockatrice/Cockatrice).

- Latest development version (recommended): [ ![Download](https://api.bintray.com/packages/cockatrice/Cockatrice/Cockatrice-git/images/download.svg) ](https://bintray.com/cockatrice/Cockatrice/Cockatrice-git/_latestVersion#files)
- Latest full release: [ ![Download](https://api.bintray.com/packages/cockatrice/Cockatrice/Cockatrice/images/download.svg) ](https://bintray.com/cockatrice/Cockatrice/Cockatrice/_latestVersion#files)


# Get Involved [![Gitter chat](https://badges.gitter.im/Cockatrice/Cockatrice.png)](https://gitter.im/Cockatrice/Cockatrice)

Chat with the Cockatrice developers on Gitter. Come here to talk about the application, features, or just to hang out. For support regarding specific servers, please contact that server's admin or forum for support rather than asking here.<br>


# Community Resources
- [Cockatrice Official Site](https://cockatrice.github.io)
- [Cockatrice Official Wiki](https://github.com/Cockatrice/Cockatrice/wiki)
- [reddit r/Cockatrice](https://reddit.com/r/cockatrice)


# Translation Status [![Cockatrice on Transiflex](https://ds0k0en9abmn1.cloudfront.net/static/charts/images/tx-logo-micro.646b0065fce6.png)](https://www.transifex.com/projects/p/cockatrice/)

Cockatrice uses Transifex for translations. You can help us bring Cockatrice and Oracle to your language or just edit single wordings right from within your browser by simply visiting our [Transifex project page](https://www.transifex.com/projects/p/cockatrice/).<br>

| Cockatrice | Oracle |
|:-:|:-:|
| [![Cockatrice Translation Status](https://www.transifex.com/projects/p/cockatrice/resource/cockatrice/chart/image_png)](https://www.transifex.com/projects/p/cockatrice/) | [![Oracle Translation Status](https://www.transifex.com/projects/p/cockatrice/resource/oracle/chart/image_png)](https://www.transifex.com/projects/p/cockatrice/) |

Check out our [Translator FAQ](https://github.com/Cockatrice/Cockatrice/wiki/Translation-FAQ) for more information about contributing!<br>


# Building [![Travis Build Status - master](https://travis-ci.org/Cockatrice/Cockatrice.svg?branch=master)](https://travis-ci.org/Cockatrice/Cockatrice) [![Appveyor Build Status - master](https://ci.appveyor.com/api/projects/status/lp5h0dhk4mhmeps7/branch/master?svg=true)](https://ci.appveyor.com/project/Daenyth/cockatrice/branch/master)

**Detailed compiling instructions are on the Cockatrice wiki under [Compiling Cockatrice](https://github.com/Cockatrice/Cockatrice/wiki/Compiling-Cockatrice)**

Dependencies:
- [Qt](https://www.qt.io/developers/) 
- [protobuf](https://github.com/google/protobuf)
- [CMake](https://www.cmake.org/)

Oracle can optionally use zlib to load zipped files:
- [zlib](http://www.zlib.net/) (no https!)

The server requires an additional dependency when compiled under Qt4:
- [libgcrypt](https://www.gnu.org/software/libgcrypt/)


To compile:

    mkdir build
    cd build
    cmake ..
    make

You can then run

    make install

to get a cockatrice installation inside the `release` folder, or:

    make package

to create a system-specific installation package.

The following flags can be passed to `cmake`:

- `-DWITH_SERVER=1` Whether to build the server (default 0 = no).
- `-DWITH_CLIENT=0` Whether to build the client (default 1 = yes).
- `-DWITH_ORACLE=0` Whether to build oracle (default 1 = yes).
- `-DPORTABLE=1` Build portable versions of client & oracle (default 0 = no).
- `-DWITH_QT4=1` Force compilation to use Qt4 instead of Qt5 (default 0 = no).
- `-DCMAKE_BUILD_TYPE=Debug` Compile in debug mode. Enables extra logging output, debug symbols, and much more verbose compiler warnings (default `Release`).
- `-DUPDATE_TRANSLATIONS=1` Configure `make` to update the translation .ts files for new strings in the source code. Note: Running `make clean` will remove the .ts files (default 0 = no).
- `-DTEST=1` Enable regression tests (default 0 = no). Note: needs googletest, will be downloaded on the fly if unavailable. To run tests: ```make test```.


#### Building servatrice Docker container
`docker build -t servatrice .`<br>


# Running

`oracle` fetches card data  
`cockatrice` is the game client  
`servatrice` is the server<br>


# License [![GPLv2 License](https://img.shields.io/badge/License-GPLv2-blue.svg)](https://github.com/Cockatrice/Cockatrice/blob/master/COPYING)

Cockatrice is free software, licensed under the [GPLv2](https://github.com/Cockatrice/Cockatrice/blob/master/COPYING).

