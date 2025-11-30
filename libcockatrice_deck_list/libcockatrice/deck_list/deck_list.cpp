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

SideboardPlan::SideboardPlan(const QString &_name, const QList<MoveCard_ToZone> &_moveList)
    : name(_name), moveList(_moveList)
{
}

void SideboardPlan::setMoveList(const QList<MoveCard_ToZone> &_moveList)
{
    moveList = _moveList;
}

bool SideboardPlan::readElement(QXmlStreamReader *xml)
{
    while (!xml->atEnd()) {
        xml->readNext();
        const QString childName = xml->name().toString();
        if (xml->isStartElement()) {
            if (childName == "name")
                name = xml->readElementText();
            else if (childName == "move_card_to_zone") {
                MoveCard_ToZone m;
                while (!xml->atEnd()) {
                    xml->readNext();
                    const QString childName2 = xml->name().toString();
                    if (xml->isStartElement()) {
                        if (childName2 == "card_name")
                            m.set_card_name(xml->readElementText().toStdString());
                        else if (childName2 == "start_zone")
                            m.set_start_zone(xml->readElementText().toStdString());
                        else if (childName2 == "target_zone")
                            m.set_target_zone(xml->readElementText().toStdString());
                    } else if (xml->isEndElement() && (childName2 == "move_card_to_zone")) {
                        moveList.append(m);
                        break;
                    }
                }
            }
        } else if (xml->isEndElement() && (childName == "sideboard_plan"))
            return true;
    }
    return false;
}

void SideboardPlan::write(QXmlStreamWriter *xml)
{
    xml->writeStartElement("sideboard_plan");
    xml->writeTextElement("name", name);
    for (auto &i : moveList) {
        xml->writeStartElement("move_card_to_zone");
        xml->writeTextElement("card_name", QString::fromStdString(i.card_name()));
        xml->writeTextElement("start_zone", QString::fromStdString(i.start_zone()));
        xml->writeTextElement("target_zone", QString::fromStdString(i.target_zone()));
        xml->writeEndElement();
    }
    xml->writeEndElement();
}

bool DeckList::Metadata::isEmpty() const
{
    return name.isEmpty() && comments.isEmpty() && bannerCard.isEmpty() && tags.isEmpty();
}

DeckList::DeckList()
{
    root = new InnerDecklistNode;
}

DeckList::DeckList(const DeckList &other)
    : metadata(other.metadata), sideboardPlans(other.sideboardPlans), root(new InnerDecklistNode(other.getRoot())),
      cachedDeckHash(other.cachedDeckHash)
{
}

DeckList::DeckList(const QString &nativeString)
{
    root = new InnerDecklistNode;
    loadFromString_Native(nativeString);
}

DeckList::~DeckList()
{
    delete root;

    QMapIterator<QString, SideboardPlan *> i(sideboardPlans);
    while (i.hasNext())
        delete i.next().value();
}

QList<MoveCard_ToZone> DeckList::getCurrentSideboardPlan()
{
    SideboardPlan *current = sideboardPlans.value(QString(), 0);
    if (!current)
        return QList<MoveCard_ToZone>();
    else
        return current->getMoveList();
}

void DeckList::setCurrentSideboardPlan(const QList<MoveCard_ToZone> &plan)
{
    SideboardPlan *current = sideboardPlans.value(QString(), 0);
    if (!current) {
        current = new SideboardPlan;
        sideboardPlans.insert(QString(), current);
    }

    current->setMoveList(plan);
}

bool DeckList::readElement(QXmlStreamReader *xml)
{
    const QString childName = xml->name().toString();
    if (xml->isStartElement()) {
        if (childName == "lastLoadedTimestamp") {
            metadata.lastLoadedTimestamp = xml->readElementText();
        } else if (childName == "deckname") {
            metadata.name = xml->readElementText();
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
            InnerDecklistNode *newZone = getZoneObjFromName(xml->attributes().value("name").toString());
            newZone->readElement(xml);
        } else if (childName == "sideboard_plan") {
            SideboardPlan *newSideboardPlan = new SideboardPlan;
            if (newSideboardPlan->readElement(xml)) {
                sideboardPlans.insert(newSideboardPlan->getName(), newSideboardPlan);
            } else {
                delete newSideboardPlan;
            }
        }
    } else if (xml->isEndElement() && (childName == "cockatrice_deck")) {
        return false;
    }
    return true;
}

