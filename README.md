<p align='center'><img src=https://cloud.githubusercontent.com/assets/9874850/7516775/b00b8e36-f4d1-11e4-8da4-3df294d01f86.png></p>

---

**Table of Contents** &nbsp;&nbsp; [Cockatrice](#cockatrice) | [Downloads](#downloads) | [Get Involved] (#get-involved-) | [Community](#community-resources) | [Translation](#translation-status-) | [Building](#building--) | [Running](#running) | [License](#license-)

---

<br><pre>
<b>If you're getting started &#8674; [view our webpage](https://cockatrice.github.io/)</b><br>
<b>If you're trying to get support or suggest changes &#8674; [file an issue](https://github.com/Cockatrice/Cockatrice/issues) ([How?](https://github.com/Cockatrice/Cockatrice/wiki/How-to-Create-a-GitHub-Ticket))</b>
</pre><br>


# Cockatrice

Cockatrice is an open-source multiplatform supported program for playing tabletop card games over a network. The program's server design prevents any kind of client modifications to gain an unfair advantage in a game. The client also has a built in single-player mode where you can brew without being connected to a server. This project is written in C++ and is using the Qt5 libraries.<br>


# Downloads

We offer downloads for all full releases (recommended) and the latest development versions. Full releases are checkpoints with major feature or UI enhancements between them, but currently we don't have a set schedule for releasing new updates. The development version contains the most recently added features and bugfixes, but can be more unstable. Downloads for development versions are updated automatically with every change.
Downloads are hosted on [BinTray](https://bintray.com/cockatrice/Cockatrice).

- Latest full release (**recommended**): [ ![Download](https://api.bintray.com/packages/cockatrice/Cockatrice/Cockatrice/images/download.svg) ](https://bintray.com/cockatrice/Cockatrice/Cockatrice/_latestVersion#files)<br>

- Latest development version: [ ![Download](https://api.bintray.com/packages/cockatrice/Cockatrice/Cockatrice-git/images/download.svg) ](https://bintray.com/cockatrice/Cockatrice/Cockatrice-git/_latestVersion#files)<br>
*Development builds may not be stable or contain several bugs. Especially if from a branch other than `master`!*



# Get Involved [![Gitter chat](https://badges.gitter.im/Cockatrice/Cockatrice.png)](https://gitter.im/Cockatrice/Cockatrice)

[Chat](https://gitter.im/Cockatrice/Cockatrice) with the Cockatrice developers on Gitter. Come here to talk about the application, features, or just to hang out. For support regarding specific servers, please contact that server's admin or forum for support rather than asking here.<br>

If you'd like to contribute code to the project, we maintain a tag for "easy" changes on our issue tracker. Issues tagged in this way provide a simple way to get started. [Issues tagged as Easy Changes](https://github.com/Cockatrice/Cockatrice/issues?q=is%3Aopen+is%3Aissue+label%3A%22Easy+Change%22)


# Community Resources
- [Cockatrice Official Site](https://cockatrice.github.io)
- [Cockatrice Official Wiki](https://github.com/Cockatrice/Cockatrice/wiki)
- [reddit r/Cockatrice](https://reddit.com/r/cockatrice)


# Translation Status [![Cockatrice on Transiflex](https://tx-assets.scdn5.secure.raxcdn.com/static/charts/images/tx-logo-micro.c5603f91c780.png)](https://www.transifex.com/projects/p/cockatrice/)

Cockatrice uses Transifex for translations. You can help us bring Cockatrice and Oracle to your language or just edit single wordings right from within your browser by simply visiting our [Transifex project page](https://www.transifex.com/projects/p/cockatrice/).<br>

| Cockatrice | Oracle |
|:-:|:-:|
| [![Cockatrice Translation Status](https://www.transifex.com/projects/p/cockatrice/resource/cockatrice/chart/image_png/)](https://www.transifex.com/projects/p/cockatrice/) | [![Oracle Translation Status](https://www.transifex.com/projects/p/cockatrice/resource/oracle/chart/image_png/)](https://www.transifex.com/projects/p/cockatrice/) |

Check out our [Translator FAQ](https://github.com/Cockatrice/Cockatrice/wiki/Translation-FAQ) for more information about contributing!<br>


# Building [![Travis Build Status - master](https://travis-ci.org/Cockatrice/Cockatrice.svg?branch=master)](https://travis-ci.org/Cockatrice/Cockatrice) [![Appveyor Build Status - master](https://ci.appveyor.com/api/projects/status/lp5h0dhk4mhmeps7/branch/master?svg=true)](https://ci.appveyor.com/project/Daenyth/cockatrice/branch/master)

**Detailed compiling instructions are on the Cockatrice wiki under [Compiling Cockatrice](https://github.com/Cockatrice/Cockatrice/wiki/Compiling-Cockatrice)**

Dependencies:
- [Qt](https://www.qt.io/developers/) 
- [protobuf](https://github.com/google/protobuf)
- [CMake](https://www.cmake.org/)

Oracle can optionally use zlib to load zipped files:
- [zlib](http://www.zlib.net/) (no https!)

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
- `-DCMAKE_BUILD_TYPE=Debug` Compile in debug mode. Enables extra logging output, debug symbols, and much more verbose compiler warnings (default `Release`).
- `-DUPDATE_TRANSLATIONS=1` Configure `make` to update the translation .ts files for new strings in the source code. Note: Running `make clean` will remove the .ts files (default 0 = no).
- `-DTEST=1` Enable regression tests (default 0 = no). Note: needs googletest, will be downloaded on the fly if unavailable. To run tests: ```make test```.


# Running

`cockatrice` is the game client    
`oracle` fetches card data    
`servatrice` is the server<br>


#### Servatrice Docker container

A Dockerfile is provided to run Servatrice (the Cockatrice server) using [Docker](https://www.docker.com/what-docker).<br>

You just need to create an image from the Dockerfile<br>
`docker build -t servatrice .`<br>
And then run it<br>
`docker run -i -p 4747:4747/tcp -t servatrice:latest`<br>

Please note that running this command will expose the TCP port 4747 of the docker container to permit connections to the server.<br>
More infos on how to use Servatrice with Docker can be found in our [wiki](https://github.com/Cockatrice/Cockatrice/wiki/Setting-up-Servatrice#using-docker).


# License [![GPLv2 License](https://img.shields.io/badge/License-GPLv2-blue.svg)](https://github.com/Cockatrice/Cockatrice/blob/master/COPYING)

Cockatrice is free software, licensed under the [GPLv2](https://github.com/Cockatrice/Cockatrice/blob/master/COPYING).

