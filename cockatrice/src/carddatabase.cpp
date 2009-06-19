#include "carddatabase.h"
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QSettings>

CardSet::CardSet(const QString &_shortName, const QString &_longName)
	: shortName(_shortName), longName(_longName)
{
	updateSortKey();
}

void CardSet::loadFromStream(QDataStream &stream)
{
	stream >> shortName >> longName;
	updateSortKey();
	qDebug(QString("set loaded: %1, %2").arg(shortName).arg(longName).toLatin1());
}

void CardSet::saveToStream(QDataStream &stream)
{
	stream << shortName << longName;
}

void CardSet::setSortKey(unsigned int _sortKey)
{
	sortKey = _sortKey;

	QSettings settings;
	settings.beginGroup("sets");
	settings.beginGroup(shortName);
	settings.setValue("sortkey", sortKey);
}

void CardSet::updateSortKey()
{
	QSettings settings;
	settings.beginGroup("sets");
	settings.beginGroup(shortName);
	sortKey = settings.value("sortkey", 0).toInt();
}

CardInfo::CardInfo(CardDatabase *_db, const QString &_name, const QString &_manacost, const QString &_cardtype, const QString &_powtough, const QStringList &_text)
	: db(_db), name(_name), manacost(_manacost), cardtype(_cardtype), powtough(_powtough), text(_text), pixmap(NULL)
{
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

void CardInfo::addToSet(CardSet *set)
{
	set->append(this);
	sets << set;
}

class CardInfo::SetCompareFunctor {
public:
	inline bool operator()(CardSet *a, CardSet *b) const
	{
		return a->getSortKey() < b->getSortKey();
	}
};

QPixmap *CardInfo::loadPixmap()
{
	if (pixmap)
		return pixmap;
	pixmap = new QPixmap();
	if (getName().isEmpty()) {
		pixmap->load("../pics/back.jpg");
		return pixmap;
	}
	qSort(sets.begin(), sets.end(), SetCompareFunctor());

	QString debugOutput = QString("CardDatabase: loading pixmap for '%1' from ").arg(getName());
	for (int i = 0; i < sets.size(); i++)
		debugOutput.append(QString("%1, ").arg(sets[i]->getShortName()));
	qDebug(debugOutput.toLatin1());

	for (int i = 0; i < sets.size(); i++) {
		// Fire // Ice, Circle of Protection: Red
		QString correctedName = getName().remove(" // ").remove(":");
		if (pixmap->load(QString("../pics/%1/%2.full.jpg").arg(sets[i]->getShortName()).arg(correctedName)))
			return pixmap;
		if (pixmap->load(QString("../pics/%1/%2%3.full.jpg").arg(sets[i]->getShortName()).arg(correctedName).arg(1)))
			return pixmap;
	}
	return pixmap;
}

QPixmap *CardInfo::getPixmap(QSize size)
{
	qDebug(QString("CardInfo::getPixmap(%1, %2) for %3").arg(size.width()).arg(size.height()).arg(getName()).toLatin1());
	QPixmap *cachedPixmap = scaledPixmapCache.value(size.width());
	if (cachedPixmap)
		return cachedPixmap;
	QPixmap *bigPixmap = loadPixmap();
	if (bigPixmap->isNull())
		return 0;
	QPixmap *result = new QPixmap(bigPixmap->scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
	scaledPixmapCache.insert(size.width(), result);
	return result;
}

void CardInfo::loadFromStream(QDataStream &stream)
{
	QStringList setNames;
	stream >> name
	       >> setNames
	       >> manacost
	       >> cardtype
	       >> powtough
	       >> text;

	for (int i = 0; i < setNames.size(); i++)
		addToSet(db->getSet(setNames[i]));
}

void CardInfo::saveToStream(QDataStream &stream)
{
	QStringList setNames;
	for (int i = 0; i < sets.size(); i++)
		setNames << sets[i]->getShortName();

	stream << name
	       << setNames
	       << manacost
	       << cardtype
	       << powtough
	       << text;
}

CardDatabase::CardDatabase()
{
	noCard = new CardInfo(this);
	noCard->loadPixmap(); // cache pixmap for card back
}

CardDatabase::~CardDatabase()
{
	clear();
}

void CardDatabase::clear()
{
	QHashIterator<QString, CardSet *> setIt(setHash);
	while (setIt.hasNext()) {
		setIt.next();
		delete setIt.value();
	}
	setHash.clear();

	QHashIterator<QString, CardInfo *> i(cardHash);
	while (i.hasNext()) {
		i.next();
		delete i.value();
	}
	cardHash.clear();
}

CardInfo *CardDatabase::getCard(const QString &cardName)
{
	if (cardName.isEmpty())
		return noCard;
	else if (cardHash.contains(cardName))
		return cardHash.value(cardName);
	else {
		qDebug(QString("CardDatabase: card not found: %1").arg(cardName).toLatin1());
		CardInfo *newCard = new CardInfo(this, cardName);
		newCard->addToSet(getSet("TK"));
		cardHash.insert(cardName, newCard);
		return newCard;
	}
}

CardSet *CardDatabase::getSet(const QString &setName)
{
	if (setHash.contains(setName))
		return setHash.value(setName);
	else {
		qDebug(QString("CardDatabase: set not found: %1").arg(setName).toLatin1());
		CardSet *newSet = new CardSet(setName);
		setHash.insert(setName, newSet);
		return newSet;
	}
}

void CardDatabase::importOracleFile(const QString &fileName, CardSet *set)
{
	QFile file(fileName);
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	QTextStream in(&file);
	while (!in.atEnd()) {
		QString cardname = in.readLine();
		if (cardname.isEmpty())
			continue;
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
		if (cardHash.contains(cardname))
			card = cardHash.value(cardname);
		else {
			card = new CardInfo(this, cardname, manacost, cardtype, powtough, text);
			cardHash.insert(cardname, card);
		}
		card->addToSet(set);
	}
}

void CardDatabase::importOracleDir()
{
	clear();
	QDir dir("../db");

	// XXX User soll selber auswählen können, welche Karten ihm am besten gefallen.
	// XXX Muss momentan schmutzig über Zahlen vor den Dateinamen gemacht werden.

	dir.setSorting(QDir::Name | QDir::IgnoreCase);
	QFileInfoList files = dir.entryInfoList(QStringList() << "*.txt");
	for (int k = 0; k < files.size(); k++) {
		QFileInfo i = files[k];
		QString shortName = i.fileName().left(i.fileName().indexOf('_'));
		QString longName = i.fileName().mid(i.fileName().indexOf('_') + 1);
		longName = longName.left(longName.indexOf('.'));
		CardSet *set = new CardSet(shortName, longName);
		setHash.insert(shortName, set);

		importOracleFile(i.filePath(), set);
	}

	qDebug(QString("CardDatabase: %1 cards imported").arg(cardHash.size()).toLatin1());
}

int CardDatabase::loadFromFile(const QString &fileName)
{
	QFile file(fileName);
	file.open(QIODevice::ReadOnly);
	QDataStream in(&file);
	in.setVersion(QDataStream::Qt_4_4);

	quint32 _magicNumber, _fileVersion, setCount, cardCount;
	in >> _magicNumber
	   >> _fileVersion
	   >> setCount
	   >> cardCount;

	if (_magicNumber != magicNumber)
		return -1;
	if (_fileVersion != fileVersion)
		return -2;

	clear();
	setHash.reserve(setCount);
	qDebug(QString("setCount = %1").arg(setCount).toLatin1());
	for (unsigned int i = 0; i < setCount; i++) {
		CardSet *newSet = new CardSet;
		newSet->loadFromStream(in);
		setHash.insert(newSet->getShortName(), newSet);
	}
	cardHash.reserve(cardCount);
	for (unsigned int i = 0; i < cardCount; i++) {
		CardInfo *newCard = new CardInfo(this);
		newCard->loadFromStream(in);
		cardHash.insert(newCard->getName(), newCard);
	}
	qDebug(QString("%1 cards in %2 sets loaded").arg(cardCount).arg(setHash.size()).toLatin1());

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
	    << (quint32) setHash.size()
	    << (quint32) cardHash.size();

	QHashIterator<QString, CardSet *> setIt(setHash);
	while (setIt.hasNext()) {
		setIt.next();
		setIt.value()->saveToStream(out);
	}
	QHashIterator<QString, CardInfo *> i(cardHash);
	while (i.hasNext()) {
		i.next();
		i.value()->saveToStream(out);
	}

	return true;
}
