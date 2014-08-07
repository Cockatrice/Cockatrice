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
