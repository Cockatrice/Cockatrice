@page querying_the_card_database Querying the Card Database

# The CardDatabaseQuerier Class

The CardDatabaseQuerier is the only class used for querying the database. The CardDatabase is an in-memory map and thus
provides no structured query language. CardDatabaseQuerier offers methods to retrieve cards by name, by providerID or in
bulk, as CardSet%s.

## Obtaining a handle to the CardDatabaseQuerier for usage

To obtain the CardDatabaseQuerier related to the global CardDatabase singleton, use CardDatabaseManager::query().

## Querying for known cards

There are, essentially, two ways to ensure card equality, with the second being optional but necessitating the first.
These two ways are CardInfo name equality and PrintingInfo provider ID equality.

Because of this, most queries require, at the very least, a card name to match against and optionally a providerID to
narrow the results.

### Generic Card Infos

To check if a card with the exact provided name exists as a CardInfo in the CardDatabase use,
CardDatabaseQuerier::getCardInfo() or CardDatabaseQuerier::getCardInfos() for multiple cards.

### Guessing Cards

If the exact name might not be present in the CardDatabase, you can use CardDatabaseQuerier::getCardBySimpleName(),
which automatically simplifies the card name and matches it against simplified card names in the CardDatabase.

Alternatively, you can use CardDatabaseQuerier::lookupCardByName(), which first attempts an exact match search and then
uses CardDatabaseQuerier::getCardBySimpleName() as a fallback.

### ExactCard%s

To obtain an ExactCard from the CardDatabaseQuerier, you must use a CardRef as a parameter to
CardDatabaseQuerier::getCard(), CardDatabaseQuerier::getCards(), or CardDatabaseQuerier::guessCard().

CardRef is a simple struct consisting of a card name and a card provider ID as QString%s.