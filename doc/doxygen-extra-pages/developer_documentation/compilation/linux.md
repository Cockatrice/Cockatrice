@page compilation_linux Compiling for Linux

[&nbsp; &larr; Back to Compiling Overview](@ref compilation_index)

- [Precompiled Packages](#Packages)
- [Installing Dependencies](#Dependencies)
- [Compiling](#Compiling)
- [Installation](#Installing)
- [Troubleshooting](#Troubleshooting)

<br>

> ### Note
> All command lines indicated by `$` are to be run as a normal user, while `#` requires root privilege, for example by prefacing it with `sudo` or executing through `sudo sh -c 'command'` or `su -c 'command'`.

## Packages
We provide packages for some distros on our [release page](https://github.com/Cockatrice/Cockatrice/releases). You can install these packages with your package manager or a tool like Eddy. Make sure the package name matches the specific version of your operating system.

> to install a downloaded package on a debian based system for example you can open a terminal at the location that you downloaded the package to and use `sudo apt install ./Cockatrice*.deb`

Some distros already include packages for cockatrice in their repositories, see [[Unofficial Developer Builds]].

<br>

## Dependencies
In order to compile Cockatrice we recommend to install the following tools. Instructions are split up for different package sources:

### Qt6
Newer distros will have both Qt6 and Qt5 available, we recommend using Qt6 if available. Alternatively check our requirements using [Qt5](#Qt5).

#### Arch Linux based systems using pacman
(Manjaro and EndeavourOS are based on Arch Linux)
```
# pacman --sync --needed \
    cmake \
    git \
    mariadb-libs \
    protobuf \
    qt6-base \
    qt6-multimedia \
    qt6-svg \
    qt6-tools \
    qt6-translations \
    qt6-websockets \
    zlib
```

#### Debian based systems using apt
(Ubuntu, Linux Mint, PopOS and KDE Neon are based on Debian)


Ubuntu 22.04 and 22.10, and their derivative distros
(Note that this DOES NOT work for Debian 11 or lower; use Qt5 or see note below.)
```
# apt install \
        build-essential \
        ccache \
        clang-format \
        cmake \
        file \
        g++ \
        git \
        libgl-dev \
        liblzma-dev \
        libmariadb-dev-compat \
        libprotobuf-dev \
        libqt6multimedia6 \
        libqt6sql6-mysql \
        libqt6svg6-dev \
        libqt6websockets6-dev \
        protobuf-compiler \
        qt6-l10n-tools \
        qt6-multimedia-dev \
        qt6-tools-dev \
        qt6-tools-dev-tools
```


Debian 12 and newer, Ubuntu 23.04 and newer, and their derivative distros
(Note that this _should_ also work for Debian 11 if you [enable Backports](https://backports.debian.org/Instructions/). Otherwise, use Qt5.)
```
# apt install \
        build-essential \
        ccache \
        clang-format \
        cmake \
        file \
        g++ \
        git \
        libgl-dev \
        liblzma-dev \
        libmariadb-dev-compat \
        libprotobuf-dev \
        libqt6multimedia6 \
        libqt6sql6-mysql \
        protobuf-compiler \
        qt6-l10n-tools \
        qt6-multimedia-dev \
        qt6-svg-dev \
        qt6-tools-dev \
        qt6-tools-dev-tools \
        qt6-websockets-dev
```

#### Fedora
(Fedora 36 and newer)
```
# dnf install \
    @development-tools \
    cmake \
    desktop-file-utils \
    gcc-c++ \
    hicolor-icon-theme \
    libappstream-glib \
    mariadb-devel \
    protobuf-devel \
    qt6-{qttools,qtsvg,qtmultimedia,qtwebsockets}-devel \
    wget \
    xz-devel \
    zlib-devel
```


#### Gentoo
```
# emerge -av \
    protobuf \
    qt{base,tools,svg,multimedia,websockets}:6
```

#### Qt6 Minimum Dependencies
- C++ compiler w/ C++17 support (eg. GCC 8 or Newer)
- CMake 3.16 or Newer
- Protobuf 3.0 or Newer
- Qt 6.2.3 or Newer with components `qtmultimedia` and `qtwebsockets`

@anchor compilation_linux_qt5
### Qt5
Qt5 is available on most distros and can be relied upon if the newer version isn't available.

#### Arch Linux based systems using pacman
(Manjaro and EndeavourOS are based on Arch Linux)
```
# pacman --sync --needed \
    cmake \
    git \
    mariadb-libs \
    protobuf \
    qt5-base \
    qt5-multimedia \
    qt5-svg \
    qt5-tools \
    qt5-websockets \
    zlib
```

#### Debian based systems using apt
(Ubuntu, Linux Mint, PopOS and KDE Neon are based on Debian)
```
# apt install \
    build-essential \
    cmake \
    g++ \
    git \
    liblzma-dev \
    libmysqlclient-dev \
    libprotobuf-dev \
    libqt5multimedia5-plugins \
    libqt5sql5-mysql \
    libqt5svg5-dev \
    libqt5websockets5-dev \
    protobuf-compiler \
    qt5-qmake \
    qtbase5-dev \
    qtbase5-dev-tools \
    qtchooser \
    qtmultimedia5-dev \
    qttools5-dev \
    qttools5-dev-tools
```

#### Fedora
```
# dnf install \
    @development-tools \
    cmake \
    desktop-file-utils \
    gcc-c++ \
    hicolor-icon-theme \
    libappstream-glib \
    protobuf-devel \
    qt5-{qttools,qtsvg,qtmultimedia,qtwebsockets}-devel \
    wget \
    {zlib,sqlite}-devel
```

#### FreeBSD
```
# pkg install \
    cmake \
    protobuf \
    qt5-buildtools \
    qt5-concurrent \
    qt5-network \
    qt5-qmake \
    qt5-websockets \
    qt5-widgets \
    qt5-gui \
    qt5-multimedia \
    qt5-printsupport \
    qt5-svg \
    qt5-linguisttools \
    qt5-sql \
    qt5-sqldrivers-mysql
```

#### Gentoo
```
# emerge -av \
    protobuf \
    qt{svg,multimedia,websockets,sql}:5
```

#### Sabayon
```
# equo i -av \
    dev-libs/protobuf \
    dev-qt/qt{core,multimedia,websockets,sql}:5 \
    dev-util/cmake \
    sys-devel/gcc
```

#### Void Linux
```
# xbps-install \
    cmake \
    liblzma \
    liblzma-devel \
    protobuf \
    protobuf-devel \
    qt5 \
    qt5-devel \
    qt5-multimedia \
    qt5-multimedia-devel \
    qt5-svg \
    qt5-svg-devel \
    qt5-tools \
    qt5-tools-devel \
    qt5-websockets \
    qt5-websockets-devel
```

#### Qt5 Minimum Dependencies
- C++ compiler w/ C++17 support (eg. GCC 8 or Newer)
- CMake 3.10 or Newer
- Protobuf 3.0 or Newer
- Qt 5.8.0 or Newer with components `websockets` and `multimedia`


Conditional requirements:
- mysql or mariadb (only for running a server database)
- if using mysql the qt sql connector for C++ is required
- liblzma (recommended for oracle)

> In case your os is not listed you can try to find these components yourself.
> Make sure the packages you install meet our version requirements.
> You might have to compile these components from their sources if no suitable
> packages are available, please refer to their respective documentation for
> information on how to do so.

<br>

## Compiling
Get a copy of Cockatrice's source code:
```
$ git clone https://github.com/Cockatrice/Cockatrice
$ cd Cockatrice
```

Create a directory to host the build process:
```
$ mkdir build
$ cd build
```

Run cmake to configure the build:
```
$ cmake ..
```

### CMake options
When configuring you can select multiple options, by default Cockatrice will only select the tools needed to run the client, in order to compile the server you should add the following option to the cmake command:
```
$ cmake .. -DWITH_SERVER=1
```

Alternatively, if you want to create a debug build as a developer or to troubleshoot issues:
```
$ cmake .. -DWITH_SERVER=1 -DCMAKE_BUILD_TYPE=Debug -DTEST=1
```

> We use googletest in our tests, if you enable them the components will be downloaded to your build dir, alternatively you can install googletest separately

In order to compile only the server you can exclude the client components:
```
cmake .. -DWITH_SERVER=1 -DWITH_CLIENT=0 -DWITH_ORACLE=0 -DWITH_DBCONVERTER=0
```

If you have a version of Qt6 installed but would rather use Qt5 you can use this flag:
```
cmake .. -DFORCE_USE_QT5=1
```

### Compiling
You can now start compiling, you can configure the amount of cpu cores used, the recommended amount is the total amount of threads available to the system minus one.
```
$ cmake --build . --parallel7
```

Alternatively you can use nice to have the compiler be nicer to your system resources:
```
$ nice make -j
```

## Installing

### Using Cpack
On debian or fedora based systems you can create a .deb or .rpm package like this:
```
$ make package
```

At last, install the package:
```
# dpkg -i ./Cockatrice*.deb  # for debian packages
# dnf --install ./Cockatrice*.rpm  # for rpm packages
```

### Other systems
Alternatively, you can install Cockatrice directly:
```
# make install
```
> Cockatrice does not provide an uninstall target, when running install cmake will create a file named `install_manifest.txt`, in order to completely uninstall you'll want to remove all files listed there [(issue ticket)](/Cockatrice/Cockatrice/issues/430)

<br>

## Troubleshooting

### Warnings in debug builds
If you're using an older version of protobuf you might have warnings in protobuf be caught as errors in your debug build, add this flag to cmake to prevent this from stopping the build: ` -DWARNING_AS_ERROR=0 `

### Error with shared libraries on Ubuntu
If you get the following error: `/usr/lib/qt5/bin/lrelease: error while loading shared libraries: libQt5Core.so.5: cannot open shared object file: No such file or directory`, you'll want to refer to [these instructions](https://github.com/dnschneid/crouton/wiki/Fix-error-while-loading-shared-libraries:-libQt5Core.so.5). The tl;dr version is to run the following command, replacing the file path with whatever path contains your libQt5Core.so.5 file:

```
sudo strip --remove-section=.note.ABI-tag /usr/lib64/libQt5Core.so.5
```

### Debug logging on Fedora
Some of the debug logging functionality (`qDebug()` output) of the Debug builds (`-DCMAKE_BUILD_TYPE=Debug`) may not work immediately with Fedora.  Qt logging is turned off system-wide, but can be enabled by creating a `qtlogging.ini` file at `~/.config/QtProject/qtlogging.ini`, and adding the following rules into it:
```
[Rules]
*.debug=true
qt.*.debug=false
```

[https://brendanwhitfield.wordpress.com/2016/06/08/enabling-qdebug-on-fedora/](https://brendanwhitfield.wordpress.com/2016/06/08/enabling-qdebug-on-fedora/)

[https://ask.fedoraproject.org/en/question/87522/qt-56-qdebug-and-qlog-no-longer-work/](https://ask.fedoraproject.org/en/question/87522/qt-56-qdebug-and-qlog-no-longer-work/)

### Memory shortages on Raspberry Pi
Due to the amount of memory that is required in order to compile the application you will also need to make sure you have at least 2GB of swap space configured.  If you are unfamiliar with how to increase your swap file size on the Raspberry Pi one decent article available is located at http://raspberrypimaker.com/adding-swap-to-the-raspberrypi.

The cards database has grown lately and Oracle might need more memory. So to be sure to correctly update your database, you need more swap on your Raspberry. Follow the following steps to use a dynamic swapfile that will grow when needed:

1- Edit the /etc/dphys-swapfile
```
$ sudo nano /etc/dphys-swapfile
```
2- Uncomment the **CONF_SWAPFILE=/var/swap** line.<br>
3- Uncomment the **CONF_SWAPFACTOR=2** line.<br>
4- Check that the **CONF_SWAPSIZE=100** line stays commented.<br>
5- Restart the swap process.
```
$ sudo /etc/init.d/dphys-swapfile restart
```

Once installation has completed, it is recommended to restart your Raspberry Pi.  This will allow for the X Windows manager to shutdown properly and upon restart add the Cockatrice application shortcut to the Games menu.

### Using Windows Subsystem for Linux
One final note: If you're building for Linux using WSL (Windows Subsystem for Linux) you'll need to install an X Server to be able to actually run the program. One option is [Xming](https://sourceforge.net/projects/xming/). Since Windows 11 this might not be necessary anymore.

<br>

> TODO:
> - trim down package requirements to minimum
> - should we provide instructions for centos?
> - instructions on how to make appimages?

<br>
