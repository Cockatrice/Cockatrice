<p align='center'><img src=https://cloud.githubusercontent.com/assets/9874850/7516775/b00b8e36-f4d1-11e4-8da4-3df294d01f86.png></p>

---

<p align='center'>
   <a href="#cockatrice"><b>Cockatrice</b></a> <b>|</b>
   <a href="#download-">Download</a> <b>|</b>
   <a href="#get-involved-">Get Involved</a> <b>|</b>
   <a href="#community-resources">Community</a> <b>|</b>
   <a href="#translations-">Translations</a> <b>|</b>
   <a href="#build--">Build</a> <b>|</b>
   <a href="#run">Run</a> <b>|</b>
   <a href="#license-">License</a>
</p>

---

<br><pre>
<b>To get started, &#8674; [view our webpage](https://cockatrice.github.io/)</b><br>
<b>To get support or suggest changes &#8674; [file an issue](https://github.com/Cockatrice/Cockatrice/issues) ([How?](https://github.com/Cockatrice/Cockatrice/wiki/How-to-Create-a-GitHub-Ticket-Regarding-Cockatrice))</b>
<b>To help with development, see how to [get involved](#get-involved-)</b>
</pre><br>


# Cockatrice

Cockatrice is an open-source, multiplatform program for playing tabletop card games over a network. The program's server design prevents users from manipulating the game for unfair advantage. The client also provides a single-player mode, which allows users to brew while offline. This project uses C++ and the Qt5 libraries.<br>


# Download [![Cockatrice Eternal Download Count](https://img.shields.io/github/downloads/cockatrice/cockatrice/total.svg)](https://tooomm.github.io/github-release-stats/?username=Cockatrice&repository=Cockatrice)

Downloads are available for full releases and the current beta version in development. There is no strict release schedule for either of them.

- Latest `stable` release: [![Download from GitHub Releases](https://img.shields.io/github/release/cockatrice/cockatrice.svg)](https://github.com/cockatrice/cockatrice/releases/latest) [![DL Count on Latest Release](https://img.shields.io/github/downloads/cockatrice/cockatrice/latest/total.svg?label=downloads)](https://tooomm.github.io/github-release-stats/?username=Cockatrice&repository=Cockatrice)<br>
  - Stable versions are checkpoints featuring major feature and UI enhancements.
  - **Recommended for most users!**

- Latest `beta` release: [![Download from GitHub Pre-Releases](https://img.shields.io/github/release/cockatrice/cockatrice/all.svg)](https://github.com/cockatrice/cockatrice/releases) [![DL Count on Latest Pre-Release](https://img.shields.io/github/downloads-pre/cockatrice/cockatrice/latest/total.svg?label=downloads)](https://tooomm.github.io/github-release-stats/?username=Cockatrice&repository=Cockatrice)
   - Beta versions include the most recently added features and bugfixes, but can be unstable.
   - To be a Cockatrice Beta Tester, use this version. Find more information [here](https://github.com/Cockatrice/Cockatrice/wiki/Release-Channels)!
   

# Get Involved [![Discord](https://img.shields.io/discord/314987288398659595?label=Discord&logo=discord&logoColor=white)](https://discord.gg/3Z9yzmA)

Join our [Discord community](https://discord.gg/3Z9yzmA) to connect with the project, contributors or fellow users of the app. Come here to talk about the application, features, or just to hang out.<br>
For support regarding specific servers, please contact that server's admin or forum for support rather than asking here.<br>

To contribute code to the project, please review [the guidelines](https://github.com/Cockatrice/Cockatrice/blob/master/.github/CONTRIBUTING.md).
We maintain two tags for contributors to find issues to work on:
- [Good first issue](https://github.com/Cockatrice/Cockatrice/issues?utf8=%E2%9C%93&q=is%3Aopen%20is%3Aissue%20label%3A%22Good%20first%20issue%22%20): issues tagged in this way provide a simple way to get started. They don't require much experience to be worked on.
- [Help wanted](https://github.com/Cockatrice/Cockatrice/issues?utf8=%E2%9C%93&q=is%3Aopen%20is%3Aissue%20label%3A%22Help%20Wanted%22%20): This tag is used for issues that we are looking for a contributor to work on. Often this is for feature suggestions we are willing to accept, but don't have the time to work on ourselves.

For both tags, we're willing to provide help to contributors in showing them where and how they can make changes, as well as code review for changes they submit.

We try to be responsive to new issues. We'll provide advice on how best to implement a feature; alternately, we can show you where the codebase is doing something similar before you get too far along.

Cockatrice uses the [Google Developer Documentation Style Guide](https://developers.google.com/style/) to ensure consistent documentation. We encourage you to improve the documentation by suggesting edits based on this guide.


# Community Resources

- [Cockatrice Official Site](https://cockatrice.github.io)
- [Cockatrice Official Wiki](https://github.com/Cockatrice/Cockatrice/wiki)
- [Cockatrice Official Discord](https://discord.gg/3Z9yzmA)
- [reddit r/Cockatrice](https://reddit.com/r/cockatrice)


# Translations [![Transifex Project](https://img.shields.io/badge/translate-on%20transifex-brightgreen)](https://transifex.com/cockatrice/cockatrice/)

Cockatrice uses Transifex for translations. You can help us bring Cockatrice, Oracle and Webatrice to your language or just adjust single wordings right from within your browser by visiting our [Transifex project page](https://transifex.com/cockatrice/cockatrice/).<br>

Check out our [Translator FAQ](https://github.com/Cockatrice/Cockatrice/wiki/Translation-FAQ) for more information about contributing!<br>


# Build [![CI Desktop](https://github.com/Cockatrice/Cockatrice/actions/workflows/desktop-build.yml/badge.svg?branch=master&event=push)](https://github.com/Cockatrice/Cockatrice/actions/workflows/desktop-build.yml?query=branch%3Amaster+event%3Apush) [![CI Web](https://github.com/Cockatrice/Cockatrice/actions/workflows/web-build.yml/badge.svg?branch=master&event=push)](https://github.com/Cockatrice/Cockatrice/actions/workflows/web-build.yml?query=branch%3Amaster+event%3Apush)

**Detailed compiling instructions can be found on the Cockatrice wiki under [Compiling Cockatrice](https://github.com/Cockatrice/Cockatrice/wiki/Compiling-Cockatrice)**

Dependencies: *(for minimum requirements search our [CMake file](https://github.com/Cockatrice/Cockatrice/blob/master/CMakeLists.txt))*
- [Qt](https://www.qt.io/developers/)
- [protobuf](https://github.com/protocolbuffers/protobuf)
- [CMake](https://www.cmake.org/)

Oracle can optionally use zlib and xz to load compressed files:
- [xz](https://tukaani.org/xz/)
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
- `-DWARNING_AS_ERROR=0` Whether to treat compilation warnings as errors in debug mode (default 1 = yes).
- `-DUPDATE_TRANSLATIONS=1` Configure `make` to update the translation .ts files for new strings in the source code. Note: Running `make clean` will remove the .ts files (default 0 = no).
- `-DTEST=1` Enable regression tests (default 0 = no). Note: needs googletest, will be downloaded on the fly if unavailable. To run tests: ```make test```.
- `-DFORCE_USE_QT5=1` Skip looking for Qt6 before trying to find Qt5


# Run

`Cockatrice` is the game client<br>
`Oracle` fetches card data<br>
`Servatrice` is the server<br>

**Servatrice Docker container**

You can run an instance of Servatrice (the Cockatrice server) using [Docker](https://www.docker.com/what-docker) and the Cockatrice Dockerfile.<br>

First, create an image from the Dockerfile<br>
`cd /path/to/Cockatrice-Repo/`
`docker build -t servatrice .`<br>
And then run it<br>
`docker run -i -p 4748:4748 -t servatrice:latest`<br>

>Note: Running this command exposes the port 4748 of the docker container<br>
to permit connections to the server.

Find more information on how to use Servatrice with Docker in our [wiki](https://github.com/Cockatrice/Cockatrice/wiki/Setting-up-Servatrice#using-docker).

**Docker compose**

There is also a docker-compose file available which will configure and run both a MySQL server and Servatrice. The docker-compose setup scripts can be found in the `servatrice/docker` folder and vary only slightly from the default sql and server .ini files. The setup scripts can either be modified in place, or docker-compose can mount alternative files into the images, as you prefer.

To run Servatrice via docker-compose, first install docker-compose following the [install instructions](https://docs.docker.com/compose/install/). Once installed, run the following from the root of the repository:
```bash
docker-compose build    # Build the Servatrice image using the same Dockerfile as above.
docker-compose up       # Setup and run both the MySQL server and Servatrice.
```

>Note: Similar to the above Docker setup, this will expose port 4748.

>Note: The first time running the docker-compose setup, the MySQL server will take a little time to run the initial setup scripts. Due to this, the Servatrice instance may fail the first few attempts to connect to the database. Servatrice is set to `restart: always` in the docker-compose.yml, which will allow it to continue attempting to start up. Once the MySQL scripts have completed, Servatrice should then connect automatically on the next attempt.

**Docker compose in Windows**
A out of box working docker-compose file has been added to help setup in Windows.

Docker in Windows requires additional steps in form of using Docker Desktop to allow resource sharing from the drive the volumes are mapped from, as well as potential workarounds needed to get file sharing working in Windows. This [StackOverflow discussion sheds some light on it](https://stackoverflow.com/questions/42203488/settings-to-windows-firewall-to-allow-docker-for-windows-to-share-drive)


# License [![GPLv2 License](https://img.shields.io/github/license/Cockatrice/Cockatrice.svg)](https://github.com/Cockatrice/Cockatrice/blob/master/LICENSE)

Cockatrice is free software, licensed under the [GPLv2](https://github.com/Cockatrice/Cockatrice/blob/master/LICENSE).
