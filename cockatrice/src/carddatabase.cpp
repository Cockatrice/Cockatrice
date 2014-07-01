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

const int CardDatabase::versionNeeded = 3;

static QXmlStreamWriter &operator<<(QXmlStreamWriter &xml, const CardSet *set)
{
    xml.writeStartElement("set");
    xml.writeTextElement("name", set->getShortName());
    xml.writeTextElement("longname", set->getLongName());
    xml.writeEndElement();

    return xml;
}

CardSet::CardSet(const QString &_shortName, const QString &_longName)
    : shortName(_shortName), longName(_longName)
{
    updateSortKey();
}

QString CardSet::getCorrectedShortName() const
{
    // Because windows is horrible.
    QSet<QString> invalidFileNames;
    invalidFileNames << "CON" << "PRN" << "AUX" << "NUL" << "COM1" << "COM2" <<
        "COM3" << "COM4" << "COM5" << "COM6" << "COM7" << "COM8" << "COM9" <<
        "LPT1" << "LPT2" << "LPT3" << "LPT4" << "LPT5" << "LPT6" << "LPT7" <<
        "LPT8" << "LPT9";

    return invalidFileNames.contains(shortName) ? shortName + "_" : shortName;
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

PictureLoader::PictureLoader(const QString &__picsPath, bool _picDownload, bool _picDownloadHq, QObject *parent)
    : QObject(parent),
      _picsPath(__picsPath), picDownload(_picDownload), picDownloadHq(_picDownloadHq),
      downloadRunning(false), loadQueueRunning(false)
{
    connect(this, SIGNAL(startLoadQueue()), this, SLOT(processLoadQueue()), Qt::QueuedConnection);

    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(picDownloadFinished(QNetworkReply *)));
}

