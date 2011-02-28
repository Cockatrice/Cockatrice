#include "carddatabase.h"
#include "settingscache.h"
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QSvgRenderer>
#include <QPainter>
#include <QUrl>
#include <QSet>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDebug>

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

PictureToLoad::PictureToLoad(CardInfo *_card, bool _stripped, bool _hq)
	: card(_card), stripped(_stripped), setIndex(0), hq(_hq)
{
	if (card) {
		sortedSets = card->getSets();
		sortedSets.sortByKey();
	}
}

bool PictureToLoad::nextSet()
{
	if (setIndex == sortedSets.size() - 1)
		return false;
	++setIndex;
	return true;
}

PictureLoader::PictureLoader(QObject *parent)
	: QObject(parent), downloadRunning(false), loadQueueRunning(false)
{
	connect(this, SIGNAL(startLoadQueue()), this, SLOT(processLoadQueue()), Qt::QueuedConnection);
	
	networkManager = new QNetworkAccessManager(this);
	connect(networkManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(picDownloadFinished(QNetworkReply *)));
}

void PictureLoader::processLoadQueue()
{
	if (loadQueueRunning)
		return;
	
	loadQueueRunning = true;
	forever {
		mutex.lock();
		if (loadQueue.isEmpty()) {
			mutex.unlock();
			loadQueueRunning = false;
			return;
		}
		PictureToLoad ptl = loadQueue.takeFirst();
		mutex.unlock();
		QString correctedName = ptl.getCard()->getCorrectedName();
		QString picsPath = _picsPath;
		QString setName = ptl.getSetName();
		
		QImage image;
		if (!image.load(QString("%1/%2/%3.full.jpg").arg(picsPath).arg(setName).arg(correctedName)))
			if (!image.load(QString("%1/%2/%3%4.full.jpg").arg(picsPath).arg(setName).arg(correctedName).arg(1)))
				if (!image.load(QString("%1/%2/%3/%4.full.jpg").arg(picsPath).arg("downloadedPics").arg(setName).arg(correctedName))) {
					if (picDownload) {
						cardsToDownload.append(ptl);
						if (!downloadRunning)
							startNextPicDownload();
					} else {
						if (ptl.nextSet())
							loadQueue.prepend(ptl);
						else
							emit imageLoaded(ptl.getCard(), QImage());
					}
					continue;
				}
		
		emit imageLoaded(ptl.getCard(), image);
	}
}

void PictureLoader::startNextPicDownload()
{
	if (cardsToDownload.isEmpty()) {
		cardBeingDownloaded = 0;
		downloadRunning = false;
		return;
	}
	
	downloadRunning = true;
	
	cardBeingDownloaded = cardsToDownload.takeFirst();
	QString picUrl;
	if (cardBeingDownloaded.getStripped())
		picUrl = cardBeingDownloaded.getCard()->getPicURLSt(cardBeingDownloaded.getSetName());
	else if (cardBeingDownloaded.getHq())
		picUrl = cardBeingDownloaded.getCard()->getPicURLHq(cardBeingDownloaded.getSetName());
	else
		picUrl = cardBeingDownloaded.getCard()->getPicURL(cardBeingDownloaded.getSetName());
	QUrl url(picUrl);
	
	QNetworkRequest req(url);
	qDebug() << "starting picture download:" << req.url();
	networkManager->get(req);
}

void PictureLoader::picDownloadFinished(QNetworkReply *reply)
{
	QString picsPath = _picsPath;
	const QByteArray &picData = reply->readAll();
	QImage testImage;
	if (testImage.loadFromData(picData)) {
		if (!QDir(QString(picsPath + "/downloadedPics/")).exists()) {
			QDir dir(picsPath);
			if (!dir.exists())
				return;
			dir.mkdir("downloadedPics");
		}
		if (!QDir(QString(picsPath + "/downloadedPics/" + cardBeingDownloaded.getSetName())).exists()) {
			QDir dir(QString(picsPath + "/downloadedPics"));
			dir.mkdir(cardBeingDownloaded.getSetName());
		}
		
		QString suffix;
		if (!cardBeingDownloaded.getStripped())
			suffix = ".full";
		
		QFile newPic(picsPath + "/downloadedPics/" + cardBeingDownloaded.getSetName() + "/" + cardBeingDownloaded.getCard()->getCorrectedName() + suffix + ".jpg");
		if (!newPic.open(QIODevice::WriteOnly))
			return;
		newPic.write(picData);
		newPic.close();
		
		emit imageLoaded(cardBeingDownloaded.getCard(), testImage);
	} else if (cardBeingDownloaded.getHq()) {
		qDebug() << "HQ: received invalid picture. URL:" << reply->request().url();
		cardBeingDownloaded.setHq(false);
		cardsToDownload.prepend(cardBeingDownloaded);
	} else {
		qDebug() << "LQ: received invalid picture. URL:" << reply->request().url();
		if (cardBeingDownloaded.nextSet()) {
			cardBeingDownloaded.setHq(true);
			mutex.lock();
			loadQueue.prepend(cardBeingDownloaded);
			mutex.unlock();
			emit startLoadQueue();
		} else
			emit imageLoaded(cardBeingDownloaded.getCard(), QImage());
	}
	
	reply->deleteLater();
	startNextPicDownload();
}

