&nbsp; [Introduction](#contributing-to-cockatrice) | [Code Style Guide](#code-style-guide) | [Translations](#translations) | [Release Management](#release-management)

----

<br>

# Contributing to Cockatrice #
First off, thanks for taking the time to contribute to our project! 🎉 ❤ ️✨

The following is a set of guidelines for contributing to Cockatrice. These are mostly guidelines, not rules. Use your best judgment, and feel free to propose changes to this document in a pull request.


# Recommended Setups #

For those developers who like the Linux or MacOS environment, many of our developers like working with a nifty program called [CLion](https://www.jetbrains.com/clion/). The program's a great asset and one of the best tools you'll find on these systems, but you're welcomed to use any IDE you most enjoy.

Developers who like Windows development tend to find [Visual Studio](https://www.visualstudio.com/) the best tool for the job.

If you have any questions on IDEs, feel free to chat with us on [Gitter](https://gitter.im/Cockatrice/Cockatrice) and we would love to help answer your questions!  


# Code Style Guide #

### Compatibility ###

Cockatrice is currently compiled on all platforms using <kbd>C++11</kbd>. You'll notice <kbd>C++03</kbd> code throughout the codebase. Please feel free to help convert it over!

For consistency, we use Qt data structures where possible. For example, `QString` over
`std::string` and `QList` over `std::vector`.

### Header files ###

Use header files with the extension `.h` and source files with the extension
`.cpp`.

Use header guards in the form of `FILE_NAME_H`.

Simple functions, such as getters, may be written inline in the header file,
but other functions should be written in the source file.

Keep library includes and project includes grouped together. So this is okay:
```c++
// Good
#include <QList>
#include <QString>
#include "card.h"
#include "deck.h"

// Good
#include "card.h"
#include "deck.h"
#include <QList>
#include <QString>

// Bad:
#include <QList>
#include "card.h"
#include <QString>
#include "deck.h"
```

### Naming ###

Use `UpperCamelCase` for classes, structs, enums, etc. and `lowerCamelCase` for
function and variable names.

Member variables aren't decorated in any way. Don't prefix or suffix with
underscores, etc.

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

### Braces ###

Braces should almost always go on their own line:
```c++
int main()
{
    if (someCondition)
    {
        doSomething();
    }
    else if (someOtherCondition1)
    {
        for (int i = 0; i < 100; i++)
        {
            doSomethingElse();
        }
    }
    else
    {
        while (someOtherCondition2)
        {
            doSomethingElse();
        }
    }
}
```
Braces should never be omitted for single-statement. Keeping the code legibile is a high priority of ours and we hope you share a similar belief :)

### Tabs vs Spaces ###

We _highly_ encourate the use of spaces. If you use tabs, please readjust them to 4 spaces per tab before submitting.

### Lines ###

Do not have trailing whitespace in your lines, if possible. Most IDEs check for this nowadays and clean it up for you.

Lines should be 120 characters or less, but you can exceed this if you find it necessary.

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
 2. Increment the value of `DATABASE_SCHEMA_VERSION` in `servatrice_database_interface.h` accordingly;
 3. Create a new migration file inside the `servatrice/migrations` directory named after the new schema version.
 4. Run the `servatrice/check_schema_version.sh` script to ensure everything is fine.

The migration file should include the sql statements needed to migrate the database schema and data from the previous to the new version, and an additional statement that updates `cockatrice_schema_version` to the correct value.

Ensure that the migration produces the expected effects; e.g. if you add a new column, make sure the migration places it in the same order as servatrice.sql.

### Protocol buffer ###

Cockatrice and Servatrice exchange data using binary messages. The syntax of these messages is defined in the `proto` files in the `common/pb` folder. These files defines the way data contained in each message is serialized using Google's [protocol buffer](https://developers.google.com/protocol-buffers/).
Any change to the `proto` file should be taken with caution and tested intensively before being merged, becaus a change to the protocol could make new clients incompatible to the old server and vice versa.

You can find more information on how we use Protobuf on [our wiki!](https://github.com/Cockatrice/Cockatrice/wiki/Client-server-protocol)


# Translations #

**Basic workflow for translations:**
 1. Developer adds a `tr("foo")` string in the code;
 2. Every few days, a maintainer updates the `*_en.ts files` with the new strings;
 3. Transifex picks up the new files from github every 24 hours;
 4. Translators translate the new untraslated strings on Transifex;
 5. Before a release, a maintainer fetches the updated translations from Transifex.

### Translations (for developers) ###

**Step 1: Adding translatable strings to the code (`tr("foo")`)**

All the user-interface strings inside Cockatrice's source code must be written in
english language.<br>
Translations to other languages are managed using [Transifex](https://www.transifex.com/projects/p/cockatrice/).

If you're about to propose a change that adds or modifies any translatable string
in the code, you don't need to take care of adding the new strings to the
translation files. Every few days, or when a lot of new strings have been added,
someone from the development team will take care of extracing all the new strings,
adding them to the english translation files and making them available to
translators on Transifex.

### Translations (for maintainers) ###

**Step 2: Updating `*_en.ts` files with new strings**

When new translatable strings have been added to the code, it would be nice to
make them available to translators on Transifex. Every few days, or when a lot
of new strings have been added, a maintainer should take care of extracing all
the new strings and add them to the english translation files.

To update the english translation files, re-run cmake enabling the appropriate
parameter and then run make:
```sh
cd cockatrice/build
cmake .. -DUPDATE_TRANSLATIONS=ON
make
```
If the parameter has been enabled correctly, when running "make" you should see
a line similar to this one (the numbers may vary):
```sh
[ 76%] Generating ../../cockatrice/translations/cockatrice_en.ts
Updating '../../cockatrice/translations/cockatrice_en.ts'...
Found 857 source text(s) (8 new and 849 already existing)
```
You should then notice that the following files have uncommitted changes:

    cockatrice/translations/cockatrice_en.ts
    oracle/translations/oracle_en.ts

It's now suggested to disable the parameter using:
```sh
cmake .. -DUPDATE_TRANSLATIONS=OFF
```
Now you are ready to propose your change.

**Step 3: Automatic pushing to Transifex**

Once your change gets merged, Transifex will pick up the modified files automatically (checks every 24 hours)
and update the interface where translators will be able to translate the new strings.

**Step 5: Fetching new translations from Transifex**

Before rushing out a new release, it would be nice to fetch the most up to date
translations from Transifex and commit them into the Cockatrice source code.
This can be done manually from the Transifex web interface, but it's quite time
consuming.

As an alternative, you can install the Transifex CLI:

    http://docs.transifex.com/developer/client/

You'll then be able to use a git-like cli command to push and pull translations
from Transifex to the source code and vice versa.

### Translations (for translators) ###

**Step 4: Editing translations at Transifex**

Please have a look at the specific [FAQ for translators](https://github.com/Cockatrice/Cockatrice/wiki/Translation-FAQ).


# Release Management #

### Publishing A New Beta Release ###

Travis and AppVeyor have been configured to upload files to GitHub Releases whenever a <kbd>tag</kbd> is pushed.<br>
Usually, tags are created through publishing a (pre-)release, but there's a way around that.

To trigger Travis and AppVeyor, simply do the following:
```bash
cd $COCKATRICE_REPO
git checkout master
git remote update -p
git pull
git tag $TAG_NAME
git push upstream $TAG_NAME
```
You should define the variables as such:
```
upstream - git@github.com:Cockatrice/Cockatrice.git
$COCKATRICE_REPO - /Location/of/repository/cockatrice.git
`$TAG_NAME` should be:
  - `YYYY-MM-DD-Release-MAJ.MIN.PATCH` for **stable releases**
  - `YYYY-MM-DD-Development-MAJ.MIN.PATCH-beta.X` for **beta releases**<br>
    With *MAJ.MIN.PATCH* being the NEXT release version!
```

This will cause a tagged release to be established on the GitHub repository, which will then lead to the upload of binaries. If you use this method, the tags (releases) that you create will be marked as a "Pre-release". The `/latest` URL will not be impacted (for stable release downloads) so that's good.

If you accidentally push a tag incorrectly (the tag is outdated, you didn't pull in the latest branch accidentally, you named the tag wrong, etc.) you can revoke the tag by doing the following:
```bash
git push --delete upstream $TAG_NAME
git tag -d $TAG_NAME
```

**NOTE:** Unfortunately, due to the method of how Travis and AppVeyor work, to publish a stable release you will need to make a copy of the release notes locally and then paste them into the GitHub GUI once the binaries have been uploaded by them. These CI services will automatically overwrite the name of the release (to "Cockatrice $TAG_NAME"), the status of the release (to "Pre-release"), and the release body (to "Beta build of Cockatrice").

**NOTE 2:** In the first lines of https://github.com/Cockatrice/Cockatrice/blob/master/CMakeLists.txt there's an hardcoded version number used when compiling custom (not tagged) versions. While on tagged versions these numbers are overriden by the version numbers coming from the tag title, it's a good practice to keep them aligned with the real ones.
The preferred flow of operations is:
 * just before a release, update the version number in CMakeLists.txt to "next release version";
 * tag the release following the previously described syntax in order to get it built by CI;
 * wait for CI to upload the binaries, double check if everything is in order
 * after the release is complete, update the version number again to "next targeted beta version", typically increasing `PROJECT_VERSION_PATCH` by one.

**NOTE 3:** When releasing a new stable version, all the previous beta versions should be deleted. This is needed for Cockatrice to pick up the stable release also for users that chose the "beta" release channel.
