#include "decklist.h"
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QTextStream>

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

AbstractDecklistNode::AbstractDecklistNode(InnerDecklistNode *_parent) : parent(_parent), sortMethod(Default)
{
    if (parent) {
        parent->append(this);
    }
}

int AbstractDecklistNode::depth() const
{
    if (parent) {
        return parent->depth() + 1;
    } else {
        return 0;
    }
}

InnerDecklistNode::InnerDecklistNode(InnerDecklistNode *other, InnerDecklistNode *_parent)
    : AbstractDecklistNode(_parent), name(other->getName())
{
    for (int i = 0; i < other->size(); ++i) {
        auto *inner = dynamic_cast<InnerDecklistNode *>(other->at(i));
        if (inner) {
            new InnerDecklistNode(inner, this);
        } else {
            new DecklistCardNode(dynamic_cast<DecklistCardNode *>(other->at(i)), this);
        }
    }
}

InnerDecklistNode::~InnerDecklistNode()
{
    clearTree();
}

QString InnerDecklistNode::visibleNameFromName(const QString &_name)
{
    if (_name == DECK_ZONE_MAIN) {
        return QObject::tr("Maindeck");
    } else if (_name == DECK_ZONE_SIDE) {
        return QObject::tr("Sideboard");
    } else if (_name == DECK_ZONE_TOKENS) {
        return QObject::tr("Tokens");
    } else {
        return _name;
    }
}

void InnerDecklistNode::setSortMethod(DeckSortMethod method)
{
    sortMethod = method;
    for (int i = 0; i < size(); i++) {
        at(i)->setSortMethod(method);
    }
}

QString InnerDecklistNode::getVisibleName() const
{
    return visibleNameFromName(name);
}

void InnerDecklistNode::clearTree()
{
    for (int i = 0; i < size(); i++)
        delete at(i);
    clear();
}

DecklistCardNode::DecklistCardNode(DecklistCardNode *other, InnerDecklistNode *_parent)
    : AbstractDecklistCardNode(_parent), name(other->getName()), number(other->getNumber())
{
}

AbstractDecklistNode *InnerDecklistNode::findChild(const QString &name)
{
    for (int i = 0; i < size(); i++) {
        if (at(i)->getName() == name) {
            return at(i);
        }
    }
    return nullptr;
}

int InnerDecklistNode::height() const
{
    return at(0)->height() + 1;
}

int InnerDecklistNode::recursiveCount(bool countTotalCards) const
{
    int result = 0;
    for (int i = 0; i < size(); i++) {
        auto *node = dynamic_cast<InnerDecklistNode *>(at(i));

        if (node) {
            result += node->recursiveCount(countTotalCards);
        } else if (countTotalCards) {
            result += dynamic_cast<AbstractDecklistCardNode *>(at(i))->getNumber();
        } else {
            result++;
        }
    }
    return result;
}

bool InnerDecklistNode::compare(AbstractDecklistNode *other) const
{
    switch (sortMethod) {
        case ByNumber:
            return compareNumber(other);
        case ByName:
            return compareName(other);
        default:
            return false;
    }
}

bool InnerDecklistNode::compareNumber(AbstractDecklistNode *other) const
{
    auto *other2 = dynamic_cast<InnerDecklistNode *>(other);
    if (other2) {
        int n1 = recursiveCount(true);
        int n2 = other2->recursiveCount(true);
        return (n1 != n2) ? (n1 > n2) : compareName(other);
    } else {
        return false;
    }
}

bool InnerDecklistNode::compareName(AbstractDecklistNode *other) const
{
    auto *other2 = dynamic_cast<InnerDecklistNode *>(other);
    if (other2) {
        return (getName() > other2->getName());
    } else {
        return false;
    }
}

bool AbstractDecklistCardNode::compare(AbstractDecklistNode *other) const
{
    switch (sortMethod) {
        case ByNumber:
            return compareNumber(other);
        case ByName:
            return compareName(other);
        default:
            return false;
    }
}

bool AbstractDecklistCardNode::compareNumber(AbstractDecklistNode *other) const
{
    auto *other2 = dynamic_cast<AbstractDecklistCardNode *>(other);
    if (other2) {
        int n1 = getNumber();
        int n2 = other2->getNumber();
        return (n1 != n2) ? (n1 > n2) : compareName(other);
    } else {
        return true;
    }
}

bool AbstractDecklistCardNode::compareName(AbstractDecklistNode *other) const
{
    auto *other2 = dynamic_cast<AbstractDecklistCardNode *>(other);
    if (other2) {
        return (getName() > other2->getName());
    } else {
        return true;
    }
}

