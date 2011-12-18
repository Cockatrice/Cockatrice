#include <QFile>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QVariant>
#include <QCryptographicHash>
#include "decklist.h"

MoveCardToZone::MoveCardToZone(const QString &_cardName, const QString &_startZone, const QString &_targetZone)
	: SerializableItem_Map("move_card_to_zone")
{
	insertItem(new SerializableItem_String("card_name", _cardName));
	insertItem(new SerializableItem_String("start_zone", _startZone));
	insertItem(new SerializableItem_String("target_zone", _targetZone));
}

MoveCardToZone::MoveCardToZone(MoveCardToZone *other)
	: SerializableItem_Map("move_card_to_zone")
{
	insertItem(new SerializableItem_String("card_name", other->getCardName()));
	insertItem(new SerializableItem_String("start_zone", other->getStartZone()));
	insertItem(new SerializableItem_String("target_zone", other->getTargetZone()));
}

SideboardPlan::SideboardPlan(const QString &_name, const QList<MoveCardToZone *> &_moveList)
	: SerializableItem_Map("sideboard_plan")
{
	insertItem(new SerializableItem_String("name", _name));
	
	for (int i = 0; i < _moveList.size(); ++i)
		itemList.append(_moveList[i]);
}

void SideboardPlan::setMoveList(const QList<MoveCardToZone *> &_moveList)
{
	for (int i = 0; i < itemList.size(); ++i)
		delete itemList[i];
	itemList.clear();
	
	for (int i = 0; i < _moveList.size(); ++i)
		itemList.append(_moveList[i]);
}

AbstractDecklistNode::AbstractDecklistNode(InnerDecklistNode *_parent)
	: parent(_parent), currentItem(0)
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
	if (currentItem) {
		if (currentItem->readElement(xml))
			currentItem = 0;
		return false;
	}
	if (xml->isStartElement() && (xml->name() == "zone"))
		currentItem = new InnerDecklistNode(xml->attributes().value("name").toString(), this);
        else if (xml->isStartElement() && (xml->name() == "card")) {
                float price = (xml->attributes().value("price") != NULL) ? xml->attributes().value("price").toString().toFloat() : 0;
                currentItem = new DecklistCardNode(xml->attributes().value("name").toString(), xml->attributes().value("number").toString().toInt(), price, this);
        } else if (xml->isEndElement() && (xml->name() == "zone"))
		return true;

	return false;
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
	if (xml->isEndElement())
		return true;
	else
		return false;
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

const QStringList DeckList::fileNameFilters = QStringList()
	<< QObject::tr("Cockatrice decks (*.cod)")
	<< QObject::tr("Plain text decks (*.dec *.mwDeck)")
	<< QObject::tr("All files (*.*)");

DeckList::DeckList()
	: SerializableItem("cockatrice_deck"), currentZone(0), currentSideboardPlan(0)
{
	root = new InnerDecklistNode;
}

DeckList::DeckList(DeckList *other)
	: SerializableItem("cockatrice_deck"), currentZone(0), currentSideboardPlan(0)
{
	root = new InnerDecklistNode(other->getRoot());
	
	QMapIterator<QString, SideboardPlan *> spIterator(other->getSideboardPlans());
	while (spIterator.hasNext()) {
		spIterator.next();
		QList<MoveCardToZone *> newMoveList;
		QList<MoveCardToZone *> oldMoveList = spIterator.value()->getMoveList();
		for (int i = 0; i < oldMoveList.size(); ++i)
			newMoveList.append(new MoveCardToZone(oldMoveList[i]));
		sideboardPlans.insert(spIterator.key(), new SideboardPlan(spIterator.key(), newMoveList));
	}
	updateDeckHash();
}

DeckList::DeckList(const QString &nativeString)
	: SerializableItem("cockatrice_deck"), currentZone(0), currentSideboardPlan(0)
{
	root = new InnerDecklistNode;
	
	QXmlStreamReader xml(nativeString);
	loadFromXml(&xml);
}

