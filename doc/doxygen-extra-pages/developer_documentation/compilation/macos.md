@page compilation_macos Compiling for MacOS

[&nbsp; &larr; Back to Compiling Overview](@ref compilation_index)

# Newbie Intro

Hi! If you're new to Terminal and perhaps here just to get a version of Cockatrice you can run on an older macOS, let me explain a few things quickly.
* **Terminal** is an application pre-installed on all Macs that allows deeper control of your operating system (OS) through the use of written commands. It can be dangerous to use if you don't know what you're doing, but all the commands here are safe and do not involve any deleting of data so don't worry. You can copy and paste the commands from this page and press enter to use them, or type them very carefully.
* **Homebrew** is a package manager. Basically, Cockatrice needs a few different packages or "dependencies" in order to function (e.g. one called "Qt" for you to actually be able have menus and click around). Homebrew is a easy way to collect all these packages so you can shove them all together at the end. By installing Homebrew, you will have access to more commands in Terminal; it won't appear in Applications or be openable like other apps you may have installed in the past.
* In this case and in a grossly simplified way, **Compiling** just means taking our packages and source code, shoving it together (with a compiler), then getting our Cockatrice apps at the end. Apps can also be referred to as "executables/exes" or "binaries".

> **Disclaimer and requirements**: There is no guarantee the instructions below will work for older OS versions, you may get errors and get stuck. You should try to install the newest version of macOS that your computer is compatible with incase of errors. You may need a bit of free hard drive space to install all the packages and tools, but the packages won't add up to much at the end (2gb including Xcode?). If you're worried about clogging your computer with all these packages don't worry, they will be only accessible through Terminal or hidden folders and won't effect performance. If you're looking to compile a version quickly, unfortunately it can take several hours to a day to install everything and compile if you're on an old OS, have no packages installed beforehand, and maybe run into some errors on the way. If at some point you decide to uninstall the dependencies and/or homebrew, there are uninstall guides on Google and your built version of Cockatrice shouldn't be affected.



## Compiling

1) To compile Cockatrice on macOS you need to first **install the dependencies manager**, [Homebrew](http://brew.sh/).

2) Install the dependencies, this may take a long time.
    - Just Cockatrice: **`brew install cmake protobuf qt@5 git`**
        - _User Comment_: If you are compiling on a version of macOS that is older than what Cockatrice currently supports, you will likely run into errors installing these packages as Homebrew won't support it either. For High Sierra at least, I used the command Homebrew recommends i.e. **`brew install --build-from-source [the package you want]`** and used it for protobuf, qt5, git, and their respective dependency packages you'll be told about when trying to install them. Also, if you're not able to automatically download Xcode Command Line Tools when prompted, go [here](https://developer.apple.com/download/more/) (you will need an Apple account) and download the newest version that supports your OS. For High Sierra, "Command Line Tools (macOS 10.13) for Xcode 10.1" worked fine for me. Expect all these packages to take several hours to install in total and an 1hr+ build time in step 5.
    - Cockatrice & Servatrice: **`brew install cmake protobuf qt@5 git libgcrypt --with-mysql`**
        - Note that this requires compiling mysql, which can take a long time. It may be easier to use the servatrice docker container instead.



3) Download the Cockatrice source code:
```bash
cd ~
git clone https://github.com/Cockatrice/Cockatrice
cd Cockatrice
```

4) Determine the location of Qt on your system.
    - **`ls /usr/local/Cellar/qt@5/`** will give you a number (5.9.2, 5.12.2, etc.)

5) Make the installer and open it
```bash
mkdir build
cd build
QT5_DIR="/usr/local/Cellar/qt@5/NUMBER_FOUND_IN_STEP_4"
cmake .. -DWITH_SERVER=0 -DCMAKE_PREFIX_PATH=$QT5_DIR/  # Use -DWITH_SERVER=1 to include servatrice.
make -j
make package
open Cockatrice *.dmg
```

You can then install the application from the dmg as normal. If you want to share the dmg around, you can find it in ~/Users/username/Cockatrice/build/.

- Note: The created dmg and it's binaries should be compatible with all versions of MacOS higher than the minimum supported OS of the version of cmake you used.

# Advanced
## About using Qt's official packages
If you plan to use Qt's official packages from https://www.qt.io/download-open-source/ to build servatrice, you'll get an error when building the package. This is due to the fact that the Qt's Mysql plugin is built with an hardcoded path that needs to be fixed depending on where MySql / MariaDB has been installed on your box.

Example: if you installed MariaDB using Homebrew, you can fix the plugin running this command:
```bash
install_name_tool -change \
    /opt/local/lib/mysql55/mysql/libmysqlclient.18.dylib \
    /usr/local/lib/libmysqlclient.18.dylib \
    /Users/foo/Qt/5.5/clang_64/plugins/sqldrivers/libqsqlmysql.dylib 
```

This command will fix the `qsqlmysql` plugin pointing it from the old, hardcoded path in `/opt/` to the correct path in `/usr/local/`.