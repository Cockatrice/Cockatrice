@page primer_cards A Primer on Cards

# The Cockatrice Card Library

All non-gui code related to cards used by Cockatrice is contained within libcockatrice_card.

# A Basic Card Object: CardInfo

A CardInfo object is the translational unit used by the CardDatabase to represent an on-disk XML entry as an in-memory
Qt object.

For a complete overview of the fields that define a CardInfo object, see the class documentation and more specifically
@ref PrivateCardProperties.

These fields correspond to either entries or a combination of entries in the XML card entry to which the
CardDatabaseParser applies special logic to map, populate or determine their respective values in the CardInfo object.

Any property to which special parsing is not applied is stored in CardInfo::properties and may be retrieved or set with
CardInfo::getProperty() or CardInfo::setProperty() as well as CardInfo::getProperties() to get a collection of the
property keys a CardInfo contains.

For more information on how special fields contained within @ref PrivateCardProperties are parsed, see
CockatriceXml4Parser::loadCardsFromXml().

Apart from access to the basic and extended properties of a card, CardInfo also provides two important mechanisms to us.
The first is the CardInfo::pixmapUpdated() signal, which allows the CardPictureLoader to signal when it is done
manipulating (in most cases: loading) the QPixmap of an ExactCard::getPixmapCacheKey(), where the ExactCard::card
corresponds to this CardInfo object.

Put simply: Whenever the CardPictureLoader loads a new PrintingInfo which is not already in the cache, it emits this
signal so that any widget using a CardInfo object can update the display of it to possibly use this new QPixmap.

\attention It should be noted that it is not possible to use CardPictureLoader::getPixmap() with anything but an
ExactCard, which is a CardInfo object associated with a PrintingInfo, i.e. a definitive printing of a specific card

The other signal, CardInfo::cardInfoChanged() conceptually works much the same way, except it is concerned with the
properties of the CardInfo object.

# Getting specific: PrintingInfo and ExactCard

## Printing Info

A CardInfo object describes the basic properties of a card in much the same way that we say "Two cards with the same
name are and should be equal if their properties are equal". However, there are certain (mostly visual) properties which
might differ between printings of a card but still conceptually classify two instances of a card as "the same card".
The most obvious example to this are cards which fundamentally share all properties except the artwork depicted on the
card.

We refer to such differences as Printings and track them in the PrintingInfo class. A PrintingInfo is always related to
the CardSet that introduced the variation. Multiple variations can exist in the same CardSet and the respective
PrintingInfo::getProperty() can be used to determine the differences, such as the collector numbers on the printings, if
there are any.

## Exact Card

A CardInfo object is used to hold the functional properties of a card and a PrintingInfo object can be used to hold the
visual properties of a card. To represent a 'physical' card, as in, a concrete immutable instance of a card, we can use
ExactCard, which combines a CardInfo and a PrintingInfo object into one class. The class is used as a container around
CardInfo objects whenever the user (and thus the program) expects to be presented with a defined card printing.

# Using Cards

For more information on the XML database schema which the CardDatabaseParser parses to generate CardInfo objects, see
@ref card_database_schema_and_parsing.

For more information on querying the CardDatabase for CardInfo objects, see @ref querying_the_card_database.

For more information on displaying CardInfo and ExactCard objects using Qt Widgets, see @ref displaying_cards.

For more information on how card pictures are loaded from disk or the network, see @ref loading_card_pictures as well as
the CardPictureLoader documentation.