void writeMetadata(QXmlStreamWriter *xml, const DeckList::Metadata &metadata)
{
    xml->writeTextElement("lastLoadedTimestamp", metadata.lastLoadedTimestamp);
    xml->writeTextElement("deckname", metadata.name);
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
    for (int i = 0; i < root->size(); i++) {
        root->at(i)->writeElement(xml);
    }

    // Write sideboard plans
    QMapIterator<QString, SideboardPlan *> i(sideboardPlans);
    while (i.hasNext()) {
        i.next().value()->write(xml);
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

bool DeckList::saveToFile_Native(QIODevice *device)
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
        new DecklistCardNode(cardName, amount, getZoneObjFromName(zoneName), -1, setCode, collectorNumber);
    }

    refreshDeckHash();
    return true;
}

InnerDecklistNode *DeckList::getZoneObjFromName(const QString &zoneName)
{
    for (int i = 0; i < root->size(); i++) {
        auto *node = dynamic_cast<InnerDecklistNode *>(root->at(i));
        if (node->getName() == zoneName) {
            return node;
        }
    }

    return new InnerDecklistNode(zoneName, root);
}

bool DeckList::loadFromFile_Plain(QIODevice *device)
{
    QTextStream in(device);
    return loadFromStream_Plain(in, false);
}

bool DeckList::saveToStream_Plain(QTextStream &stream, bool prefixSideboardCards, bool slashTappedOutSplitCards)
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

bool DeckList::saveToFile_Plain(QIODevice *device, bool prefixSideboardCards, bool slashTappedOutSplitCards)
{
    QTextStream out(device);
    return saveToStream_Plain(out, prefixSideboardCards, slashTappedOutSplitCards);
}

QString DeckList::writeToString_Plain(bool prefixSideboardCards, bool slashTappedOutSplitCards)
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
    root->clearTree();
    if (!preserveMetadata) {
        metadata = {};
    }
    refreshDeckHash();
}

void DeckList::getCardListHelper(InnerDecklistNode *item, QSet<QString> &result)
{
    for (int i = 0; i < item->size(); ++i) {
        auto *node = dynamic_cast<DecklistCardNode *>(item->at(i));

        if (node) {
            result.insert(node->getName());
        } else {
            getCardListHelper(dynamic_cast<InnerDecklistNode *>(item->at(i)), result);
        }
    }
}

void DeckList::getCardRefListHelper(InnerDecklistNode *item, QList<CardRef> &result)
{
    for (int i = 0; i < item->size(); ++i) {
        auto *node = dynamic_cast<DecklistCardNode *>(item->at(i));

        if (node) {
            result.append(node->toCardRef());
        } else {
            getCardRefListHelper(dynamic_cast<InnerDecklistNode *>(item->at(i)), result);
        }
    }
}

QStringList DeckList::getCardList() const
{
    QSet<QString> result;
    getCardListHelper(root, result);
    return result.values();
}

QList<CardRef> DeckList::getCardRefList() const
{
    QList<CardRef> result;
    getCardRefListHelper(root, result);
    return result;
}

QList<DecklistCardNode *> DeckList::getCardNodes(const QStringList &restrictToZones) const
{
    QList<DecklistCardNode *> result;

    for (auto *node : *root) {
        auto *zoneNode = dynamic_cast<InnerDecklistNode *>(node);
        if (zoneNode == nullptr) {
            continue;
        }
        if (!restrictToZones.isEmpty() && !restrictToZones.contains(node->getName())) {
            continue;
        }
        for (auto *cardNode : *zoneNode) {
            auto *cardCardNode = dynamic_cast<DecklistCardNode *>(cardNode);
            if (cardCardNode != nullptr) {
                result.append(cardCardNode);
            }
        }
    }

    return result;
}

