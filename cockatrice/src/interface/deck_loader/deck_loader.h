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
    DeckList deckList;
    QString lastFileName;
    FileFormat lastFileFormat;
    int lastRemoteDeckId;

public:
    DeckLoader(QObject *parent);
    DeckLoader(QObject *parent, const DeckList *_deckList);
    DeckLoader(const DeckLoader &) = delete;
    DeckLoader &operator=(const DeckLoader &) = delete;

    const QString &getLastFileName() const
    {
        return lastFileName;
    }
    void setLastFileName(const QString &_lastFileName)
    {
        lastFileName = _lastFileName;
    }
    FileFormat getLastFileFormat() const
    {
        return lastFileFormat;
    }
    int getLastRemoteDeckId() const
    {
        return lastRemoteDeckId;
    }

    bool hasNotBeenLoaded() const
    {
        return getLastFileName().isEmpty() && getLastRemoteDeckId() == -1;
    }

    void setName(const QString &_name = QString())
    {
        deckList.setName(_name);
    }

    void setComments(const QString &_comments = QString())
    {
        deckList.setComments(_comments);
    }
    void setTags(const QStringList &_tags = QStringList())
    {
        deckList.setTags(_tags);
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

    /// @name Serialization (XML)
    ///@{
    bool loadFromXml(QXmlStreamReader *xml)
    {
        return deckList.loadFromXml(xml);
    };
    bool loadFromString_Native(const QString &nativeString)
    {
        return deckList.loadFromString_Native(nativeString);
    };
    QString writeToString_Native() const
    {
        return deckList.writeToString_Native();
    };
    bool loadFromFile_Native(QIODevice *device)
    {
        return deckList.loadFromFile_Native(device);
    };
    bool saveToFile_Native(QIODevice *device)
    {
        return deckList.saveToFile_Native(device);
    };
    ///@}

    /// @name Serialization (Plain text)
    ///@{
    bool loadFromStream_Plain(QTextStream &stream, bool preserveMetadata)
    {
        return deckList.loadFromStream_Plain(stream, preserveMetadata);
    };
    bool loadFromFile_Plain(QIODevice *device)
    {
        return deckList.loadFromFile_Plain(device);
    };
    bool saveToStream_Plain(QTextStream &stream, bool prefixSideboardCards, bool slashTappedOutSplitCards)
    {
        return deckList.saveToStream_Plain(stream, prefixSideboardCards, slashTappedOutSplitCards);
    };
    bool saveToFile_Plain(QIODevice *device, bool prefixSideboardCards = true, bool slashTappedOutSplitCards = false)
    {
        return deckList.saveToFile_Plain(device, prefixSideboardCards, slashTappedOutSplitCards);
    };
    QString writeToString_Plain(bool prefixSideboardCards = true, bool slashTappedOutSplitCards = false)
    {
        return deckList.writeToString_Plain(prefixSideboardCards, slashTappedOutSplitCards);
    };
    ///@}

    /**
     * @brief Prints the decklist to the provided QPrinter.
     * @param printer The printer to render the decklist to.
     * @param deckList
     */
    static void printDeckList(QPrinter *printer, const DeckList *deckList);

    bool convertToCockatriceFormat(QString fileName);

    DeckList *getDeckList()
    {
        return &deckList;
    }

    DecklistCardNode *addCard(const QString &cardName,
                              const QString &zoneName,
                              int position,
                              const QString &cardSetName = QString(),
                              const QString &cardSetCollectorNumber = QString(),
                              const QString &cardProviderId = QString())
    {
        return deckList.addCard(cardName, zoneName, position, cardSetName, cardSetCollectorNumber, cardProviderId);
    };

    void setBannerCard(const CardRef &_bannerCard = {})
    {
        deckList.setBannerCard(_bannerCard);
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
