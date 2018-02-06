#include "deck_loader.h"
#include "carddatabase.h"
#include "decklist.h"
#include "main.h"
#include <QDebug>
#include <QFile>
#include <QStringList>

const QStringList DeckLoader::fileNameFilters =
    QStringList() << QObject::tr("Common deck formats (*.cod *.dec *.txt *.mwDeck)") << QObject::tr("All files (*.*)");

DeckLoader::DeckLoader() : DeckList(), lastFileName(QString()), lastFileFormat(CockatriceFormat), lastRemoteDeckId(-1)
{
}

DeckLoader::DeckLoader(const QString &nativeString)
    : DeckList(nativeString), lastFileName(QString()), lastFileFormat(CockatriceFormat), lastRemoteDeckId(-1)
{
}

DeckLoader::DeckLoader(const DeckList &other)
    : DeckList(other), lastFileName(QString()), lastFileFormat(CockatriceFormat), lastRemoteDeckId(-1)
{
}

DeckLoader::DeckLoader(const DeckLoader &other)
    : DeckList(other), lastFileName(other.lastFileName), lastFileFormat(other.lastFileFormat),
      lastRemoteDeckId(other.lastRemoteDeckId)
{
}

bool DeckLoader::loadFromFile(const QString &fileName, FileFormat fmt)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    bool result = false;
    switch (fmt) {
        case PlainTextFormat:
            result = loadFromFile_Plain(&file);
            break;
        case CockatriceFormat: {
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

        default:
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
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    bool result = false;
    switch (fmt) {
        case PlainTextFormat:
            result = saveToFile_Plain(&file);
            break;
        case CockatriceFormat:
            result = saveToFile_Native(&file);
            break;
    }

    if (result) {
        lastFileName = fileName;
        lastFileFormat = fmt;
    }
    return result;
}

// This struct is here to support the forEachCard function call, defined in decklist. It
// requires a function to be called for each card, and passes an inner node and a card for
// each card in the decklist.
struct FormatDeckListForExport
{
    // Create refrences for the strings that will be passed in.
    QString &mainBoardCards;
    QString &sideBoardCards;
    // create main operator for struct, allowing the foreachcard to work.
    FormatDeckListForExport(QString &_mainBoardCards, QString &_sideBoardCards)
        : mainBoardCards(_mainBoardCards), sideBoardCards(_sideBoardCards){};

    void operator()(const InnerDecklistNode *node, const DecklistCardNode *card) const
    {
        // Get the card name
        CardInfoPtr dbCard = db->getCard(card->getName());
        if (!dbCard || dbCard->getIsToken()) {
            // If it's a token, we don't care about the card.
            return;
        }

        // Check if it's a sideboard card.
        if (node->getName() == DECK_ZONE_SIDE) {
            // Get the number of cards and add the card name
            sideBoardCards += QString::number(card->getNumber());
            // Add a space between card num and name
            sideBoardCards += "%20";
            // Add card name
            sideBoardCards += card->getName();
            // Add a return at the end of the card
            sideBoardCards += "%0A";
        } else // If it's a mainboard card, do the same thing, but for the mainboard card string
        {
            mainBoardCards += QString::number(card->getNumber());
            mainBoardCards += "%20";
            mainBoardCards += card->getName();
            mainBoardCards += "%0A";
        }
    }
};

// Export deck to decklist function, called to format the deck in a way to be sent to a server
QString DeckLoader::exportDeckToDecklist()
{
    // Add the base url
    QString deckString = "https://www.decklist.org/?";
    // Create two strings to pass to function
    QString mainBoardCards, sideBoardCards;
    // Set up the struct to call.
    FormatDeckListForExport formatDeckListForExport(mainBoardCards, sideBoardCards);
    // call our struct function for each card in the deck
    forEachCard(formatDeckListForExport);
    // Remove the extra return at the end of the last cards
    mainBoardCards.chop(3);
    sideBoardCards.chop(3);
    // if after we've called it for each card, and the strings are empty, we know that
    // there were no non-token cards in the deck, so show an error message.
    if ((QString::compare(mainBoardCards, "", Qt::CaseInsensitive) == 0) &&
        (QString::compare(sideBoardCards, "", Qt::CaseInsensitive) == 0)) {
        return "";
    }
    // return a string with the url for decklist export
    deckString += "deckmain=" + mainBoardCards + "&deckside=" + sideBoardCards;
    return deckString;
}

DeckLoader::FileFormat DeckLoader::getFormatFromName(const QString &fileName)
{
    if (fileName.endsWith(".cod", Qt::CaseInsensitive)) {
        return CockatriceFormat;
    }
    return PlainTextFormat;
}

bool DeckLoader::saveToStream_Plain(QTextStream &out, bool addComments)
{
    if (addComments) {
        saveToStream_DeckHeader(out);
    }

    // loop zones
    for (int i = 0; i < getRoot()->size(); i++) {
        const auto *zoneNode = dynamic_cast<InnerDecklistNode *>(getRoot()->at(i));

        saveToStream_DeckZone(out, zoneNode, addComments);

        // end of zone
        out << "\n";
    }

    return true;
}

void DeckLoader::saveToStream_DeckHeader(QTextStream &out)
{
    if (!getName().isEmpty()) {
        out << "// " << getName() << "\n\n";
    }

    if (!getComments().isEmpty()) {
        QStringList commentRows = getComments().split(QRegExp("\n|\r\n|\r"));
        foreach (QString row, commentRows) {
            out << "// " << row << "\n";
        }
        out << "\n";
    }
}

void DeckLoader::saveToStream_DeckZone(QTextStream &out, const InnerDecklistNode *zoneNode, bool addComments)
{
    // group cards by card type and count the subtotals
    QMultiMap<QString, DecklistCardNode *> cardsByType;
    QMap<QString, int> cardTotalByType;
    int cardTotal = 0;

    for (int j = 0; j < zoneNode->size(); j++) {
        auto *card = dynamic_cast<DecklistCardNode *>(zoneNode->at(j));

        CardInfoPtr info = db->getCard(card->getName());
        QString cardType = info ? info->getMainCardType() : "unknown";

        cardsByType.insert(cardType, card);

        if (cardTotalByType.contains(cardType)) {
            cardTotalByType[cardType] += card->getNumber();
        } else {
            cardTotalByType[cardType] = card->getNumber();
        }

        cardTotal += card->getNumber();
    }

    if (addComments) {
        out << "// " << cardTotal << " " << zoneNode->getVisibleName() << "\n";
    }

    // print cards to stream
    foreach (QString cardType, cardsByType.uniqueKeys()) {
        if (addComments) {
            out << "// " << cardTotalByType[cardType] << " " << cardType << "\n";
        }

        QList<DecklistCardNode *> cards = cardsByType.values(cardType);

        saveToStream_DeckZoneCards(out, zoneNode, cards, addComments);

        if (addComments) {
            out << "\n";
        }
    }
}

void DeckLoader::saveToStream_DeckZoneCards(QTextStream &out,
                                            const InnerDecklistNode *zoneNode,
                                            QList<DecklistCardNode *> cards,
                                            bool addComments)
{
    // QMultiMap sorts values in reverse order
    for (int i = cards.size() - 1; i >= 0; --i) {
        DecklistCardNode *card = cards[i];

        if (zoneNode->getName() == DECK_ZONE_SIDE && addComments) {
            out << "SB: ";
        }

        out << card->getNumber() << " " << card->getName() << "\n";
    }
}

QString DeckLoader::getCardZoneFromName(QString cardName, QString currentZoneName)
{
    CardInfoPtr card = db->getCard(cardName);

    if (card && card->getIsToken()) {
        return DECK_ZONE_TOKENS;
    }

    return currentZoneName;
}

QString DeckLoader::getCompleteCardName(const QString cardName) const
{
    if (db) {
        CardInfoPtr temp = db->getCardBySimpleName(cardName);
        if (temp) {
            return temp->getName();
        }
    }

    return cardName;
}