int DeckList::getSideboardSize() const
{
    int size = 0;
    for (int i = 0; i < root->size(); ++i) {
        auto *node = dynamic_cast<InnerDecklistNode *>(root->at(i));
        if (node->getName() != DECK_ZONE_SIDE) {
            continue;
        }

        for (int j = 0; j < node->size(); j++) {
            auto *card = dynamic_cast<DecklistCardNode *>(node->at(j));
            size += card->getNumber();
        }
    }
    return size;
}

DecklistCardNode *DeckList::addCard(const QString &cardName,
                                    const QString &zoneName,
                                    const int position,
                                    const QString &cardSetName,
                                    const QString &cardSetCollectorNumber,
                                    const QString &cardProviderId)
{
    auto *zoneNode = dynamic_cast<InnerDecklistNode *>(root->findChild(zoneName));
    if (zoneNode == nullptr) {
        zoneNode = new InnerDecklistNode(zoneName, root);
    }

    auto *node =
        new DecklistCardNode(cardName, 1, zoneNode, position, cardSetName, cardSetCollectorNumber, cardProviderId);
    refreshDeckHash();

    return node;
}

bool DeckList::deleteNode(AbstractDecklistNode *node, InnerDecklistNode *rootNode)
{
    if (node == root) {
        return true;
    }

    bool updateHash = false;
    if (rootNode == nullptr) {
        rootNode = root;
        updateHash = true;
    }

    int index = rootNode->indexOf(node);
    if (index != -1) {
        delete rootNode->takeAt(index);

        if (rootNode->empty()) {
            deleteNode(rootNode, rootNode->getParent());
        }

        if (updateHash) {
            refreshDeckHash();
        }

        return true;
    }

    for (int i = 0; i < rootNode->size(); i++) {
        auto *inner = dynamic_cast<InnerDecklistNode *>(rootNode->at(i));
        if (inner) {
            if (deleteNode(node, inner)) {
                if (updateHash) {
                    refreshDeckHash();
                }

                return true;
            }
        }
    }

    return false;
}

static QString computeDeckHash(const InnerDecklistNode *root)
{
    QStringList cardList;
    QSet<QString> hashZones, optionalZones;

    hashZones << DECK_ZONE_MAIN << DECK_ZONE_SIDE; // Zones in deck to be included in hashing process
    optionalZones << DECK_ZONE_TOKENS;             // Optional zones in deck not included in hashing process

    for (int i = 0; i < root->size(); i++) {
        auto *node = dynamic_cast<InnerDecklistNode *>(root->at(i));
        for (int j = 0; j < node->size(); j++) {
            if (hashZones.contains(node->getName())) // Mainboard or Sideboard
            {
                auto *card = dynamic_cast<DecklistCardNode *>(node->at(j));
                for (int k = 0; k < card->getNumber(); ++k) {
                    cardList.append((node->getName() == DECK_ZONE_SIDE ? "SB:" : "") + card->getName().toLower());
                }
            }
        }
    }
    cardList.sort();
    QByteArray deckHashArray = QCryptographicHash::hash(cardList.join(";").toUtf8(), QCryptographicHash::Sha1);
    quint64 number = (((quint64)(unsigned char)deckHashArray[0]) << 32) +
                     (((quint64)(unsigned char)deckHashArray[1]) << 24) +
                     (((quint64)(unsigned char)deckHashArray[2] << 16)) +
                     (((quint64)(unsigned char)deckHashArray[3]) << 8) + (quint64)(unsigned char)deckHashArray[4];
    return QString::number(number, 32).rightJustified(8, '0');
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

    cachedDeckHash = computeDeckHash(root);
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
    // Support for this is only possible if the internal structure
    // doesn't get more complicated.
    for (int i = 0; i < root->size(); i++) {
        InnerDecklistNode *node = dynamic_cast<InnerDecklistNode *>(root->at(i));
        for (int j = 0; j < node->size(); j++) {
            DecklistCardNode *card = dynamic_cast<DecklistCardNode *>(node->at(j));
            func(node, card);
        }
    }
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
