#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QVariant>
#include "decklist.h"

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
	: AbstractDecklistCardNode(_parent), name(other->getName()), number(other->getNumber())
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
	Q_ASSERT(!isEmpty());
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
	else if (xml->isStartElement() && (xml->name() == "card"))
		currentItem = new DecklistCardNode(xml->attributes().value("name").toString(), xml->attributes().value("number").toString().toInt(), this);
	else if (xml->isEndElement() && (xml->name() == "zone"))
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
	: SerializableItem("cockatrice_deck"), currentZone(0)
{
	root = new InnerDecklistNode;
}

DeckList::DeckList(DeckList *other)
	: SerializableItem("cockatrice_deck"), currentZone(0)
{
	root = new InnerDecklistNode(other->getRoot());
}

DeckList::~DeckList()
{
	delete root;
}

void DeckList::readElement(QXmlStreamReader *xml)
{
	if (currentZone) {
		if (currentZone->readElement(xml))
			currentZone = 0;
	} else if (xml->isEndElement()) {
		if (xml->name() == "deckname")
			name = currentElementText;
		else if (xml->name() == "comments")
			comments = currentElementText;
		
		currentElementText.clear();
	} else if (xml->isStartElement() && (xml->name() == "zone"))
		currentZone = new InnerDecklistNode(xml->attributes().value("name").toString(), root);
	else if (xml->isCharacters() && !xml->isWhitespace())
		currentElementText = xml->text().toString();
}

void DeckList::writeElement(QXmlStreamWriter *xml)
{
	xml->writeAttribute("version", "1");
	xml->writeTextElement("deckname", name);
	xml->writeTextElement("comments", comments);

	for (int i = 0; i < root->size(); i++)
		root->at(i)->writeElement(xml);
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

bool DeckList::loadFromFile_Plain(QIODevice *device)
{
	InnerDecklistNode *main = 0, *side = 0;

	QTextStream in(device);
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
		new DecklistCardNode(line.mid(i + 1), number, zone);
	}
	return true;
}

bool DeckList::saveToFile_Plain(QIODevice *device)
{
	// Support for this is only possible if the internal structure doesn't get more complicated.
	QTextStream out(device);
	for (int i = 0; i < root->size(); i++) {
		InnerDecklistNode *node = dynamic_cast<InnerDecklistNode *>(root->at(i));
		for (int j = 0; j < node->size(); j++) {
			DecklistCardNode *card = dynamic_cast<DecklistCardNode *>(node->at(j));
			out << QString("%1%2 %3\n").arg(node->getName() == "side" ? "SB: " : "").arg(card->getNumber()).arg(card->getName());
		}
	}
	return true;
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
}

DecklistCardNode *DeckList::addCard(const QString &cardName, const QString &zoneName)
{
	InnerDecklistNode *zoneNode = dynamic_cast<InnerDecklistNode *>(root->findChild(zoneName));
	if (!zoneNode)
		zoneNode = new InnerDecklistNode(zoneName, root);
	
	return new DecklistCardNode(cardName, 1, zoneNode);
}

bool DeckList::deleteNode(AbstractDecklistNode *node, InnerDecklistNode *rootNode)
{
	if (node == root)
		return true;
	if (!rootNode)
		rootNode = root;
	
	int index = rootNode->indexOf(node);
	if (index != -1) {
		delete rootNode->takeAt(index);
		if (!rootNode->size())
			deleteNode(rootNode, rootNode->getParent());
		return true;
	}
	for (int i = 0; i < rootNode->size(); i++) {
		InnerDecklistNode *inner = dynamic_cast<InnerDecklistNode *>(rootNode->at(i));
		if (inner)
			if (deleteNode(node, inner))
				return true;
	}
	return false;
}
