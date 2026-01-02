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

    /**
     * @brief Asynchronously loads a deck from a local file into this DeckLoader.
     * The `loadFinished` signal will be emitted when the load finishes.
     * Once the loading finishes, the deck can be accessed with `getDeck`
     * @param fileName The file to load
     * @param fmt The format of the file to load
     * @param userRequest Whether the load was manually requested by the user, instead of being done in the background.
     */
    void loadFromFileAsync(const QString &fileName, DeckFileFormat::Format fmt, bool userRequest);

    /**
     * @brief Loads a deck from a local file.
     * @param fileName The file to load
     * @param fmt The format of the file to load
     * @param userRequest Whether the load was manually requested by the user, instead of being done in the background.
     * @return An optional containing the LoadedDeck, or empty if the load failed.
     */
    static std::optional<LoadedDeck>
    loadFromFile(const QString &fileName, DeckFileFormat::Format fmt, bool userRequest = false);

    /**
     * @brief Loads a deck from the response of a remote deck request
     * @param nativeString The deck string, in cod format
     * @param remoteDeckId The remote deck id
     * @return An optional containing the LoadedDeck, or empty if the load failed.
     */
    static std::optional<LoadedDeck> loadFromRemote(const QString &nativeString, int remoteDeckId);

    /**
     * @brief Saves a DeckList to a local file.
     * @param deck The DeckList
     * @param fileName The file to write to
     * @param fmt The deck file format to use
     * @return An optional containing the LoadInfo for the new file, or empty if the save failed.
     */
    static std::optional<LoadedDeck::LoadInfo>
    saveToFile(const DeckList &deck, const QString &fileName, DeckFileFormat::Format fmt);

    /**
     * @brief Saves a LoadedDeck a local file.
     * Uses the lastLoadInfo in the LoadedDeck to determine where to save to.
     * @param deck The LoadedDeck to save. Should have valid lastLoadInfo.
     * @return Whether the save succeeded.
     */
    static bool saveToFile(const LoadedDeck &deck);

    /**
     * @brief Saves a LoadedDeck to a new local file.
     * @param deck The LoadedDeck to save. Will update the lastLoadInfo.
     * @param fileName The file to write to
     * @param fmt The deck file format to use
     * @return Whether the save succeeded.
     */
    static bool saveToNewFile(LoadedDeck &deck, const QString &fileName, DeckFileFormat::Format fmt);

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

    /**
     * Converts the given deck's file to the cockatrice file format, using the lastLoadInfo to determine the current
     * file format and where to write to.
     * @param deck The deck to convert. Should have valid lastLoadInfo. Will update the lastLoadInfo.
     * @return Whether the conversion succeeded.
     */
    static bool convertToCockatriceFormat(LoadedDeck &deck);

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
    static bool updateLastLoadedTimestamp(LoadedDeck &deck);
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