void PictureLoader::loadImage(CardInfo *card, bool stripped)
{
	QMutexLocker locker(&mutex);
	
	loadQueue.append(PictureToLoad(card, stripped));
	emit startLoadQueue();
}

void PictureLoader::setPicsPath(const QString &path)
{
	QMutexLocker locker(&mutex);
	_picsPath = path;
}

void PictureLoader::setPicDownload(bool _picDownload)
{
	QMutexLocker locker(&mutex);
	picDownload = _picDownload;
}

PictureLoadingThread::PictureLoadingThread(const QString &_picsPath, bool _picDownload, QObject *parent)
	: QThread(parent), picsPath(_picsPath), picDownload(_picDownload)
{
}

PictureLoadingThread::~PictureLoadingThread()
{
	quit();
	wait();
}

void PictureLoadingThread::run()
{
	pictureLoader = new PictureLoader;
	connect(pictureLoader, SIGNAL(imageLoaded(CardInfo *, const QImage &)), this, SIGNAL(imageLoaded(CardInfo *, const QImage &)));
	pictureLoader->setPicsPath(picsPath);
	pictureLoader->setPicDownload(picDownload);
	
	usleep(100);
	initWaitCondition.wakeAll();
	
	exec();
	
	delete pictureLoader;
}

void PictureLoadingThread::waitForInit()
{
	QMutex mutex;
	mutex.lock();
	initWaitCondition.wait(&mutex);
	mutex.unlock();
}

CardInfo::CardInfo(CardDatabase *_db, const QString &_name, const QString &_manacost, const QString &_cardtype, const QString &_powtough, const QString &_text, const QStringList &_colors, bool _cipt, int _tableRow, const SetList &_sets, const QMap<QString, QString> &_picURLs, const QMap<QString, QString> &_picURLsHq, const QMap<QString, QString> &_picURLsSt)
	: db(_db), name(_name), sets(_sets), manacost(_manacost), cardtype(_cardtype), powtough(_powtough), text(_text), colors(_colors), picURLs(_picURLs), picURLsHq(_picURLsHq), picURLsSt(_picURLsSt), cipt(_cipt), tableRow(_tableRow), pixmap(NULL)
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

QString CardInfo::getPicURL() const
{
	SetList sortedSets = sets;
	sortedSets.sortByKey();
	return picURLs.value(sortedSets.first()->getShortName());
}

QPixmap *CardInfo::loadPixmap()
{
	if (pixmap)
		return pixmap;
	pixmap = new QPixmap();
	
	if (getName().isEmpty()) {
		pixmap->load(settingsCache->getCardBackPicturePath());
		return pixmap;
	}
	db->loadImage(this);
	return pixmap;
}

void CardInfo::imageLoaded(const QImage &image)
{
	if (!image.isNull()) {
		*pixmap = QPixmap::fromImage(image);
		emit pixmapUpdated();
	}
}