class InnerDecklistNode::compareFunctor
{
private:
    Qt::SortOrder order;

public:
    explicit compareFunctor(Qt::SortOrder _order) : order(_order)
    {
    }
    inline bool operator()(QPair<int, AbstractDecklistNode *> a, QPair<int, AbstractDecklistNode *> b) const
    {
        return (order == Qt::AscendingOrder) ^ (a.second->compare(b.second));
    }
};

bool InnerDecklistNode::readElement(QXmlStreamReader *xml)
{
    while (!xml->atEnd()) {
        xml->readNext();
        const QString childName = xml->name().toString();
        if (xml->isStartElement()) {
            if (childName == "zone") {
                InnerDecklistNode *newZone = new InnerDecklistNode(xml->attributes().value("name").toString(), this);
                newZone->readElement(xml);
            } else if (childName == "card") {
                DecklistCardNode *newCard =
                    new DecklistCardNode(xml->attributes().value("name").toString(),
                                         xml->attributes().value("number").toString().toInt(), this);
                newCard->readElement(xml);
            }
        } else if (xml->isEndElement() && (childName == "zone"))
            return false;
    }
    return true;
}

void InnerDecklistNode::writeElement(QXmlStreamWriter *xml)
{
    xml->writeStartElement("zone");
    xml->writeAttribute("name", name);
    for (int i = 0; i < size(); i++)
        at(i)->writeElement(xml);
    xml->writeEndElement(); // zone
}

bool AbstractDecklistCardNode::readElement(QXmlStreamReader *xml)
{
    while (!xml->atEnd()) {
        xml->readNext();
        if (xml->isEndElement() && xml->name() == "card")
            return false;
    }
    return true;
}

void AbstractDecklistCardNode::writeElement(QXmlStreamWriter *xml)
{
    xml->writeEmptyElement("card");
    xml->writeAttribute("number", QString::number(getNumber()));
    xml->writeAttribute("name", getName());
}

QVector<QPair<int, int>> InnerDecklistNode::sort(Qt::SortOrder order)
{
    QVector<QPair<int, int>> result(size());

    // Initialize temporary list with contents of current list
    QVector<QPair<int, AbstractDecklistNode *>> tempList(size());
    for (int i = size() - 1; i >= 0; --i) {
        tempList[i].first = i;
        tempList[i].second = at(i);
    }

    // Sort temporary list
    compareFunctor cmp(order);
    qSort(tempList.begin(), tempList.end(), cmp);

    // Map old indexes to new indexes and
    // copy temporary list to the current one
    for (int i = size() - 1; i >= 0; --i) {
        result[i].first = tempList[i].first;
        result[i].second = i;
        replace(i, tempList[i].second);
    }

    return result;
}

DeckList::DeckList()
{
    root = new InnerDecklistNode;
}

// TODO: http://qt-project.org/doc/qt-4.8/qobject.html#no-copy-constructor-or-assignment-operator
DeckList::DeckList(const DeckList &other)
    : QObject(), name(other.name), comments(other.comments), deckHash(other.deckHash)
{
    root = new InnerDecklistNode(other.getRoot());

    QMapIterator<QString, SideboardPlan *> spIterator(other.getSideboardPlans());
    while (spIterator.hasNext()) {
        spIterator.next();
        sideboardPlans.insert(spIterator.key(), new SideboardPlan(spIterator.key(), spIterator.value()->getMoveList()));
    }
    updateDeckHash();
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
        if (childName == "deckname")
            name = xml->readElementText();
        else if (childName == "comments")
            comments = xml->readElementText();
        else if (childName == "zone") {
            InnerDecklistNode *newZone = getZoneObjFromName(xml->attributes().value("name").toString());
            newZone->readElement(xml);
        } else if (childName == "sideboard_plan") {
            SideboardPlan *newSideboardPlan = new SideboardPlan;
            if (newSideboardPlan->readElement(xml))
                sideboardPlans.insert(newSideboardPlan->getName(), newSideboardPlan);
            else
                delete newSideboardPlan;
        }
    } else if (xml->isEndElement() && (childName == "cockatrice_deck"))
        return false;
    return true;
}

