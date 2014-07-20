# Cockatrice

[![Build Status](https://travis-ci.org/Daenyth/Cockatrice.svg?branch=master)](https://travis-ci.org/Daenyth/Cockatrice)

Cockatrice is an open-source multiplatform software for playing card games,
such as Magic: The Gathering, over a network. It is fully client-server based
to prevent any kind of cheating, though it supports single-player games without
a network interface as well. Both client and server are written in Qt 4.

# License

Cockatrice is free software, licensed under the GPLv2; see COPYING for details.

# Building

Dependencies:

- [Qt](http://qt-project.org/) 
- [protobuf](http://code.google.com/p/protobuf/)
- [CMake](http://www.cmake.org/)

The server requires an additional dependency:

- [libgcrypt](http://www.gnu.org/software/libgcrypt/)

To compile:

    mkdir build
    cd build
    cmake ..
    make
    make install

The following flags can be passed to `cmake`:

- `-DWITH_SERVER=1` Build the server
- `-DWITHOUT_CLIENT=1` Do not build the client
- `-DWITH_QT4=1` Force compilation to use Qt4 instead of Qt5.

# Running

`oracle` fetches card data  
`cockatrice` is the game client  
`servatrice` is the server
