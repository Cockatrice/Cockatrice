/**
 * @file deck_loader.h
 * @ingroup ImportExport
 * @brief TODO: Document this.
 */

#ifndef DECK_LOADER_H
#define DECK_LOADER_H

#include <QLoggingCategory>
#include <QPrinter>
#include <QTextCursor>
#include <libcockatrice/deck_list/deck_list.h>

inline Q_LOGGING_CATEGORY(DeckLoaderLog, "deck_loader")

    class DeckLoader : public QObject
{
    Q_OBJECT
signals:
    void deckLoaded();
    void loadFinished(bool success);

public:
    enum FileFormat
    {
        PlainTextFormat,
        CockatriceFormat
    };

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
    };

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
    DeckList *deckList;
    LoadInfo lastLoadInfo;

public:
    DeckLoader(QObject *parent);
    DeckLoader(QObject *parent, DeckList *_deckList);
    DeckLoader(const DeckLoader &) = delete;
    DeckLoader &operator=(const DeckLoader &) = delete;

    const LoadInfo &getLastLoadInfo() const
    {
        return lastLoadInfo;
    }

    void setLastLoadInfo(const LoadInfo &info)
    {
        lastLoadInfo = info;
    }

    [[nodiscard]] bool hasNotBeenLoaded() const
    {
        return lastLoadInfo.fileName.isEmpty() && lastLoadInfo.remoteDeckId == LoadInfo::NON_REMOTE_ID;
    }

    static void clearSetNamesAndNumbers(const DeckList *deckList);
    static FileFormat getFormatFromName(const QString &fileName);

    bool loadFromFile(const QString &fileName, FileFormat fmt, bool userRequest = false);
    bool loadFromFileAsync(const QString &fileName, FileFormat fmt, bool userRequest);
    bool loadFromRemote(const QString &nativeString, int remoteDeckId);
    bool saveToFile(const QString &fileName, FileFormat fmt);
    bool updateLastLoadedTimestamp(const QString &fileName, FileFormat fmt);

    static QString exportDeckToDecklist(const DeckList *deckList, DecklistWebsite website);

    static void setProviderIdToPreferredPrinting(const DeckList *deckList);
    static void resolveSetNameAndNumberToProviderID(const DeckList *deckList);

    static void saveToClipboard(const DeckList *deckList, bool addComments = true, bool addSetNameAndNumber = true);
    static bool saveToStream_Plain(QTextStream &out,
                                   const DeckList *deckList,
                                   bool addComments = true,
                                   bool addSetNameAndNumber = true);

    /**
     * @brief Prints the decklist to the provided QPrinter.
     * @param printer The printer to render the decklist to.
     * @param deckList
     */
    static void printDeckList(QPrinter *printer, const DeckList *deckList);

    bool convertToCockatriceFormat(QString fileName);

    DeckList *getDeckList()
    {
        return deckList;
    }

private:
    static void printDeckListNode(QTextCursor *cursor, InnerDecklistNode *node);
    static void saveToStream_DeckHeader(QTextStream &out, const DeckList *deckList);

    static void saveToStream_DeckZone(QTextStream &out,
                                      const InnerDecklistNode *zoneNode,
                                      bool addComments = true,
                                      bool addSetNameAndNumber = true);
    static void saveToStream_DeckZoneCards(QTextStream &out,
                                           const InnerDecklistNode *zoneNode,
                                           QList<DecklistCardNode *> cards,
                                           bool addComments = true,
                                           bool addSetNameAndNumber = true);

    [[nodiscard]] static QString getCardZoneFromName(const QString &cardName, QString currentZoneName);
    [[nodiscard]] static QString getCompleteCardName(const QString &cardName);
};

#endif