void DeckList::write(QXmlStreamWriter *xml)
{
    xml->writeStartElement("cockatrice_deck");
    xml->writeAttribute("version", "1");
    xml->writeTextElement("deckname", name);
    xml->writeTextElement("comments", comments);

    for (int i = 0; i < root->size(); i++)
        root->at(i)->writeElement(xml);

    QMapIterator<QString, SideboardPlan *> i(sideboardPlans);
    while (i.hasNext())
        i.next().value()->write(xml);
    xml->writeEndElement();
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
            if (xml->name() != "cockatrice_deck")
                return false;
            while (!xml->atEnd()) {
                xml->readNext();
                if (!readElement(xml))
                    break;
            }
        }
    }
    updateDeckHash();
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

QString DeckList::writeToString_Native()
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

bool DeckList::loadFromStream_Plain(QTextStream &in)
{
    cleanList();
    QVector<QString> inputs; // QTextStream -> QVector

    bool priorEntryIsBlank = true, isAtBeginning = true;
    int blankLines = 0;
    while (!in.atEnd()) {
        QString line = in.readLine().simplified().toLower();

        /*
         * Removes all blank lines at start of inputs
         * Ex: ("", "", "", "Card1", "Card2") => ("Card1", "Card2")
         *
         * This will also concise multiple blank lines in a row to just one blank
         * Ex: ("Card1", "Card2", "", "", "", "Card3") => ("Card1", "Card2", "", "Card3")
         */
        if (line.isEmpty()) {
            if (priorEntryIsBlank || isAtBeginning) {
                continue;
            }

            priorEntryIsBlank = true;
            blankLines++;
        } else {
            isAtBeginning = false;
            priorEntryIsBlank = false;
        }

        inputs.push_back(line);
    }

    /*
     * Removes  blank line at end of inputs (if applicable)
     * Ex: ("Card1", "Card2", "") => ("Card1", "Card2")
     * NOTE: Any duplicates were taken care of above, so there can be
     * at most one blank line at the very end
     */
    if (!inputs.empty() && inputs.last().isEmpty()) {
        blankLines--;
        inputs.erase(inputs.end() - 1);
    }

    // If "Sideboard" line appears in inputs, then blank lines mean nothing
    if (inputs.contains("sideboard")) {
        blankLines = 2;
    }

    bool inSideboard = false, titleFound = false, isSideboard;
    int okRows = 0;

    foreach (QString line, inputs) {
        // This is a comment line, ignore it
        if (line.startsWith("//")) {
            if (!titleFound) // Set the title to the first comment
            {
                name = line.mid(2).trimmed();
                titleFound = true;
            } else if (okRows == 0) // We haven't processed any cards yet
            {
                comments += line.mid(2).trimmed() + "\n";
            }

            continue;
        }

        // If we have a blank line and it's the _ONLY_ blank line in the paste
        // Then we assume it means to start the sideboard section of the paste.
        // If we have the word "Sideboard" appear on any line, then that will
        // also indicate the start of the sideboard.
        if ((line.isEmpty() && blankLines == 1) || line.startsWith("sideboard")) {
            inSideboard = true;
            continue; // The line isn't actually a card
        }

        isSideboard = inSideboard;

        if (line.startsWith("sb:")) {
            line = line.mid(3).trimmed();
            isSideboard = true;
        }

        if (line.trimmed().isEmpty()) {
            continue; // The line was "    " instead of "\n"
        }

        // Filter out MWS edition symbols and basic land extras
        QRegExp rx("\\[.*\\]\s?");
        line.remove(rx);
        rx.setPattern("\\\s(.*\\)");
        line.remove(rx);

        // Filter out post card name editions
        rx.setPattern("\\|.*$");
        line.remove(rx);

        // If the user inputs "Quicksilver Elemental" then it will cut it off
        // 1x Squishy Treaker
        int i = line.indexOf(' ');
        int cardNameStart = i + 1;

        if (i > 0) {
            // If the count ends with an 'x', ignore it. For example,
            // "4x Storm Crow" will count 4 correctly.
            if (line.at(i - 1) == 'x') {
                i--;
            } else if (!line.at(i - 1).isDigit()) {
                // If the user inputs "Quicksilver Elemental" then it will work as 1x of that card
                cardNameStart = 0;
            }
        }

        bool ok;
        int number = line.left(i).toInt(&ok);

        if (!ok) {
            number = 1; // If input is "cardName" assume it's "1x cardName"
        }

        QString cardName = line.mid(cardNameStart);

        // Common differences between Cockatrice's card names
        // and what's commonly used in decklists
        rx.setPattern("’");
        cardName.replace(rx, "'");
        rx.setPattern("Æ");
        cardName.replace(rx, "Ae");
        rx.setPattern("\\s*[|/]{1,2}\\s*");
        cardName.replace(rx, " // ");

        // Replace only if the ampersand is preceded by a non-capital letter,
        // as would happen with acronyms. So 'Fire & Ice' is replaced but not
        // 'R&D' or 'R & D'.
        // Qt regexes don't support lookbehind so we capture and replace instead.
        rx.setPattern("([^A-Z])\\s*&\\s*");
        if (rx.indexIn(cardName) != -1) {
            cardName.replace(rx, QString("%1 // ").arg(rx.cap(1)));
        }

        // We need to get the name of the card from the database,
        // but we can't do that until we get the "real" name
        // (name stored in database for the card)
        // and establish a card info that is of the card, then it's
        // a simple getting the _real_ name of the card
        // (i.e. "STOrm, CrOW" => "Storm Crow")
        cardName = getCompleteCardName(cardName);

        // Look for the correct card zone of where to place the new card
        QString zoneName = getCardZoneFromName(cardName, isSideboard ? DECK_ZONE_SIDE : DECK_ZONE_MAIN);

        okRows++;
        new DecklistCardNode(cardName, number, getZoneObjFromName(zoneName));
    }

    updateDeckHash();
    return (okRows > 0);
}

