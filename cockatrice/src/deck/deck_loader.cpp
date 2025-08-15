#include "deck_loader.h"

#include "../game/cards/card_database.h"
#include "../game/cards/card_database_manager.h"
#include "../main.h"
#include "decklist.h"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QRegularExpression>
#include <QStringList>
#include <QtConcurrentRun>

const QStringList DeckLoader::ACCEPTED_FILE_EXTENSIONS = {"*.cod", "*.dec", "*.dek", "*.txt", "*.mwDeck"};

const QStringList DeckLoader::FILE_NAME_FILTERS = {
    tr("Common deck formats (%1)").arg(ACCEPTED_FILE_EXTENSIONS.join(" ")), tr("All files (*.*)")};

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

bool DeckLoader::loadFromFile(const QString &fileName, FileFormat fmt, bool userRequest)
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
            qCInfo(DeckLoaderLog) << "Loaded from" << fileName << "-" << result;
            if (!result) {
                qCInfo(DeckLoaderLog) << "Retrying as plain format";
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
        if (userRequest) {
            updateLastLoadedTimestamp(fileName, fmt);
        }

        emit deckLoaded();
    }

    qCInfo(DeckLoaderLog) << "Deck was loaded -" << result;
    return result;
}

bool DeckLoader::loadFromFileAsync(const QString &fileName, FileFormat fmt, bool userRequest)
{
    auto *watcher = new QFutureWatcher<bool>(this);

    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, fileName, fmt, userRequest]() {
        const bool result = watcher->result();
        watcher->deleteLater();

        if (result) {
            lastFileName = fileName;
            lastFileFormat = fmt;
            if (userRequest) {
                updateLastLoadedTimestamp(fileName, fmt);
            }
            emit deckLoaded();
        }

        emit loadFinished(result);
    });

    QFuture<bool> future = QtConcurrent::run([=, this]() {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return false;
        }

        switch (fmt) {
            case PlainTextFormat:
                return loadFromFile_Plain(&file);
            case CockatriceFormat: {
                bool result = false;
                result = loadFromFile_Native(&file);
                if (!result) {
                    file.seek(0);
                    return loadFromFile_Plain(&file);
                }
                return result;
            }
            default:
                return false;
                break;
        }
    });

    watcher->setFuture(future);
    return true; // Return immediately to indicate the async task was started
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

    file.flush();
    file.close();

    return result;
}

bool DeckLoader::updateLastLoadedTimestamp(const QString &fileName, FileFormat fmt)
{
    QFileInfo fileInfo(fileName);
    if (!fileInfo.exists()) {
        qCWarning(DeckLoaderLog) << "File does not exist:" << fileName;
        return false;
    }

    QDateTime originalTimestamp = fileInfo.lastModified();

    // Open the file for writing
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(DeckLoaderLog) << "Failed to open file for writing:" << fileName;
        return false;
    }

    bool result = false;

    // Perform file modifications
    switch (fmt) {
        case PlainTextFormat:
            result = saveToFile_Plain(&file);
            break;
        case CockatriceFormat:
            setLastLoadedTimestamp(QDateTime::currentDateTime().toString());
            result = saveToFile_Native(&file);
            break;
    }

    file.close(); // Close the file to ensure changes are flushed

    if (result) {
        lastFileName = fileName;
        lastFileFormat = fmt;

        // Re-open the file and set the original timestamp
        if (!file.open(QIODevice::ReadWrite)) {
            qCWarning(DeckLoaderLog) << "Failed to re-open file to set timestamp:" << fileName;
            return false;
        }

        if (!file.setFileTime(originalTimestamp, QFileDevice::FileModificationTime)) {
            qCWarning(DeckLoaderLog) << "Failed to set modification time for file:" << fileName;
            file.close();
            return false;
        }

        file.close();
    }

    return result;
}

static QString getDomainForWebsite(DeckLoader::DecklistWebsite website)
{
    switch (website) {
        case DeckLoader::DecklistOrg:
            return "www.decklist.org";
        case DeckLoader::DecklistXyz:
            return "www.decklist.xyz";
        default:
            qCWarning(DeckLoaderLog) << "Invalid decklist website enum:" << website;
            return "";
    }
}

/**
 * Converts the card to the String that represents it in the decklist export
 */
static QString toDecklistExportString(const DecklistCardNode *card)
{
    QString cardString;
    // Get the number of cards and add the card name
    cardString += QString::number(card->getNumber());
    // Add a space between card num and name
    cardString += "%20";
    // Add card name
    cardString += card->getName();

    if (!card->getCardSetShortName().isNull()) {
        cardString += "%20";
        cardString += "(" + card->getCardSetShortName() + ")";
    }
    if (!card->getCardCollectorNumber().isNull()) {
        cardString += "%20";
        cardString += card->getCardCollectorNumber();
    }

    // Add a return at the end of the card
    cardString += "%0A";

    return cardString;
}