DeckList::~DeckList()
{
	delete root;
	
	QMapIterator<QString, SideboardPlan *> i(sideboardPlans);
	while (i.hasNext())
		delete i.next().value();
}

QList<MoveCardToZone *> DeckList::getCurrentSideboardPlan()
{
	SideboardPlan *current = sideboardPlans.value(QString(), 0);
	if (!current)
		return QList<MoveCardToZone *>();
	else
		return current->getMoveList();
}

void DeckList::setCurrentSideboardPlan(const QList<MoveCardToZone *> &plan)
{
	SideboardPlan *current = sideboardPlans.value(QString(), 0);
	if (!current) {
		current = new SideboardPlan;
		sideboardPlans.insert(QString(), current);
	}
	
	QList<MoveCardToZone *> newList;
	for (int i = 0; i < plan.size(); ++i)
		newList.append(new MoveCardToZone(plan[i]));
	current->setMoveList(newList);
}

bool DeckList::readElement(QXmlStreamReader *xml)
{
	if (currentZone) {
		if (currentZone->readElement(xml))
			currentZone = 0;
	} else if (currentSideboardPlan) {
		if (currentSideboardPlan->readElement(xml)) {
			sideboardPlans.insert(currentSideboardPlan->getName(), currentSideboardPlan);
			currentSideboardPlan = 0;
		}
		return false;
	} else if (xml->isEndElement()) {
		if (xml->name() == "deckname")
			name = currentElementText;
		else if (xml->name() == "comments")
			comments = currentElementText;
		
		currentElementText.clear();
	} else if (xml->isStartElement() && (xml->name() == "zone"))
		currentZone = new InnerDecklistNode(xml->attributes().value("name").toString(), root);
	else if (xml->isStartElement() && (xml->name() == "sideboard_plan")) {
		currentSideboardPlan = new SideboardPlan;
		if (currentSideboardPlan->readElement(xml)) {
			sideboardPlans.insert(currentSideboardPlan->getName(), currentSideboardPlan);
			currentSideboardPlan = 0;
		}
	} else if (xml->isCharacters() && !xml->isWhitespace())
		currentElementText = xml->text().toString();
	return SerializableItem::readElement(xml);
}

void DeckList::writeElement(QXmlStreamWriter *xml)
{
	xml->writeAttribute("version", "1");
	xml->writeTextElement("deckname", name);
	xml->writeTextElement("comments", comments);

	for (int i = 0; i < root->size(); i++)
		root->at(i)->writeElement(xml);
	
	QMapIterator<QString, SideboardPlan *> i(sideboardPlans);
	while (i.hasNext())
		i.next().value()->write(xml);
}

void DeckList::loadFromXml(QXmlStreamReader *xml)
{
	while (!xml->atEnd()) {
		xml->readNext();
		if (xml->isStartElement()) {
			if (xml->name() != "cockatrice_deck")
				return;
			while (!xml->atEnd()) {
				xml->readNext();
				readElement(xml);
			}
		}
	}
	updateDeckHash();
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

bool DeckList::loadFromFile(const QString &fileName, FileFormat fmt)
{
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	cleanList();

	bool result = false;
	switch (fmt) {
		case PlainTextFormat: result = loadFromFile_Plain(&file); break;
		case CockatriceFormat: result = loadFromFile_Native(&file); break;
	}
	if (result)
		emit deckLoaded();
	return result;
}

bool DeckList::saveToFile(const QString &fileName, FileFormat fmt)
{
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return false;

	bool result = false;
	switch (fmt) {
		case PlainTextFormat: result = saveToFile_Plain(&file); break;
		case CockatriceFormat: result = saveToFile_Native(&file); break;
	}
	return result;
}

DeckList::FileFormat DeckList::getFormatFromNameFilter(const QString &selectedNameFilter)
{
	switch (fileNameFilters.indexOf(selectedNameFilter)) {
		case 0: return CockatriceFormat;
		case 1: return PlainTextFormat;
	}
	return PlainTextFormat;
}

void DeckList::cleanList()
{
	root->clearTree();
	setName();
	setComments();
	updateDeckHash();
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
