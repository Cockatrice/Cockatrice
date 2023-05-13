&nbsp; [Introduction](#contributing-to-cockatrice) | [Code Style Guide](
#code-style-guide) | [Translations](#translations) | [Release Management](
#release-management)

----

<br>

# Contributing to Cockatrice #
First off, thanks for taking the time to contribute to our project! 🎉 ❤ ️✨

The following is a set of guidelines for contributing to Cockatrice. These are
mostly guidelines, not rules. Use your best judgment, and feel free to propose
changes to this document in a pull request.


# Recommended Setups #

For those developers who like the Linux or MacOS environment, many of our
developers like working with a nifty program called [CLion](
https://www.jetbrains.com/clion/). The program's a great asset and one of the
best tools you'll find on these systems, but you're welcomed to use any IDE
you most enjoy.

Developers who like Windows development tend to find [Visual Studio](
https://www.visualstudio.com/) the best tool for the job.

[![Discord](https://img.shields.io/discord/314987288398659595?label=Discord&logo=discord&logoColor=white&color=7289da)](https://discord.gg/ZASRzKu)
[![Gitter Chat](https://img.shields.io/gitter/room/Cockatrice/Cockatrice.svg)](https://gitter.im/Cockatrice/Cockatrice)

If you'd like to ask questions, get advice, or just want to say hi,
the Cockatrice Development Team uses [Discord](https://discord.gg/ZASRzKu)
for communications in the #dev channel. If you're not into Discord, we also
have a [Gitter](https://gitter.im/Cockatrice/Cockatrice) channel available,
albeit slightly less active.


# Code Style Guide #

### Formatting and continuous integration (CI) ###

We use a separate job on the CI to check your code for formatting issues. If
your pull request failed the test, you can check the output on the checks tab.
It's the first job called "linter", you can click the "Check code formatting"
step to see the output of the test.

The message will look like this:
```
***********************************************************
***                                                     ***
***   Your code does not comply with our style guide.   ***
***                                                     ***
***   Please correct it or run the "format.sh" script.  ***
***  Then commit and push those changes to this branch. ***
***   Check our CONTRIBUTING.md file for more details.  ***
***                                                     ***
***                    Thank you ❤️                      ***
***                                                     ***
***********************************************************
```
The CONTRIBUTING.md file mentioned in that message is the file you are
currently reading. Please see [this section](#formatting) below for full
information on our formatting guidelines.

### Compatibility ###

Cockatrice is currently compiled on all platforms using <kbd>C++11</kbd>.
You'll notice <kbd>C++03</kbd> code throughout the codebase. Please feel free
to help convert it over!

For consistency, we use Qt data structures where possible. For example,
`QString` over `std::string` and `QList` over `std::vector`.

Do not use old C style casts in new code, instead use a [`static_cast<>`](
https://en.cppreference.com/w/cpp/language/static_cast)
or other appropriate conversion.

### Formatting ###

The handy tool `clang-format` can format your code for you, it is available for
almost any environment. A special `.clang-format` configuration file is
included in the project and is used to format your code.

We've also included a bash script, `format.sh`, that will use clang-format to
format all files in your pr in one go. Use `./format.sh --help` to show a full
help page.

To run clang-format on a single source file simply use the command
`clang-format -i <filename>` to format it in place. (Some systems install
clang-format with a specific version number appended,
`find /usr/bin -name clang-format*` should find it for you)

See [the clang-format documentation](
https://clang.llvm.org/docs/ClangFormat.html) for more information about the tool.

#### Header files ####

Use header files with the extension `.h` and source files with the extension
`.cpp`.

Use header guards in the form of `FILE_NAME_H`.

Simple functions, such as getters, may be written inline in the header file,
but other functions should be written in the source file.

Group project includes first, followed by library includes. All in alphabetic order.
Like this:
```c++
// Good
#include "card.h"
#include "deck.h"
#include <QList>
#include <QString>

// Bad
#include <QList>
#include "card.h"
#include <QString>
#include "deck.h"

// Bad
#include "card.h"
#include "deck.h"
#include <QString>
#include <QList>
```

#### Naming ####

Use `UpperCamelCase` for classes, structs, enums, etc. and `lowerCamelCase` for
function and variable names.

Don't use [Hungarian Notation](
https://en.wikipedia.org/wiki/Hungarian_notation).

Member variables aren't decorated in any way. Don't prefix or suffix them with
underscores, etc.

Use a separate line for each declaration, don't use a single line like this
`int one = 1, two = 2` and instead split them into two lines.

For arguments to constructors which have the same names as member variables,
prefix those arguments with underscores:
```c++
MyClass::MyClass(int _myData) : myData(_myData)
{

}
```
Pointers and references should be denoted with the `*` or `&` going with the
variable name:
```c++
// Good
Foo *foo1 = new Foo;
Foo &foo2 = *foo1;

// Bad
Bar* bar1 = new Bar;
Bar& bar2 = *bar1;
```
Use `nullptr` instead of `NULL` (or `0`) for null pointers.
If you find any usage of the old keywords, we encourage you to fix it.

#### Braces ####

Braces should go on their own line except for control statements, the use of
braces around single line statements is preferred.
See the following example:
```c++
int main()
{                                     // function or class: own line
    if (someCondition) {              // control statement: same line
        doSomething();                // single line statement, braces preferred
    } else if (someOtherCondition1) { // else goes on the same line as a closing brace
        for (int i = 0; i < 100; i++) {
            doSomethingElse();
        }
    } else {
        while (someOtherCondition2) {
            doSomethingElse();
        }
    }
}
```

#### Indentation and Spacing ####

Always indent using 4 spaces, do not use tabs. Opening and closing braces
should be on the same indentation layer, member access specifiers in classes or
structs should not be indented.

All operators and braces should be separated by spaces, do not add a space next
to the inside of a brace.

If multiple lines of code that follow eachother have single line comments
behind them, place all of them on the same indentation level. This indentation
level should be equal to the longest line of code for each of these comments,
without added spacing.

#### Lines ####

Do not leave trailing whitespace on any line. Most IDEs check for this
nowadays and clean it up for you.

Lines should be 120 characters or less. Please break up lines that are too long
into smaller parts, for example at spaces or after opening a brace.

### Memory Management ###

New code should be written using references over pointers and stack allocation
over heap allocation wherever possible.
```c++
// Good: uses stack allocation and references
void showCard(const Card &card);
int main()
{
    Card card;
    showCard(card);
}

// Bad: relies on manual memory management and doesn't give us much
// null-safety.
void showCard(const Card *card);
int main()
{
    Card *card = new Card;
    showCard(card);
    delete card;
}
```
(Remember to pass by `const` reference wherever possible, to avoid accidentally
mutating objects.)

When pointers can't be avoided, try to use a smart pointer of some sort, such
as `QScopedPointer`, or, less preferably, `QSharedPointer`.

### Database migrations ###

The servatrice database's schema can be found at `servatrice/servatrice.sql`.
Everytime the schema gets modified, some other steps are due:
 1. Increment the value of `cockatrice_schema_version` in `servatrice.sql`;
 2. Increment the value of `DATABASE_SCHEMA_VERSION` in
 `servatrice_database_interface.h` accordingly;
 3. Create a new migration file inside the `servatrice/migrations` directory
 named after the new schema version.
 4. Run the `servatrice/check_schema_version.sh` script to ensure everything is
 fine.

The migration file should include the sql statements needed to migrate the
database schema and data from the previous to the new version, and an
additional statement that updates `cockatrice_schema_version` to the correct
value.

Ensure that the migration produces the expected effects; e.g. if you add a
new column, make sure the migration places it in the same order as
servatrice.sql.

### Protocol buffer ###

Cockatrice and Servatrice exchange data using binary messages. The syntax of
these messages is defined in the `proto` files in the `common/pb` folder. These
files define the way data contained in each message is serialized using
Google's [protocol buffers](https://developers.google.com/protocol-buffers/).
Any change to the `proto` files should be taken with caution and tested
intensively before being merged, because a change to the protocol could make
new clients incompatible to the old server and vice versa.

You can find more information on how we use Protobuf on [our wiki!](
https://github.com/Cockatrice/Cockatrice/wiki/Client-server-protocol)

# Reviewing Pull Requests #

After you have finished your changes to the project you should put them on a
separate branch of your fork on GitHub and open a [pull request](
https://docs.github.com/en/free-pro-team@latest/desktop/contributing-and-collaborating-using-github-desktop/creating-an-issue-or-pull-request
).
Your code will then be automatically compiled by GitHub actions for Linux and
macOS, and by Appveyor for Windows. Additionally GitHub will perform a [Linting
check](#formatting-and-continuous-integration-ci). If any issues come up you
can check their status at the bottom of the pull request page, click on details
to go to the CI website and see the different build logs.

If your pull request passes our tests and has no merge conflicts, it will be
reviewed by our team members. You can then address any requested changes. When
all changes have been approved your pull request will be squashed into a single
commit and merged into the master branch by a team member. Your change will then
be included in the next release 👍

# Translations #

Basic workflow for translations:
 1. Developer adds a `tr("foo")` string in the code;
 2. CI updates the `*_en@source.ts files` regularly and creates a PR automatically;
 3. Maintainer verifies and merges the change;
 4. Transifex picks up the new files from GitHub automatically;
 5. Translators translate the new untranslated strings on Transifex;
 6. Before a release, a maintainer fetches the updated translations from Transifex.

### Using Translations (for developers) ###

All user interface strings inside Cockatrice's source code must be written
in English (US).
Translations to other languages are managed using [Transifex](
https://www.transifex.com/projects/p/cockatrice/).

Adding a new string to translate is as easy as adding the string in the
`tr("")` function, the string will be picked up as translatable automatically
and translated as needed.
For example, setting the text of a label in the way that the string
`"My name is:"` can be translated:
```c++
nameLabel.setText(tr("My name is:"));
```

To translate a string that would have plural forms you can add the amount to
the tr() call, also you can add an extra string as a hint for translators:
```c++
QString message = tr("Everyone draws %n cards", "pop up message", amount);
```
See [QT's wiki on translations](
https://doc.qt.io/qt-5/i18n-source-translation.html#handling-plurals)

If you're about to propose a change that adds or modifies any translatable
string in the code, you don't need to take care of adding the new strings to
the translation files.<br>
We have an automated process to update our language source files on a schedule
and provide the translators on Transifex with the new contents.<br>
Maintainers can also manually trigger this on demand.

### Maintaining Translations (for maintainers) ###

When new translatable strings have been added to the code, a maintainer has to
make them available to translators on Transifex.

To help with that, we have an automated CI workflow, that regularly looks at the
code in the master branch, extracts all strings and updates dedicated source string
files with any changes. These updates are not commited right away, the CI creates a
PR for reviewing instead.<br>
After approval, our translation tool automatically picks the changes up and deploys
them to our translators. Be mindful when merging only a few changes!

Once a release is planned, or when a lot of strings have been added or changed, a
maintainer can manually trigger a CI run to extract all strings on demand.

<details>
<summary><b>Manually trigger CI run (Workflow Dispatch)</b></summary>

Maintainers can always request the CI to run on demand if it's required.

Go to the `Actions` tab and select our dedicated translation workflow:
https://github.com/Cockatrice/Cockatrice/actions/workflows/translations.yml

You see a "This workflow has a workflow_dispatch event trigger." hint at the top of
the list.<br>
Select `Run workflow` on the right and trigger a run from master branch.

The CI will now check for changed strings and create a PR if there are any updates.

</details>

<details>
<summary><b>Manually update source strings locally</b></summary>

To update the english source files for translation, re-run cmake enabling the appropriate
parameter and then run make:
```sh
cd cockatrice/build
cmake .. -DUPDATE_TRANSLATIONS=ON
make
```
If the parameter has been enabled correctly, when running "make" you should see
a line similar to this one (the numbers may vary):
```sh
[ 76%] Generating ../../cockatrice/translations/cockatrice_en@source.ts
Updating '../../cockatrice/translations/cockatrice_en@source.ts'...
Found 857 source text(s) (8 new and 849 already existing)
```
You should then notice that the following files have uncommitted changes:

    cockatrice/translations/cockatrice_en@source.ts
    oracle/translations/oracle_en@source.ts

It is recommended to disable the parameter afterwards using:
```sh
cmake .. -DUPDATE_TRANSLATIONS=OFF
```
Now you are ready to commit your changes and open a PR.

</details>

Once the changes get merged, Transifex will pick up the modified files
automatically (checked every few hours) and update their online editor where
translators will be able to translate the new strings right in the browser.

### Releasing Translations (for maintainers) ###

Before rushing out a new release, a maintainer should fetch the most up to date
translations from Transifex and commit them into the Cockatrice source code.
This can be done manually from the Transifex web interface, but it's quite time
consuming.

As an alternative, you can install the Transifex CLI:

    http://docs.transifex.com/developer/client/

You'll then be able to use a git-like cli command to push and pull translations
from Transifex to the source code and vice versa.

### Adding Translations (for translators) ###

As a translator you can help translate the new strings on [Transifex](
https://www.transifex.com/projects/p/cockatrice/).
Please have a look at the specific [FAQ for translators](
https://github.com/Cockatrice/Cockatrice/wiki/Translation-FAQ).


# Release Management #

### Publishing A New Release ###

We use [GitHub Releases](https://github.com/Cockatrice/Cockatrice/releases) to
publish new stable versions and betas.
Whenever a git tag is pushed to the repository github will create a draft
release and upload binaries automatically.

To create a tag, simply do the following:
```bash
git checkout master
git remote update -p
git pull
git tag $TAG_NAME
git push $UPSTREAM $TAG_NAME
```

You should define the variables as such:
```
`$UPSTREAM` - the remote for git@github.com:Cockatrice/Cockatrice.git
`$TAG_NAME` should be formatted as:
  - `YYYY-MM-DD-Release-MAJ.MIN.PATCH` for **stable releases**
  - `YYYY-MM-DD-Development-MAJ.MIN.PATCH-beta.X` for **beta releases**<br>
    With *MAJ.MIN.PATCH* being the NEXT release version!
```

This will cause a tagged release to be established on the GitHub repository,
with the binaries being added to the release whenever they are ready.
The release is initially a draft, where the path notes can be edited and after
all is good and ready it can be published on GitHub.
If you use a SemVer tag including "beta", the release that will be created at
GitHub will be marked as a "Pre-release" automatically.
The target of the `.../latest` URL will not be changed in that case, it always
points to the latest stable release and not pre-releases/betas.

If you accidentally push a tag incorrectly (the tag is outdated, you didn't
pull in the latest branch accidentally, you named the tag wrong, etc.) you can
revoke the tag by doing the following:
```bash
git push --delete upstream $TAG_NAME
git tag -d $TAG_NAME
```
You can also do this on GitHub, you'll also want to delete the new release.

In the first lines of [CMakeLists.txt](
https://github.com/Cockatrice/Cockatrice/blob/master/CMakeLists.txt)
there's an hardcoded version number and pretty name used when compiling from
master or custom (not tagged) versions.
While on tagged versions these numbers are overridden by the version numbers
coming from the tag title, it's good practice to increment the ones at CMake
after every full release to stress that master is ahead of the last stable
release.
The preferred flow of operation is:
 * Just before a release, make sure the version number in CMakeLists.txt is set
 to the same release version you are about to tag.
 * This is also the time to change the pretty name in CMakeLists.txt called
 GIT_TAG_RELEASENAME and commit and push these changes.
 * Tag the release following the previously described syntax in order to get it
 correctly built and deployed by CI.
 * Wait for the configure step to create the release and update the patch
 notes.
 * Check on the github actions page for build progress which should be the top
 listed [here](
 https://github.com/Cockatrice/Cockatrice/actions?query=event%3Apush+-branch%3Amaster
 ).
 * When the build has been completed you can verify all uploaded files on the
 release are in order and hit the publish button.
 * After the release is complete, update the CMake version number again to the
 next targeted beta version, typically increasing `PROJECT_VERSION_PATCH` by
 one.

When releasing a new stable version, all previous beta releases (and tags)
should be deleted. This is needed for Cockatrice to update users of the "Beta"
release channel correctly to the latest stable version as well.
This can be done the same way as revoking tags, mentioned above.
