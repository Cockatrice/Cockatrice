#include <QStringList>
#include <QFile>
#include <QDebug>
#include "deck_loader.h"
#include "decklist.h"
#include "carddatabase.h"
#include "main.h"

const QStringList DeckLoader::fileNameFilters = QStringList()
    << QObject::tr("Common deck formats (*.cod *.dec *.txt *.mwDeck)")
    << QObject::tr("All files (*.*)");

DeckLoader::DeckLoader()
    : DeckList(),
      lastFileName(QString()),
      lastFileFormat(CockatriceFormat),
      lastRemoteDeckId(-1)
{
}

DeckLoader::DeckLoader(const QString &nativeString)
    : DeckList(nativeString),
      lastFileName(QString()),
      lastFileFormat(CockatriceFormat),
      lastRemoteDeckId(-1)
{
}

DeckLoader::DeckLoader(const DeckList &other)
    : DeckList(other),
      lastFileName(QString()),
      lastFileFormat(CockatriceFormat),
      lastRemoteDeckId(-1)
{
}

DeckLoader::DeckLoader(const DeckLoader &other)
    : DeckList(other),
      lastFileName(other.lastFileName),
      lastFileFormat(other.lastFileFormat),
      lastRemoteDeckId(other.lastRemoteDeckId)
{
}

bool DeckLoader::loadFromFile(const QString &fileName, FileFormat fmt)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    bool result = false;
    switch (fmt) {
        case PlainTextFormat: result = loadFromFile_Plain(&file); break;
        case CockatriceFormat:
            result = loadFromFile_Native(&file);
            qDebug() << "Loaded from" << fileName << "-" << result;
            if (!result) {
                qDebug() << "Retying as plain format";
                file.seek(0);
                result = loadFromFile_Plain(&file);
                fmt = PlainTextFormat;
            }
            break;
    }
    if (result) {
        lastFileName = fileName;
        lastFileFormat = fmt;

        emit deckLoaded();
    }
    qDebug() << "Deck was loaded -" << result;
    return result;
}

bool DeckLoader::loadFromRemote(const QString &nativeString, int remoteDeckId)
{
    bool result = loadFromString_Native(nativeString);
    if (result) {
        lastFileName = QString();
        lastFileFormat = CockatriceFormat;
        lastRemoteDeckId = remoteDeckId;

        emit deckLoaded();
    }
    return result;
}

bool DeckLoader::saveToFile(const QString &fileName, FileFormat fmt)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    bool result = false;
    switch (fmt) {
        case PlainTextFormat: result = saveToFile_Plain(&file); break;
        case CockatriceFormat: result = saveToFile_Native(&file); break;
    }
    if (result) {
        lastFileName = fileName;
        lastFileFormat = fmt;
    }
    return result;
}

DeckLoader::FileFormat DeckLoader::getFormatFromName(const QString &fileName)
{
    if (fileName.endsWith(".cod", Qt::CaseInsensitive)) {
        return CockatriceFormat;
    }
    return PlainTextFormat;
}

bool DeckLoader::saveToStream_Plain(QTextStream &out)
{
    saveToStream_DeckHeader(out);

    // loop zones
    for (int i = 0; i < getRoot()->size(); i++) {
        const InnerDecklistNode *zoneNode =
            dynamic_cast<InnerDecklistNode *>(getRoot()->at(i));

        saveToStream_DeckZone(out, zoneNode);

        // end of zone
        out << "\n";
    }

    return true;
}

void DeckLoader::saveToStream_DeckHeader(QTextStream &out)
{
    if(!getName().isEmpty())
        out << "// " << getName() << "\n\n";
    if(!getComments().isEmpty())
    {
        QStringList commentRows = getComments().split(QRegExp("\n|\r\n|\r"));
        foreach(QString row, commentRows)
            out << "// " << row << "\n";
        out << "\n";
    }
}

void DeckLoader::saveToStream_DeckZone(QTextStream &out, const InnerDecklistNode *zoneNode)
{
   // group cards by card type and count the subtotals
    QMultiMap<QString, DecklistCardNode*> cardsByType;
    QMap<QString, int> cardTotalByType;
    int cardTotal = 0;

    for (int j = 0; j < zoneNode->size(); j++) {
        DecklistCardNode *card =
            dynamic_cast<DecklistCardNode *>(
                zoneNode->at(j)
            );

        CardInfo *info = db->getCard(card->getName());
        QString cardType = info ? info->getMainCardType() : "unknown";

        cardsByType.insert(cardType, card);

        if(cardTotalByType.contains(cardType))
            cardTotalByType[cardType] += card->getNumber();
        else
            cardTotalByType[cardType] = card->getNumber();

        cardTotal += card->getNumber();
    }

    out << "// " << cardTotal << " " << zoneNode->getVisibleName() << "\n";

    // print cards to stream
    foreach(QString cardType, cardsByType.uniqueKeys())
    {

        out << "// " << cardTotalByType[cardType] << " " << cardType << "\n";
        QList <DecklistCardNode*> cards = cardsByType.values(cardType);

        saveToStream_DeckZoneCards(out, zoneNode, cards);

        out << "\n";
    }
}

void DeckLoader::saveToStream_DeckZoneCards(QTextStream &out, const InnerDecklistNode *zoneNode, QList <DecklistCardNode*> cards)
{
    // QMultiMap sorts values in reverse order
    for(int i = cards.size() - 1; i >= 0; --i)
    {
        DecklistCardNode* card = cards[i];

        if (zoneNode->getName() == "side")
            out << "SB: ";

       out << card->getNumber() << " " << card->getName() << "\n";
    }
}
