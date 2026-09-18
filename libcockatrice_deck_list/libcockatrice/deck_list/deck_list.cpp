#include "deck_list.h"

#include "deck_list_memento.h"
#include "deck_list_plain_text_parser.h"
#include "tree/abstract_deck_list_node.h"
#include "tree/deck_list_card_node.h"
#include "tree/inner_deck_list_node.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QSet>
#include <QTextStream>
#include <algorithm>

static const QString CURRENT_SIDEBOARD_PLAN_KEY = "";

/**
 * @brief Parses a floating point XML attribute into a clamped playmat parameter.
 *
 * Falls back to @p fallback when the attribute is missing or malformed, so
 * malformed deck files cannot produce degenerate art rectangles (e.g. a zoom
 * of 0 dividing by zero).
 *
 * @param valueString Raw attribute text.
 * @param fallback Value used when the text cannot be parsed.
 * @param min Lower clamp bound.
 * @param max Upper clamp bound.
 * @return The parsed value clamped to [min, max], or @p fallback.
 */
static double parseClampedParam(const QString &valueString, double fallback, double min, double max)
{
    bool ok = false;
    const double value = valueString.toDouble(&ok);
    if (!ok) {
        return fallback;
    }
    return qBound(min, value, max);
}

/**
 * @brief Reads a `bannerCard` element from the XML stream.
 *
 * @param xml Reader positioned at the element.
 * @return The referenced card.
 */
static CardRef readBannerCard(QXmlStreamReader *xml)
{
    QString providerId = xml->attributes().value("providerId").toString();
    QString cardName = xml->readElementText();
    return {cardName, providerId};
}

/**
 * @brief Reads a `playmatCard` element from the XML stream.
 *
 * Attribute values are read before readElementText consumes the element, and
 * the params are clamped to the same ranges as the settings dialog and the
 * remote player-properties path so malformed deck files cannot produce
 * degenerate art rectangles (e.g. a zoom of 0 dividing by zero).
 *
 * @param xml Reader positioned at the element.
 * @return The referenced card plus its clamped positioning parameters.
 */
static PlaymatInfo readPlaymatCard(QXmlStreamReader *xml)
{
    QString providerId = xml->attributes().value("providerId").toString();
    QString marginLStr = xml->attributes().value("marginPctL").toString();
    QString marginRStr = xml->attributes().value("marginPctR").toString();
    QString vOffStr = xml->attributes().value("verticalOffset").toString();
    QString zoomStr = xml->attributes().value("zoom").toString();
    QString cardName = xml->readElementText();

    return {
        .card = {cardName, providerId},
        .params = {.marginPctL = parseClampedParam(marginLStr, 0.07, 0.0, 0.95),
                   .marginPctR = parseClampedParam(marginRStr, 0.07, 0.0, 0.95),
                   .verticalOffset = parseClampedParam(vOffStr, 0.33, 0.0, 1.0),
                   .zoom = parseClampedParam(zoomStr, 1.0, 0.1, 4.0)},
    };
}

bool DeckList::Metadata::isEmpty() const
{
    return name.isEmpty() && comments.isEmpty() && bannerCard.isEmpty() && tags.isEmpty() && playmat.card.isEmpty();
}

bool DeckList::Metadata::readElement(QXmlStreamReader *xml, const QString &childName)
{
    if (childName == "lastLoadedTimestamp") {
        lastLoadedTimestamp = xml->readElementText();
    } else if (childName == "deckname") {
        name = xml->readElementText();
    } else if (childName == "format") {
        gameFormat = xml->readElementText();
    } else if (childName == "comments") {
        comments = xml->readElementText();
    } else if (childName == "bannerCard") {
        bannerCard = readBannerCard(xml);
    } else if (childName == "playmatCard") {
        playmat = readPlaymatCard(xml);
    } else if (childName == "tags") {
        tags.clear(); // Clear existing tags
        while (xml->readNextStartElement()) {
            if (xml->name().toString() == "tag") {
                tags.append(xml->readElementText());
            }
        }
    } else {
        return false;
    }
    return true;
}

