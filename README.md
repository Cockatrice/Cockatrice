<p align='center'><img src=https://cloud.githubusercontent.com/assets/9874850/7516775/b00b8e36-f4d1-11e4-8da4-3df294d01f86.png></p>

---

**Table of Contents** &nbsp;&nbsp; [Cockatrice](#cockatrice) | [Downloads](#downloads-) | [Get Involved](#get-involved-) | [Community](#community-resources) | [Translation](#translation-status-) | [Building](#building--) | [Running](#running) | [License](#license-)

---

<br><pre>
<b>To get started, &#8674; [view our webpage](https://cockatrice.github.io/)</b><br>
<b>To get support or suggest changes &#8674; [file an issue](https://github.com/Cockatrice/Cockatrice/issues) ([How?](https://github.com/Cockatrice/Cockatrice/wiki/How-to-Create-a-GitHub-Ticket))</b>
<b>To help with development, see how to [get involved](#get-involved-)</b>
</pre><br>


# Cockatrice

Cockatrice is an open-source, multiplatform program for playing tabletop card games over a network. The program's server design prevents users from manipulating the game for unfair advantage. The client also provides a single-player mode, which allows users to brew while offline. This project uses C++ and the Qt5 libraries.<br>


# Downloads [![Cockatrice Eternal Download Count](https://img.shields.io/github/downloads/cockatrice/cockatrice/total.svg)](https://tooomm.github.io/github-release-stats/?username=Cockatrice&repository=Cockatrice)

Downloads are available for full releases and the current development version.<br>
Full releases are checkpoints featuring major feature or UI enhancements - we recommend to use those. There is no strict schedule for new full releases.

The development version contains the most recently added features and bugfixes, but can be unstable. They are released as we feel need.

- Latest full release (**recommended**): [![Download from GitHub Releases](https://img.shields.io/github/release/cockatrice/cockatrice.svg)](https://github.com/cockatrice/cockatrice/releases/latest) [![Download Count on Latest Release](https://img.shields.io/github/downloads/cockatrice/cockatrice/latest/total.svg)](https://tooomm.github.io/github-release-stats/?username=Cockatrice&repository=Cockatrice)<br>

- Latest development version: [![Download from GitHub Pre-Releases](https://img.shields.io/github/release/cockatrice/cockatrice/all.svg)](https://github.com/cockatrice/cockatrice/releases) 
   - Development builds may be unstable and contain bugs.
   - To be a Cockatrice Beta Tester, use this version.
   - Find more information on the development build [here](https://github.com/Cockatrice/Cockatrice/wiki/Release-Channels)


# Get Involved [![Gitter Chat](https://img.shields.io/gitter/room/Cockatrice/Cockatrice.svg)](https://gitter.im/Cockatrice/Cockatrice)

[Chat](https://gitter.im/Cockatrice/Cockatrice) with the Cockatrice developers on Gitter. Come here to talk about the application, features, or just to hang out. For support regarding specific servers, please contact that server's admin or forum for support rather than asking here.<br>

To contribute code to the project, please review [the guidelines](https://github.com/Cockatrice/Cockatrice/blob/master/.github/CONTRIBUTING.md).
We maintain two tags for contributors to find issues to work on:
- [Good first issue](https://github.com/Cockatrice/Cockatrice/issues?utf8=%E2%9C%93&q=is%3Aopen%20is%3Aissue%20label%3A%22Good%20first%20issue%22%20): issues tagged in this way provide a simple way to get started. They don't require much experience to be worked on.
- [Help wanted](https://github.com/Cockatrice/Cockatrice/issues?utf8=%E2%9C%93&q=is%3Aopen%20is%3Aissue%20label%3A%22Help%20Wanted%22%20): This tag is used for issues that we are looking for a contributor to work on. Often this is for feature suggestions we are willing to accept, but don't have the time to work on ourselves.

For both tags, we're willing to provide help to contributors in showing them where and how they can make changes, as well as code review for changes they submit.

Read the long-term project **roadmap** to see planned edits and milestones [here](https://docs.google.com/document/d/1Ewe5uSaRE2nR2pNPMaGmP6gVZdqgFbBgwSscGqIr4W0/edit).

We try to be responsive to new issues. We'll provide advice on how best to implement a feature; alternately, we can show you where the codebase is doing something similar before you get too far along.

Cockatrice uses the [Google Developer Documentation Style Guide](https://developers.google.com/style/) to ensure consistent documentation. We encourage you to improve the documentation by suggesting edits based on this guide.


# Community Resources

- [Cockatrice Official Site](https://cockatrice.github.io)
- [Cockatrice Official Wiki](https://github.com/Cockatrice/Cockatrice/wiki)
- [reddit r/Cockatrice](https://reddit.com/r/cockatrice)


# Translation Status [![Cockatrice on Transiflex](https://tx-assets.scdn5.secure.raxcdn.com/static/charts/images/tx-logo-micro.c5603f91c780.png)](https://www.transifex.com/projects/p/cockatrice/)

Cockatrice uses Transifex for translations. You can help us bring Cockatrice and Oracle to your language or just edit single wordings right from within your browser by visiting our [Transifex project page](https://www.transifex.com/projects/p/cockatrice/).<br>

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
- [zlib](https://www.zlib.net/)

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
- `-DCMAKE_BUILD_TYPE=Debug` Compile in debug mode. Enables extra logging output, debug symbols, and much more verbose compiler warnings (default `Release`).
- `-DUPDATE_TRANSLATIONS=1` Configure `make` to update the translation .ts files for new strings in the source code. Note: Running `make clean` will remove the .ts files (default 0 = no).
- `-DTEST=1` Enable regression tests (default 0 = no). Note: needs googletest, will be downloaded on the fly if unavailable. To run tests: ```make test```.


# Running

`Cockatrice` is the game client    
`Oracle` fetches card data    
`Servatrice` is the server<br>


#### Servatrice Docker container

You can run an instance of Servatrice (the Cockatrice server) using [Docker](https://www.docker.com/what-docker) and the Cockatrice Dockerfile.<br>

First, create an image from the Dockerfile<br>
`cd /path/to/Cockatrice-Repo/`
`docker build -t servatrice .`<br>
And then run it<br>
`docker run -i -p 4747:4747/tcp -t servatrice:latest`<br>

>Note: Running this command exposes the TCP port 4747 of the docker container<br>
to permit connections to the server.

Find more information on how to use Servatrice with Docker in our [wiki](https://github.com/Cockatrice/Cockatrice/wiki/Setting-up-Servatrice#using-docker).

# License [![GPLv2 License](https://img.shields.io/github/license/Cockatrice/Cockatrice.svg)](https://github.com/Cockatrice/Cockatrice/blob/master/LICENSE)

Cockatrice is free software, licensed under the [GPLv2](https://github.com/Cockatrice/Cockatrice/blob/master/LICENSE).
