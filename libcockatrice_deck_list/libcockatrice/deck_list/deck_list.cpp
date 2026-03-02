#include "deck_list.h"

#include "deck_list_memento.h"
#include "tree/abstract_deck_list_node.h"
#include "tree/deck_list_card_node.h"
#include "tree/inner_deck_list_node.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QSet>
#include <QTextStream>
#include <algorithm>

#if QT_VERSION < 0x050600
// qHash on QRegularExpression was added in 5.6, FIX IT
uint qHash(const QRegularExpression &key, uint seed) noexcept
{
    return qHash(key.pattern(), seed); // call qHash on pattern QString instead
}
#endif

static const QString CURRENT_SIDEBOARD_PLAN_KEY = "";

bool DeckList::Metadata::isEmpty() const
{
    return name.isEmpty() && comments.isEmpty() && bannerCard.isEmpty() && tags.isEmpty();
}

DeckList::DeckList()
{
}

DeckList::DeckList(const QString &nativeString)
{
    loadFromString_Native(nativeString);
}

DeckList::DeckList(const Metadata &metadata,
                   const DecklistNodeTree &tree,
                   const QMap<QString, SideboardPlan> &sideboardPlans)
    : metadata(metadata), sideboardPlans(sideboardPlans), tree(tree)
{
}

QList<MoveCard_ToZone> DeckList::getCurrentSideboardPlan() const
{
    if (!sideboardPlans.contains(CURRENT_SIDEBOARD_PLAN_KEY)) {
        return {};
    }

    return sideboardPlans.value(CURRENT_SIDEBOARD_PLAN_KEY).getMoveList();
}

void DeckList::setCurrentSideboardPlan(const QList<MoveCard_ToZone> &plan)
{
    sideboardPlans[CURRENT_SIDEBOARD_PLAN_KEY].setMoveList(plan);
}

bool DeckList::readElement(QXmlStreamReader *xml)
{
    const QString childName = xml->name().toString();
    if (xml->isStartElement()) {
        if (childName == "lastLoadedTimestamp") {
            metadata.lastLoadedTimestamp = xml->readElementText();
        } else if (childName == "deckname") {
            metadata.name = xml->readElementText();
        } else if (childName == "format") {
            metadata.gameFormat = xml->readElementText();
        } else if (childName == "comments") {
            metadata.comments = xml->readElementText();
        } else if (childName == "bannerCard") {
            QString providerId = xml->attributes().value("providerId").toString();
            QString cardName = xml->readElementText();
            metadata.bannerCard = {cardName, providerId};
        } else if (childName == "tags") {
            metadata.tags.clear(); // Clear existing tags
            while (xml->readNextStartElement()) {
                if (xml->name().toString() == "tag") {
                    metadata.tags.append(xml->readElementText());
                }
            }
        } else if (childName == "zone") {
            tree.readZoneElement(xml);
        } else if (childName == "sideboard_plan") {
            SideboardPlan newSideboardPlan;
            if (newSideboardPlan.readElement(xml)) {
                sideboardPlans.insert(newSideboardPlan.getName(), newSideboardPlan);
            }
        }
    } else if (xml->isEndElement() && (childName == "cockatrice_deck")) {
        return false;
    }
    return true;
}

static void writeMetadata(QXmlStreamWriter *xml, const DeckList::Metadata &metadata)
{
    xml->writeTextElement("lastLoadedTimestamp", metadata.lastLoadedTimestamp);
    xml->writeTextElement("deckname", metadata.name);
    xml->writeTextElement("format", metadata.gameFormat);
    xml->writeStartElement("bannerCard");
    xml->writeAttribute("providerId", metadata.bannerCard.providerId);
    xml->writeCharacters(metadata.bannerCard.name);
    xml->writeEndElement();
    xml->writeTextElement("comments", metadata.comments);

    // Write tags
    xml->writeStartElement("tags");
    for (const QString &tag : metadata.tags) {
        xml->writeTextElement("tag", tag);
    }
    xml->writeEndElement();
}

void DeckList::write(QXmlStreamWriter *xml) const
{
    xml->writeStartElement("cockatrice_deck");
    xml->writeAttribute("version", "1");

    writeMetadata(xml, metadata);

    // Write zones
    tree.write(xml);

    // Write sideboard plans
    for (auto &sideboardPlan : sideboardPlans.values()) {
        sideboardPlan.write(xml);
    }

    xml->writeEndElement(); // Close "cockatrice_deck"
}