/**
 * Export deck to decklist function, called to format the deck in a way to be sent to a server
 * @param website The website we're sending the deck to
 */
QString DeckLoader::exportDeckToDecklist(DecklistWebsite website)
{
    // Add the base url
    QString deckString = "https://" + getDomainForWebsite(website) + "/?";
    // Create two strings to pass to function
    QString mainBoardCards, sideBoardCards;

    // Set up the function to call
    auto formatDeckListForExport = [&mainBoardCards, &sideBoardCards](const auto *node, const auto *card) {
        // Get the card name
        CardInfoPtr dbCard = CardDatabaseManager::getInstance()->getCardInfo(card->getName());
        if (!dbCard || dbCard->getIsToken()) {
            // If it's a token, we don't care about the card.
            return;
        }

        // Check if it's a sideboard card.
        if (node->getName() == DECK_ZONE_SIDE) {
            sideBoardCards += toDecklistExportString(card);
        } else {
            // If it's a mainboard card, do the same thing, but for the mainboard card string
            mainBoardCards += toDecklistExportString(card);
        }
    };

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

// This struct is here to support the forEachCard function call, defined in decklist.
// It requires a function to be called for each card, and it will set the providerId to the preferred printing.
struct SetProviderIdToPreferred
{
    // Main operator for struct, allowing the foreachcard to work.
    SetProviderIdToPreferred()
    {
    }

    void operator()(const InnerDecklistNode *node, DecklistCardNode *card) const
    {
        Q_UNUSED(node);
        PrintingInfo preferredPrinting = CardDatabaseManager::getInstance()->getPreferredPrinting(card->getName());
        QString providerId = preferredPrinting.getUuid();
        QString setShortName = preferredPrinting.getSet()->getShortName();
        QString collectorNumber = preferredPrinting.getProperty("num");

        card->setCardProviderId(providerId);
        card->setCardCollectorNumber(collectorNumber);
        card->setCardSetShortName(setShortName);
    }
};

/**
 * This function iterates through each card in the decklist and sets the providerId
 * on each card based on its set name and collector number.
 */
void DeckLoader::setProviderIdToPreferredPrinting()
{
    // Set up the struct to call.
    SetProviderIdToPreferred setProviderIdToPreferred;

    // Call the forEachCard method for each card in the deck
    forEachCard(setProviderIdToPreferred);
}

/**
 * Sets the providerId on each card in the decklist based on its set name and collector number.
 */
void DeckLoader::resolveSetNameAndNumberToProviderID()
{
    auto setProviderId = [](const auto node, const auto card) {
        Q_UNUSED(node);
        // Retrieve the providerId based on setName and collectorNumber
        QString providerId =
            CardDatabaseManager::getInstance()
                ->getSpecificPrinting(card->getName(), card->getCardSetShortName(), card->getCardCollectorNumber())
                .getUuid();

        // Set the providerId on the card
        card->setCardProviderId(providerId);
    };

    forEachCard(setProviderId);
}

// This struct is here to support the forEachCard function call, defined in decklist.
// It requires a function to be called for each card, and it will set the providerId.
struct ClearSetNameNumberAndProviderId
{
    // Main operator for struct, allowing the foreachcard to work.
    ClearSetNameNumberAndProviderId()
    {
    }

    void operator()(const InnerDecklistNode *node, DecklistCardNode *card) const
    {
        Q_UNUSED(node);
        // Set the providerId on the card
        card->setCardSetShortName(nullptr);
        card->setCardCollectorNumber(nullptr);
        card->setCardProviderId(nullptr);
    }
};

/**
 * Clears the set name and numbers on each card in the decklist.
 */
void DeckLoader::clearSetNamesAndNumbers()
{
    auto clearSetNameAndNumber = [](const auto node, auto card) {
        Q_UNUSED(node)
        // Set the providerId on the card
        card->setCardSetShortName(nullptr);
        card->setCardCollectorNumber(nullptr);
        card->setCardProviderId(nullptr);
    };

    forEachCard(clearSetNameAndNumber);
}

DeckLoader::FileFormat DeckLoader::getFormatFromName(const QString &fileName)
{
    if (fileName.endsWith(".cod", Qt::CaseInsensitive)) {
        return CockatriceFormat;
    }
    return PlainTextFormat;
}

void DeckLoader::saveToClipboard(bool addComments, bool addSetNameAndNumber) const
{
    QString buffer;
    QTextStream stream(&buffer);
    saveToStream_Plain(stream, addComments, addSetNameAndNumber);
    QApplication::clipboard()->setText(buffer, QClipboard::Clipboard);
    QApplication::clipboard()->setText(buffer, QClipboard::Selection);
}

bool DeckLoader::saveToStream_Plain(QTextStream &out, bool addComments, bool addSetNameAndNumber) const
{
    if (addComments) {
        saveToStream_DeckHeader(out);
    }

    // loop zones
    for (int i = 0; i < getRoot()->size(); i++) {
        const auto *zoneNode = dynamic_cast<InnerDecklistNode *>(getRoot()->at(i));

        saveToStream_DeckZone(out, zoneNode, addComments, addSetNameAndNumber);

        // end of zone
        out << "\n";
    }

    return true;
}

void DeckLoader::saveToStream_DeckHeader(QTextStream &out) const
{
    if (!getName().isEmpty()) {
        out << "// " << getName() << "\n\n";
    }

    if (!getComments().isEmpty()) {
        QStringList commentRows = getComments().split(QRegularExpression("\n|\r\n|\r"));
        for (const QString &row : commentRows) {
            out << "// " << row << "\n";
        }
        out << "\n";
    }
}

void DeckLoader::saveToStream_DeckZone(QTextStream &out,
                                       const InnerDecklistNode *zoneNode,
                                       bool addComments,
                                       bool addSetNameAndNumber) const
{
    // group cards by card type and count the subtotals
    QMultiMap<QString, DecklistCardNode *> cardsByType;
    QMap<QString, int> cardTotalByType;
    int cardTotal = 0;

    for (int j = 0; j < zoneNode->size(); j++) {
        auto *card = dynamic_cast<DecklistCardNode *>(zoneNode->at(j));

        CardInfoPtr info = CardDatabaseManager::getInstance()->getCardInfo(card->getName());
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
    for (const QString &cardType : cardsByType.uniqueKeys()) {
        if (addComments) {
            out << "// " << cardTotalByType[cardType] << " " << cardType << "\n";
        }

        QList<DecklistCardNode *> cards = cardsByType.values(cardType);

        saveToStream_DeckZoneCards(out, zoneNode, cards, addComments, addSetNameAndNumber);

        if (addComments) {
            out << "\n";
        }
    }
}

void DeckLoader::saveToStream_DeckZoneCards(QTextStream &out,
                                            const InnerDecklistNode *zoneNode,
                                            QList<DecklistCardNode *> cards,
                                            bool addComments,
                                            bool addSetNameAndNumber) const
{
    // QMultiMap sorts values in reverse order
    for (int i = cards.size() - 1; i >= 0; --i) {
        DecklistCardNode *card = cards[i];

        if (zoneNode->getName() == DECK_ZONE_SIDE && addComments) {
            out << "SB: ";
        }

        if (card->getNumber()) {
            out << card->getNumber();
        }
        if (!card->getName().isNull() && !card->getName().isEmpty()) {
            out << " " << card->getName();
        }
        if (addSetNameAndNumber) {
            if (!card->getCardSetShortName().isNull() && !card->getCardSetShortName().isEmpty()) {
                out << " "
                    << "(" << card->getCardSetShortName() << ")";
            }
            if (!card->getCardCollectorNumber().isNull()) {
                out << " " << card->getCardCollectorNumber();
            }
        }
        out << "\n";
    }
}

bool DeckLoader::convertToCockatriceFormat(QString fileName)
{
    // Change the file extension to .cod
    QFileInfo fileInfo(fileName);
    QString newFileName = QDir::toNativeSeparators(fileInfo.path() + "/" + fileInfo.completeBaseName() + ".cod");

    // Open the new file for writing
    QFile file(newFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(DeckLoaderLog) << "Failed to open file for writing:" << newFileName;
        return false;
    }

    bool result = false;

    // Perform file modifications based on the detected format
    switch (getFormatFromName(fileName)) {
        case PlainTextFormat:
            // Save in Cockatrice's native format
            result = saveToFile_Native(&file);
            break;
        case CockatriceFormat:
            qCInfo(DeckLoaderLog) << "File is already in Cockatrice format. No conversion needed.";
            result = true;
            break;
        default:
            qCWarning(DeckLoaderLog) << "Unsupported file format for conversion:" << fileName;
            result = false;
            break;
    }

    file.close();

    // Delete the old file if conversion was successful
    if (result) {
        if (!QFile::remove(fileName)) {
            qCWarning(DeckLoaderLog) << "Failed to delete original file:" << fileName;
        } else {
            qCInfo(DeckLoaderLog) << "Original file deleted successfully:" << fileName;
        }
        lastFileName = newFileName;
        lastFileFormat = CockatriceFormat;
    }

    return result;
}

QString DeckLoader::getCardZoneFromName(QString cardName, QString currentZoneName)
{
    CardInfoPtr card = CardDatabaseManager::getInstance()->getCardInfo(cardName);

    if (card && card->getIsToken()) {
        return DECK_ZONE_TOKENS;
    }

    return currentZoneName;
}

QString DeckLoader::getCompleteCardName(const QString &cardName) const
{
    if (CardDatabaseManager::getInstance()) {
        ExactCard temp = CardDatabaseManager::getInstance()->guessCard({cardName});
        if (temp) {
            return temp.getName();
        }
    }

    return cardName;
}
