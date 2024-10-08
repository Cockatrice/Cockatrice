#ifndef DECK_LOADER_H
#define DECK_LOADER_H

#include "decklist.h"

class DeckLoader : public DeckList
{
    Q_OBJECT
signals:
    void deckLoaded();

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
    DeckLoader(const QString &nativeString);
    DeckLoader(const DeckList &other);
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

    static FileFormat getFormatFromName(const QString &fileName);

    bool loadFromFile(const QString &fileName, FileFormat fmt);
    bool loadFromRemote(const QString &nativeString, int remoteDeckId);
    bool saveToFile(const QString &fileName, FileFormat fmt);
    QString exportDeckToDecklist();

    // overload
    bool saveToStream_Plain(QTextStream &out, bool addComments = true);

protected:
    void saveToStream_DeckHeader(QTextStream &out);
    void saveToStream_DeckZone(QTextStream &out, const InnerDecklistNode *zoneNode, bool addComments = true);
    void saveToStream_DeckZoneCards(QTextStream &out,
                                    const InnerDecklistNode *zoneNode,
                                    QList<DecklistCardNode *> cards,
                                    bool addComments = true);
    [[nodiscard]] QString getCardZoneFromName(QString cardName, QString currentZoneName) override;
    [[nodiscard]] QString getCompleteCardName(const QString &cardName) const override;
};

#endif
