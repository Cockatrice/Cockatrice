#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QProgressDialog>
#include "decklist.h"
#include "carddatabase.h"

QString DecklistZone::getVisibleName() const
{
	if (name == "main")
		return QObject::tr("Maindeck");
	else if (name == "side")
		return QObject::tr("Sideboard");
	else
		return QString();
}

DeckList::DeckList(CardDatabase *_db, QObject *parent)
	: QObject(parent), db(_db)
{
	initZones();
}

DeckList::~DeckList()
{
	cleanList();
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
				else if (xml.name() == "zone") {
					DecklistZone *zone = new DecklistZone(xml.attributes().value("name").toString());
					zones.append(zone);
					while (!xml.atEnd()) {
						if (xml.readNext() == QXmlStreamReader::EndElement)
							break;
						if (xml.name() == "card") {
							const int number = xml.attributes().value("number").toString().toInt();
							const QString card = xml.attributes().value("name").toString();
							zone->append(new DecklistRow(number, card));
							while (!xml.atEnd())
								if (xml.readNext() == QXmlStreamReader::EndElement)
									break;
						}
					}
				}
			}
		}
	}
	return true;
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

	for (int i = 0; i < zones.size(); i++) {
		xml.writeStartElement("zone");
		xml.writeAttribute("name", zones[i]->getName());
		for (int j = 0; j < zones[i]->size(); j++) {
			DecklistRow *r = zones[i]->at(j);
			xml.writeEmptyElement("card");
			xml.writeAttribute("number", QString::number(r->getNumber()));
			xml.writeAttribute("name", r->getCard());
		}
		xml.writeEndElement(); // zone
	}

	xml.writeEndElement(); // cockatrice_deck

	xml.writeEndDocument();
	return true;
}

bool DeckList::loadFromFile_Plain(QIODevice *device)
{
	initZones();

	QTextStream in(device);
	while (!in.atEnd()) {
		QString line = in.readLine().simplified();
		if (line.startsWith("//"))
			continue;

		DecklistZone *zone;
		if (line.startsWith("SB:", Qt::CaseInsensitive)) {
			line = line.mid(3).trimmed();
			zone = zones[1];
		} else
			zone = zones[0];

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
		zone->append(new DecklistRow(number, line.mid(i + 1)));
	}
	return true;
}

bool DeckList::saveToFile_Plain(QIODevice *device)
{
	QTextStream out(device);
	for (int i = 0; i < zones.size(); i++)
		for (int j = 0; j < zones[i]->size(); j++) {
			DecklistRow *r = zones[i]->at(j);
			out << QString("%1%2 %3\n").arg(zones[i]->getName() == "side" ? "SB: " : "").arg(r->getNumber()).arg(r->getCard());
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
		cacheCardPictures(parent);
		emit deckLoaded();
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

void DeckList::cacheCardPictures(QWidget *parent)
{
	int totalCards = 0;
	for (int i = 0; i < zones.size(); i++)
		totalCards += zones[i]->size();

	QProgressDialog progress(tr("Caching card pictures..."), QString(), 0, totalCards, parent);
	progress.setMinimumDuration(1000);
	progress.setWindowModality(Qt::WindowModal);

	for (int i = 0; i < zones.size(); i++)
		for (int j = 0; j < zones[i]->size(); j++) {
			db->getCard(zones[i]->at(j)->getCard())->getPixmap();
			progress.setValue(progress.value() + 1);
		}
}

void DeckList::cleanList()
{
	for (int i = 0; i < zones.size(); i++) {
		for (int j = 0; j < zones[i]->size(); j++)
			delete zones[i]->at(j);
		zones[i]->clear();
		delete zones[i];
	}
	zones.clear();
	setName();
	setComments();
}

void DeckList::initZones()
{
	// possibly Magic specific
	zones.append(new DecklistZone("main"));
	zones.append(new DecklistZone("side"));
}