InnerDecklistNode *DeckList::getZoneObjFromName(const QString zoneName)
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
    return loadFromStream_Plain(in);
}

struct WriteToStream
{
    QTextStream &stream;
    bool prefixSideboardCards;
    bool slashTappedOutSplitCards;

    WriteToStream(QTextStream &_stream, bool _prefixSideboardCards, bool _slashTappedOutSplitCards)
        : stream(_stream), prefixSideboardCards(_prefixSideboardCards),
          slashTappedOutSplitCards(_slashTappedOutSplitCards)
    {
    }

    void operator()(const InnerDecklistNode *node, const DecklistCardNode *card)
    {
        if (prefixSideboardCards && node->getName() == DECK_ZONE_SIDE) {
            stream << "SB: ";
        }
        if (!slashTappedOutSplitCards) {
            stream << QString("%1 %2\n").arg(card->getNumber()).arg(card->getName());
        } else {
            stream << QString("%1 %2\n").arg(card->getNumber()).arg(card->getName().replace("//", "/"));
        }
    }
};

bool DeckList::saveToStream_Plain(QTextStream &out, bool prefixSideboardCards, bool slashTappedOutSplitCards)
{
    WriteToStream writeToStream(out, prefixSideboardCards, slashTappedOutSplitCards);
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

void DeckList::cleanList()
{
    root->clearTree();
    setName();
    setComments();
    deckHash = QString();
    emit deckHashChanged();
}

void DeckList::getCardListHelper(InnerDecklistNode *item, QSet<QString> &result) const
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

QStringList DeckList::getCardList() const
{
    QSet<QString> result;
    getCardListHelper(root, result);
    return result.toList();
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

DecklistCardNode *DeckList::addCard(const QString &cardName, const QString &zoneName)
{
    auto *zoneNode = dynamic_cast<InnerDecklistNode *>(root->findChild(zoneName));
    if (zoneNode == nullptr) {
        zoneNode = new InnerDecklistNode(zoneName, root);
    }

    auto *node = new DecklistCardNode(cardName, 1, zoneNode);
    updateDeckHash();

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
            updateDeckHash();
        }

        return true;
    }

    for (int i = 0; i < rootNode->size(); i++) {
        auto *inner = dynamic_cast<InnerDecklistNode *>(rootNode->at(i));
        if (inner) {
            if (deleteNode(node, inner)) {
                if (updateHash) {
                    updateDeckHash();
                }

                return true;
            }
        }
    }

    return false;
}

void DeckList::updateDeckHash()
{
    QStringList cardList;
    bool isValidDeckList = true;
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
            } else if (!optionalZones.contains(node->getName())) // Not a valid zone -> cheater?
            {
                isValidDeckList = false; // Deck is invalid
            }
        }
    }
    cardList.sort();
    QByteArray deckHashArray = QCryptographicHash::hash(cardList.join(";").toUtf8(), QCryptographicHash::Sha1);
    quint64 number = (((quint64)(unsigned char)deckHashArray[0]) << 32) +
                     (((quint64)(unsigned char)deckHashArray[1]) << 24) +
                     (((quint64)(unsigned char)deckHashArray[2] << 16)) +
                     (((quint64)(unsigned char)deckHashArray[3]) << 8) + (quint64)(unsigned char)deckHashArray[4];
    deckHash = (isValidDeckList) ? QString::number(number, 32).rightJustified(8, '0') : "INVALID";

    emit deckHashChanged();
}
