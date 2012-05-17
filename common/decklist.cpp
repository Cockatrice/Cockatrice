#include <QFile>
#include <QTextStream>
#include <QVariant>
#include <QCryptographicHash>
#include "decklist.h"

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
	for (int i = 0; i < moveList.size(); ++i) {
		xml->writeStartElement("move_card_to_zone");
		xml->writeTextElement("card_name", QString::fromStdString(moveList[i].card_name()));
		xml->writeTextElement("start_zone", QString::fromStdString(moveList[i].start_zone()));
		xml->writeTextElement("target_zone", QString::fromStdString(moveList[i].target_zone()));
		xml->writeEndElement();
	}
	xml->writeEndElement();
}

AbstractDecklistNode::AbstractDecklistNode(InnerDecklistNode *_parent)
	: parent(_parent)
{
	if (parent)
		parent->append(this);
}

int AbstractDecklistNode::depth() const
{
	if (parent)
		return parent->depth() + 1;
	else
		return 0;
}

InnerDecklistNode::InnerDecklistNode(InnerDecklistNode *other, InnerDecklistNode *_parent)
	: AbstractDecklistNode(_parent), name(other->getName())
{
	for (int i = 0; i < other->size(); ++i) {
		InnerDecklistNode *inner = dynamic_cast<InnerDecklistNode *>(other->at(i));
		if (inner)
			new InnerDecklistNode(inner, this);
		else
			new DecklistCardNode(dynamic_cast<DecklistCardNode *>(other->at(i)), this);
	}
}

InnerDecklistNode::~InnerDecklistNode()
{
	clearTree();
}

QString InnerDecklistNode::visibleNameFromName(const QString &_name)
{
	if (_name == "main")
		return QObject::tr("Maindeck");
	else if (_name == "side")
		return QObject::tr("Sideboard");
	else if (_name == "tokens")
		return QObject::tr("Tokens");
	else
		return _name;
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
	: AbstractDecklistCardNode(_parent), name(other->getName()), number(other->getNumber()), price(other->getPrice())
{
}

AbstractDecklistNode *InnerDecklistNode::findChild(const QString &name)
{
	for (int i = 0; i < size(); i++)
		if (at(i)->getName() == name)
			return at(i);
	return 0;
}

int InnerDecklistNode::height() const
{
	return at(0)->height() + 1;
}

int InnerDecklistNode::recursiveCount(bool countTotalCards) const
{
	int result = 0;
	for (int i = 0; i < size(); i++) {
		InnerDecklistNode *node = dynamic_cast<InnerDecklistNode *>(at(i));
		if (node)
			result += node->recursiveCount(countTotalCards);
		else if (countTotalCards)
			result += dynamic_cast<AbstractDecklistCardNode *>(at(i))->getNumber();
		else
			result += 1;
	}
	return result;
}

float InnerDecklistNode::recursivePrice(bool countTotalCards) const
{
        float result = 0;
        for (int i = 0; i < size(); i++) {
                InnerDecklistNode *node = dynamic_cast<InnerDecklistNode *>(at(i));
                if (node)
                        result += node->recursivePrice(countTotalCards);
                else if (countTotalCards)
                        result += dynamic_cast<AbstractDecklistCardNode *>(at(i))->getTotalPrice();
        }
        return result;
}

bool InnerDecklistNode::compare(AbstractDecklistNode *other) const
{
	InnerDecklistNode *other2 = dynamic_cast<InnerDecklistNode *>(other);
	if (other2)
		return (getName() > other->getName());
	else
		return false;
}

bool AbstractDecklistCardNode::compare(AbstractDecklistNode *other) const
{
	AbstractDecklistCardNode *other2 = dynamic_cast<AbstractDecklistCardNode *>(other);
	if (other2)
		return (getName() > other->getName());
	else
		return true;
}

class InnerDecklistNode::compareFunctor {
private:
	Qt::SortOrder order;
public:
	compareFunctor(Qt::SortOrder _order) : order(_order) { }
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
				float price = (xml->attributes().value("price") != NULL) ? xml->attributes().value("price").toString().toFloat() : 0;
				DecklistCardNode *newCard = new DecklistCardNode(xml->attributes().value("name").toString(), xml->attributes().value("number").toString().toInt(), price, this);
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
        xml->writeAttribute("price", QString::number(getPrice()));
	xml->writeAttribute("name", getName());
}

QVector<QPair<int, int> > InnerDecklistNode::sort(Qt::SortOrder order)
{
	QVector<QPair<int, int> > result(size());
	
	// Initialize temporary list with contents of current list
	QVector<QPair<int, AbstractDecklistNode *> > tempList(size());
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

DeckList::DeckList(const DeckList &other)
	: name(other.name),
	  comments(other.comments),
	  deckHash(other.deckHash)
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
			InnerDecklistNode *newZone = new InnerDecklistNode(xml->attributes().value("name").toString(), root);
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
	loadFromXml(&xml);
	return true;
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
	
	InnerDecklistNode *main = 0, *side = 0;

	int okRows = 0;
	while (!in.atEnd()) {
		QString line = in.readLine().simplified();
		if (line.startsWith("//"))
			continue;

		InnerDecklistNode *zone;
		if (line.startsWith("SB:", Qt::CaseInsensitive)) {
			line = line.mid(3).trimmed();
			if (!side)
				side = new InnerDecklistNode("side", root);
			zone = side;
		} else {
			if (!main)
				main = new InnerDecklistNode("main", root);
			zone = main;
		}

		// Filter out MWS edition symbols and basic land extras
		QRegExp rx("\\[.*\\]");
		line.remove(rx);
		rx.setPattern("\\(.*\\)");
		line.remove(rx);
		line = line.simplified();

		int i = line.indexOf(' ');
		bool ok;
		int number = line.left(i).toInt(&ok);
		if (!ok)
			continue;
		++okRows;
		new DecklistCardNode(line.mid(i + 1), number, zone);
	}
	updateDeckHash();
	return (okRows > 0);
}

bool DeckList::loadFromFile_Plain(QIODevice *device)
{
	QTextStream in(device);
	return loadFromStream_Plain(in);
}

bool DeckList::saveToStream_Plain(QTextStream &out)
{
	// Support for this is only possible if the internal structure doesn't get more complicated.
	for (int i = 0; i < root->size(); i++) {
		InnerDecklistNode *node = dynamic_cast<InnerDecklistNode *>(root->at(i));
		for (int j = 0; j < node->size(); j++) {
			DecklistCardNode *card = dynamic_cast<DecklistCardNode *>(node->at(j));
			out << QString("%1%2 %3\n").arg(node->getName() == "side" ? "SB: " : "").arg(card->getNumber()).arg(card->getName());
		}
	}
	return true;
}

bool DeckList::saveToFile_Plain(QIODevice *device)
{
	QTextStream out(device);
	return saveToStream_Plain(out);
}

QString DeckList::writeToString_Plain()
{
	QString result;
	QTextStream out(&result);
	saveToStream_Plain(out);
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
		DecklistCardNode *node = dynamic_cast<DecklistCardNode *>(item->at(i));
		if (node)
			result.insert(node->getName());
		else
			getCardListHelper(dynamic_cast<InnerDecklistNode *>(item->at(i)), result);
	}
}