bool DeckList::loadFromXml(QXmlStreamReader *xml)
{
    if (xml->error()) {
        qDebug() << "Error loading deck from xml: " << xml->errorString();
        return false;
    }

    cleanList();
    while (!xml->atEnd()) {
        xml->readNext();
        if (xml->isStartElement()) {
            if (xml->name().toString() != "cockatrice_deck")
                return false;
            while (!xml->atEnd()) {
                xml->readNext();
                if (!readElement(xml))
                    break;
            }
        }
    }
    refreshDeckHash();
    if (xml->error()) {
        qDebug() << "Error loading deck from xml: " << xml->errorString();
        return false;
    }
    return true;
}

bool DeckList::loadFromString_Native(const QString &nativeString)
{
    QXmlStreamReader xml(nativeString);
    return loadFromXml(&xml);
}

QString DeckList::writeToString_Native() const
{
    QString result;
    QXmlStreamWriter xml(&result);
    xml.writeStartDocument();
    write(&xml);
    xml.writeEndDocument();
    return result;
}

bool DeckList::loadFromFile_Native(QIODevice *device)
{
    QXmlStreamReader xml(device);
    return loadFromXml(&xml);
}

bool DeckList::saveToFile_Native(QIODevice *device) const
{
    QXmlStreamWriter xml(device);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();

    write(&xml);

    xml.writeEndDocument();
    return true;
}

/**
 * Clears the decklist and loads in a new deck from text
 *
 * @param in The text to load
 * @param preserveMetadata If true, don't clear the existing metadata
 * @return False if the input was empty, true otherwise.
 */
