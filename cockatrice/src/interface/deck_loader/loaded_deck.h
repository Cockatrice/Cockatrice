#ifndef COCKATRICE_LOADED_DECK_H
#define COCKATRICE_LOADED_DECK_H
#include "libcockatrice/deck_list/deck_list.h"

#include <QString>

/**
 * @brief Represents a deck that was loaded from somewhere.
 * Contains the DeckList itself, as well as info about where it was loaded from.
 */
struct LoadedDeck
{
    enum FileFormat
    {
        PlainTextFormat,
        CockatriceFormat
    };

    /**
     * Determines what deck file format the given filename corresponds to.
     *
     * @param fileName The filename
     * @return The deck format
     */
    static FileFormat getFormatFromName(const QString &fileName);

    /**
     * @brief Information about where the deck was loaded from.
     *
     * For local decks, the remoteDeckId field will always be -1.
     * For remote decks, fileName will be empty and fileFormat will always be CockatriceFormat
     */
    struct LoadInfo
    {
        static constexpr int NON_REMOTE_ID = -1;

        QString fileName = "";
        FileFormat fileFormat = CockatriceFormat;
        int remoteDeckId = NON_REMOTE_ID;

        bool isEmpty() const;
    };

    DeckList deckList;     ///< The decklist itself
    LoadInfo lastLoadInfo; ///< info about where the deck was loaded from

    bool isEmpty() const;
};

#endif // COCKATRICE_LOADED_DECK_H
