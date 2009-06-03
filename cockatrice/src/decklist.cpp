#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QProgressDialog>
#include <QVariant>
#include "decklist.h"
#include "carddatabase.h"

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

InnerDecklistNode::~InnerDecklistNode()
{
	clearTree();
}

QString InnerDecklistNode::getVisibleName() const
{
	if (name == "main")
		return QObject::tr("Maindeck");
	else if (name == "side")
		return QObject::tr("Sideboard");
	else
		return getName();
}

void InnerDecklistNode::clearTree()
{
	for (int i = 0; i < size(); i++)
		delete at(i);
	clear();
}

AbstractDecklistNode *InnerDecklistNode::findChild(const QString &name)
{
	for (int i = 0; i < size(); i++)
		if (at(i)->getName() == name)
			return at(i);
	return 0;
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
	inline bool operator()(AbstractDecklistNode *a, AbstractDecklistNode *b) const
	{
		return (order == Qt::AscendingOrder) ^ (a->compare(b));
	}
};

void InnerDecklistNode::sort(Qt::SortOrder order)
{
	compareFunctor cmp(order);
	qSort(begin(), end(), cmp);
	for (int i = 0; i < size(); i++) {
		InnerDecklistNode *node = dynamic_cast<InnerDecklistNode *>(at(i));
		if (node)
			node->sort(order);
	}
}

DeckList::DeckList(CardDatabase *_db, QObject *parent)
	: QObject(parent), db(_db)
{
	root = new InnerDecklistNode;
}

DeckList::~DeckList()
{
	delete root;
}

QXmlStreamReader &operator>>(QXmlStreamReader &xml, InnerDecklistNode *node)
{
	while (!xml.atEnd()) {
		if (xml.readNext() == QXmlStreamReader::EndElement)
			break;
		if (xml.name() == "zone")
			xml >> new InnerDecklistNode(xml.attributes().value("name").toString(), node);
		else if (xml.name() == "card") {
			const int number = xml.attributes().value("number").toString().toInt();
			const QString card = xml.attributes().value("name").toString();
			new DecklistCardNode(card, number, node);
			while (!xml.atEnd())
				if (xml.readNext() == QXmlStreamReader::EndElement)
					break;
		}
	}
	return xml;
}

bool DeckList::loadFromFile_Native(QIODevice *device)
{
	QXmlStreamReader xml(device);
	while (!xml.atEnd()) {
		if (xml.readNext() == QXmlStreamReader::StartElement) {
			if (xml.name() != "cockatrice_deck")
				return false;
			while (!xml.atEnd()) {
				if (xml.readNext() == QXmlStreamReader::EndElement)
					break;
				if (xml.name() == "deckname")
					name = xml.readElementText();
				else if (xml.name() == "comments")
					comments = xml.readElementText();
				else if (xml.name() == "zone")
					xml >> new InnerDecklistNode(xml.attributes().value("name").toString(), root);
			}
		}
	}
	return true;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &xml, const AbstractDecklistNode *node)
{
	const InnerDecklistNode *inner = dynamic_cast<const InnerDecklistNode *>(node);
	if (inner) {
		xml.writeStartElement("zone");
		xml.writeAttribute("name", inner->getName());
		for (int i = 0; i < inner->size(); i++)
			xml << inner->at(i);
		xml.writeEndElement(); // zone
	} else {
		const AbstractDecklistCardNode *card = dynamic_cast<const AbstractDecklistCardNode *>(node);
		xml.writeEmptyElement("card");
		xml.writeAttribute("number", QString::number(card->getNumber()));
		xml.writeAttribute("name", card->getName());
	}
	return xml;
}

bool DeckList::saveToFile_Native(QIODevice *device)
{
	QXmlStreamWriter xml(device);
	xml.setAutoFormatting(true);
	xml.writeStartDocument();

	xml.writeStartElement("cockatrice_deck");
	xml.writeAttribute("version", "1");
	xml.writeTextElement("deckname", name);
	xml.writeTextElement("comments", comments);

	for (int i = 0; i < root->size(); i++)
		xml << root->at(i);

	xml.writeEndElement(); // cockatrice_deck

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

bool DeckList::loadFromFile(const QString &fileName, FileFormat fmt, QWidget *parent)
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
	if (result) {
		emit deckLoaded();
		cacheCardPictures(parent);
	}
	return result;
}

const QStringList DeckList::fileNameFilters = QStringList()
	<< QObject::tr("Cockatrice decks (*.cod)")
	<< QObject::tr("Plain text decks (*.dec *.mwDeck)")
	<< QObject::tr("All files (*.*)");

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

bool DeckList::loadDialog(QWidget *parent)
{
	QFileDialog dialog(parent, tr("Load deck"));
	dialog.setNameFilters(fileNameFilters);
	if (!dialog.exec())
		return false;

	QString fileName = dialog.selectedFiles().at(0);
	FileFormat fmt;
	switch (fileNameFilters.indexOf(dialog.selectedNameFilter())) {
		case 0: fmt = CockatriceFormat; break;
		case 1: fmt = PlainTextFormat; break;
		default: fmt = PlainTextFormat; break;
	}

	if (loadFromFile(fileName, fmt, parent)) {
		lastFileName = fileName;
		lastFileFormat = fmt;
		return true;
	}
	return false;
}

bool DeckList::saveDialog(QWidget *parent)
{
	QFileDialog dialog(parent, tr("Save deck"));
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setConfirmOverwrite(true);
	dialog.setDefaultSuffix("cod");
	dialog.setNameFilters(fileNameFilters);
	if (!dialog.exec())
		return false;

	QString fileName = dialog.selectedFiles().at(0);
	DeckList::FileFormat fmt;
	switch (fileNameFilters.indexOf(dialog.selectedNameFilter())) {
		case 0: fmt = DeckList::CockatriceFormat; break;
		case 1: fmt = DeckList::PlainTextFormat; break;
		default: fmt = DeckList::PlainTextFormat; break;
	}

	if (saveToFile(fileName, fmt)) {
		lastFileName = fileName;
		lastFileFormat = fmt;
		return true;
	}
	return false;
}

void DeckList::cacheCardPicturesHelper(InnerDecklistNode *item, QProgressDialog *progress)
{
	for (int i = 0; i < item->size(); i++) {
		DecklistCardNode *node = dynamic_cast<DecklistCardNode *>(item->at(i));
		if (node) {
			db->getCard(node->getName())->loadPixmap();
			progress->setValue(progress->value() + 1);
		} else
			cacheCardPicturesHelper(dynamic_cast<InnerDecklistNode *>(item->at(i)), progress);
	}
}
	
void DeckList::cacheCardPictures(QWidget *parent)
{
	int totalCards = root->recursiveCount();

	QProgressDialog progress(tr("Caching card pictures..."), QString(), 0, totalCards, parent);
	progress.setMinimumDuration(1000);
	progress.setWindowModality(Qt::WindowModal);

	cacheCardPicturesHelper(root, &progress);
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