QStringList DeckList::getCardList() const
{
	QSet<QString> result;
	getCardListHelper(root, result);
	return result.toList();
}

DecklistCardNode *DeckList::addCard(const QString &cardName, const QString &zoneName)
{
	InnerDecklistNode *zoneNode = dynamic_cast<InnerDecklistNode *>(root->findChild(zoneName));
	if (!zoneNode)
		zoneNode = new InnerDecklistNode(zoneName, root);
	
	DecklistCardNode *node = new DecklistCardNode(cardName, 1, zoneNode);
	updateDeckHash();
	return node;
}

bool DeckList::deleteNode(AbstractDecklistNode *node, InnerDecklistNode *rootNode)
{
	if (node == root)
		return true;
	bool updateHash = false;
	if (!rootNode) {
		rootNode = root;
		updateHash = true;
	}
	
	int index = rootNode->indexOf(node);
	if (index != -1) {
		delete rootNode->takeAt(index);
		if (!rootNode->size())
			deleteNode(rootNode, rootNode->getParent());
		if (updateHash)
			updateDeckHash();
		return true;
	}
	for (int i = 0; i < rootNode->size(); i++) {
		InnerDecklistNode *inner = dynamic_cast<InnerDecklistNode *>(rootNode->at(i));
		if (inner)
			if (deleteNode(node, inner)) {
				if (updateHash)
					updateDeckHash();
				return true;
			}
	}
	return false;
}

void DeckList::updateDeckHash()
{
	QStringList cardList;
	for (int i = 0; i < root->size(); i++) {
		InnerDecklistNode *node = dynamic_cast<InnerDecklistNode *>(root->at(i));
		for (int j = 0; j < node->size(); j++) {
			DecklistCardNode *card = dynamic_cast<DecklistCardNode *>(node->at(j));
			for (int k = 0; k < card->getNumber(); ++k)
				cardList.append((node->getName() == "side" ? "SB:" : "") + card->getName().toLower());
		}
	}
	cardList.sort();
	QByteArray deckHashArray = QCryptographicHash::hash(cardList.join(";").toUtf8(), QCryptographicHash::Sha1);
	quint64 number = (((quint64) (unsigned char) deckHashArray[0]) << 32)
	                + (((quint64) (unsigned char) deckHashArray[1]) << 24)
	                + (((quint64) (unsigned char) deckHashArray[2] << 16))
	                + (((quint64) (unsigned char) deckHashArray[3]) << 8)
	                + (quint64) (unsigned char) deckHashArray[4];
	deckHash = QString::number(number, 32).rightJustified(8, '0');
	
	emit deckHashChanged();
}
