@page compilation_chromebooks Compiling for Chromebooks

[&nbsp; &larr; Back to Compiling Overview](@ref compilation_index)

## Installing Cockatrice

There are many ways to install Cockatrice on a Chromebook: via Flatpak, via pre-compiled .deb, and directly from source. No matter which method you choose, **you must first Enable Linux Apps** in order to have a Linux environment available to work in.

Please note that Cockatrice is not officially supported on ChromeOS and may behave unexpectedly or run poorly as a result. We are happy to help with issues to the best of our ability, but cannot guarantee solutions to all problems.

### Enabling Linux Apps

These steps may change! If they do not work for you, you can find the official instructions [here](https://support.google.com/chromebook/answer/9145439).

1. On your Chromebook, at the bottom right, select the time.
2. Select `Settings` > `About ChromeOS` >`Developers`.
3. Next to "Linux development environment," select `Set up`.
4. Follow the on-screen instructions. This can take some time.

### Installing via Flatpak

#### Setting up Flatpak

1. Open the Launcher, and select Terminal
2. Update your Linux environment:
    - `sudo apt update && sudo apt upgrade`
3. Install Flatpak
    - `sudo apt install flatpak`
4. Enable the Flathub repository
    - `flatpak --user remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo`
5. Fetch updated repo information
    - `flatpak update`

#### Installing and Launching Cockatrice

1. Install the Cockatrice flatpak
    - `flatpak --user install io.github.Cockatrice.cockatrice`
2. Open the Launcher, and select Cockatrice

#### Updating Cockatrice

A flatpak install of Cockatrice can be updated by running `flatpak update` in Terminal.

### Installing via deb Package

#### Note on Compatibility

As of this writing, the Linux environment used by ChromeOS is either Debian 11 (Bullseye) or Debian 12 (Bookworm). You can check which version of Debian your environment is by running `cat /etc/os-release` in Terminal. It should be listed in the VERSION field. If you have a higher version of Debian, you can still use these instructions, simply use our package for the higher version, if available. If you have a lower version of Debian, try to update to at least 11 using the instructions under "Fix Problems with Linux" [here](https://support.google.com/chromebook/answer/9145439?hl=en). If you are unable to find the right package for your system or unable to upgrade past Debian 10, please use the Flatpak instructions above or the Compile from Source instructions below.

#### Installation Instructions

1. Download the .deb file for the [latest release](https://github.com/Cockatrice/Cockatrice/releases/latest) that matches your version of Debian.
2. Open the Launcher, and select Terminal
3. Install Cockatrice
    - `sudo apt install {the full path to the downloaded file}`.
    - A common path is `MyFiles/Downloads/[name of the installation file]`, but please ascertain where the downloaded file ended up on your system and use the appropriate file path with `apt install`.
3. Open the Launcher, and select Cockatrice to open the program.
4. When a new version of Cockatrice is released, you will have to repeat these instructions to upgrade to the newer version. This **will not** delete any deck file, custom sets, or other local files.

### Installing from Source

To install Cockatrice from source, follow the steps to enable Linux apps, and then follow our [Linux compilation guide](@ref compilation_linux) using the [dependencies for Qt5 on Debian based systems](@ref compilation_linux_qt5).

## Updating your Cards List

Regardless of how you install Cockatrice, you will need to update your cards list regularly in order to have the most current cards and tokens to play with. The most straight forward way to update the list of cards available in Cockatrice is via `Help -> Check for card updates...`. This runs Oracle, which will download and parse the official list of cards and tokens. However, parsing the cards file takes a fair about of RAM (about 2.5 GB at the time of this writing), which is often too strenuous for Chromebooks or other computers with low amounts of RAM. If you run a check for card updates and your updater crashes, it is likely due to low RAM. There are a few things you can do to work around this issue (listed in the order that you should try them):

**Method 1: Free Up RAM.** Close every other program running on your computer (especially things like Chrome, Firefox, or Discord that can use a lot of RAM) and then try to run the check again.

**Method 2 Get a copy of the cards list manually from a friend.** You will need to get the files `cards.xml` and `tokens.xml` from a friend who is also using Cockatrice. Where these files should be located on each computer can be found by going to `Cockatrice > Settings` and looking for the `Card Database:` path in the `Paths` section. Copy `cards.xml` and `tokens.xml` from the appropriate folder on a friend's computer and place them in the corresponding folder on your computer (If there is a `.` before any of the folder names in the path, you may need to Show Hidden Files to be able to navigate there in a file explorer). This method avoids checking for card updates entirely and so avoids the problem of low RAM; however, the downside to this method is that you will need to repeat this process with each new set that comes out.

**Method 3 Get a copy of the cards list manually from a different download source.** This method requires Cockatrice version 2.10.0 or later (see `Help > About Cockatrice` if you are unsure of your version). You can configure Oracle to fetch the final, parsed version of cards.xml instead of needing to parse it locally on your computer and therefore use significantly less RAM. **Currently, this version of cards.xml is UNOFFICIAL and is not affiliated with the maintainers of Cockatrice. It is a copy of cards.xml from a personal installation of Cockatrice that is maintained by one person and may not be completely up to date. It is intended as a fallback measure if other methods fail.**

To use this version, run `Help > Check for card updates...` as normal and stop when you get to this screen:

![Screenshot from 2023-10-10 13-40-35](https://github.com/Cockatrice/Cockatrice/assets/71394296/ba4647e4-1157-4966-8380-35635c34b061)

Delete the URL in `Download URL:` and replace it with `https://github.com/SlightlyCircuitous/image-storage/raw/main/cards.zip` to point it to a zip file containing cards.xml:

![Screenshot from 2023-10-10 13-02-20](https://github.com/Cockatrice/Cockatrice/assets/71394296/656c861a-607c-43b8-82a7-d1681b8ccbc7)

Click `Next>` as normal and Oracle should fetch the files without crashing. It will move on to getting the tokens file, which should not need much RAM and so proceed normally. Once you have set this up, you can simply run `Help > Check for Card Updates` periodically to get cards from new sets. However, as stated, this version of the file is updated manually and may not update as frequently as the official version. If you want to go back to the official card source URL at any point, you can click `Restore default URL` in the updater dialogue.

If, for some reason, Oracle still will not update your cards when given the link to the zip file, you can follow the link in your browser, download and unzip the zip file yourself, and place it in the correct folder as detailed in Method 2 above. You can get [an official copy of the tokens file here](https://github.com/Cockatrice/Magic-Token) and place it in the same spot.
