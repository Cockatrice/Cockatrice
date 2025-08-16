<!-- this template comes from .ci/release_template.md -->

<!-- Don't forget to delete the previous betas after publishing this!
git push -d origin --REPLACE-WITH-BETA-LIST--
 -->

<!-- This list of binaries should be updated every time the CI is changed to include all targets -->
<pre>
Available pre-compiled binaries for installation:

  <b>Windows</b>
   • <kbd>Windows 10+</kbd>
   • <kbd>Windows 7+</kbd>

  <b>macOS</b>
   • <kbd>macOS 15+</kbd> <sub><i>Sequoia</i></sub> <sub>Apple M</sub>
   • <kbd>macOS 14+</kbd> <sub><i>Sonoma</i></sub> <sub>Apple M</sub>
   • <kbd>macOS 13+</kbd> <sub><i>Ventura</i></sub> <sub>Intel</sub>

  <b>Linux</b>
   • <kbd>Ubuntu 24.04 LTS</kbd> <sub><i>Noble Numbat</i></sub>
   • <kbd>Ubuntu 22.04 LTS</kbd> <sub><i>Jammy Jellyfish</i></sub>
   • <kbd>Debian 13</kbd> <sub><i>Trixie</i></sub>
   • <kbd>Debian 12</kbd> <sub><i>Bookworm</i></sub>
   • <kbd>Debian 11</kbd> <sub><i>Bullseye</i></sub>
   • <kbd>Fedora 42</kbd>
   • <kbd>Fedora 41</kbd>

<sub>We are also packaged in <kbd>Arch Linux</kbd>'s <a href="https://archlinux.org/packages/extra/x86_64/cockatrice">official extra repository</a>, courtesy of @FFY00.</sub>
<sub>General Linux support is available via a <kbd>flatpak</kbd> package at <a href="https://flathub.org/apps/io.github.Cockatrice.cockatrice">Flathub</a>!</sub>
</pre>


## General Notes

We're pleased to announce the newest official release: <kbd>--REPLACE-WITH-RELEASE-TITLE--</kbd>

We hope you enjoy the changes made! All improvements with their corresponding tickets since the last version of Cockatrice are listed in the changelog below.

If you ever encounter a bug, have a suggestion or idea, or feel a need for a developer to look into something, please feel free to [open a ticket](https://github.com/Cockatrice/Cockatrice/issues). ([How to create a Ticket for Cockatrice](https://github.com/Cockatrice/Cockatrice/wiki/How-to-Create-a-GitHub-Ticket-Regarding-Cockatrice))

For basic information related to the app and getting started, please take a look at our official site: **https://cockatrice.github.io**

If you'd like to help and contribute to Cockatrice in any way, check out our [README](https://github.com/Cockatrice/Cockatrice#contribute).  
We're always available to answer questions you may have on how the program works and how you can provide a meaningful contribution.


## Upgrading Cockatrice
<!-- this optional section puts a warning banner for problems with updating
> [!IMPORTANT]  
> **With this release, we no longer provide a ready-to-install binary for:**  
> --DEPRECATED-OS-HERE--
 -->

Run the internal software updater: <kbd>Help → Check for Client Updates</kbd>

Don't forget to update your card database right after! (<kbd>Help → Check for Card Updates...</kbd>)


## Changelog
<!--
This list is generated and should be moved to their respective header and
possibly edited a little.
Append PR numbers of fixups to their main PR to keep the list coherent.
Put the quantity of remaining PR's below the highlights section.
Remove empty headers when done.

--REPLACE-WITH-GENERATED-LIST--
 -->

<!-- Highlights of the release -->
### 🔖 Highlights:
### ✨ New Features:
### 🐛 Fixed Bugs / Resolved Issues:

<!-- Complete list of changes (foldable) -->
<details>
<summary>
<b>Show all changes</b> (--REPLACE-WITH-COMMIT-COUNT-- commits)
</summary>

### User Interface
### Under the Hood
### Oracle
### Servatrice
### Webatrice

</details>


## Translations
- **Thanks for over 300 people contributing to 20+ different languages up to now!**
- Without the help of the community we couldn't offer that great language support... keep up the good work!
- It's actually very easy to join and help for yourself - find out more here:
    - [Help us Translate Cockatrice into your native language!](https://github.com/Cockatrice/Cockatrice/wiki/Translation-FAQ)


## Special Thanks
<!-- Personalise this a bit! -->
It's amazing that so many people contribute their time, knowledge, code, testing and more to the project.  
We'd like to thank the entire Cockatrice community for their efforts! 🙏
<!-- We'd like to especially recognize @ZeldaZach, --ADD-CONTRIBUTORS-HERE-- for their help in preparing so many amazing new features for the user base. -->