bool DeckList::loadFromStream_Plain(QTextStream &in, bool preserveMetadata)
{
    const QRegularExpression reCardLine(R"(^\s*[\w\[\(\{].*$)", QRegularExpression::UseUnicodePropertiesOption);
    const QRegularExpression reEmpty("^\\s*$");
    const QRegularExpression reComment(R"([\w\[\(\{].*$)", QRegularExpression::UseUnicodePropertiesOption);
    const QRegularExpression reSBMark("^\\s*sb:\\s*(.+)", QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression reSBComment("^sideboard\\b.*$", QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression reDeckComment("^((main)?deck(list)?|mainboard)\\b",
                                           QRegularExpression::CaseInsensitiveOption);

    // Regex for advanced card parsing
    const QRegularExpression reMultiplier(R"(^[xX\(\[]*(\d+)[xX\*\)\]]* ?(.+))");
    const QRegularExpression reSplitCard(R"( ?\/\/ ?)");
    const QRegularExpression reBrace(R"( ?[\[\{][^\]\}]*[\]\}] ?)"); // not nested
    const QRegularExpression reRoundBrace(R"(^\([^\)]*\) ?)");       // () are only matched at start of string
    const QRegularExpression reDigitBrace(R"( ?\(\d*\) ?)");         // () are matched if containing digits
    const QRegularExpression reBraceDigit(
        R"( ?\([\dA-Z]+\) *\d+$)"); // () are matched if containing setcode then a number
    const QRegularExpression reDoubleFacedMarker(R"( ?\(Transform\) ?)");

    // Regex for extracting set code and collector number with attached symbols
    const QRegularExpression reHyphenFormat(R"(\((\w{3,})\)\s+(\w{3,})-(\d+[^\w\s]*))");
    const QRegularExpression reRegularFormat(R"(\((\w{3,})\)\s+(\d+[^\w\s]*))");

    const QHash<QRegularExpression, QString> differences{{QRegularExpression("’"), QString("'")},
                                                         {QRegularExpression("Æ"), QString("Ae")},
                                                         {QRegularExpression("æ"), QString("ae")},
                                                         {QRegularExpression(" ?[|/]+ ?"), QString(" // ")}};

    cleanList(preserveMetadata);

    auto inputs = in.readAll().trimmed().split('\n');
    auto max_line = inputs.size();

    // Start at the first empty line before the first card line
    auto deckStart = inputs.indexOf(reCardLine);
    if (deckStart == -1) {
        if (inputs.indexOf(reComment) == -1) {
            return false; // Input is empty
        }
        deckStart = max_line;
    } else {
        deckStart = inputs.lastIndexOf(reEmpty, deckStart);
        if (deckStart == -1) {
            deckStart = 0;
        }
    }

    // find sideboard position, if marks are used this won't be needed
    int sBStart = -1;
    if (inputs.indexOf(reSBMark, deckStart) == -1) {
        sBStart = inputs.indexOf(reSBComment, deckStart);
        if (sBStart == -1) {
            sBStart = inputs.indexOf(reEmpty, deckStart + 1);
            if (sBStart == -1) {
                sBStart = max_line;
            }
            auto nextCard = inputs.indexOf(reCardLine, sBStart + 1);
            if (inputs.indexOf(reEmpty, nextCard + 1) != -1) {
                sBStart = max_line;
            }
        }
    }

    int index = 0;
    QRegularExpressionMatch match;

    // Parse name and comments
    while (index < deckStart) {
        const auto &current = inputs.at(index++);
        if (!current.contains(reEmpty)) {
            match = reComment.match(current);
            metadata.name = match.captured();
            break;
        }
    }
    while (index < deckStart) {
        const auto &current = inputs.at(index++);
        if (!current.contains(reEmpty)) {
            match = reComment.match(current);
            metadata.comments += match.captured() + '\n';
        }
    }
    metadata.comments.chop(1);

    // Discard empty lines
    while (index < max_line && inputs.at(index).contains(reEmpty)) {
        ++index;
    }

    // Discard line if it starts with deck or mainboard, all cards until the sideboard starts are in the mainboard
    if (inputs.at(index).contains(reDeckComment)) {
        ++index;
    }

    // Parse decklist
    for (; index < max_line; ++index) {
        // check if line is a card
        match = reCardLine.match(inputs.at(index));
        if (!match.hasMatch())
            continue;

        QString cardName = match.captured().simplified();
        bool sideboard = false;

        // Sideboard detection
        if (sBStart < 0) {
            match = reSBMark.match(cardName);
            if (match.hasMatch()) {
                sideboard = true;
                cardName = match.captured(1);
            }
        } else {
            if (index == sBStart)
                continue;
            sideboard = index > sBStart;
        }

        // Extract set code, collector number, and foil
        QString setCode;
        QString collectorNumber;
        bool isFoil = false;

        // Check for foil status at the end of the card name
        if (cardName.endsWith("*F*", Qt::CaseInsensitive)) {
            isFoil = true;
            cardName.chop(3); // Remove the "*F*" from the card name
        }
        Q_UNUSED(isFoil);

        // Attempt to match the hyphen-separated format (PLST-2094)
        match = reHyphenFormat.match(cardName);
        if (match.hasMatch()) {
            setCode = match.captured(2).toUpper();
            collectorNumber = match.captured(3);
            cardName = cardName.left(match.capturedStart()).trimmed();
        } else {
            // Attempt to match the regular format (PLST) 2094
            match = reRegularFormat.match(cardName);
            if (match.hasMatch()) {
                setCode = match.captured(1).toUpper();
                collectorNumber = match.captured(2);
                cardName = cardName.left(match.capturedStart()).trimmed();
            }
        }

        // check if a specific amount is mentioned
        int amount = 1;
        match = reMultiplier.match(cardName);
        if (match.hasMatch()) {
            amount = match.captured(1).toInt();
            cardName = match.captured(2);
        }

        // Handle advanced card types
        if (cardName.contains(reSplitCard)) {
            cardName = cardName.split(reSplitCard).join(" // ");
        }

        if (cardName.contains(reDoubleFacedMarker)) {
            QStringList faces = cardName.split(reDoubleFacedMarker);
            cardName = faces.first().trimmed();
        }

        // Remove unnecessary characters
        cardName.remove(reBrace);
        cardName.remove(reRoundBrace); // I'll be entirely honest here, these are split to accommodate just three cards
        cardName.remove(reDigitBrace); // from un-sets that have a word in between round braces at the end
        cardName.remove(reBraceDigit); // very specific format with the set code in () and collectors number after

        // Normalize names
        for (auto diff = differences.constBegin(); diff != differences.constEnd(); ++diff) {
            cardName.replace(diff.key(), diff.value());
        }

        // Determine the zone (mainboard/sideboard)
        QString zoneName = sideboard ? DECK_ZONE_SIDE : DECK_ZONE_MAIN;

        // make new entry in decklist
        tree.addCard(cardName, amount, zoneName, -1, setCode, collectorNumber);
    }

    refreshDeckHash();
    return true;
}

bool DeckList::loadFromFile_Plain(QIODevice *device)
{
    QTextStream in(device);
    return loadFromStream_Plain(in, false);
}

bool DeckList::saveToStream_Plain(QTextStream &stream, bool prefixSideboardCards, bool slashTappedOutSplitCards) const
{
    auto writeToStream = [&stream, prefixSideboardCards, slashTappedOutSplitCards](const auto node, const auto card) {
        if (prefixSideboardCards && node->getName() == DECK_ZONE_SIDE) {
            stream << "SB: ";
        }
        if (!slashTappedOutSplitCards) {
            stream << QString("%1 %2\n").arg(card->getNumber()).arg(card->getName());
        } else {
            stream << QString("%1 %2\n").arg(card->getNumber()).arg(card->getName().replace("//", "/"));
        }
    };

    forEachCard(writeToStream);
    return true;
}

bool DeckList::saveToFile_Plain(QIODevice *device, bool prefixSideboardCards, bool slashTappedOutSplitCards) const
{
    QTextStream out(device);
    return saveToStream_Plain(out, prefixSideboardCards, slashTappedOutSplitCards);
}

QString DeckList::writeToString_Plain(bool prefixSideboardCards, bool slashTappedOutSplitCards) const
{
    QString result;
    QTextStream out(&result);
    saveToStream_Plain(out, prefixSideboardCards, slashTappedOutSplitCards);
    return result;
}

/**
 * Clears all cards and other data from the decklist
 *
 * @param preserveMetadata If true, only clear the cards
 */
void DeckList::cleanList(bool preserveMetadata)
{
    tree.clear();
    if (!preserveMetadata) {
        metadata = {};
    }
    refreshDeckHash();
}

QStringList DeckList::getCardList(const QSet<QString> &restrictToZones) const
{
    auto nodes = tree.getCardNodes(restrictToZones);

    QStringList result;
    std::transform(nodes.cbegin(), nodes.cend(), std::back_inserter(result), [](auto node) { return node->getName(); });

    return result;
}

QList<CardRef> DeckList::getCardRefList(const QSet<QString> &restrictToZones) const
{
    auto nodes = tree.getCardNodes(restrictToZones);

    QList<CardRef> result;
    std::transform(nodes.cbegin(), nodes.cend(), std::back_inserter(result),
                   [](auto node) { return node->toCardRef(); });

    return result;
}

QList<const DecklistCardNode *> DeckList::getCardNodes(const QSet<QString> &restrictToZones) const
{
    return tree.getCardNodes(restrictToZones);
}

QList<const InnerDecklistNode *> DeckList::getZoneNodes(const QSet<QString> &restrictToZones) const
{
    return tree.getZoneNodes(restrictToZones);
}

int DeckList::getSideboardSize() const
{
    auto cards = tree.getCardNodes({DECK_ZONE_SIDE});

    int size = 0;
    for (auto card : cards) {
        size += card->getNumber();
    }

    return size;
}

DecklistCardNode *DeckList::addCard(const QString &cardName,
                                    const QString &zoneName,
                                    int position,
                                    const QString &cardSetName,
                                    const QString &cardSetCollectorNumber,
                                    const QString &cardProviderId,
                                    bool formatLegal)
{
    auto node =
        tree.addCard(cardName, 1, zoneName, position, cardSetName, cardSetCollectorNumber, cardProviderId, formatLegal);
    refreshDeckHash();
    return node;
}

/**
 * Gets the deck hash.
 * The hash is computed on the first call to this method, and is cached until the decklist is modified.
 *
 * @return The deck hash
 */
QString DeckList::getDeckHash() const
{
    if (!cachedDeckHash.isEmpty()) {
        return cachedDeckHash;
    }

    cachedDeckHash = tree.computeDeckHash();
    return cachedDeckHash;
}

/**
 * Invalidates the cached deckHash.
 */
void DeckList::refreshDeckHash()
{
    cachedDeckHash = QString();
}

/**
 * Calls a given function on each card in the deck.
 */
void DeckList::forEachCard(const std::function<void(InnerDecklistNode *, DecklistCardNode *)> &func) const
{
    tree.forEachCard(func);
}

DeckListMemento DeckList::createMemento(const QString &reason) const
{
    return DeckListMemento(writeToString_Native(), reason);
}

void DeckList::restoreMemento(const DeckListMemento &m)
{
    cleanList();
    loadFromString_Native(m.getMemento());
}
