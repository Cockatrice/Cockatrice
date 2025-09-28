@page compilation_android Compiling for Android

[&nbsp; &larr; Back to Compiling Overview](@ref compilation_index)

Hello everybody, this is probably the first thing I've ever contributed to the internet, so here goes.
Please excuse my lack of knowledge on how to format all this. (EDITed 9/25/21)

To start with you will need:
* A device running Android 7 or later.  What matters is that you have ARMhf/x86/ARM64 (all I had to test on....)(update:def works on Samsung Galaxy Tab A also, Android 9)
* Active WiFi connection
* Between 1.5 - 3GB free space to play with; you're gonna need it. Plus space to save the images for your cards
* Patience....this will take the better part of 2 or 4 hours depending on your device CPU.
* The ability to copy commands without typos into a Linux terminal (JK)

Here's how this works!
Android is essentially Linux on an ARM device.  So there's been plenty of development in the ARM space
for what we need, which isn't much really, just some repositories and a bit of code and we'll have you
running cockatrice on your smartphone in no time.

To start, download Termux, a terminal emulator or shell with which we will run our linux box on our device.
Then download your favorite VNC viewer of choice from the Play Store(UPDATE: Termux available through the F-Droid store, look it up.  Dont get the one from the PLAY STORE)
***
There are multiple ways to do the next step;
Next, you need to get Linux on your device any way you like it; for me, I used AndroNix (free from the play store) to download Debian.  This is where you have the freedom to choose what distro you are familiar with, but as a proof of concept, I used Debian.  AndroNix basically copies the necessary text to a clipboard, which you then run inside Termux, thus downloading the basic Debian FileSystem (about 50MB).  For me, at 1MB/s it took 2 mins or so.  It then decompresses the .tar.xz file into a useable file system.

For me, the next step was to startup this Linux via './start-debian.sh'(EDIT: your name to startup linux may vary wildly.  A simple LS command will show your startup scripts)

If all goes well, you are greeted with the bash prompt 'root@localhost:~#'
Now the nitty-gritty.  We need a bunch of things to make this work, but a GUI would be nice so I used LXDE, again copying code from AndroNix.  It downloads everything it needs (over 400 MB for LXDE) for a GUI and at 1MB/s it takes about 15 minutes.


Pay attention during your install. You do not want it to timeout on something simple like keyboard selection.
At some point you will be prompted to input your keyboard settings, this can be region specific. I put 1 for US default.
It will then "Get" a bunch of stuff downloaded and unpacked
Eventually you will get to this screen saying, you can run a vnc server.  You put a password for the connection to the server, which we will use a VNC client for.


Before we see what we've done, we need to download the cockatrice source code so we can compile it on our device.

# Installs necessary packages, clones the repo, makes the build directory, cmakes, compiles, installs on Debian 10
# Run the following:
* sudo apt-get update
* sudo apt-get upgrade
* sudo apt-get install  build-essential cmake git qt5-default qtbase5-dev qttools5-dev-tools qtmultimedia5-dev libqt5websockets5 libqt5websockets5-dev libqt5svg5-dev protobuf-compiler libprotoc17 libprotobuf17 libprotobuf-dev

You can leave out the lzma-dev bit since i tried that and it didnt add lzma support, but there is a way to get it I just cant remember how I got it. You will get an error message during compile but Cockatrice works for me without it. Search for lzma packages for your distro, and get the relevant -dev ones

<a href="https://app.photobucket.com/u/vindicate7/p/7752652b-37e3-4bc8-89e8-1485a7589127" target="_blank"> <img src="https://hosting.photobucket.com/images/ac357/vindicate7/0/2020-03-18%2001_12.png" border="0" alt="2020-03-18 01_12"/> </a>


Hold your breath.  LOL not really cause this takes like 20 mins.  But this next part, where we build Cockatrice from the ground up for our device, can take well over an hour; go ahead and start making dinner at this point.

* git clone git://github.com/Cockatrice/Cockatrice
* cd Cockatrice
* mkdir build
* cd build
* cmake -DWITH_SERVER=0 -DWITH_CLIENT=1 -DWITH_ORACLE=1 -DWITH_DBCONVERTER=1 ..
* make
  <a href="https://app.photobucket.com/u/vindicate7/p/ea472184-21f8-4109-ac12-a1b2abc87113" target="_blank"> <img src="https://hosting.photobucket.com/images/ac357/vindicate7/0/2020-03-18%2002_04.png" border="0" alt="2020-03-18 02_04"/> </a>

* make install

Once you do that, it's all downhill from here bby.  At the bash prompt type
* vncserver-start (if your vnc server isnt already up)
  Now, you can switch to your VNC app you downloaded earlier.  Use 127.0.0.1:5901 as your host or whatnot.  Basically port 5901 is your connection to this Termux
***

<a href="https://app.photobucket.com/u/vindicate7/p/262ee328-239c-4bfe-88b9-5222b9d1ef7e" target="_blank"> <img src="https://hosting.photobucket.com/images/ac357/vindicate7/0/vnc.png" border="0" alt="vnc"/> </a>
<a href="https://app.photobucket.com/u/vindicate7/p/7db8493c-25d5-4438-9c27-219bb7f33b33" target="_blank"> <img src="https://hosting.photobucket.com/images/ac357/vindicate7/0/debian%20screen.png" border="0" alt="debian screen"/> </a>


***
When running Oracle for the first time, try the AllSets.json.xz like recommended.  If your device can parse that file, good for you.  On my device, I had to shorten the list to ModernPrintings.json myself since A)I don't play anything older than modern and B)My app would crash with the AllSets.  Your mileage may vary.  Essentially, checkout https://mtgjson.com/downloads/all-files/ and see what set you want to import into Oracle.  Maybe you just want standard, or pioneer.  There's a JSON for that.  Oh I think here you may have to get the .zip version if you have no lzma support. At any rate it works so well.

In VNC Viewer for Android, a right click is with two fingers tap the screen. To click and drag, tap then tap-hold and drag.

Now that I've taken you this far, I think you should have no problem connecting to a server and playing; albeit it takes some getting used to.

You're welcome.  Feel free to email me at dslman@gmail.com