PictureLoader::~PictureLoader()
{
    // This does not work with the destroyed() signal as this destructor is called after the main event loop is done.
    thread()->quit();
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

QString PictureLoader::getPicUrl(CardInfo *card)
{
    if (!picDownload) return 0;

    QString picUrl = picDownloadHq ? settingsCache->getPicUrlHq() : settingsCache->getPicUrl();
    picUrl.replace("!name!", QUrl::toPercentEncoding(card->getCorrectedName()));
    CardSet *set = card->getPreferredSet();
    picUrl.replace("!setcode!", QUrl::toPercentEncoding(set->getShortName()));
    picUrl.replace("!setname!", QUrl::toPercentEncoding(set->getLongName()));
    picUrl.replace("!cardid!", QUrl::toPercentEncoding(QString::number(card->getPreferredMuId())));

    return picUrl;
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

    // TODO: Do something useful when picUrl is 0 or empty, etc
    QString picUrl = getPicUrl(cardBeingDownloaded.getCard());

    QUrl url(picUrl);

    QNetworkRequest req(url);
    qDebug() << "starting picture download:" << cardBeingDownloaded.getCard()->getName() << "Url:" << req.url();
    networkManager->get(req);
}

void PictureLoader::picDownloadFinished(QNetworkReply *reply)
{
    QString picsPath = _picsPath;
    if (reply->error()) {
        qDebug() << "Download failed:" << reply->errorString();
    }

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

void PictureLoader::setPicDownloadHq(bool _picDownloadHq)
{
    QMutexLocker locker(&mutex);
    picDownloadHq = _picDownloadHq;
}

CardInfo::CardInfo(CardDatabase *_db,
                   const QString &_name,
                   bool _isToken,
                   const QString &_manacost,
                   const QString &_cardtype,
                   const QString &_powtough,
                   const QString &_text,
                   const QStringList &_colors,
                   int _loyalty,
                   bool _cipt,
                   int _tableRow,
                   const SetList &_sets,
                   MuidMap _muIds)
    : db(_db),
      name(_name),
      isToken(_isToken),
      sets(_sets),
      manacost(_manacost),
      cardtype(_cardtype),
      powtough(_powtough),
      text(_text),
      colors(_colors),
      loyalty(_loyalty),
      muIds(_muIds),
      cipt(_cipt),
      tableRow(_tableRow),
      pixmap(NULL)
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
    if ((pos = result.indexOf("—")) != -1)
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
    // Fire // Ice, Circle of Protection: Red, "Ach! Hans, Run!", Who/What/When/Where/Why, Question Elemental?
    return result.remove(" // ").remove(':').remove('"').remove('?').replace('/', ' ');
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

CardSet* CardInfo::getPreferredSet()
{
    SetList sortedSets = sets;
    sortedSets.sortByKey();
    return sortedSets.first();
}

int CardInfo::getPreferredMuId()
{
    return muIds[getPreferredSet()->getShortName()];
}

static QXmlStreamWriter &operator<<(QXmlStreamWriter &xml, const CardInfo *info)
{
    xml.writeStartElement("card");
    xml.writeTextElement("name", info->getName());

    const SetList &sets = info->getSets();
    QString tmpString;
    QString tmpSet;
    for (int i = 0; i < sets.size(); i++) {
        xml.writeStartElement("set");

        tmpSet=sets[i]->getShortName();
        xml.writeAttribute("muId", QString::number(info->getMuId(tmpSet)));

        xml.writeCharacters(tmpSet);
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
    if (info->getMainCardType() == "Planeswalker")
        xml.writeTextElement("loyalty", QString::number(info->getLoyalty()));
    if (info->getCipt())
        xml.writeTextElement("cipt", "1");
    if (info->getIsToken())
        xml.writeTextElement("token", "1");
    xml.writeEndElement(); // card

    return xml;
}

CardDatabase::CardDatabase(QObject *parent)
    : QObject(parent), noCard(0), loadStatus(NotLoaded)
{
    connect(settingsCache, SIGNAL(picsPathChanged()), this, SLOT(picsPathChanged()));
    connect(settingsCache, SIGNAL(cardDatabasePathChanged()), this, SLOT(loadCardDatabase()));
    connect(settingsCache, SIGNAL(tokenDatabasePathChanged()), this, SLOT(loadTokenDatabase()));
    connect(settingsCache, SIGNAL(picDownloadChanged()), this, SLOT(picDownloadChanged()));
    connect(settingsCache, SIGNAL(picDownloadHqChanged()), this, SLOT(picDownloadHqChanged()));

    loadCardDatabase();
    loadTokenDatabase();

    pictureLoaderThread = new QThread;
    pictureLoader = new PictureLoader(settingsCache->getPicsPath(), settingsCache->getPicDownload(), settingsCache->getPicDownloadHq());
    pictureLoader->moveToThread(pictureLoaderThread);
    connect(pictureLoader, SIGNAL(imageLoaded(CardInfo *, const QImage &)), this, SLOT(imageLoaded(CardInfo *, const QImage &)));
    pictureLoaderThread->start(QThread::LowPriority);

    noCard = new CardInfo(this);
    noCard->loadPixmap(); // cache pixmap for card back
    connect(settingsCache, SIGNAL(cardBackPicturePathChanged()), noCard, SLOT(updatePixmapCache()));
}

CardDatabase::~CardDatabase()
{
    clear();
    delete noCard;
    
    pictureLoader->deleteLater();
    pictureLoaderThread->wait();
    delete pictureLoaderThread;
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

void CardDatabase::addCard(CardInfo *card)
{
    cardHash.insert(card->getName(), card);
    emit cardAdded(card);
}

void CardDatabase::removeCard(CardInfo *card)
{
    cardHash.remove(card->getName());
    emit cardRemoved(card);
}

CardInfo *CardDatabase::getCard(const QString &cardName, bool createIfNotFound)
{
    if (cardName.isEmpty())
        return noCard;
    else if (cardHash.contains(cardName))
        return cardHash.value(cardName);
    else if (createIfNotFound) {
        CardInfo *newCard = new CardInfo(this, cardName, true);
        newCard->addToSet(getSet("TK"));
        cardHash.insert(cardName, newCard);
        return newCard;
    } else
        return 0;
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
            MuidMap muids;
            SetList sets;
            int tableRow = 0;
            int loyalty = 0;
            bool cipt = false;
            bool isToken = false;
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
                    QXmlStreamAttributes attrs = xml.attributes();
                    QString setName = xml.readElementText();
                    sets.append(getSet(setName));
                    if (attrs.hasAttribute("muId")) {
                        muids[setName] = attrs.value("muId").toString().toInt();
                    }
                } else if (xml.name() == "color")
                    colors << xml.readElementText();
                else if (xml.name() == "tablerow")
                    tableRow = xml.readElementText().toInt();
                else if (xml.name() == "cipt")
                    cipt = (xml.readElementText() == "1");
                else if (xml.name() == "loyalty")
                    loyalty = xml.readElementText().toInt();
                else if (xml.name() == "token")
                    isToken = xml.readElementText().toInt();
            }
            cardHash.insert(name, new CardInfo(this, name, isToken, manacost, type, pt, text, colors, loyalty, cipt, tableRow, sets, muids));
        }
    }
}

