@page compilation_windows Compiling for Windows

[&nbsp; &larr; Back to Compiling Overview](@ref compilation_index)

## Zach's New Guide (January 2023)

Looking to get setup on Windows? This guide should help you start with CLion! Windows is finicky and this guide should hopefully help you recreate whatever mess I ended up with that is fully functional.

### Software
- [Visual Studio 2022 Community Edition](https://visualstudio.microsoft.com/free-developer-offers/)
    - Select "Desktop Development with C++" and then tick all boxes in the Installation Details section (Windows 10/11 SDKs optional)
- [Jetbrains CLion 2022.3 (or newer)](https://www.jetbrains.com/clion/)
    - This is a paid software, but you can do this completely in Visual Studio, if necessary
- [Python 3.11 (or newer)](https://www.python.org/downloads/)
    - Download the installer
    - Tick `Add python.exe to PATH`
    - Custom Installation > Next
    - Tick all boxes
    - Install
- Qt and CMake via Python
    - ```
    python3 -m pip install aqtinstall cmake
    aqt install-qt windows desktop 6.6.3 msvc2019_64 -m all
    aqt install-tool windows desktop tools_opensslv3_x64
    ```
- [Git Bash 2.39.0 (or newer)](https://git-scm.com/download/win)
    - It's a good piece of software to have, regardless
- [Nullsoft Scriptable Install System (NSIS) 3.08 (or newer)](https://sourceforge.net/projects/nsis/files/latest/download)
    - This is how we can package our software for distribution, if necessary

### Development Environment Configurations
- Add Qt6 to your PATH Environment Variable
    - `Windows Start > Edit System Environment Variables > Environment variables... > PATH > Edit...`
        - Add the absolute path of your Qt6 binary directory
            - Ex: `C:\Users\Zach\Documents\Development\Qt\6.6.3\msvc2019_64\bin`
- Establish VCPKG repository
    - `Windows Start > Git Bash`
        - ```
      git clone https://github.com/Cockatrice/Cockatrice.git ~/Desktop/Cockatrice
      cd ~/Desktop/Cockatrice
      git submodule update --init   
      ```
- Setup CLion with the proper Toolchain and CMake profile
    - Open Cockatrice project in CLion
    - Set up the VS2022 Toolchain Configuration
        - `File > Settings > Build, Execution, Deployment > Toolchains > +`
        - Name: `Visual Studio 17 2022`
        - Toolset: `C:\Program Files\Microsoft Visual Studio\2022\Community`
    - Set up a CMake profile
        - `File > Settings > Build, Execution, Deployment > CMake > +`
            - Build type: `Release`
            - Toolchain: `Visual Studio 17 2022`
            - Generator: `Visual Studio 17 2022`
            - CMake options: `-DCMAKE_GENERATOR_PLATFORM="x64" -DWITH_SERVER=1`
            - Build options: `--parallel 8 -- -p:CL_MPcount=8 -p:UseMultiToolTask=true`
    - Modify CMake Application to build All Targets
        - `Run > Edit Configurations... > CMake Application > cockatrice > Target > All targets`

### Building and Testing
- Just run it in CLion now, it should (hopefully) work!

## Old Guide

The windows compiling instructions have been tested successfully using a Windows 7 SP1 and Windows 10 computer when using Visual Studio 2017 community editions.
The following instructions refers to building a 64-bit package using 64-bit libraries (the most common configuration on new pcs). If you want to build a 32-bit package instead, ensure to install the 32-bit libraries.  
As an additional reference, you can check out the `build-windows` section of the script we use to build the official packages on GitHub Actions: [desktop-build.yml](https://github.com/Cockatrice/Cockatrice/blob/master/.github/workflows/desktop-build.yml)

## Required Software
1. [Visual Studio 2017 Community Edition](https://go.microsoft.com/?linkid=9832280)

   > Workloads
   >   &rarr; Desktop development with C++ &rarr; Check all boxes under summary section (right side of window)

2. [Qt libraries](https://download.qt.io/official_releases/online_installers/)

   > Download the Windows installer, usually named qt-unified-windows-x86-online.exe
   > You only need to install **Qt &rarr; Qt5.x &rarr; msvc2017 64-bit**
   > Set an environmental variable of QTDIR to the path of your qt library e. g.  C:\Qt\5.12.11\msvc2017_64

3. [CMake](https://cmake.org/download/)
   > Download the Windows win32-x86 Installer, usually named cmake-3.x.x-win64-x64.msi
   > Choose to add CMake to the System path for all users.

4. [Git for Windows](https://github.com/git-for-windows/git/releases/)

   > Download the Windows installer, usually named Git-2.x.x-64-bit.exe
   > During installation select the defaults except for<br>
   > **Use Git from Git Bash only &rarr; Use Git from the Windows Command Prompt**<br>
   > **Use MinTTY &rarr; Use Windows default console window**

5.[Nullsoft Scriptable Install System (NSIS)](https://sourceforge.net/projects/nsis/files/latest/download)
> Download the latest installer version, usually named nsis-3.x-setup.exe

6. [Vcpkg](https://github.com/Microsoft/vcpkg)

   > Vcpkg it's a library manager and is the easiest way to install all the other needed dependencies; to install it, follow the official instructions, here reported for brevity; from a command prompt, run:

   ```bat
   $ cd \
   $ git clone https://github.com/Microsoft/vcpkg.git c:\vcpkg
   $ cd vcpkg
   ```

   > Then open a powershell prompt, enter the C:\vcpkg folder and run:

   ```ps
   PS> .\bootstrap-vcpkg.bat
   ```

   > Once vcpkg is installed while still in powershell, you can then run it to fetch and install the needed development libraries:

   ```bat
   $ vcpkg install openssl protobuf liblzma zlib
   ```

   > Note that in most cases, vcpkg defaults to installing x86 versions for packages. If you are intending to compile for x64, run the following command instead: (for any other architecture, call "vcpkg --triplet help")

   ```bat
   $ vcpkg --triplet x64-windows install openssl protobuf liblzma zlib
   ```

7. [MySql Database Connector](https://dev.mysql.com/downloads/installer/)
   > The Mysql Database connector is only needed by servatrice to support storing the configuration on a database.

## Compiling Cockatrice
01. Open Git CMD
02. Execute the following commands from inside the shell window:

```bat
$ git clone --recurse-submodules https://github.com/Cockatrice/Cockatrice C:\Cockatrice
$ mkdir C:\Cockatrice\build
$ cd C:\Cockatrice\build
$ cmake .. \
```
3. This will fail. Do not be alarmed. Open up the CMakeCache.txt file inside your git clone. change: these variables
```bat
WITH_SERVER:BOOL=OFF to ON
```
4. in CMD go in to the vcpkg folder of cockatrice and run this code to enter powershell
```bat
$powershell
.\bootstrap-vcpkg.bat
```
5. In PS run this line to install the libraries needed for this project.

```bat
 PS vcpkg --triplet x64-windows install openssl protobuf liblzma zlib
```
Exit PowerShell using exit.

6. On CMD run cmake .. \ This will fail again if you have activated the server, but you will now have the server variables loaded in to the cache. Now change the MYSQLCLIENT_LIBRARIES:FILEPATH= in the cache to the directory of your MYSQL Connector C++ libraries. Example Below
```bat 
MYSQLCLIENT_LIBRARIES:FILEPATH=C:\Program Files\MySQL\Connector C++ 8.0\lib64
```

If you get an error about missing libraries, make sure the paths those libraries are installed in match with the libraries in the above command. For example, you will probably have to replace `"C:/Qt/5.x/..."` with your actual version number.

07. Open VS2017 and open the Cockatrice.sln project (C:\Cockatrice\build)
08. Choose "Release" in the Solutions Configurations drop down along the top toolbar.
09. Right click and choose build for the following projects in the solutions explorer along the right.

        PACKAGE

If all goes well, each should compile successfully. Once completed there will be an executable installer file located under the `C:\Cockatrice\build` directory.

## Configuring Build Environment to run application from with-in Visual Studios

01. Open VS2017 and open the Cockatrice.sln project (C:\Cockatrice\build)
02. Choose "Release" in the Solutions Configurations drop down along the top toolbar.
    > Note: Compiling in Debug will require updating the vcpkg cmake path to point to the debug folder instead. Example: `C:\vcpkg\installed\x64-windows\debug\bin`
03. Right click and choose build for the following projects in the solutions explorer along the right.

        INSTALL

If all goes well, each should compile successfully.  Once completed there will be a folder under `C:\Cockatrice\build` directory named `Release` that contains all the files needed to execute the application.

04. Open Windows file explorer and select all the files under the `Release` folder described in step 03.
05. Copy the selected files to `C:\Cockatrice\build\cockatrice\Release` folder choose to "skip" any files that are trying to be overwritten.
06. Copy the selected files to `C:\Cockatrice\build\oracle\Release` folder choose to "skip" any files that are trying to be overwritten.
07. Copy the selected files to `C:\Cockatrice\build\servatrice\Release` folder choose to "skip" any files that are trying to be overwritten.

You should now be able to open the Cockatrice project in Visual Studio, right click on any of the 3 project solutions (cockatrice, oracle, or servatrice) and choose Debug &rarr; Start new instance and the application will now start from with-in Visual Studio allowing you to see various statistics and outputs.
