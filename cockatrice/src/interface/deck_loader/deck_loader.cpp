#include "deck_loader.h"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QPrinter>
#include <QRegularExpression>
#include <QStringList>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextStream>
#include <QTextTable>
#include <QtConcurrentRun>
#include <libcockatrice/card/database/card_database.h>
#include <libcockatrice/card/database/card_database_manager.h>
#include <libcockatrice/deck_list/deck_list.h>
#include <libcockatrice/deck_list/tree/deck_list_card_node.h>

const QStringList DeckLoader::ACCEPTED_FILE_EXTENSIONS = {"*.cod", "*.dec", "*.dek", "*.txt", "*.mwDeck"};

const QStringList DeckLoader::FILE_NAME_FILTERS = {
    tr("Common deck formats (%1)").arg(ACCEPTED_FILE_EXTENSIONS.join(" ")), tr("All files (*.*)")};

DeckLoader::DeckLoader(QObject *parent) : QObject(parent)
{
}

bool DeckLoader::loadFromFile(const QString &fileName, DeckFileFormat::Format fmt, bool userRequest)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    bool result = false;
    DeckList deckList = DeckList();
    switch (fmt) {
        case DeckFileFormat::PlainText:
            result = deckList.loadFromFile_Plain(&file);
            break;
        case DeckFileFormat::Cockatrice: {
            result = deckList.loadFromFile_Native(&file);
            qCInfo(DeckLoaderLog) << "Loaded from" << fileName << "-" << result;
            if (!result) {
                qCInfo(DeckLoaderLog) << "Retrying as plain format";
                file.seek(0);
                result = deckList.loadFromFile_Plain(&file);
                fmt = DeckFileFormat::PlainText;
            }
            break;
        }

        default:
            break;
    }

    if (result) {
        loadedDeck.deckList = deckList;
        loadedDeck.lastLoadInfo = {
            .fileName = fileName,
            .fileFormat = fmt,
        };
        if (userRequest) {
            updateLastLoadedTimestamp(fileName, fmt);
        }

        emit deckLoaded();
    }

    qCInfo(DeckLoaderLog) << "Deck was loaded -" << result;
    return result;
}

