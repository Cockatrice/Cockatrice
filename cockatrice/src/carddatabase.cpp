#include "carddatabase.h"
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QTextStream>

CardInfo::CardInfo(const QString &_name, const QString &_manacost, const QString &_cardtype, const QString &_powtough, const QStringList &_text)
	: name(_name), manacost(_manacost), cardtype(_cardtype), powtough(_powtough), text(_text), pixmap(NULL)
{

}

CardInfo::CardInfo(QDataStream &stream)
	: pixmap(NULL)
{
	stream >> name
	       >> editions
	       >> manacost
	       >> cardtype
	       >> powtough
	       >> text;
}

CardInfo::~CardInfo()
{
	if (pixmap) {
		qDebug(QString("Deleting pixmap for %1").arg(name).toLatin1());
		delete pixmap;
		QMapIterator<int, QPixmap *> i(scaledPixmapCache);
		while (i.hasNext()) {
			i.next();
			qDebug(QString("  Deleting cached pixmap for width %1").arg(i.key()).toLatin1());
			delete i.value();
		}
	}
}

QString CardInfo::getMainCardType() const
{
	QString result = getCardType();
	/*
	Legendary Artifact Creature - Golem
	Instant // Instant
	*/
	
	int pos;
	if ((pos = result.indexOf('-')) != -1)
		result.remove(pos, result.length());
	if ((pos = result.indexOf("//")) != -1)
		result.remove(pos, result.length());
	result = result.simplified();
	/*
	Legendary Artifact Creature
	Instant
	*/
	
	if ((pos = result.lastIndexOf(' ')) != -1)
		result = result.mid(pos + 1);
	/*
	Creature
	Instant
	*/
	
	return result;
}

void CardInfo::addEdition(const QString &edition)
{
	if (!editions.contains(edition))
		editions << edition;
}

QPixmap *CardInfo::loadPixmap()
{
	if (pixmap)
		return pixmap;
	pixmap = new QPixmap();
	if (getName().isEmpty()) {
		pixmap->load("../pics/back.jpg");
		return pixmap;
	}
	qDebug(QString("CardDatabase: loading pixmap for '%1'").arg(getName()).toLatin1());
	for (int i = 0; i < editions.size(); i++) {
		// Fire // Ice, Circle of Protection: Red
		QString correctedName = getName().remove(" // ").remove(":");
		if (pixmap->load(QString("../pics/%1/%2.full.jpg").arg(editions.at(i)).arg(correctedName)))
			return pixmap;
		if (pixmap->load(QString("../pics/%1/%2%3.full.jpg").arg(editions.at(i)).arg(correctedName).arg(1)))
			return pixmap;
	}
	pixmap->load("../pics/none.jpg");
	return pixmap;
}

QPixmap *CardInfo::getPixmap(QSize size)
{
	qDebug(QString("CardInfo::getPixmap(%1, %2) for %3").arg(size.width()).arg(size.height()).arg(getName()).toLatin1());
	if (QPixmap *result = scaledPixmapCache.value(size.width())) {
		qDebug("cache HIT");
		return result;
	}
	qDebug("cache MISS");
	QPixmap *result = new QPixmap(loadPixmap()->scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
	scaledPixmapCache.insert(size.width(), result);
	return result;
}

void CardInfo::saveToStream(QDataStream &stream)
{
	stream << name
	       << editions
	       << manacost
	       << cardtype
	       << powtough
	       << text;
}

CardDatabase::CardDatabase()
{
}

CardDatabase::~CardDatabase()
{
	clear();
}

void CardDatabase::clear()
{
	QHashIterator<QString, CardInfo *> i(hash);
	while (i.hasNext()) {
		i.next();
		delete i.value();
	}
	hash.clear();
}

CardInfo *CardDatabase::getCard(const QString &cardName)
{
	if (hash.contains(cardName))
		return hash.value(cardName);
	else {
		qDebug(QString("CardDatabase: card not found: %1").arg(cardName).toLatin1());
		CardInfo *newCard = new CardInfo(cardName);
		newCard->addEdition("TK");
		hash.insert(cardName, newCard);
		return newCard;
	}
}

QList<CardInfo *>CardDatabase::getCardList()
{
	QList<CardInfo *> cardList;
	QHashIterator<QString, CardInfo *> i(hash);
	while (i.hasNext()) {
		i.next();
		cardList.append(i.value());
	}
	return cardList;
}

void CardDatabase::importOracle()
{
	clear();
	QDir dir("../db");

	// XXX User soll selber auswählen können, welche Karten ihm am besten gefallen.
	// XXX Muss momentan schmutzig über Zahlen vor den Dateinamen gemacht werden.

	dir.setSorting(QDir::Name | QDir::IgnoreCase);
	QFileInfoList files = dir.entryInfoList(QStringList() << "*.txt");
	for (int k = 0; k < files.size(); k++) {
		QFileInfo i = files[k];
		QString edition = i.fileName().mid(i.fileName().indexOf('_') + 1);
		edition = edition.left(edition.indexOf('.'));
		QFile file(i.filePath());
		file.open(QIODevice::ReadOnly | QIODevice::Text);
		QTextStream in(&file);
		while (!in.atEnd()) {
			QString cardname = in.readLine();
			QString manacost = in.readLine();
			QString cardtype, powtough;
			QStringList text;
			if (manacost.contains("Land", Qt::CaseInsensitive)) {
				cardtype = manacost;
				manacost.clear();
			} else {
				cardtype = in.readLine();
				powtough = in.readLine();
				// Dirty hack.
				// Cards to test: Any creature, any basic land, Ancestral Vision, Fire // Ice.
				if (!powtough.contains("/") || powtough.size() > 5) {
					text << powtough;
					powtough = QString();
				}
			}
			QString line = in.readLine();
			while (!line.isEmpty()) {
				text << line;
				line = in.readLine();
			}
			CardInfo *card;
			if (hash.contains(cardname))
				card = hash.value(cardname);
			else {
				card = new CardInfo(cardname, manacost, cardtype, powtough, text);
				hash.insert(cardname, card);
			}
			card->addEdition(edition);
		}
	}

	qDebug(QString("CardDatabase: %1 cards imported").arg(hash.size()).toLatin1());

	CardInfo *empty = new CardInfo();
	empty->loadPixmap(); // cache pixmap for card back
	hash.insert("", empty);
}

int CardDatabase::loadFromFile(const QString &fileName)
{
	QFile file(fileName);
	file.open(QIODevice::ReadOnly);
	QDataStream in(&file);
	in.setVersion(QDataStream::Qt_4_4);

	quint32 _magicNumber, _fileVersion, cardCount;
	in >> _magicNumber
	   >> _fileVersion
	   >> cardCount;

	if (_magicNumber != magicNumber)
		return -1;
	if (_fileVersion != fileVersion)
		return -2;

	clear();
	hash.reserve(cardCount);
	for (unsigned int i = 0; i < cardCount; i++) {
		CardInfo *newCard = new CardInfo(in);
		hash.insert(newCard->getName(), newCard);
	}

	return cardCount;
}

bool CardDatabase::saveToFile(const QString &fileName)
{
	QFile file(fileName);
	file.open(QIODevice::WriteOnly);
	QDataStream out(&file);
	out.setVersion(QDataStream::Qt_4_4);

	out << (quint32) magicNumber
	    << (quint32) fileVersion
	    << (quint32) hash.size();

	QHashIterator<QString, CardInfo *> i(hash);
	while (i.hasNext()) {
		i.next();
		i.value()->saveToStream(out);
	}

	return true;
}
