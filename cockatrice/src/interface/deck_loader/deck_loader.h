/**
 * @file deck_loader.h
 * @ingroup ImportExport
 * @brief TODO: Document this.
 */

#ifndef DECK_LOADER_H
#define DECK_LOADER_H

#include "loaded_deck.h"

#include <QLoggingCategory>
#include <QPrinter>
#include <QTextCursor>
#include <libcockatrice/deck_list/deck_list.h>

inline Q_LOGGING_CATEGORY(DeckLoaderLog, "deck_loader");

class DeckLoader : public QObject
{
    Q_OBJECT
signals:
    void deckLoaded();
    void loadFinished(bool success);

public:
    /**
     * Supported file extensions for decklist files
     */
    static const QStringList ACCEPTED_FILE_EXTENSIONS;

    /**
     * For use with `QFileDialog::setNameFilters`
     */
    static const QStringList FILE_NAME_FILTERS;

    enum DecklistWebsite
    {
        DecklistOrg,
        DecklistXyz
    };

private:
    LoadedDeck loadedDeck;

public:
    DeckLoader(QObject *parent);
    DeckLoader(const DeckLoader &) = delete;
    DeckLoader &operator=(const DeckLoader &) = delete;

    [[nodiscard]] bool hasNotBeenLoaded() const
    {
        return loadedDeck.lastLoadInfo.isEmpty();
    }

    bool loadFromFile(const QString &fileName, DeckFileFormat::Format fmt, bool userRequest = false);
    bool loadFromFileAsync(const QString &fileName, DeckFileFormat::Format fmt, bool userRequest);
    bool loadFromRemote(const QString &nativeString, int remoteDeckId);
    bool saveToFile(const QString &fileName, DeckFileFormat::Format fmt);
    bool updateLastLoadedTimestamp(const QString &fileName, DeckFileFormat::Format fmt);

    static QString exportDeckToDecklist(const DeckList &deckList, DecklistWebsite website);

    static void saveToClipboard(const DeckList &deckList, bool addComments = true, bool addSetNameAndNumber = true);
    static bool saveToStream_Plain(QTextStream &out,
                                   const DeckList &deckList,
                                   bool addComments = true,
                                   bool addSetNameAndNumber = true);

    /**
     * @brief Prints the decklist to the provided QPrinter.
     * @param printer The printer to render the decklist to.
     * @param deckList
     */
    static void printDeckList(QPrinter *printer, const DeckList &deckList);

    bool convertToCockatriceFormat(const QString &fileName);

    LoadedDeck &getDeck()
    {
        return loadedDeck;
    }
    const LoadedDeck &getDeck() const
    {
        return loadedDeck;
    }
    void setDeck(const LoadedDeck &deck)
    {
        loadedDeck = deck;
    }

private:
    static void printDeckListNode(QTextCursor *cursor, const InnerDecklistNode *node);
    static void saveToStream_DeckHeader(QTextStream &out, const DeckList &deckList);

    static void saveToStream_DeckZone(QTextStream &out,
                                      const InnerDecklistNode *zoneNode,
                                      bool addComments = true,
                                      bool addSetNameAndNumber = true);
    static void saveToStream_DeckZoneCards(QTextStream &out,
                                           const InnerDecklistNode *zoneNode,
                                           QList<DecklistCardNode *> cards,
                                           bool addComments = true,
                                           bool addSetNameAndNumber = true);
};

#endif
