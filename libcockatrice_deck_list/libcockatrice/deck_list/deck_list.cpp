#include "deck_list.h"

#include "deck_list_memento.h"
#include "deck_list_metadata_xml.h"
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

bool DeckList::Metadata::isEmpty() const
{
    return name.isEmpty() && comments.isEmpty() && bannerCard.isEmpty() && tags.isEmpty() && playmat.card.isEmpty();
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
        if (DeckListMetadataXml::readElement(xml, childName, metadata)) {
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

    DeckListMetadataXml::write(xml, metadata);

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
            if (xml->name().toString() != "cockatrice_deck") {
                return false;
            }
            while (!xml->atEnd()) {
                xml->readNext();
                if (!readElement(xml)) {
                    break;
                }
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
