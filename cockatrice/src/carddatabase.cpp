#include "carddatabase.h"
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QSvgRenderer>
#include <QPainter>
#include <QUrl>
#include <QSet>

CardSet::CardSet(const QString &_shortName, const QString &_longName)
	: shortName(_shortName), longName(_longName)
{
	updateSortKey();
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &xml, const CardSet *set)
{
	xml.writeStartElement("set");
	xml.writeTextElement("name", set->getShortName());
	xml.writeTextElement("longname", set->getLongName());
	xml.writeEndElement();

	return xml;
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

class SetList::CompareFunctor {
public:
	inline bool operator()(CardSet *a, CardSet *b) const
	{
		return a->getSortKey() < b->getSortKey();
	}
};

void SetList::sortByKey()
{
	qSort(begin(), end(), CompareFunctor());
}

CardInfo::CardInfo(CardDatabase *_db, const QString &_name, const QString &_manacost, const QString &_cardtype, const QString &_powtough, const QString &_text, const QStringList &_colors, int _tableRow, const SetList &_sets, const QString &_picURL)
	: db(_db), name(_name), sets(_sets), manacost(_manacost), cardtype(_cardtype), powtough(_powtough), text(_text), colors(_colors), picURL(_picURL), tableRow(_tableRow), pixmap(NULL)
{
	for (int i = 0; i < sets.size(); i++)
		sets[i]->append(this);
}

CardInfo::~CardInfo()
{
	clearPixmapCache();
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

QString CardInfo::getCorrectedName() const
{
	QString result = name;
	// Fire // Ice, Circle of Protection: Red
	return result.remove(" // ").remove(":");
}

void CardInfo::addToSet(CardSet *set)
{
	set->append(this);
	sets << set;
}

QPixmap *CardInfo::loadPixmap()
{
	if (pixmap)
		return pixmap;
	pixmap = new QPixmap();
	QString picsPath = db->getPicsPath();
	if (!QDir(picsPath).exists())
		return pixmap;
	
	if (getName().isEmpty()) {
		pixmap->load(QString("%1/back.jpg").arg(picsPath));
		return pixmap;
	}
	sets.sortByKey();

	QString debugOutput = QString("CardDatabase: loading pixmap for '%1' from ").arg(getName());
	for (int i = 0; i < sets.size(); i++)
		debugOutput.append(QString("%1, ").arg(sets[i]->getShortName()));
	qDebug(debugOutput.toLatin1());

	QString correctedName = getCorrectedName();
	for (int i = 0; i < sets.size(); i++) {
		if (pixmap->load(QString("%1/%2/%3.full.jpg").arg(picsPath).arg(sets[i]->getShortName()).arg(correctedName)))
			return pixmap;
		if (pixmap->load(QString("%1/%2/%3%4.full.jpg").arg(picsPath).arg(sets[i]->getShortName()).arg(correctedName).arg(1)))
			return pixmap;
	}
	if (pixmap->load(QString("%1/%2/%3.full.jpg").arg(picsPath).arg("downloadedPics").arg(correctedName)))
		return pixmap;
	if (db->getPicDownload())
		db->startPicDownload(this);
	return pixmap;
}

QPixmap *CardInfo::getPixmap(QSize size)
{
	qDebug(QString("CardInfo::getPixmap(%1, %2) for %3").arg(size.width()).arg(size.height()).arg(getName()).toLatin1());
	QPixmap *cachedPixmap = scaledPixmapCache.value(size.width());
	if (cachedPixmap)
		return cachedPixmap;
	QPixmap *bigPixmap = loadPixmap();
	QPixmap *result;
	if (bigPixmap->isNull()) {
		if (!getName().isEmpty())
			return 0;
		else {
			result = new QPixmap(size);
			result->fill(Qt::transparent);
			QSvgRenderer svg(QString(":/back.svg"));
			QPainter painter(result);
			svg.render(&painter, QRectF(0, 0, size.width(), size.height()));
		}
	} else
		result = new QPixmap(bigPixmap->scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
	scaledPixmapCache.insert(size.width(), result);
	return result;
}

void CardInfo::clearPixmapCache()
{
	if (pixmap) {
		qDebug(QString("Deleting pixmap for %1").arg(name).toLatin1());
		delete pixmap;
		pixmap = 0;
		QMapIterator<int, QPixmap *> i(scaledPixmapCache);
		while (i.hasNext()) {
			i.next();
			qDebug(QString("  Deleting cached pixmap for width %1").arg(i.key()).toLatin1());
			delete i.value();
		}
		scaledPixmapCache.clear();
	}
}

void CardInfo::clearPixmapCacheMiss()
{
	if (!pixmap)
		return;
	if (pixmap->isNull())
		clearPixmapCache();
}

void CardInfo::updatePixmapCache()
{
	qDebug(QString("Updating pixmap cache for %1").arg(name).toLatin1());
	clearPixmapCache();
	loadPixmap();
	
	emit pixmapUpdated();
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &xml, const CardInfo *info)
{
	xml.writeStartElement("card");
	xml.writeTextElement("name", info->getName());

	const SetList &sets = info->getSets();
	for (int i = 0; i < sets.size(); i++)
		xml.writeTextElement("set", sets[i]->getShortName());
	const QStringList &colors = info->getColors();
	for (int i = 0; i < colors.size(); i++)
		xml.writeTextElement("color", colors[i]);

	xml.writeTextElement("manacost", info->getManaCost());
	xml.writeTextElement("type", info->getCardType());
	if (!info->getPowTough().isEmpty())
		xml.writeTextElement("pt", info->getPowTough());
	xml.writeTextElement("tablerow", QString::number(info->getTableRow()));
	xml.writeTextElement("text", info->getText());
	xml.writeTextElement("picURL", info->getPicURL());
	xml.writeEndElement(); // card

	return xml;
}

CardDatabase::CardDatabase(QObject *parent)
	: QObject(parent), noCard(0)
{
	http = new QHttp(this);
	connect(http, SIGNAL(requestFinished(int, bool)), this, SLOT(picDownloadFinished(int, bool)));

	updateDatabasePath();
	updatePicDownload();
	updatePicsPath();

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

SetList CardDatabase::getSetList() const
{
	SetList result;
	QHashIterator<QString, CardSet *> i(setHash);
	while (i.hasNext()) {
		i.next();
		result << i.value();
	}
	return result;
}

void CardDatabase::clearPixmapCache()
{
	QHashIterator<QString, CardInfo *> i(cardHash);
	while (i.hasNext()) {
		i.next();
		i.value()->clearPixmapCache();
	}
	if (noCard)
		noCard->clearPixmapCache();
}

void CardDatabase::startPicDownload(CardInfo *card)
{
	QBuffer *buffer = new QBuffer(this);
	buffer->open(QIODevice::ReadWrite);
	QUrl url(card->getPicURL());
	http->setHost(url.host(), url.port(80));
	int dlID = http->get(url.path(), buffer);
	downloadBuffers.insert(dlID, QPair<CardInfo *, QBuffer *>(card, buffer));
}

void CardDatabase::picDownloadFinished(int id, bool error)
{
	if (!downloadBuffers.contains(id))
		return;
	
	const QPair<CardInfo *, QBuffer *> &temp = downloadBuffers.value(id);
	CardInfo *card = temp.first;
	QBuffer *buffer = temp.second;
	downloadBuffers.remove(id);
	buffer->close();
	
	if (!error) {
		const QByteArray &picData = buffer->data();
		QPixmap testPixmap;
		if (testPixmap.loadFromData(picData)) {
			if (!QDir(QString(picsPath + "/downloadedPics/")).exists()) {
				QDir dir(picsPath);
				if (!dir.exists())
					return;
				dir.mkdir("downloadedPics");
			}
			QFile newPic(picsPath + "/downloadedPics/" + card->getCorrectedName() + ".full.jpg");
			if (!newPic.open(QIODevice::WriteOnly))
				return;
			newPic.write(picData);
			newPic.close();
			
			card->updatePixmapCache();
		}
	}
	delete buffer;
}

void CardDatabase::loadSetsFromXml(QXmlStreamReader &xml)
{
	while (!xml.atEnd()) {
		if (xml.readNext() == QXmlStreamReader::EndElement)
			break;
		if (xml.name() == "set") {
			QString shortName, longName;
			while (!xml.atEnd()) {
				if (xml.readNext() == QXmlStreamReader::EndElement)
					break;
				if (xml.name() == "name")
					shortName = xml.readElementText();
				else if (xml.name() == "longname")
					longName = xml.readElementText();
			}
			setHash.insert(shortName, new CardSet(shortName, longName));
		}
	}
}

void CardDatabase::loadCardsFromXml(QXmlStreamReader &xml)
{
	while (!xml.atEnd()) {
		if (xml.readNext() == QXmlStreamReader::EndElement)
			break;
		if (xml.name() == "card") {
			QString name, manacost, type, pt, text, picURL;
			QStringList colors;
			SetList sets;
			int tableRow = 0;
			while (!xml.atEnd()) {
				if (xml.readNext() == QXmlStreamReader::EndElement)
					break;
				if (xml.name() == "name")
					name = xml.readElementText();
				else if (xml.name() == "manacost")
					manacost = xml.readElementText();
				else if (xml.name() == "type")
					type = xml.readElementText();
				else if (xml.name() == "pt")
					pt = xml.readElementText();
				else if (xml.name() == "text")
					text = xml.readElementText();
				else if (xml.name() == "set")
					sets << getSet(xml.readElementText());
				else if (xml.name() == "color")
					colors << xml.readElementText();
				else if (xml.name() == "tablerow")
					tableRow = xml.readElementText().toInt();
				else if (xml.name() == "picURL")
					picURL = xml.readElementText();
			}
			cardHash.insert(name, new CardInfo(this, name, manacost, type, pt, text, colors, tableRow, sets, picURL));
		}
	}
}

int CardDatabase::loadFromFile(const QString &fileName)
{
	QFile file(fileName);
	file.open(QIODevice::ReadOnly);
	if (!file.isOpen())
		return -1;
	QXmlStreamReader xml(&file);
	clear();
	while (!xml.atEnd()) {
		if (xml.readNext() == QXmlStreamReader::StartElement) {
			if (xml.name() != "cockatrice_carddatabase")
				return false;
			while (!xml.atEnd()) {
				if (xml.readNext() == QXmlStreamReader::EndElement)
					break;
				if (xml.name() == "sets")
					loadSetsFromXml(xml);
				else if (xml.name() == "cards")
					loadCardsFromXml(xml);
			}
		}
	}
	qDebug(QString("%1 cards in %2 sets loaded").arg(cardHash.size()).arg(setHash.size()).toLatin1());
	return cardHash.size();
}

bool CardDatabase::saveToFile(const QString &fileName)
{
	QFile file(fileName);
	file.open(QIODevice::WriteOnly);
	QXmlStreamWriter xml(&file);

	xml.setAutoFormatting(true);
	xml.writeStartDocument();
	xml.writeStartElement("cockatrice_carddatabase");
	xml.writeAttribute("version", "1");

	xml.writeStartElement("sets");
	QHashIterator<QString, CardSet *> setIterator(setHash);
	while (setIterator.hasNext())
		xml << setIterator.next().value();
	xml.writeEndElement(); // sets

	xml.writeStartElement("cards");
	QHashIterator<QString, CardInfo *> cardIterator(cardHash);
	while (cardIterator.hasNext())
		xml << cardIterator.next().value();
	xml.writeEndElement(); // cards

	xml.writeEndElement(); // cockatrice_carddatabase
	xml.writeEndDocument();

	return true;
}

void CardDatabase::updatePicDownload(int _picDownload)
{
	if (_picDownload == -1) {
		QSettings settings;
		picDownload = settings.value("personal/picturedownload", 0).toInt();
	} else
		picDownload = _picDownload;
		
	if (picDownload) {
		QHashIterator<QString, CardInfo *> cardIterator(cardHash);
		while (cardIterator.hasNext()) {
			CardInfo *c = cardIterator.next().value();
			c->clearPixmapCacheMiss();
		}
	}
}

void CardDatabase::updatePicsPath(const QString &path)
{
	if (path.isEmpty()) {
		QSettings settings;
		settings.beginGroup("paths");
		picsPath = settings.value("pics").toString();
	} else
		picsPath = path;
	clearPixmapCache();
}

void CardDatabase::updateDatabasePath(const QString &path)
{
	if (path.isEmpty()) {
		QSettings settings;
		settings.beginGroup("paths");
		cardDatabasePath = settings.value("carddatabase").toString();
	} else
		cardDatabasePath = path;
	if (!cardDatabasePath.isEmpty())
		loadFromFile(cardDatabasePath);
}

QStringList CardDatabase::getAllColors() const
{
	QSet<QString> colors;
	QHashIterator<QString, CardInfo *> cardIterator(cardHash);
	while (cardIterator.hasNext()) {
		const QStringList &cardColors = cardIterator.next().value()->getColors();
		for (int i = 0; i < cardColors.size(); ++i)
			colors.insert(cardColors[i]);
	}
	return colors.toList();
}

QStringList CardDatabase::getAllMainCardTypes() const
{
	QSet<QString> types;
	QHashIterator<QString, CardInfo *> cardIterator(cardHash);
	while (cardIterator.hasNext())
		types.insert(cardIterator.next().value()->getMainCardType());
	return types.toList();
}