void DeckList::Metadata::write(QXmlStreamWriter *xml) const
{
    xml->writeTextElement("lastLoadedTimestamp", lastLoadedTimestamp);
    xml->writeTextElement("deckname", name);
    xml->writeTextElement("format", gameFormat);
    xml->writeStartElement("bannerCard");
    xml->writeAttribute("providerId", bannerCard.providerId);
    xml->writeCharacters(bannerCard.name);
    xml->writeEndElement();
    if (!playmat.card.isEmpty()) {
        xml->writeStartElement("playmatCard");
        xml->writeAttribute("providerId", playmat.card.providerId);
        xml->writeAttribute("marginPctL", QString::number(playmat.params.marginPctL, 'f', 4));
        xml->writeAttribute("marginPctR", QString::number(playmat.params.marginPctR, 'f', 4));
        xml->writeAttribute("verticalOffset", QString::number(playmat.params.verticalOffset, 'f', 4));
        xml->writeAttribute("zoom", QString::number(playmat.params.zoom, 'f', 4));
        xml->writeCharacters(playmat.card.name);
        xml->writeEndElement();
    }
    xml->writeTextElement("comments", comments);

    // Write tags
    xml->writeStartElement("tags");
    for (const QString &tag : tags) {
        xml->writeTextElement("tag", tag);
    }
    xml->writeEndElement();
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
        if (metadata.readElement(xml, childName)) {
            return true;
        }
        if (childName == "zone") {
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

void DeckList::write(QXmlStreamWriter *xml) const
{
    xml->writeStartElement("cockatrice_deck");
    xml->writeAttribute("version", "1");

    metadata.write(xml);

    // Write zones
    tree.write(xml);

    // Write sideboard plans
    for (auto &sideboardPlan : sideboardPlans.values()) {
        sideboardPlan.write(xml);
    }

    xml->writeEndElement(); // Close "cockatrice_deck"
}

bool DeckList::seekToNextElement(QXmlStreamReader *xml)
{
    while (!xml->atEnd()) {
        xml->readNext();
        if (xml->isStartElement()) {
            return true;
        }
    }
    return false;
}

void DeckList::readDeckBody(QXmlStreamReader *xml)
{
    while (!xml->atEnd()) {
        xml->readNext();
        if (!readElement(xml)) {
            break;
        }
    }
}

bool DeckList::loadFromXml(QXmlStreamReader *xml)
{
    if (xml->error()) {
        qDebug() << "Error loading deck from xml: " << xml->errorString();
        return false;
    }

    cleanList();
    while (seekToNextElement(xml)) {
        if (xml->name().toString() != "cockatrice_deck") {
            return false;
        }
        readDeckBody(xml);
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
 * @param cardNameNormalizer Function that takes the parsed card name string in the text and
 * @return False if the input was empty, true otherwise.
 */
bool DeckList::loadFromStream_Plain(QTextStream &in,
                                    bool preserveMetadata,
                                    const std::function<QString(const QString &)> &cardNameNormalizer)
{
    if (!preserveMetadata) {
        metadata = {};
    }
    bool ok = DeckListPlainText::parse(in, cardNameNormalizer, metadata, tree);
    refreshDeckHash();
    return ok;
}

bool DeckList::loadFromFile_Plain(QIODevice *device, const std::function<QString(const QString &)> &cardNameNormalizer)
{
    QTextStream in(device);
    return loadFromStream_Plain(in, false, cardNameNormalizer);
}

bool DeckList::saveToStream_Plain(QTextStream &stream, bool prefixSideboardCards, bool slashTappedOutSplitCards) const
{
    auto writeToStream = [&stream, prefixSideboardCards, slashTappedOutSplitCards](const auto node, const auto card) {
        // The maybeboard is scratch space and never exported.
        if (node->getName() == DECK_ZONE_MAYBEBOARD) {
            return;
        }
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