bool DeckLoader::loadFromFileAsync(const QString &fileName, DeckFileFormat::Format fmt, bool userRequest)
{
    auto *watcher = new QFutureWatcher<bool>(this);

    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, fileName, fmt, userRequest]() {
        const bool result = watcher->result();
        watcher->deleteLater();

        if (result) {
            loadedDeck.lastLoadInfo = {
                .fileName = fileName,
                .fileFormat = fmt,
            };
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
            case DeckFileFormat::PlainText:
                return loadedDeck.deckList.loadFromFile_Plain(&file);
            case DeckFileFormat::Cockatrice: {
                bool result = false;
                result = loadedDeck.deckList.loadFromFile_Native(&file);
                if (!result) {
                    file.seek(0);
                    return loadedDeck.deckList.loadFromFile_Plain(&file);
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
    bool result = loadedDeck.deckList.loadFromString_Native(nativeString);
    if (result) {
        loadedDeck.lastLoadInfo = {
            .remoteDeckId = remoteDeckId,
        };

        emit deckLoaded();
    }
    return result;
}

bool DeckLoader::saveToFile(const QString &fileName, DeckFileFormat::Format fmt)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    bool result = false;
    switch (fmt) {
        case DeckFileFormat::PlainText:
            result = loadedDeck.deckList.saveToFile_Plain(&file);
            break;
        case DeckFileFormat::Cockatrice:
            result = loadedDeck.deckList.saveToFile_Native(&file);
            qCInfo(DeckLoaderLog) << "Saving to " << fileName << "-" << result;
            break;
    }

    if (result) {
        loadedDeck.lastLoadInfo = {
            .fileName = fileName,
            .fileFormat = fmt,
        };
        qCInfo(DeckLoaderLog) << "Deck was saved -" << result;
    }

    file.flush();
    file.close();

    return result;
}

bool DeckLoader::updateLastLoadedTimestamp(const QString &fileName, DeckFileFormat::Format fmt)
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
        case DeckFileFormat::PlainText:
            result = loadedDeck.deckList.saveToFile_Plain(&file);
            break;
        case DeckFileFormat::Cockatrice:
            loadedDeck.deckList.setLastLoadedTimestamp(QDateTime::currentDateTime().toString());
            result = loadedDeck.deckList.saveToFile_Native(&file);
            break;
    }

    file.close(); // Close the file to ensure changes are flushed

    if (result) {
        loadedDeck.lastLoadInfo = {
            .fileName = fileName,
            .fileFormat = fmt,
        };

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
 * Converts all cards in the list to their decklist export string and joins them into one string
 */
static QString toDecklistExportString(const QList<const DecklistCardNode *> &cardNodes)
{
    QString result;

    for (auto cardNode : cardNodes) {
        result += toDecklistExportString(cardNode);
    }

    return result;
}

/**
 * Export deck to decklist function, called to format the deck in a way to be sent to a server
 *
 * @param deckList The decklist to export
 * @param website The website we're sending the deck to
 */
QString DeckLoader::exportDeckToDecklist(const DeckList &deckList, DecklistWebsite website)
{
    // Add the base url
    QString deckString = "https://" + getDomainForWebsite(website) + "/?";

    // export all cards in zone
    QString mainBoardCards = toDecklistExportString(deckList.getCardNodes({DECK_ZONE_MAIN}));
    QString sideBoardCards = toDecklistExportString(deckList.getCardNodes({DECK_ZONE_SIDE}));

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

void DeckLoader::saveToClipboard(const DeckList &deckList, bool addComments, bool addSetNameAndNumber)
{
    QString buffer;
    QTextStream stream(&buffer);
    saveToStream_Plain(stream, deckList, addComments, addSetNameAndNumber);
    QApplication::clipboard()->setText(buffer, QClipboard::Clipboard);
    QApplication::clipboard()->setText(buffer, QClipboard::Selection);
}

bool DeckLoader::saveToStream_Plain(QTextStream &out,
                                    const DeckList &deckList,
                                    bool addComments,
                                    bool addSetNameAndNumber)
{
    if (addComments) {
        saveToStream_DeckHeader(out, deckList);
    }

    // loop zones
    for (auto zoneNode : deckList.getZoneNodes()) {
        saveToStream_DeckZone(out, zoneNode, addComments, addSetNameAndNumber);

        // end of zone
        out << "\n";
    }

    return true;
}

void DeckLoader::saveToStream_DeckHeader(QTextStream &out, const DeckList &deckList)
{
    if (!deckList.getName().isEmpty()) {
        out << "// " << deckList.getName() << "\n\n";
    }

    if (!deckList.getComments().isEmpty()) {
        QStringList commentRows = deckList.getComments().split(QRegularExpression("\n|\r\n|\r"));
        for (const QString &row : commentRows) {
            out << "// " << row << "\n";
        }
        out << "\n";
    }
}

void DeckLoader::saveToStream_DeckZone(QTextStream &out,
                                       const InnerDecklistNode *zoneNode,
                                       bool addComments,
                                       bool addSetNameAndNumber)
{
    // group cards by card type and count the subtotals
    QMultiMap<QString, DecklistCardNode *> cardsByType;
    QMap<QString, int> cardTotalByType;
    int cardTotal = 0;

    for (int j = 0; j < zoneNode->size(); j++) {
        auto *card = dynamic_cast<DecklistCardNode *>(zoneNode->at(j));

        CardInfoPtr info = CardDatabaseManager::query()->getCardInfo(card->getName());
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
                                            bool addSetNameAndNumber)
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

bool DeckLoader::convertToCockatriceFormat(const QString &fileName)
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
    switch (DeckFileFormat::getFormatFromName(fileName)) {
        case DeckFileFormat::PlainText:
            // Save in Cockatrice's native format
            result = loadedDeck.deckList.saveToFile_Native(&file);
            break;
        case DeckFileFormat::Cockatrice:
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
        loadedDeck.lastLoadInfo = {
            .fileName = newFileName,
            .fileFormat = DeckFileFormat::Cockatrice,
        };
    }

    return result;
}

void DeckLoader::printDeckListNode(QTextCursor *cursor, const InnerDecklistNode *node)
{
    const int totalColumns = 2;

    if (node->height() == 1) {
        QTextBlockFormat blockFormat;
        QTextCharFormat charFormat;
        charFormat.setFontPointSize(11);
        charFormat.setFontWeight(QFont::Bold);
        cursor->insertBlock(blockFormat, charFormat);

        QTextTableFormat tableFormat;
        tableFormat.setCellPadding(0);
        tableFormat.setCellSpacing(0);
        tableFormat.setBorder(0);
        QTextTable *table = cursor->insertTable(node->size() + 1, totalColumns, tableFormat);
        for (int i = 0; i < node->size(); i++) {
            auto *card = dynamic_cast<AbstractDecklistCardNode *>(node->at(i));

            QTextCharFormat cellCharFormat;
            cellCharFormat.setFontPointSize(9);

            QTextTableCell cell = table->cellAt(i, 0);
            cell.setFormat(cellCharFormat);
            QTextCursor cellCursor = cell.firstCursorPosition();
            cellCursor.insertText(QString("%1 ").arg(card->getNumber()));

            cell = table->cellAt(i, 1);
            cell.setFormat(cellCharFormat);
            cellCursor = cell.firstCursorPosition();
            cellCursor.insertText(card->getName());
        }
    } else if (node->height() == 2) {
        QTextBlockFormat blockFormat;
        QTextCharFormat charFormat;
        charFormat.setFontPointSize(14);
        charFormat.setFontWeight(QFont::Bold);

        cursor->insertBlock(blockFormat, charFormat);

        QTextTableFormat tableFormat;
        tableFormat.setCellPadding(10);
        tableFormat.setCellSpacing(0);
        tableFormat.setBorder(0);
        QVector<QTextLength> constraints;
        for (int i = 0; i < totalColumns; i++) {
            constraints << QTextLength(QTextLength::PercentageLength, 100.0 / totalColumns);
        }
        tableFormat.setColumnWidthConstraints(constraints);

        QTextTable *table = cursor->insertTable(1, totalColumns, tableFormat);
        for (int i = 0; i < node->size(); i++) {
            QTextCursor cellCursor = table->cellAt(0, (i * totalColumns) / node->size()).lastCursorPosition();
            printDeckListNode(&cellCursor, dynamic_cast<InnerDecklistNode *>(node->at(i)));
        }
    }

    cursor->movePosition(QTextCursor::End);
}

void DeckLoader::printDeckList(QPrinter *printer, const DeckList &deckList)
{
    QTextDocument doc;

    QFont font("Serif");
    font.setStyleHint(QFont::Serif);
    doc.setDefaultFont(font);

    QTextCursor cursor(&doc);

    QTextBlockFormat headerBlockFormat;
    QTextCharFormat headerCharFormat;
    headerCharFormat.setFontPointSize(16);
    headerCharFormat.setFontWeight(QFont::Bold);

    cursor.insertBlock(headerBlockFormat, headerCharFormat);
    cursor.insertText(deckList.getName());

    headerCharFormat.setFontPointSize(12);
    cursor.insertBlock(headerBlockFormat, headerCharFormat);
    cursor.insertText(deckList.getComments());
    cursor.insertBlock(headerBlockFormat, headerCharFormat);

    for (auto zoneNode : deckList.getZoneNodes()) {
        cursor.insertHtml("<br><img src=theme:hr.jpg>");
        cursor.insertBlock(headerBlockFormat, headerCharFormat);

        printDeckListNode(&cursor, zoneNode);
    }

    doc.print(printer);
}
