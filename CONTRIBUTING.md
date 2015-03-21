# Style Guide #

### Compatibility ###

Cockatrice is written in C++-03, so do not use C++11 constructs such as `auto`.

Cockatrice support both Qt 4 and Qt 5, so make sure that your code compiles
with both. If there have been breaking changes between Qt 4 and 5, use a
statement such as

    #if QT_VERSION >= 0x500000
        doSomethingWithQt5();
    #else
        doSomethingWithQt4();
    #endif

For consistency, use Qt data structures where possible, such as `QString` over
`std::string` or `QList` over `std::vector`.

### Header files ###

Use header files with the extension `.h` and source files with the extension
`.cpp`.

Use header guards in the form of `FILE_NAME_H`.

Simple functions, such as getters, may be written inline in the header file,
but other functions should be written in the source file.

Keep library includes and project includes grouped together. So this is okay:

    // Good:
    #include <QList>
    #include <QString>
    #include "card.h"
    #include "deck.h"

    // Good:
    #include "card.h"
    #include "deck.h"
    #include <QList>
    #include <QString>

    // Bad:
    #include <QList>
    #include "card.h"
    #include <QString>
    #include "deck.h"

### Naming ###

Use `UpperCamelCase` for classes, structs, enums, etc. and `lowerCamelCase` for
function and variable names.

Member variables aren't decorated in any way. Don't prefix or suffix with
underscores, etc.

For arguments to constructors which have the same names as member variables,
prefix those arguments with underscores:

    MyClass::MyClass(int _myData)
        : myData(_myData)
    {}

Pointers and references should be denoted with the `*` or `&` going with the
variable name:

    // Good:
    Foo *foo1 = new Foo;
    Foo &foo2 = *foo1;

    // Bad:
    Bar* bar1 = new Bar;
    Bar& bar2 = *bar1;

Use `0` instead of `NULL` (or `nullptr`) for null pointers.

### Braces ###

Use K&R-style braces. Braces for function implementations go on their own
lines, but they go on the same line everywhere else:

    int main()
    {
        if (someCondition) {
            doSomething();
        } else {
            while (someOtherCondition) {
                doSomethingElse();
            }
        }
    }

Braces can be omitted for single-statement if's and the like, as long as it is
still legible.

### Tabs ###

Use only spaces. Four spaces per tab.

### Lines ###

Do not have trailing whitespace in your lines.

Lines should be 80 characters or less, as a soft limit.

### Memory Management ###

New code should be written using references over pointers and stack allocation
over heap allocation wherever possible.

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

(Remember to pass by `const` reference wherever possible, to avoid accidentally
mutating objects.)

When pointers can't be avoided, try to use a smart pointer of some sort, such
as `QScopedPointer`, or, less preferably, `QSharedPointer`.

### Translations: introduction ###

Basic workflow for translations:
 1. developer adds a tr("foo") string in the code;
 2. every few days, a mantainer updates the *_en.ts files adding the new strings;
 3. Trasifex picks up the new files from github every 24 hours;
 4. translators translate the new untraslated strings on Transifex;
 5. before a release, a mantainer fetches the updated translations from Transifex.

### Translations (for developers) ###

All the user-interface strings inside Cockatrice's source code must be written in
english language. Translations to other languages are managed using [Transifex](https://www.transifex.com/projects/p/cockatrice/).

If you're about to propose a change that adds or modifies any translatable string
in the code, you don't need to take care of adding the new strings to the
translation files. Every few days, or when a lot of new strings have been added, 
someone from the development team will take care of extracing all the new strings,
adding them to the english translation file and making them available to
translators on Transifex.

### Translations (for mantainers) ###

#### step 2: updating *_en.ts files ####

When new translatable strings have been added to the code, it would be nice to
make them available to translators on Transifex. Every few days, or when a lot
of new strings have been added, a mantainer should take care of extracing all
the new strings and add them to the english translation.

To update the english translation files, re-run cmake enabling the appropriate
parameter and then run make:

    cd cockatrice/build
    cmake .. -DUPDATE_TRANSLATIONS=ON
    make

If the parameter has been enabled correctly, when running "make" you should see
a line similar to this one (the numbers may vary):

    [ 76%] Generating ../../cockatrice/translations/cockatrice_en.ts
    Updating '../../cockatrice/translations/cockatrice_en.ts'...
    Found 857 source text(s) (8 new and 849 already existing)

You should then notice that the following files have uncommitted changes:

    cockatrice/translations/cockatrice_en.ts
    oracle/translations/oracle_en.ts

It's now suggested to disable the parameter using:

    cmake .. -DUPDATE_TRANSLATIONS=OFF

Now you are ready to propose your change. Once your change gets merged, 
Transifex will pick up the modified files automatically (checks every 24 hours)
and update the interface where translators will be able to translate the new
strings.

#### step 5: fetch new translations ####

Before rushing out a new release, it would be nice to fetch the most up to date
translations from Transifex and commit them into the Cockatrice source code.
This can be done manually from the Transifex web interface, but it's quite time
consuming.

As an alternative, you can install the Transifex CLI: 
    
    http://docs.transifex.com/developer/client/

You'll then be able to use a git-like cli command to push and pull translations
from Transifex to the source code and vice versa.

### Translations (for translators) ###

Please have a look at the specific [FAQ for translators](https://github.com/Cockatrice/Cockatrice/wiki/Translation-FAQ).