QPixmap *CardInfo::getPixmap(QSize size)
{
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
		qDebug() << "Deleting pixmap for" << name;
		delete pixmap;
		pixmap = 0;
		QMapIterator<int, QPixmap *> i(scaledPixmapCache);
		while (i.hasNext()) {
			i.next();
			qDebug() << "  Deleting cached pixmap for width" << i.key();
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
	qDebug() << "Updating pixmap cache for" << name;
	clearPixmapCache();
	loadPixmap();
	
	emit pixmapUpdated();
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &xml, const CardInfo *info)
{
	xml.writeStartElement("card");
	xml.writeTextElement("name", info->getName());

	const SetList &sets = info->getSets();
	for (int i = 0; i < sets.size(); i++) {
		xml.writeStartElement("set");
		xml.writeAttribute("picURL", info->getPicURL(sets[i]->getShortName()));
		xml.writeAttribute("picURLHq", info->getPicURLHq(sets[i]->getShortName()));
		xml.writeAttribute("picURLSt", info->getPicURLSt(sets[i]->getShortName()));
		xml.writeCharacters(sets[i]->getShortName());
		xml.writeEndElement();
	}
	const QStringList &colors = info->getColors();
	for (int i = 0; i < colors.size(); i++)
		xml.writeTextElement("color", colors[i]);

	xml.writeTextElement("manacost", info->getManaCost());
	xml.writeTextElement("type", info->getCardType());
	if (!info->getPowTough().isEmpty())
		xml.writeTextElement("pt", info->getPowTough());
	xml.writeTextElement("tablerow", QString::number(info->getTableRow()));
	xml.writeTextElement("text", info->getText());
	if (info->getCipt())
		xml.writeTextElement("cipt", "1");
	xml.writeEndElement(); // card

	return xml;
}

CardDatabase::CardDatabase(QObject *parent)
	: QObject(parent), loadSuccess(false), noCard(0)
{
	connect(settingsCache, SIGNAL(picsPathChanged()), this, SLOT(picsPathChanged()));
	connect(settingsCache, SIGNAL(cardDatabasePathChanged()), this, SLOT(loadCardDatabase()));
	connect(settingsCache, SIGNAL(picDownloadChanged()), this, SLOT(picDownloadChanged()));
	
	loadCardDatabase();
	
	loadingThread = new PictureLoadingThread(settingsCache->getPicsPath(), settingsCache->getPicDownload(), this);
	connect(loadingThread, SIGNAL(imageLoaded(CardInfo *, QImage)), this, SLOT(imageLoaded(CardInfo *, QImage)));
	loadingThread->start(QThread::LowPriority);
	loadingThread->waitForInit();

	noCard = new CardInfo(this);
	noCard->loadPixmap(); // cache pixmap for card back
	connect(settingsCache, SIGNAL(cardBackPicturePathChanged()), noCard, SLOT(updatePixmapCache()));
}

CardDatabase::~CardDatabase()
{
	clear();
	delete noCard;
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
			QString name, manacost, type, pt, text;
			QStringList colors;
			QMap<QString, QString> picURLs, picURLsHq, picURLsSt;
			SetList sets;
			int tableRow = 0;
			bool cipt = false;
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
				else if (xml.name() == "set") {
					QString picURL = xml.attributes().value("picURL").toString();
					QString picURLHq = xml.attributes().value("picURLHq").toString();
					QString picURLSt = xml.attributes().value("picURLSt").toString();
					QString setName = xml.readElementText();
					sets.append(getSet(setName));
					picURLs.insert(setName, picURL);
					picURLsHq.insert(setName, picURLHq);
					picURLsSt.insert(setName, picURLSt);
				} else if (xml.name() == "color")
					colors << xml.readElementText();
				else if (xml.name() == "tablerow")
					tableRow = xml.readElementText().toInt();
				else if (xml.name() == "cipt")
					cipt = (xml.readElementText() == "1");
			}
			cardHash.insert(name, new CardInfo(this, name, manacost, type, pt, text, colors, cipt, tableRow, sets, picURLs, picURLsHq, picURLsSt));
		}
	}
}

bool CardDatabase::loadFromFile(const QString &fileName)
{
	QFile file(fileName);
	file.open(QIODevice::ReadOnly);
	if (!file.isOpen())
		return false;
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
	qDebug() << cardHash.size() << "cards in" << setHash.size() << "sets loaded";
	return !cardHash.isEmpty();
}

bool CardDatabase::saveToFile(const QString &fileName)
{
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly))
		return false;
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

void CardDatabase::picDownloadChanged()
{
	loadingThread->getPictureLoader()->setPicDownload(settingsCache->getPicDownload());
	if (settingsCache->getPicDownload()) {
		QHashIterator<QString, CardInfo *> cardIterator(cardHash);
		while (cardIterator.hasNext())
			cardIterator.next().value()->clearPixmapCacheMiss();
	}
}

bool CardDatabase::loadCardDatabase(const QString &path)
{
	if (!path.isEmpty())
		loadSuccess = loadFromFile(path);
	else loadSuccess = false;
	
	if (loadSuccess) {
		SetList allSets;
		QHashIterator<QString, CardSet *> setsIterator(setHash);
		while (setsIterator.hasNext())
			allSets.append(setsIterator.next().value());
		allSets.sortByKey();
		for (int i = 0; i < allSets.size(); ++i)
			allSets[i]->setSortKey(i);
		
		emit cardListChanged();
	}
	
	return loadSuccess;
}

bool CardDatabase::loadCardDatabase()
{
	return loadCardDatabase(settingsCache->getCardDatabasePath());
}

QStringList CardDatabase::getAllColors() const
{
	QSet<QString> colors;
	QHashIterator<QString, CardInfo *> cardIterator(cardHash);
	while (cardIterator.hasNext()) {
		const QStringList &cardColors = cardIterator.next().value()->getColors();
		if (cardColors.isEmpty())
			colors.insert("X");
		else
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

void CardDatabase::cacheCardPixmaps(const QStringList &cardNames)
{
	for (int i = 0; i < cardNames.size(); ++i)
		getCard(cardNames[i])->loadPixmap();
}

void CardDatabase::loadImage(CardInfo *card)
{
	loadingThread->getPictureLoader()->loadImage(card, false);
}

void CardDatabase::imageLoaded(CardInfo *card, QImage image)
{
	card->imageLoaded(image);
}

void CardDatabase::picsPathChanged()
{
	loadingThread->getPictureLoader()->setPicsPath(settingsCache->getPicsPath());
	clearPixmapCache();
}