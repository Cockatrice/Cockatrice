#ifndef CARDDATABASE_DATA_H
#define CARDDATABASE_DATA_H

#include <libcockatrice/card/card_info.h>

/**
 * @struct CardDatabaseData
 * @ingroup CardDatabase
 * @brief Value type holding the full parsed state of a card database.
 *
 * Parsers fill a CardDatabaseData off the UI thread; the finished snapshot is
 * then swapped into the live CardDatabase atomically. Keeping this separate from
 * the live container avoids emitting per-card change notifications and prevents
 * readers from observing a half-loaded database.
 */
struct CardDatabaseData
{
    CardNameMap cards;
    CardNameMap simpleNameCards;
    SetNameMap sets;
    FormatRulesNameMap formats;
};

#endif // CARDDATABASE_DATA_H
