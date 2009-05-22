#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QProgressDialog>
#include "decklist.h"
#include "carddatabase.h"

DeckList::DeckList(CardDatabase *_db)
	: db(_db)
{

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
				else if (xml.name() == "decklist") {
					while (!xml.atEnd()) {
						if (xml.readNext() == QXmlStreamReader::EndElement)
							break;
						if (xml.name() == "card") {
							const int number = xml.attributes().value("number").toString().toInt();
							const QString card = xml.attributes().value("name").toString();
							const bool sb = xml.attributes().value("zone") == "side";
							append(new DecklistRow(number, card, sb));
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

	xml.writeStartElement("decklist");
	for (int i = 0; i < size(); i++) {
		DecklistRow *r = at(i);
		xml.writeEmptyElement("card");
		if (r->isSideboard())
			xml.writeAttribute("zone", "side");
		else
			xml.writeAttribute("zone", "main");
		xml.writeAttribute("number", QString::number(r->getNumber()));
		xml.writeAttribute("name", r->getCard());
	}
	xml.writeEndElement(); // decklist

	xml.writeEndElement(); // cockatrice_deck

	xml.writeEndDocument();
	return true;
}

bool DeckList::loadFromFile_Plain(QIODevice *device)
{
	QTextStream in(device);
	while (!in.atEnd()) {
		QString line = in.readLine().simplified();
		if (line.startsWith("//"))
			continue;

		bool isSideboard = false;
		if (line.startsWith("SB:", Qt::CaseInsensitive)) {
			line = line.mid(3).trimmed();
			isSideboard = true;
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
		append(new DecklistRow(number, line.mid(i + 1), isSideboard));
	}
	return true;
}

bool DeckList::saveToFile_Plain(QIODevice *device)
{
	QTextStream out(device);
	for (int i = 0; i < size(); i++) {
		DecklistRow *r = at(i);
		out << QString("%1%2 %3\n").arg(r->isSideboard() ? "SB: " : "").arg(r->getNumber()).arg(r->getCard());
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
	if (result)
		cacheCardPictures(parent);
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
	QProgressDialog progress(tr("Caching card pictures..."), QString(), 0, size(), parent);
	progress.setWindowModality(Qt::WindowModal);

	for (int i = 0; i < size(); i++) {
		db->getCard(at(i)->getCard())->getPixmap();
		progress.setValue(i + 1);
	}
}

void DeckList::cleanList()
{
	for (int i = 0; i < size(); i++)
		delete at(i);
	clear();
}
