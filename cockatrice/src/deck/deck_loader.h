#ifndef DECK_LOADER_H
#define DECK_LOADER_H

#include "decklist.h"

#include <QLoggingCategory>

inline Q_LOGGING_CATEGORY(DeckLoaderLog, "deck_loader")

    class DeckLoader : public DeckList
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
    static const QStringList fileNameFilters;

private:
    QString lastFileName;
    FileFormat lastFileFormat;
    int lastRemoteDeckId;

public:
    DeckLoader();
    explicit DeckLoader(const QString &nativeString);
    explicit DeckLoader(const DeckList &other);
    DeckLoader(const DeckLoader &other);
    const QString &getLastFileName() const
    {
        return lastFileName;
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

    void clearSetNamesAndNumbers();
    static FileFormat getFormatFromName(const QString &fileName);

    bool loadFromFile(const QString &fileName, FileFormat fmt, bool userRequest = false);
    bool loadFromFileAsync(const QString &fileName, FileFormat fmt, bool userRequest);
    bool loadFromRemote(const QString &nativeString, int remoteDeckId);
    bool saveToFile(const QString &fileName, FileFormat fmt);
    bool updateLastLoadedTimestamp(const QString &fileName, FileFormat fmt);
    QString exportDeckToDecklist();

    void resolveSetNameAndNumberToProviderID();

    // overload
    bool saveToStream_Plain(QTextStream &out, bool addComments = true, bool addSetNameAndNumber = true) const;
    bool convertToCockatriceFormat(QString fileName);

protected:
    void saveToStream_DeckHeader(QTextStream &out) const;
    void saveToStream_DeckZone(QTextStream &out,
                               const InnerDecklistNode *zoneNode,
                               bool addComments = true,
                               bool addSetNameAndNumber = true) const;
    void saveToStream_DeckZoneCards(QTextStream &out,
                                    const InnerDecklistNode *zoneNode,
                                    QList<DecklistCardNode *> cards,
                                    bool addComments = true,
                                    bool addSetNameAndNumber = true) const;
    [[nodiscard]] QString getCardZoneFromName(QString cardName, QString currentZoneName) override;
    [[nodiscard]] QString getCompleteCardName(const QString &cardName) const override;
};

#endif
