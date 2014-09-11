#TODOs

This is an unordered list of possible todo items for Cockatrice.
Note that "improve" and "write" always also means: "document and comment"

##Docs
* Improve README.md, it is very minimalistic and technical

##Improve packaging:
* Add SSL libraries to Windows build to support https links in oracle.exe
* Improve nsis file git hash extraction, it only works if the build directory is cleared as version_string.cpp does not seem to get updated by git pull/cmake
* Fix prepareMacRelease.sh to use default installation paths for Qt and protobuf
* Add Mac compile+package howto to the documentation
* Create script/... for creating Linux packages (deb, rpm, ebuild, ...) or at least an official tarball/git tags; package maintainers dislike using git snapshots so much that they rather ignore software without stable tarballs.
* Move everything package related into one directory

##Scripts
* Write example init script for servatrice.

##Webinterface
* Write a default cgi webinterface in some language for the servatrice database. Keep it simple and think about SQL-injections and other security risks - especially if you do it in PHP :-P

##Improve usermanual:
* Improve Layout, Text, Screenshots; nearly everything is still quick n dirty copy and paste.
* Add missing TODOs
* Translate the Documentation
* Include faq.txt and shortcuts.txt in usermanual, then remove them.
* Rename the picture filenames to something more meaningful.
* Create an index, lists of tables/figures/...

## Storage
* Find a better place for sets.xml than doc.

##Create developer documentation:
* Create developer manual
* Add comments to code
* Describe which components exist and how they work and interact
* Describe the *.proto files
* Comment and document servatrice.sql
* Document everything!1!!
* Coding guidelines

##Else

* Update SFMT library (http://www.math.sci.hiroshima-u.ac.jp/~m-mat@math.sci.hiroshima-u.ac.jp/MT/SFMT/) in common/sfmt and adapt common/rng_sfmt.cpp

* Prepare to update to Qt5, as Qt4 will die some day. See http://qt-project.org/wiki/Transition_from_Qt_4.x_to_Qt5 for relevant information.

* Move hardcoded URLs (especially from oracle and cockatrice) into a config file.

* Update the oracle to find all MtG extensions by itself, why the need for sets.xml when the oracle could ask the Gatherer or magiccards.info directly for this information?

* Update the oracle with some intelligence or improve it otherwise to deal with changes from the Gatherer.

* Remove cockatrice/resources/mtgforum.png as it is unused. Find other dead files or dead references to websites and files in the program code and documentation.

* Search git log for useful information/problems/bugs/...