LoadStatus CardDatabase::loadFromFile(const QString &fileName, bool tokens)
{
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen())
        return FileError;

    if (tokens) {
        QMutableHashIterator<QString, CardInfo *> i(cardHash);
        while (i.hasNext()) {
            i.next();
            if (i.value()->getIsToken()) {
                delete i.value();
                i.remove();
            }
        }
    } else {
        QHashIterator<QString, CardSet *> setIt(setHash);
        while (setIt.hasNext()) {
            setIt.next();
            delete setIt.value();
        }
        setHash.clear();

        QMutableHashIterator<QString, CardInfo *> i(cardHash);
        while (i.hasNext()) {
            i.next();
            if (!i.value()->getIsToken()) {
                delete i.value();
                i.remove();
            }
        }
        cardHash.clear();
    }

    QXmlStreamReader xml(&file);
    while (!xml.atEnd()) {
        if (xml.readNext() == QXmlStreamReader::StartElement) {
            if (xml.name() != "cockatrice_carddatabase")
                return Invalid;
            int version = xml.attributes().value("version").toString().toInt();
            if (version < versionNeeded) {
                qDebug() << "loadFromFile(): Version too old: " << version;
                return VersionTooOld;
            }
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

    if (cardHash.isEmpty()) return NoCards;

    return Ok;
}

bool CardDatabase::saveToFile(const QString &fileName, bool tokens)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    QXmlStreamWriter xml(&file);

    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("cockatrice_carddatabase");
    xml.writeAttribute("version", QString::number(versionNeeded));

    if (!tokens) {
        xml.writeStartElement("sets");
        QHashIterator<QString, CardSet *> setIterator(setHash);
        while (setIterator.hasNext())
            xml << setIterator.next().value();
        xml.writeEndElement(); // sets
    }

    xml.writeStartElement("cards");
    QHashIterator<QString, CardInfo *> cardIterator(cardHash);
    while (cardIterator.hasNext()) {
        CardInfo *card = cardIterator.next().value();
        if (card->getIsToken() == tokens)
            xml << card;
    }
    xml.writeEndElement(); // cards

    xml.writeEndElement(); // cockatrice_carddatabase
    xml.writeEndDocument();

    return true;
}

void CardDatabase::picDownloadChanged()
{
    pictureLoader->setPicDownload(settingsCache->getPicDownload());
    if (settingsCache->getPicDownload()) {
        QHashIterator<QString, CardInfo *> cardIterator(cardHash);
        while (cardIterator.hasNext())
            cardIterator.next().value()->clearPixmapCacheMiss();
    }
}

void CardDatabase::picDownloadHqChanged()
{
    pictureLoader->setPicDownloadHq(settingsCache->getPicDownloadHq());
    if (settingsCache->getPicDownloadHq()) {
        QHashIterator<QString, CardInfo *> cardIterator(cardHash);
        while (cardIterator.hasNext())
            cardIterator.next().value()->clearPixmapCacheMiss();
    }
}

LoadStatus CardDatabase::loadCardDatabase(const QString &path, bool tokens)
{
    LoadStatus tempLoadStatus = NotLoaded;
    if (!path.isEmpty())
        tempLoadStatus = loadFromFile(path, tokens);

    if (tempLoadStatus == Ok) {
        SetList allSets;
        QHashIterator<QString, CardSet *> setsIterator(setHash);
        while (setsIterator.hasNext())
            allSets.append(setsIterator.next().value());
        allSets.sortByKey();
        for (int i = 0; i < allSets.size(); ++i)
            allSets[i]->setSortKey(i);

        emit cardListChanged();
    }

    if (!tokens) {
        loadStatus = tempLoadStatus;
        qDebug() << "loadCardDatabase(): Status = " << loadStatus;
    }


    return tempLoadStatus;
}

void CardDatabase::loadCardDatabase()
{
    loadCardDatabase(settingsCache->getCardDatabasePath(), false);
}

void CardDatabase::loadTokenDatabase()
{
    loadCardDatabase(settingsCache->getTokenDatabasePath(), true);
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
    pictureLoader->loadImage(card, false);
}

void CardDatabase::imageLoaded(CardInfo *card, QImage image)
{
    card->imageLoaded(image);
}

void CardDatabase::picsPathChanged()
{
    pictureLoader->setPicsPath(settingsCache->getPicsPath());
    clearPixmapCache();
}
